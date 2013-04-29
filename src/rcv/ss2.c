/*------------------------------------------------------------------------------
* ss2.c : superstar II receiver dependent functions
*
*          Copyright (C) 2007-2013 by T.TAKASU, All rights reserved.
*
* reference:
*     [1] NovAtel, OM-20000086 Superstar II Firmware Reference Manuall, 2005
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2008/05/18 1.0 new
*           2008/06/16 1.2 separate common functions to rcvcmn.c
*           2009/04/01 1.3 fix bug on decode #21 message
*           2010/08/20 1.4 fix problem with minus value of time slew in #23
*                          (2.4.0_p5)
*           2011/05/27 1.5 fix problem with ARM compiler
*           2013/02/23 1.6 fix memory access violation problem on arm
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define SS2SOH      0x01        /* ss2 start of header */

#define ID_SS2LLH   20          /* ss2 message ID#20 navigation data (user) */
#define ID_SS2ECEF  21          /* ss2 message ID#21 navigation data (ecef) */
#define ID_SS2EPH   22          /* ss2 message ID#22 ephemeris data */
#define ID_SS2RAW   23          /* ss2 message ID#23 measurement block */
#define ID_SS2SBAS  67          /* ss2 message ID#67 sbas data */

static const char rcsid[]="$Id: ss2.c,v 1.2 2008/07/14 00:05:05 TTAKA Exp $";

/* get/set fields (little-endian) --------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

/* checksum ------------------------------------------------------------------*/
static int chksum(const unsigned char *buff, int len)
{
    int i;
    unsigned short sum=0;
    
    for (i=0;i<len-2;i++) sum+=buff[i];
    return (sum>>8)==buff[len-1]&&(sum&0xFF)==buff[len-2];
}
/* adjust week ---------------------------------------------------------------*/
static int adjweek(raw_t *raw, double sec)
{
    double tow;
    int week;
    
    if (raw->time.time==0) return 0;
    tow=time2gpst(raw->time,&week);
    if      (sec<tow-302400.0) sec+=604800.0;
    else if (sec>tow+302400.0) sec-=604800.0;
    raw->time=gpst2time(week,sec);
    return 1;
}
/* decode id#20 navigation data (user) ---------------------------------------*/
static int decode_ss2llh(raw_t *raw)
{
	double ep[6];
    unsigned char *p=raw->buff+4;
    
    trace(4,"decode_ss2llh: len=%d\n",raw->len);
    
    if (raw->len!=77) {
        trace(2,"ss2 id#20 length error: len=%d\n",raw->len);
        return -1;
    }
    ep[3]=U1(p   ); ep[4]=U1(p+ 1); ep[5]=R8(p+ 2);
    ep[2]=U1(p+10); ep[1]=U1(p+11); ep[0]=U2(p+12);
    raw->time=utc2gpst(epoch2time(ep));
    return 0;
}
/* decode id#21 navigation data (ecef) ---------------------------------------*/
static int decode_ss2ecef(raw_t *raw)
{
    unsigned char *p=raw->buff+4;
    
    trace(4,"decode_ss2ecef: len=%d\n",raw->len);
    
    if (raw->len!=85) {
        trace(2,"ss2 id#21 length error: len=%d\n",raw->len);
        return -1;
    }
    raw->time=gpst2time(U2(p+8),R8(p));
    return 0;
}
/* decode id#23 measurement block --------------------------------------------*/
static int decode_ss2meas(raw_t *raw)
{
    const double freqif=1.405396825E6,tslew=1.75E-7;
    double tow,slew,code,icp,d;
    int i,j,n,prn,sat,nobs;
    unsigned char *p=raw->buff+4;
    unsigned int sc;
    
    trace(4,"decode_ss2meas: len=%d\n",raw->len);
    
    nobs=U1(p+2);
    if (17+nobs*11!=raw->len) {
        trace(2,"ss2 id#23 message length error: len=%d\n",raw->len);
        return -1;
    }
    tow=floor(R8(p+3)*1000.0+0.5)/1000.0; /* rounded by 1ms */
    if (!adjweek(raw,tow)) {
        trace(2,"ss2 id#23 message time adjustment error\n");
        return -1;
    }
    /* time slew defined as uchar (ref [1]) but minus value appears in some f/w */
    slew=*(char *)(p)*tslew;
    
    raw->icpc+=4.5803-freqif*slew-FREQ1*(slew-1E-6); /* phase correction */
    
    for (i=n=0,p+=11;i<nobs&&n<MAXOBS;i++,p+=11) {
        prn=(p[0]&0x1F)+1;
        if (!(sat=satno(p[0]&0x20?SYS_SBS:SYS_GPS,prn))) {
            trace(2,"ss2 id#23 satellite number error: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].time=raw->time;
        raw->obs.data[n].sat=sat;
        code=(tow-floor(tow))-(double)(U4(p+2))/2095104000.0;
        raw->obs.data[n].P[0]=CLIGHT*(code+(code<0.0?1.0:0.0));
        icp=(double)(U4(p+6)>>2)/1024.0+raw->off[sat-1]; /* unwrap */
        if (fabs(icp-raw->icpp[sat-1])>524288.0) {
            d=icp>raw->icpp[sat-1]?-1048576.0:1048576.0;
            raw->off[sat-1]+=d; icp+=d;
        }
        raw->icpp[sat-1]=icp;
        raw->obs.data[n].L[0]=icp+raw->icpc;
        raw->obs.data[n].D[0]=0.0;
        raw->obs.data[n].SNR[0]=(unsigned char)(floor(U1(p+1)+0.5));
        sc=U1(p+10);
        raw->obs.data[n].LLI[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        raw->obs.data[n].LLI[0]|=U1(p+6)&1?2:0;
        raw->obs.data[n].code[0]=CODE_L1C;
        raw->lockt[sat-1][0]=sc;
        
        for (j=1;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->obs.n=n;
    return 1;
}
/* decode id#22 ephemeris data ------------------------------------------------*/
static int decode_ss2eph(raw_t *raw)
{
    eph_t eph={0};
    unsigned int tow;
    int i,j,prn,sat;
    unsigned char *p=raw->buff+4,buff[90]={0};
    
    trace(4,"decode_ss2eph: len=%d\n",raw->len);
    
    if (raw->len!=79) {
        trace(2,"ss2 id#22 length error: len=%d\n",raw->len);
        return -1;
    }
    prn=(U4(p)&0x1F)+1;
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"ss2 id#22 satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (raw->time.time==0) {
        trace(2,"ss2 id#22 week number unknown error\n");
        return -1;
    }
    tow=(unsigned int)(time2gpst(raw->time,NULL)/6.0);
    for (i=0;i<3;i++) {
        buff[30*i+3]=(unsigned char)(tow>>9); /* add tow + subframe id */
        buff[30*i+4]=(unsigned char)(tow>>1);
        buff[30*i+5]=(unsigned char)(((tow&1)<<7)+((i+1)<<2));
        for (j=0;j<24;j++) buff[30*i+6+j]=p[1+24*i+j];
    }
    if (decode_frame(buff   ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(buff+30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(buff+60,&eph,NULL,NULL,NULL,NULL)!=3) {
        trace(2,"ss2 id#22 subframe error: prn=%d\n",prn);
        return -1;
    }
    if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    eph.sat=sat;
    eph.ttr=raw->time;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode id#67 sbas data ----------------------------------------------------*/
static int decode_ss2sbas(raw_t *raw)
{
    gtime_t time;
    int i,prn;
    unsigned char *p=raw->buff+4;
    
    trace(4,"decode_ss2sbas: len=%d\n",raw->len);
    
    if (raw->len!=54) {
        trace(2,"ss2 id#67 length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p+12);
    if (prn<MINPRNSBS||MAXPRNSBS<prn) return 0;
    raw->sbsmsg.week=U4(p);
    raw->sbsmsg.tow=(int)R8(p+4);
    time=gpst2time(raw->sbsmsg.week,raw->sbsmsg.tow);
    raw->sbsmsg.prn=prn;
    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=p[16+i];
    return 3;
}
/* decode superstar 2 raw message --------------------------------------------*/
static int decode_ss2(raw_t *raw)
{
    unsigned char *p=raw->buff;
    int type=U1(p+1);
    
    trace(3,"decode_ss2: type=%2d\n",type);
    
    if (!chksum(raw->buff,raw->len)) {
        trace(2,"ss2 message checksum error: type=%d len=%d\n",type,raw->len);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"SS2 %2d (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_SS2LLH : return decode_ss2llh (raw);
        case ID_SS2ECEF: return decode_ss2ecef(raw);
        case ID_SS2RAW : return decode_ss2meas(raw);
        case ID_SS2EPH : return decode_ss2eph (raw);
        case ID_SS2SBAS: return decode_ss2sbas(raw);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_ss2(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==SS2SOH&&(buff[1]^buff[2])==0xFF;
}
/* input superstar 2 raw message from stream -----------------------------------
* input next superstar 2 raw message from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
* notes  : needs #20 or #21 message to get proper week number of #23 raw
*          observation data
*-----------------------------------------------------------------------------*/
extern int input_ss2(raw_t *raw, unsigned char data)
{
    trace(5,"input_ss2: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_ss2(raw->buff,data)) return 0;
        raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==4) {
        if ((raw->len=U1(raw->buff+3)+6)>MAXRAWLEN) {
            trace(2,"ss2 length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
    if (raw->nbyte<4||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode superstar 2 raw message */
    return decode_ss2(raw);
}
/* input superstar 2 raw message from file -------------------------------------
* input next superstar 2 raw message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_ss2f(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_ss2f:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_ss2(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,1,1,fp)<1) return -2;
    raw->nbyte=4;
    
    if ((raw->len=U1(raw->buff+3)+6)>MAXRAWLEN) {
        trace(2,"ss2 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+4,1,raw->len-4,fp)<(size_t)(raw->len-4)) return -2;
    raw->nbyte=0;
    
    /* decode superstar 2 raw message */
    return decode_ss2(raw);
}
