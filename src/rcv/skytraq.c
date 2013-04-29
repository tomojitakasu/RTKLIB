/*------------------------------------------------------------------------------
* skytraq.c : skytraq receiver dependent functions
*
*          Copyright (C) 2009-2011 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] Skytraq, Application Note AN0023 Binary Message of SkyTraq Venus 6 
*         GPS Receiver, ver 1.4.8, August 21, 2008
*     [2] Skytraq, Application Note AN0024 Raw Measurement Binary Message
*         Extension of SkyTraq Venus 6 GPS Receiver, ver 0.5, October 9, 2009
*     [3] Skytraq, Application Note AN0024G2 Binary Message of SkyTraq Venus 7
*         GLONASS/GPS Receiver (Raw Measurement F/W), ver 1.4.26, April 26, 2012
*
* notes   :
*     The byte order of S1315F raw message is big-endian inconsistent to [1].
*
* version : $Revision:$
* history : 2009/10/10 1.0 new
*           2009/11/08 1.1 flip carrier-phase polarity for F/W 1.8.23-20091106
*           2011/05/27 1.2 add almanac decoding
*                          fix problem with ARM compiler
*           2011/07/01 1.3 suppress warning
*           2013/03/10 1.5 change option -invcp to -INVCP
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define STQSYNC1    0xA0        /* skytraq binary sync code 1 */
#define STQSYNC2    0xA1        /* skytraq binary sync code 2 */

#define ID_STQTIME  0xDC        /* skytraq message id: measurement time info */
#define ID_STQRAW   0xDD        /* skytraq message id: raw channel measurement */
#define ID_STQSFRB  0xE0        /* skytraq message id: subframe buffer data */
#define ID_STQGLOSTR 0xE1       /* skytraq message id: glonass string buffer */
#define ID_RESTART  0x01        /* skytraq message id: system restart */
#define ID_CFGSERI  0x05        /* skytraq message id: configure serial port */
#define ID_CFGFMT   0x09        /* skytraq message id: configure message format */
#define ID_CFGRATE  0x12        /* skytraq message id: configure message rate */

static const char rcsid[]="$Id:$";

/* extract field (big-endian) ------------------------------------------------*/
#define U1(p)       (*((unsigned char *)(p)))
#define I1(p)       (*((char *)(p)))

static unsigned short U2(unsigned char *p)
{
    unsigned short value;
    unsigned char *q=(unsigned char *)&value+1;
    int i;
    for (i=0;i<2;i++) *q--=*p++;
    return value;
}
static unsigned int U4(unsigned char *p)
{
    unsigned int value;
    unsigned char *q=(unsigned char *)&value+3;
    int i;
    for (i=0;i<4;i++) *q--=*p++;
    return value;
}
static float R4(unsigned char *p)
{
    float value;
    unsigned char *q=(unsigned char *)&value+3;
    int i;
    for (i=0;i<4;i++) *q--=*p++;
    return value;
}
static double R8(unsigned char *p)
{
    double value;
    unsigned char *q=(unsigned char *)&value+7;
    int i;
    for (i=0;i<8;i++) *q--=*p++;
    return value;
}
/* checksum ------------------------------------------------------------------*/
static unsigned char checksum(unsigned char *buff, int len)
{
    unsigned char cs=0;
    int i;
    
    for (i=4;i<len-3;i++) {
        cs^=buff[i];
    }
    return cs;
}
/* decode skytraq measurement time info --------------------------------------*/
static int decode_stqtime(raw_t *raw)
{
    unsigned char *p=raw->buff+4;
    double tow;
    int week;
    
    trace(4,"decode_stqtime: len=%d\n",raw->len);
    
    raw->iod=U1(p+1);
    week    =U2(p+2);
    tow     =U4(p+4)*0.001;
    raw->time=gpst2time(week,tow);
    return 0;
}
/* decode skytraq raw channel mesurement -------------------------------------*/
static int decode_stqraw(raw_t *raw)
{
    int i,j,iod,prn,sys,sat,n=0,nsat;
    unsigned char *p=raw->buff+4,ind;
    
    trace(4,"decode_stqraw: len=%d\n",raw->len);
    
    iod=U1(p+1);
    if (iod!=raw->iod) {
        trace(2,"stq raw iod error: iod=%d %d\n",iod,raw->iod);
        return -1;
    }
    nsat=U1(p+2);
    if (raw->len<8+23*nsat) {
        trace(2,"stq raw length error: len=%d nsat=%d\n",raw->len,nsat);
        return -1;
    }
    for (i=0,p+=3;i<nsat&&i<MAXOBS;i++,p+=23) {
        ind                    =U1(p+22);
        prn                    =U1(p);
        raw->obs.data[n].SNR[0]=(unsigned char)(U1(p+1)*4.0+0.5);
        raw->obs.data[n].P[0]  =(ind&0x1)?R8(p+ 2):0.0;
        raw->obs.data[n].L[0]  =(ind&0x4)?R8(p+10):0.0;
        raw->obs.data[n].D[0]  =(ind&0x2)?R4(p+18):0.0f;
        raw->obs.data[n].LLI[0]=(ind&0x8)?1:0; /* slip */
        raw->obs.data[n].code[0]=CODE_L1C;
        
        /* receiver dependent options */
        if (strstr(raw->opt,"-INVCP")) {
            raw->obs.data[n].L[0]=-raw->obs.data[n].L[0];
        }
        if (MINPRNGPS<=prn&&prn<=MAXPRNGPS) {
            sys=SYS_GPS;
        }
        else if (MINPRNGLO<=prn-64&&prn-64<=MAXPRNGLO) {
            sys=SYS_GLO;
            prn-=64;
        }
        else {
            trace(2,"stq raw satellite number error: prn=%d\n",prn);
            continue;
        }
        if (!(sat=satno(sys,prn))) {
            trace(2,"stq raw satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        raw->obs.data[n].time=raw->time;
        raw->obs.data[n].sat =sat;
        
        for (j=1;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->obs.n=n;
    return n>0?1:0;
}
/* save subframe -------------------------------------------------------------*/
static int save_subfrm(int sat, raw_t *raw)
{
    unsigned char *p=raw->buff+7,*q;
    int i,id;
    
    trace(4,"save_subfrm: sat=%2d\n",sat);
    
    /* check navigation subframe preamble */
    if (p[0]!=0x8B) {
        trace(2,"stq subframe preamble error: 0x%02X\n",p[0]);
        return 0;
    }
    id=(p[5]>>2)&0x7;
    
    /* check subframe id */
    if (id<1||5<id) {
        trace(2,"stq subframe id error: id=%d\n",id);
        return 0;
    }
    q=raw->subfrm[sat-1]+(id-1)*30;
    
    for (i=0;i<30;i++) q[i]=p[i];
    
    return id;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_ephem(int sat, raw_t *raw)
{
    eph_t eph={0};
    
    trace(4,"decode_ephem: sat=%2d\n",sat);
    
    if (decode_frame(raw->subfrm[sat-1]   ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(raw->subfrm[sat-1]+30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(raw->subfrm[sat-1]+60,&eph,NULL,NULL,NULL,NULL)!=3) return 0;
    
    if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode almanac and ion/utc ------------------------------------------------*/
static int decode_alm1(int sat, raw_t *raw)
{
    trace(4,"decode_alm1 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+90,NULL,raw->nav.alm,raw->nav.ion_gps,
                 raw->nav.utc_gps,&raw->nav.leaps);
    return 0;
}
/* decode almanac ------------------------------------------------------------*/
static int decode_alm2(int sat, raw_t *raw)
{
    trace(4,"decode_alm2 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+120,NULL,raw->nav.alm,NULL,NULL,NULL);
    return 0;
}
/* decode skytraq subframe buffer --------------------------------------------*/
static int decode_stqsfrb(raw_t *raw)
{
    int prn,sat,id;
    unsigned char *p=raw->buff+4;
    
    trace(4,"decode_stqsfrb: len=%d\n",raw->len);
    
    if (raw->len<40) {
        trace(2,"stq subframe length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U1(p+1);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"stq subframe satellite number error: prn=%d\n",prn);
        return -1;
    }
    id=save_subfrm(sat,raw);
    if (id==3) return decode_ephem(sat,raw);
    if (id==4) return decode_alm1 (sat,raw);
    if (id==5) return decode_alm2 (sat,raw);
    return 0;
}
/* decode skytraq glonass string buffer --------------------------------------*/
static int decode_stqglostr(raw_t *raw)
{
    int i,prn,sat,strno;
    unsigned char *p=raw->buff+4;
    
    trace(4,"decode_stqglostr: len=%d\n",raw->len);
    
    if (raw->len<19) {
        trace(2,"stq glo string length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U1(p+1)-64;
    if (!(sat=satno(SYS_GLO,prn))) {
        trace(2,"stq glo string satellite number error: prn=%d\n",prn);
        return -1;
    }
    strno=U1(p+2);
    if (strno<1||4<strno) {
        trace(2,"stq glo string number error: prn=%d strno=%d\n",prn,strno);
        return -1;
    }
    for (i=0;i<9;i++) {
        raw->subfrm[sat-1][strno*9-1-i]=p[3+i];
    }
    if (strno<5) return 0;
    
    return 0;
}
/* decode skytraq message ----------------------------------------------------*/
static int decode_stq(raw_t *raw)
{
    int type=U1(raw->buff+4);
    unsigned char cs,*p=raw->buff+raw->len-3;
    
    trace(3,"decode_stq: type=%02x len=%d\n",type,raw->len);
    
    /* checksum */
    cs=checksum(raw->buff,raw->len);
    if (cs!=*p||*(p+1)!=0x0D||*(p+2)!=0x0A) {
        trace(2,"stq checksum error: type=%02X cs=%02X tail=%02X%02X%02X\n",
              type,cs,*p,*(p+1),*(p+2));
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"SKYTRAQ 0x%02x (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_STQTIME  : return decode_stqtime  (raw);
        case ID_STQRAW   : return decode_stqraw   (raw);
        case ID_STQSFRB  : return decode_stqsfrb  (raw);
        case ID_STQGLOSTR: return decode_stqglostr(raw);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_stq(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==STQSYNC1&&buff[1]==STQSYNC2;
}
/* input skytraq raw message from stream ---------------------------------------
* fetch next skytraq raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -INVCP     : inverse polarity of carrier-phase
*
*-----------------------------------------------------------------------------*/
extern int input_stq(raw_t *raw, unsigned char data)
{
    trace(5,"input_stq: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_stq(raw->buff,data)) return 0;
        raw->nbyte=2;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==4) {
        if ((raw->len=U2(raw->buff+2)+7)>MAXRAWLEN) {
            trace(2,"stq message length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
    if (raw->nbyte<4||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode skytraq raw message */
    return decode_stq(raw);
}
/* input skytraq raw message from file -----------------------------------------
* fetch next skytraq raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_stqf(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_stqf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_stq(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+2,1,2,fp)<2) return -2;
    raw->nbyte=4;
    
    if ((raw->len=U2(raw->buff+2)+7)>MAXRAWLEN) {
        trace(2,"stq message length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+4,1,raw->len-4,fp)<(size_t)(raw->len-4)) return -2;
    raw->nbyte=0;
    
    /* decode skytraq raw message */
    return decode_stq(raw);
}
/* generate skytraq binary message ---------------------------------------------
* generate skytraq binary message from message string
* args   : char  *msg   I      message string 
*            "RESTART  [arg...]" system restart
*            "CFG-SERI [arg...]" configure serial port propperty
*            "CFG-FMT  [arg...]" configure output message format
*            "CFG-RATE [arg...]" configure binary measurement output rates
*          unsigned char *buff O binary message
* return : length of binary message (0: error)
* note   : see reference [1][2] for details.
*-----------------------------------------------------------------------------*/
extern int gen_stq(const char *msg, unsigned char *buff)
{
    const char *hz[]={"1Hz","2Hz","4Hz","5Hz","10Hz","20Hz",""};
    unsigned char *q=buff;
    char mbuff[1024],*args[32],*p;
    int i,n,narg=0;
    
    trace(4,"gen_stq: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    *q++=STQSYNC1;
    *q++=STQSYNC2;
    if (!strcmp(args[0],"RESTART")) {
        *q++=0;
        *q++=15;
        *q++=ID_RESTART;
        *q++=narg>2?(unsigned char)atoi(args[1]):0;
        for (i=1;i<15;i++) *q++=0; /* set all 0 */
    }
    else if (!strcmp(args[0],"CFG-SERI")) {
        *q++=0;
        *q++=4;
        *q++=ID_CFGSERI;
        for (i=1;i<4;i++) *q++=narg>i+1?(unsigned char)atoi(args[i]):0;
    }
    else if (!strcmp(args[0],"CFG-FMT")) {
        *q++=0;
        *q++=3;
        *q++=ID_CFGFMT;
        for (i=1;i<3;i++) *q++=narg>i+1?(unsigned char)atoi(args[i]):0;
    }
    else if (!strcmp(args[0],"CFG-RATE")) {
        *q++=0;
        *q++=8;
        *q++=ID_CFGRATE;
        if (narg>2) {
            for (i=0;*hz[i];i++) if (!strcmp(args[1],hz[i])) break;
            if (*hz[i]) *q++=i; else *q++=(unsigned char)atoi(args[1]);
        }
        else *q++=0;
        for (i=2;i<8;i++) *q++=narg>i+1?(unsigned char)atoi(args[i]):0;
    }
    else return 0;
    
    n=(int)(q-buff);
    *q++=checksum(buff,n+3);
    *q++=0x0D;
    *q=0x0A;
    return n+3;
}
