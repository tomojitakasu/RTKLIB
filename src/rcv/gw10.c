/*------------------------------------------------------------------------------
* gw10.c : furuno GW-10 receiver functions
*
*          Copyright (C) 2011-2012 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] Furuno, SBAS/GPS receiver type GW-10 III manual, July 2004
*
* version : $Revision:$ $Date:$
* history : 2011/05/27  1.0  new
*           2011/07/01  1.1  suppress warning
*           2012/02/14  1.2  add decode of gps message (0x02)
*           2017/04/11  1.3  (char *) -> (singed char *)
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define GW10SYNC    0x8B        /* gw10 sync code */

#define ID_GW10RAW  0x08        /* gw10 msg id: raw obs data */
#define ID_GW10GPS  0x02        /* gw10 msg id: gps message */
#define ID_GW10SBS  0x03        /* gw10 msg id: sbas message */
#define ID_GW10DGPS 0x06        /* gw10 msg id: dgps message */
#define ID_GW10REF  0x07        /* gw10 msg id: dgps ref info */
#define ID_GW10SOL  0x20        /* gw10 msg id: solution */
#define ID_GW10SATH 0x22        /* gw10 msg id: satellite health */
#define ID_GW10SATO 0x23        /* gw10 msg id: satellite orbit */
#define ID_GW10EPH  0x24        /* gw10 msg id: ephemeris */
#define ID_GW10ALM  0x25        /* gw10 msg id: almanac */
#define ID_GW10ION  0x26        /* gw10 msg id: ion/utc correction */
#define ID_GW10REPH 0x27        /* gw10 msg id: raw ephemeris */

#define LEN_GW10RAW 379         /* gw10 msg length: raw obs data */
#define LEN_GW10GPS 48          /* gw10 msg length: gps message */
#define LEN_GW10SBS 40          /* gw10 msg length: sbas message */
#define LEN_GW10DGPS 21         /* gw10 msg length: dgps message */
#define LEN_GW10REF 22          /* gw10 msg length: dgps ref info */
#define LEN_GW10SOL 227         /* gw10 msg length: solution */
#define LEN_GW10SATH 17         /* gw10 msg length: satellite health */
#define LEN_GW10SATO 67         /* gw10 msg length: satellite orbit */
#define LEN_GW10EPH 68          /* gw10 msg length: ephemeris */
#define LEN_GW10ALM 39          /* gw10 msg length: almanac */
#define LEN_GW10ION 32          /* gw10 msg length: ion/utc correction */
#define LEN_GW10REPH 98         /* gw10 msg length: raw ephemeris */

#define OFFWEEK     1024        /* week offset for ephemeris */

/* extract field (big-endian) ------------------------------------------------*/
#define U1(p)       (*((unsigned char *)(p)))
#define I1(p)       (*((signed char *)(p)))

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
static double R8(unsigned char *p)
{
    double value;
    unsigned char *q=(unsigned char *)&value+7;
    int i;
    for (i=0;i<8;i++) *q--=*p++;
    return value;
}
/* message length ------------------------------------------------------------*/
static int msglen(unsigned char id)
{
    switch (id) {
        case ID_GW10RAW : return LEN_GW10RAW ;
        case ID_GW10GPS : return LEN_GW10GPS ;
        case ID_GW10SBS : return LEN_GW10SBS ;
        case ID_GW10DGPS: return LEN_GW10DGPS;
        case ID_GW10REF : return LEN_GW10REF ;
        case ID_GW10SOL : return LEN_GW10SOL ;
        case ID_GW10SATH: return LEN_GW10SATH;
        case ID_GW10SATO: return LEN_GW10SATO;
        case ID_GW10EPH : return LEN_GW10EPH ;
        case ID_GW10ALM : return LEN_GW10ALM ;
        case ID_GW10ION : return LEN_GW10ION ;
        case ID_GW10REPH: return LEN_GW10REPH;
    }
    return 0;
}
/* compute checksum ----------------------------------------------------------*/
static int chksum(const unsigned char *buff, int n)
{
    unsigned char cs=0;
    int i;
    for (i=1;i<n-1;i++) cs+=buff[i];
    return buff[n-1]==cs;
}
/* adjust weekly rollover of gps time ----------------------------------------*/
static int adjweek(raw_t *raw, double tow)
{
    double tow_p;
    int week;
    
    if (raw->time.time==0) return 0;
    tow_p=time2gpst(raw->time,&week);
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    raw->time=gpst2time(week,tow);
    return 1;
}
/* bcd to number -------------------------------------------------------------*/
static int bcd2num(unsigned char bcd)
{
    return (bcd>>4)*10+(bcd&0xF);
}
/* decode raw obs data -------------------------------------------------------*/
static int decode_gw10raw(raw_t *raw)
{
    double tow,tows,toff,pr,cp;
    int i,j,n,prn,flg,sat,snr;
    unsigned char *p=raw->buff+2;
    
    trace(4,"decode_gw10raw: len=%d\n",raw->len);
    
    tow=R8(p);
    tows=floor(tow*1000.0+0.5)/1000.0; /* round by 10ms */
    toff=CLIGHT*(tows-tow);            /* time tag offset (m) */
    if (!adjweek(raw,tows)) {
        trace(2,"decode_gw10raw: no gps week infomation\n");
        return 0;
    }
    for (i=n=0,p+=8;i<16&&n<MAXOBS;i++,p+=23) {
        if (U1(p+1)!=1) continue;
        prn=U1(p);
        if (!(sat=satno(prn<=MAXPRNGPS?SYS_GPS:SYS_SBS,prn))) {
            trace(2,"gw10raw satellite number error: prn=%d\n",prn);
            continue;
        }
        pr =R8(p+ 2)-toff;
        snr=U2(p+16);
        cp =-(int)(U4(p+18))/256.0-toff/lam_carr[0];
        flg=U1(p+22);
        if (flg&0x3) {
            trace(2,"gw10raw raw data invalid: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].time=raw->time;
        raw->obs.data[n].sat =sat;
        raw->obs.data[n].P[0]=pr;
        raw->obs.data[n].L[0]=(flg&0x80)?0.0:((flg&0x40)?cp-0.5:cp);
        raw->obs.data[n].D[0]=0.0;
        raw->obs.data[n].SNR[0]=(unsigned char)(snr*4.0+0.5);
        raw->obs.data[n].LLI[0]=(flg&0x80)?1:0;
        raw->obs.data[n].code[0]=CODE_L1C;
        
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
/* check partity -------------------------------------------------------------*/
extern int check_parity(unsigned int word, unsigned char *data)
{
    const unsigned int hamming[]={
        0xBB1F3480,0x5D8F9A40,0xAEC7CD00,0x5763E680,0x6BB1F340,0x8B7A89C0
    };
    unsigned int parity=0,w;
    int i;
    
    for (i=0;i<6;i++) {
        parity<<=1;
        for (w=(word&hamming[i])>>6;w;w>>=1) parity^=w&1;
    }
    if (parity!=(word&0x3F)) return 0;
    
    for (i=0;i<3;i++) data[i]=(unsigned char)(word>>(22-i*8));
    return 1;
}
/* decode gps message --------------------------------------------------------*/
static int decode_gw10gps(raw_t *raw)
{
    eph_t eph={0};
    double tow,ion[8]={0},utc[4]={0};
    unsigned int buff=0;
    int i,prn,sat,id,leaps;
    unsigned char *p=raw->buff+2,subfrm[30];
    
    trace(4,"decode_gw10gps: len=%d\n",raw->len);
    
    tow=U4(p)/1000.0; p+=4;
    prn=U1(p);        p+=1;
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"gw10 gps satellite number error: tow=%.1f prn=%d\n",tow,prn);
        return -1;
    }
    for (i=0;i<10;i++) {
        buff=(buff<<30)|U4(p); p+=4;
        
        /* check parity of word */
        if (!check_parity(buff,subfrm+i*3)) {
            trace(2,"gw10 gps frame parity error: tow=%.1f prn=%2d word=%2d\n",
                 tow,prn,i+1);
            return -1;
        }
    }
    id=getbitu(subfrm,43,3); /* subframe id */
    
    if (id<1||5<id) {
        trace(2,"gw10 gps frame id error: tow=%.1f prn=%2d id=%d\n",tow,prn,id);
        return -1;
    }
    for (i=0;i<30;i++) raw->subfrm[sat-1][i+(id-1)*30]=subfrm[i];
    
    if (id==3) { /* decode ephemeris */
        if (decode_frame(raw->subfrm[sat-1]   ,&eph,NULL,NULL,NULL,NULL)!=1||
            decode_frame(raw->subfrm[sat-1]+30,&eph,NULL,NULL,NULL,NULL)!=2||
            decode_frame(raw->subfrm[sat-1]+60,&eph,NULL,NULL,NULL,NULL)!=3) {
            return 0;
        }
        if (!strstr(raw->opt,"-EPHALL")) {
            if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
        }
        eph.sat=sat;
        raw->nav.eph[sat-1]=eph;
        raw->ephsat=sat;
        return 2;
    }
    else if (id==4) { /* decode ion-utc parameters */
        if (decode_frame(subfrm,NULL,NULL,ion,utc,&leaps)!=4) {
            return 0;
        }
        if (norm(ion,8)>0.0&&norm(utc,4)>0.0&&leaps!=0) {
            for (i=0;i<8;i++) raw->nav.ion_gps[i]=ion[i];
            for (i=0;i<4;i++) raw->nav.utc_gps[i]=utc[i];
            raw->nav.leaps=leaps;
            return 9;
        }
    }
    return 0;
}
/* decode waas messages ------------------------------------------------------*/
static int decode_gw10sbs(raw_t *raw)
{
    double tow;
    int i,prn;
    unsigned char *p=raw->buff+2;
    
    trace(4,"decode_gw10sbs : len=%d\n",raw->len);
    
    tow=U4(p)/1000.0;
    prn=U1(p+4);
    if (prn<MINPRNSBS||MAXPRNSBS<prn) {
        trace(2,"gw10 sbs satellite number error: prn=%d\n",prn);
        return -1;
    }
    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=(int)tow;
    tow=time2gpst(raw->time,&raw->sbsmsg.week);
    if      (raw->sbsmsg.tow<tow-302400.0) raw->sbsmsg.week++;
    else if (raw->sbsmsg.tow>tow+302400.0) raw->sbsmsg.week--;
    
    for (i=0;i<29;i++) {
        raw->sbsmsg.msg[i]=*(p+5+i);
    }
    raw->sbsmsg.msg[28]&=0xC0; 
    return 3;
}
/* decode raw ephemereris ----------------------------------------------------*/
static int decode_gw10reph(raw_t *raw)
{
    eph_t eph={0};
    double tow;
    int i,week,prn,sat;
    unsigned char *p=raw->buff+2,buff[90];
    
    trace(4,"decode_gw10reph: len=%d\n",raw->len);
    
    prn=U1(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"gw10 raw ephemeris satellite number error: prn=%d\n",prn);
        return -1;
    }
    for (i=0;i<90;i++) {
        buff[i]=*(p+1+i);
    }
    if (decode_frame(buff    ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(buff+ 30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(buff+ 60,&eph,NULL,NULL,NULL,NULL)!=3) {
        trace(2,"gw10 raw ephemeris navigation frame error: prn=%d\n",prn);
        return -1;
    }
    /* set time if no time avaliable */
    if (raw->time.time==0) {
        tow=getbitu(buff,24,17)*6.0;
        week=getbitu(buff,48,10)+OFFWEEK;
        raw->time=timeadd(gpst2time(week,tow),24.0);
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode solution -----------------------------------------------------------*/
static int decode_gw10sol(raw_t *raw)
{
    gtime_t time;
    double ep[6]={0},sec;
    unsigned char *p=raw->buff+6;
    
    trace(4,"decode_gw10sol : len=%d\n",raw->len);
    
    if (U2(p+42)&0xC00) { /* time valid? */
        trace(2,"gw10 sol time/day invalid\n");
        return 0;
    }
    sec=U4(p+27)/16384.0;
    sec=floor(sec*1000.0+0.5)/1000.0;
    ep[2]=bcd2num(p[31]);
    ep[1]=bcd2num(p[32]);
    ep[0]=bcd2num(p[33])*100+bcd2num(p[34]);
    time=utc2gpst(timeadd(epoch2time(ep),sec));
    
    /* set time if no time available */
    if (raw->time.time==0) {
        raw->time=time;
    }
    return 0;
}
/* decode gw10 raw message ---------------------------------------------------*/
static int decode_gw10(raw_t *raw)
{
    int type=U1(raw->buff+1);
    
    trace(3,"decode_gw10: type=0x%02X len=%d\n",type,raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"GW10 0x%02X (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_GW10RAW : return decode_gw10raw (raw);
        case ID_GW10GPS : return decode_gw10gps (raw);
        case ID_GW10SBS : return decode_gw10sbs (raw);
        case ID_GW10REPH: return decode_gw10reph(raw);
        case ID_GW10SOL : return decode_gw10sol (raw);
    }
    return 0;
}
/* input gw10 raw message ------------------------------------------------------
* input next gw10 raw message from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL    : input all ephemerides
*
*-----------------------------------------------------------------------------*/
extern int input_gw10(raw_t *raw, unsigned char data)
{
    int stat;
    trace(5,"input_gw10: data=%02x\n",data);
    
    raw->buff[raw->nbyte++]=data;
    
    /* synchronize frame */
    if (raw->buff[0]!=GW10SYNC) {
        raw->nbyte=0;
        return 0;
    }
    if (raw->nbyte>=2&&!(raw->len=msglen(raw->buff[1]))) {
        raw->nbyte=0;
        return 0;
    }
    if (raw->nbyte<2||raw->nbyte<raw->len) return 0;
    
    if (!chksum(raw->buff,raw->len)) {
        tracet(2,"gw10 message checksum error msg=%d\n",raw->buff[1]);
        raw->buff[0]=0;
        raw->nbyte=0;
        return -1;
    }
    /* decode gw10 raw message */
    stat=decode_gw10(raw);
    
    raw->buff[0]=0;
    raw->nbyte=0;
    
    return stat;
}
/* input gw10 raw message from file --------------------------------------------
* input next gw10 raw message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_gw10f(raw_t *raw, FILE *fp)
{
    int i,data,ret;
    
    trace(4,"input_gw10f:\n");
    
    for (i=0;i<4096;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if ((ret=input_gw10(raw,(unsigned char)data))) return ret;
    }
    return 0; /* return at every 4k bytes */
}
