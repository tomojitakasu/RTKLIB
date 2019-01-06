/*------------------------------------------------------------------------------
* javad.c : javad receiver dependent functions
*
*          Copyright (C) 2011-2018 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] Javad GNSS, GREIS GNSS Receiver External Interface Specification,
*         Reflects Firmware Version 3.2.0, July 22, 2010
*     [2] Javad navigation systemms, GPS Receiver Interface Language (GRIL)
*         Reference Guide Rev 2.2, Reflects Firmware Version 2.6.0
*     [3] Javad GNSS, User visible changes in the firmware vesion 3.4.0 since
*         version 3.3.x (NEWS_3_4_0.txt)
*     [4] Javad GNSS, GREIS GNSS Receiver External Interface Specification,
*         Reflects Firmware Version 3.4.6, October 9, 2012
*     [5] Javad GNSS, GREIS GNSS Receiver External Interface Specification,
*         Reflects Firmware Version 3.5.4, January 30, 2014
*     [6] Javad GNSS, GREIS GNSS Receiver External Interface Specification,
*         Reflects Firmware Version 3.6.7, August 25, 2016
*     [7] Javad GNSS, GREIS GNSS Receiver External Interface Specification,
*         Reflects Firmware Version 3.7.2, October 11, 2017
*
* version : $Revision:$ $Date:$
* history : 2011/05/27 1.0  new
*           2011/07/07 1.1  fix QZSS IODC-only-update problem
*           2012/07/17 1.2  change GALILEO scale factor for short pseudorange
*           2012/10/18 1.3  change receiver options and rinex obs code
*           2013/01/24 1.4  change compass factor for short pseudorange
*                           add raw option -NOET
*           2013/02/23 1.6  fix memory access violation problem on arm
*           2013/05/08 1.7  fix bug on week number of galileo ephemeris
*           2014/05/23 1.8  support beidou
*           2014/06/23 1.9  support [lD] for glonass raw navigation data
*           2014/08/26 1.10 fix bug on decoding iode in glonass ephemeris [NE]
*           2014/10/20 1.11 fix bug on receiver option -GL*,-RL*,-JL*
*           2016/01/26 1.12 fix problem on bus-error on ARM CPU (#129)
*           2017/04/11 1.13 support IRNSS
*                           fix bug on carrier frequency for beidou
*                           fix bug on unchange-test for beidou ephemeris
*                           update Asys coef for [r*] short pseudorange by [6]
*                           (char *) -> (signed char *)
*           2018/10/10 1.14 update signal allocation by ref [7]
*                           update Ksys value for galileo by ref [7]
*                           fix problem to set eph->code for beidou and galileo
*                           fix bug on saving galileo bgd to ephemeris
*                           add receiver option -GALINAV, -GALFNAV
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define PREAMB_CNAV 0x8B

#define ISTXT(c)    ('0'<=(c)&&(c)<='~')
#define ISHEX(c)    (('0'<=(c)&&(c)<='9')||('A'<=(c)&&(c)<='F'))
#define ROT_LEFT(val) (((val)<<2)|((val)>>6))

/* extract field (little-endian) ---------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((signed char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}

static float R4(unsigned char *p)
{
    float value;
    unsigned char *q=(unsigned char *)&value;
    int i;
    if (U4(p)==0x7FC00000) return 0.0f; /* quiet nan */
    for (i=0;i<4;i++) *q++=*p++;
    return value;
}
static double R8(unsigned char *p)
{
    double value;
    unsigned char *q=(unsigned char *)&value;
    int i;
    if (U4(p+4)==0x7FF80000&&U4(p)==0) return 0.0; /* quiet nan */
    for (i=0;i<8;i++) *q++=*p++;
    return value;
}
/* decode message length -----------------------------------------------------*/
static int decodelen(const unsigned char *buff)
{
    unsigned int len;
    if (!ISHEX(buff[0])||!ISHEX(buff[1])||!ISHEX(buff[2])) return 0;
    if (sscanf((char *)buff,"%3X",&len)==1) return (int)len;
    return 0;
}
/* test measurement data -----------------------------------------------------*/
static int is_meas(char sig)
{
    return sig=='c'||sig=='C'||sig=='1'||sig=='2'||sig=='3'||sig=='5'||sig=='l';
}
/* convert signal to frequency and obs type ----------------------------------*/
static int tofreq(char sig, int sys, int *type)
{
    const unsigned char types[7][6]={ /* ref [7] table 3-8 */
        /*  c/C       1        2        3        5        l  */
        /* (CA/L1    P/L1     P/L2    CA/L2      L5      L1C) */
        {CODE_L1C,CODE_L1W,CODE_L2W,CODE_L2X,CODE_L5X,CODE_L1X}, /* GPS */
        {CODE_L1C,CODE_L1Z,CODE_L6X,CODE_L2X,CODE_L5X,CODE_L1X}, /* QZS */
        {CODE_L1C,0       ,0       ,0       ,CODE_L5X,0       }, /* SBS */
        {CODE_L1X,CODE_L8X,CODE_L7X,CODE_L6X,CODE_L5X,0       }, /* GAL */
        {CODE_L1C,CODE_L1P,CODE_L2P,CODE_L2C,CODE_L3X,0       }, /* GLO */
        {CODE_L1I,0       ,CODE_L7I,CODE_L6I,CODE_L5X,CODE_L1X}, /* BDS */
        {0       ,0       ,0       ,0       ,CODE_L5X,0       }  /* IRN */
    };
    const int freqs[7][6]={
        {1,1,2,2,3,1}, {1,1,4,2,3,1}, {1,0,0,0,3,0},     /* GPS,QZS,SBS */
        {1,6,5,4,3,0}, {1,1,2,2,3,0}, {1,0,2,3,3,1},     /* GAL,GLO,BDS */
        {0,0,0,0,3,0}                                    /* IRN */
    };
    int i,j;
    
    switch (sig) {
        case 'c':
        case 'C': i=0; break;
        case '1': i=1; break;
        case '2': i=2; break;
        case '3': i=3; break;
        case '5': i=4; break;
        case 'l': i=5; break;
        default: return -1;
    }
    switch (sys) {
        case SYS_GPS: j=0; break;
        case SYS_QZS: j=1; break;
        case SYS_SBS: j=2; break;
        case SYS_GAL: j=3; break;
        case SYS_GLO: j=4; break;
        case SYS_CMP: j=5; break;
        case SYS_IRN: j=6; break;
        default: return -1;
    }
    *type=types[j][i];
    
    /* 0:L1,1:L2,2:L5,3:L6,4:L7,5:L8,-1:error */
    return freqs[j][i]<=NFREQ?freqs[j][i]-1:-1;
}
/* check code priority and return obs position -------------------------------*/
static int checkpri(const char *opt, int sys, int code, int freq)
{
    int nex=NEXOBS; /* number of extended obs data */
    
    if (sys==SYS_GPS) {
        if (strstr(opt,"-GL1W")&&freq==0) return code==CODE_L1W?0:-1;
        if (strstr(opt,"-GL1X")&&freq==0) return code==CODE_L1X?0:-1;
        if (strstr(opt,"-GL2X")&&freq==1) return code==CODE_L2X?1:-1;
        if (code==CODE_L1W) return nex<1?-1:NFREQ;
        if (code==CODE_L2X) return nex<2?-1:NFREQ+1;
        if (code==CODE_L1X) return nex<3?-1:NFREQ+2;
    }
    else if (sys==SYS_GLO) {
        if (strstr(opt,"-RL1C")&&freq==0) return code==CODE_L1C?0:-1;
        if (strstr(opt,"-RL2C")&&freq==1) return code==CODE_L2C?1:-1;
        if (code==CODE_L1C) return nex<1?-1:NFREQ;
        if (code==CODE_L2C) return nex<2?-1:NFREQ+1;
    }
    else if (sys==SYS_QZS) {
        if (strstr(opt,"-JL1Z")&&freq==0) return code==CODE_L1Z?0:-1;
        if (strstr(opt,"-JL1X")&&freq==0) return code==CODE_L1X?0:-1;
        if (code==CODE_L1Z) return nex<1?-1:NFREQ;
        if (code==CODE_L1X) return nex<2?-1:NFREQ+1;
    }
    return freq<NFREQ?freq:-1;
}
/* carrier frequency for systems ---------------------------------------------*/
static double freq_sys(int sys, int freq, int freqn)
{
    if (sys==SYS_GLO) {
        switch (freq) {
            case 0: return FREQ1_GLO+DFRQ1_GLO*freqn; /* L1 */
            case 1: return FREQ2_GLO+DFRQ2_GLO*freqn; /* L2 */
            case 2: return FREQ3_GLO;                 /* L3 */
        }
        return 0.0;
    }
    else if (sys==SYS_CMP) {
        switch (freq) {
            case 0: return FREQ1_CMP; /* B1 */
            case 1: return FREQ2_CMP; /* B2 */
            case 2: return FREQ3_CMP; /* B3 */
        }
        return 0.0;
    }
    else {
        return CLIGHT/lam_carr[freq];
    }
}
/* checksum ------------------------------------------------------------------*/
static int checksum(unsigned char *buff, int len)
{
    unsigned char cs=0;
    int i;
    for (i=0;i<len-1;i++) {
        cs=ROT_LEFT(cs)^buff[i];
    }
    cs=ROT_LEFT(cs);
    return cs==buff[len-1];
}
/* adjust weekly rollover of gps time ----------------------------------------*/
static gtime_t adjweek(gtime_t time, double tow)
{
    double tow_p;
    int week;
    tow_p=time2gpst(time,&week);
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    return gpst2time(week,tow);
}
/* adjust daily rollover of time ---------------------------------------------*/
static gtime_t adjday(gtime_t time, double tod)
{
    double ep[6],tod_p;
    time2epoch(time,ep);
    tod_p=ep[3]*3600.0+ep[4]*60.0+ep[5];
    if      (tod<tod_p-43200.0) tod+=86400.0;
    else if (tod>tod_p+43200.0) tod-=86400.0;
    ep[3]=ep[4]=ep[5]=0.0;
    return timeadd(epoch2time(ep),tod);
}
/* set time tag --------------------------------------------------------------*/
static int settag(obsd_t *data, gtime_t time)
{
    char s1[64],s2[64];
    
    if (data->time.time!=0&&fabs(timediff(data->time,time))>5E-4) {
        time2str(data->time,s1,4); time2str(time,s2,4);
        trace(2,"time inconsistent: time=%s %s sat=%2d\n",s1,s2,data->sat);
        return 0;
    }
    data->time=time;
    return 1;
}
/* flush observation data buffer ---------------------------------------------*/
static int flushobuf(raw_t *raw)
{
    gtime_t time0={0};
    int i,j,n=0;
    
    trace(3,"flushobuf: n=%d\n",raw->obuf.n);
    
    /* copy observation data buffer */
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        if (!satsys(raw->obuf.data[i].sat,NULL)) continue;
        if (raw->obuf.data[i].time.time==0) continue;
        raw->obs.data[n++]=raw->obuf.data[i];
    }
    raw->obs.n=n;
    
    /* clear observation data buffer */
    for (i=0;i<MAXOBS;i++) {
        raw->obuf.data[i].time=time0;
        for (j=0;j<NFREQ+NEXOBS;j++) {
            raw->obuf.data[i].L[j]=raw->obuf.data[i].P[j]=0.0;
            raw->obuf.data[i].D[j]=0.0;
            raw->obuf.data[i].SNR[j]=raw->obuf.data[i].LLI[j]=0;
            raw->obuf.data[i].code[j]=CODE_NONE;
        }
    }
    for (i=0;i<MAXSAT;i++) raw->prCA[i]=raw->dpCA[i]=0.0;
    return n>0?1:0;
}
/* decode [~~] receiver time -------------------------------------------------*/
static int decode_RT(raw_t *raw)
{
    gtime_t time;
    char *msg;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad RT error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<10) {
        trace(2,"javad RT length error: len=%d\n",raw->len);
        return -1;
    }
    raw->tod=U4(p);
    
    if (raw->time.time==0) return 0;
    
    /* update receiver time */
    time=raw->time;
    if (raw->tbase>=1) time=gpst2utc(time); /* gpst->utc */
    time=adjday(time,raw->tod*0.001);
    if (raw->tbase>=1) time=utc2gpst(time); /* utc->gpst */
    raw->time=time;
    
    trace(3,"decode_RT: time=%s\n",time_str(time,3));
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," %s",time_str(time,3));
    }
    /* flush observation data buffer */
    return flushobuf(raw);
}
/* decode [::] epoch time ----------------------------------------------------*/
static int decode_ET(raw_t *raw)
{
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad ET checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<10) {
        trace(2,"javad ET length error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->tod!=(int)U4(p)) {
        trace(2,"javad ET inconsistent tod: tod=%d %d\n",raw->tod,U4(p));
        return -1;
    }
    raw->tod=-1; /* end of epoch */
    
    /* flush observation data buffer */
    return flushobuf(raw);
}
/* decode [RD] receiver date -------------------------------------------------*/
static int decode_RD(raw_t *raw)
{
    double ep[6]={0};
    char *msg;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad RD checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<11) {
        trace(2,"javad RD length error: len=%d\n",raw->len);
        return -1;
    }
    ep[0]=U2(p); p+=2;
    ep[1]=U1(p); p+=1;
    ep[2]=U1(p); p+=1;
    raw->tbase=U1(p);
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," %04.0f/%02.0f/%02.0f base=%d",ep[0],ep[1],ep[2],raw->tbase);
    }
    if (raw->tod<0) {
        trace(2,"javad RD lack of preceding RT\n");
        return 0;
    }
    raw->time=timeadd(epoch2time(ep),raw->tod*0.001);
    if (raw->tbase>=1) raw->time=utc2gpst(raw->time); /* utc->gpst */
    
    trace(3,"decode_RD: time=%s\n",time_str(raw->time,3));
    
    return 0;
}
/* decode [SI] satellite indices ---------------------------------------------*/
static int decode_SI(raw_t *raw)
{
    int i,usi,sat;
    char *msg;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad SI checksum error: len=%d\n",raw->len);
        return -1;
    }
    raw->obuf.n=raw->len-6;
    
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        usi=U1(p); p+=1;
        
        if      (usi<=  0) sat=0;                      /* ref [5] table 3-6 */
        else if (usi<= 37) sat=satno(SYS_GPS,usi);     /*   1- 37: GPS */
        else if (usi<= 70) sat=255;                    /*  38- 70: GLONASS */
        else if (usi<=119) sat=satno(SYS_GAL,usi-70);  /*  71-119: GALILEO */
        else if (usi<=142) sat=satno(SYS_SBS,usi);     /* 120-142: SBAS */
        else if (usi<=192) sat=0;
        else if (usi<=197) sat=satno(SYS_QZS,usi);     /* 193-197: QZSS */
        else if (usi<=210) sat=0;
        else if (usi<=240) sat=satno(SYS_CMP,usi-210); /* 211-240: BeiDou */
        else if (usi<=247) sat=satno(SYS_IRN,usi-240); /* 241-247: IRNSS */
        else               sat=0;
        
        raw->obuf.data[i].time=raw->time;
        raw->obuf.data[i].sat=sat;
        
        /* glonass fcn (frequency channel number) */
        if (sat==255) raw->freqn[i]=usi-45;
    }
    trace(4,"decode_SI: nsat=raw->obuf.n\n");
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," nsat=%2d",raw->obuf.n);
    }
    return 0;
}
/* decode [NN] glonass satellite system numbers ------------------------------*/
static int decode_NN(raw_t *raw)
{
    unsigned char *p=raw->buff+5;
    char *msg;
    int i,n,ns,slot,sat,index[MAXOBS];
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad NN checksum error: len=%d\n",raw->len);
        return -1;
    }
    for (i=n=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        if (raw->obuf.data[i].sat==255) index[n++]=i;
    }
    ns=raw->len-6;
    
    for (i=0;i<ns&&i<n;i++) {
        slot=U1(p); p+=1;
        sat=satno(SYS_GLO,slot);
        raw->obuf.data[index[i]].sat=sat;
    }
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," nsat=%2d",ns);
    }
    return 0;
}
/* decode [GA] gps almanac ---------------------------------------------------*/
static int decode_GA(raw_t *raw)
{
    trace(2,"javad GA not supported\n");
    
    return 0;
}
/* decode [NA] glonass almanac -----------------------------------------------*/
static int decode_NA(raw_t *raw)
{
    trace(2,"javad NA not supported\n");
    
    return 0;
}
/* decode [EA] galileo almanac -----------------------------------------------*/
static int decode_EA(raw_t *raw)
{
    trace(2,"javad EA not supported\n");
    
    return 0;
}
/* decode [WA] waas almanac --------------------------------------------------*/
static int decode_WA(raw_t *raw)
{
    trace(2,"javad WA not supported\n");
    
    return 0;
}
/* decode [QA] qzss almanac --------------------------------------------------*/
static int decode_QA(raw_t *raw)
{
    trace(2,"javad QA not supported\n");
    
    return 0;
}
/* decode [CA] beidou almanac ------------------------------------------------*/
static int decode_CA(raw_t *raw)
{
    trace(2,"javad CA not supported\n");
    
    return 0;
}
/* decode [IA] irnss almanac -------------------------------------------------*/
static int decode_IA(raw_t *raw)
{
    trace(2,"javad IA not supported\n");
    
    return 0;
}
/* decode gps/galileo/qzss/beidou ephemeris ----------------------------------*/
static int decode_eph(raw_t *raw, int sys)
{
    eph_t eph={0};
    double toc,sqrtA,tt;
    char *msg;
    int prn,tow,flag,week,navtype,sigtype,eph_sel=0;
    unsigned char *p=raw->buff+5;
    
    trace(3,"decode_eph: sys=%2d prn=%3d\n",sys,U1(p));
    
    if (strstr(raw->opt,"-GALINAV")) eph_sel=1;
    if (strstr(raw->opt,"-GALFNAV")) eph_sel=2;
    
    prn       =U1(p);        p+=1;
    tow       =U4(p);        p+=4;
    flag      =U1(p);        p+=1;
    eph.iodc  =I2(p);        p+=2;
    toc       =I4(p);        p+=4;
    eph.sva   =I1(p);        p+=1;
    eph.svh   =U1(p);        p+=1;
    week      =I2(p);        p+=2;
    eph.tgd[0]=R4(p);        p+=4;
    eph.f2    =R4(p);        p+=4;
    eph.f1    =R4(p);        p+=4;
    eph.f0    =R4(p);        p+=4;
    eph.toes  =I4(p);        p+=4;
    eph.iode  =I2(p);        p+=2;
    sqrtA     =R8(p);        p+=8;
    eph.e     =R8(p);        p+=8;
    eph.M0    =R8(p)*SC2RAD; p+=8;
    eph.OMG0  =R8(p)*SC2RAD; p+=8;
    eph.i0    =R8(p)*SC2RAD; p+=8;
    eph.omg   =R8(p)*SC2RAD; p+=8;
    eph.deln  =R4(p)*SC2RAD; p+=4;
    eph.OMGd  =R4(p)*SC2RAD; p+=4;
    eph.idot  =R4(p)*SC2RAD; p+=4;
    eph.crc   =R4(p);        p+=4;
    eph.crs   =R4(p);        p+=4;
    eph.cuc   =R4(p);        p+=4;
    eph.cus   =R4(p);        p+=4;
    eph.cic   =R4(p);        p+=4;
    eph.cis   =R4(p);        p+=4;
    eph.A     =sqrtA*sqrtA;
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%3d iode=%3d iodc=%3d toes=%6.0f",prn,eph.iode,
                eph.iodc,eph.toes);
    }
    if (sys==SYS_GPS||sys==SYS_QZS||sys==SYS_IRN) {
        if (!(eph.sat=satno(sys,prn))) {
            trace(2,"javad ephemeris satellite error: sys=%d prn=%d\n",sys,prn);
            return -1;
        }
        eph.flag=(flag>>1)&1;
        eph.code=(flag>>2)&3;
        eph.fit =flag&1;
        eph.week=adjgpsweek(week);
        eph.toe=gpst2time(eph.week,eph.toes);
        
        /* for week-handover problem */
        tt=timediff(eph.toe,raw->time);
        if      (tt<-302400.0) eph.week++;
        else if (tt> 302400.0) eph.week--;
        eph.toe=gpst2time(eph.week,eph.toes);
        
        eph.toc=gpst2time(eph.week,toc);
        eph.ttr=adjweek(eph.toe,tow);
    }
    else if (sys==SYS_GAL) {
        if (!(eph.sat=satno(sys,prn))) {
            trace(2,"javad ephemeris satellite error: sys=%d prn=%d\n",sys,prn);
            return -1;
        }
        eph.tgd[0]=R4(p); p+=4;    /* BGD: E1-E5A (s) */
        eph.tgd[1]=R4(p); p+=4+13; /* BGD: E1-E5B (s) */
        navtype   =U1(p);          /* navtype: 0:E1B(INAV),1:E5A(FNAV) */
                                   /*          3:GIOVE E1B,4:GIOVE E5A */
        
        /* set data source defined in rinex 3.03 */
        eph.code=(navtype==0)?((1<<0)|(1<<9)):((navtype==1)?((1<<1)|(1<<8)):0);
        
        if (eph_sel==1&&!(eph.code&(1<<9))) return 0; /* only I/NAV */
        if (eph_sel==2&&!(eph.code&(1<<8))) return 0; /* only F/NAV */
        
        /* gal-week = gst-week + 1024 */
        eph.week=week+1024;
        eph.toe=gpst2time(eph.week,eph.toes);
        
        /* for week-handover problem */
        tt=timediff(eph.toe,raw->time);
        if      (tt<-302400.0) eph.week++;
        else if (tt> 302400.0) eph.week--;
        eph.toe=gpst2time(eph.week,eph.toes);
        
        eph.toc=gpst2time(eph.week,toc);
        eph.ttr=adjweek(eph.toe,tow);
    }
    else if (sys==SYS_CMP) {
        if (!(eph.sat=satno(sys,prn))) {
            trace(2,"javad ephemeris satellite error: sys=%d prn=%d\n",sys,prn);
            return -1;
        }
        eph.tgd[1]=R4(p); p+=4;    /* TGD2 (s) */
        sigtype   =U1(p);          /* signal type: 0:B1,1:B2,2:B3 */
        eph.code=(sigtype==0)?1:((sigtype==1)?3:((sigtype==2)?5:0));
        eph.week=week;
        eph.toe=bdt2time(week,eph.toes); /* bdt -> gpst */
        eph.toc=bdt2time(week,toc);      /* bdt -> gpst */
        eph.ttr=adjweek(eph.toe,tow);
    }
    else return 0;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(raw->nav.eph[eph.sat-1].toe,eph.toe)==0.0&&
            raw->nav.eph[eph.sat-1].iode==eph.iode&&
            raw->nav.eph[eph.sat-1].iodc==eph.iodc) return 0; /* unchanged */
    }
    raw->nav.eph[eph.sat-1]=eph;
    raw->ephsat=eph.sat;
    return 2;
}
/* decode [GE] gps ephemeris -------------------------------------------------*/
static int decode_GE(raw_t *raw)
{
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad GE checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<128) {
        trace(2,"javad GE length error: len=%d\n",raw->len);
        return -1;
    }
    return decode_eph(raw,SYS_GPS);
}
/* decode [NE] glonass ephemeris ---------------------------------------------*/
static int decode_NE(raw_t *raw)
{
    geph_t geph={0};
    double tt;
    char *msg;
    int prn,tk,tb;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad NE checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len>=85) { /* firmware v 2.6.0 [2] */
        prn        =U1(p);     p+=1;
        geph.frq   =I1(p);     p+=1+2;
        tk         =I4(p);     p+=4;
        tb         =I4(p);     p+=4;
        geph.svh   =U1(p)&0x7; p+=1;
        geph.age   =U1(p);     p+=1+1;
        geph.pos[0]=R8(p)*1E3; p+=8;
        geph.pos[1]=R8(p)*1E3; p+=8;
        geph.pos[2]=R8(p)*1E3; p+=8;
        geph.vel[0]=R4(p)*1E3; p+=4;
        geph.vel[1]=R4(p)*1E3; p+=4;
        geph.vel[2]=R4(p)*1E3; p+=4;
        geph.acc[0]=R4(p)*1E3; p+=4;
        geph.acc[1]=R4(p)*1E3; p+=4;
        geph.acc[2]=R4(p)*1E3; p+=4+8;
        geph.taun  =R4(p);     p+=4;
        geph.gamn  =R4(p);     p+=4;
    }
    else {
        trace(2,"javad NE length error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len>=93) { /* firmware v 3.2.0 [1] */
        geph.dtaun =R4(p); p+=4;
        geph.sva   =U1(p);
    }
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%2d frq=%2d tk=%6d tb=%4d",prn,geph.frq,tk,tb);
    }
    if (!(geph.sat=satno(SYS_GLO,prn))) {
        trace(2,"javad NE satellite error: prn=%d\n",prn);
        return 0;
    }
    if (raw->time.time==0) return 0;
    geph.iode=(tb/900)&0x7F;
    geph.toe=utc2gpst(adjday(raw->time,tb-10800.0));
    geph.tof=utc2gpst(adjday(raw->time,tk-10800.0));
    
    /* check illegal ephemeris by toe */
    tt=timediff(raw->time,geph.toe);
    if (fabs(tt)>3600.0) {
        trace(3,"javad NE illegal toe: prn=%2d tt=%6.0f\n",prn,tt);
        return 0;
    }
    /* check illegal ephemeris by frequency number consistency */
    if (raw->nav.geph[prn-MINPRNGLO].toe.time&&
        geph.frq!=raw->nav.geph[prn-MINPRNGLO].frq) {
        trace(2,"javad NE illegal freq change: prn=%2d frq=%2d->%2d\n",prn,
              raw->nav.geph[prn-MINPRNGLO].frq,geph.frq);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(geph.toe,raw->nav.geph[prn-MINPRNGLO].toe))<1.0&&
            geph.svh==raw->nav.geph[prn-MINPRNGLO].svh) return 0; /* unchanged */
    }
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=geph.sat;
    return 2;
}
/* decode [EN] galileo ephemeris ---------------------------------------------*/
static int decode_EN(raw_t *raw)
{
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad EN checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<150) {
        trace(2,"javad EN length error: len=%d\n",raw->len);
        return -1;
    }
    return decode_eph(raw,SYS_GAL);
}
/* decode [WE] sbas ephemeris ------------------------------------------------*/
static int decode_WE(raw_t *raw)
{
    seph_t seph={0};
    unsigned int tod,tow;
    char *msg;
    int i,prn,week;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad WE checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<44) {
        trace(2,"javad WE length error: len=%d\n",raw->len);
        return -1;
    }
    prn     =U1(p); p+=1+1+1;
    seph.sva=U1(p); p+=1;
    tod     =U4(p); p+=4;
    for (i=0;i<3;i++) {seph.pos[i]=R8(p); p+=8;}
    for (i=0;i<3;i++) {seph.vel[i]=R4(p); p+=4;}
    for (i=0;i<3;i++) {seph.acc[i]=R4(p); p+=4;}
    seph.af0 =R4(p); p+=4;
    seph.af1 =R4(p); p+=4;
    tow      =U4(p); p+=4;
    week     =U2(p);
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%3d tod=%6d",prn,tod);
    }
    if (!(seph.sat=satno(SYS_SBS,prn))) {
        trace(2,"javad WE satellite error: prn=%d\n",prn);
        return -1;
    }
    seph.tof=gpst2time(adjgpsweek(week),tow);
    seph.t0=adjday(seph.tof,tod);
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(seph.t0,raw->nav.seph[prn-MINPRNSBS].t0))<1.0&&
            seph.sva==raw->nav.seph[prn-MINPRNSBS].sva) return 0; /* unchanged */
    }
    raw->nav.seph[prn-MINPRNSBS]=seph;
    raw->ephsat=seph.sat;
    return 2;
}
/* decode [QE] qzss ephemeris ------------------------------------------------*/
static int decode_QE(raw_t *raw)
{
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad QE checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<128) {
        trace(2,"javad QE length error: len=%d\n",raw->len);
        return -1;
    }
    return decode_eph(raw,SYS_QZS);
}
/* decode [CN] beidou ephemeris ----------------------------------------------*/
static int decode_CN(raw_t *raw)
{
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad CN checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<133) {
        trace(2,"javad QE length error: len=%d\n",raw->len);
        return -1;
    }
    return decode_eph(raw,SYS_CMP);
}
/* decode [IE] irnss ephemeris -----------------------------------------------*/
static int decode_IE(raw_t *raw)
{
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad IE checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<129) {
        trace(2,"javad IE length error: len=%d\n",raw->len);
        return -1;
    }
    return decode_eph(raw,SYS_IRN);
}
/* decode [UO] gps utc time parameters ---------------------------------------*/
static int decode_UO(raw_t *raw)
{
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad UO checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<29) {
        trace(2,"javad UO length error: len=%d\n",raw->len);
        return -1;
    }
    raw->nav.utc_gps[0]=R8(p); p+=8;
    raw->nav.utc_gps[1]=R4(p); p+=4;
    raw->nav.utc_gps[2]=U4(p); p+=4;
    raw->nav.utc_gps[3]=adjgpsweek((int)U2(p)); p+=2;
    raw->nav.leaps     =I1(p);
    return 9;
}
/* decode [NU] glonass utc and gps time parameters ---------------------------*/
static int decode_NU(raw_t *raw)
{
    trace(2,"javad NU not supported\n");
    
    return 0;
}
/* decode [EU] galileo utc and gps time parameters ---------------------------*/
static int decode_EU(raw_t *raw)
{
    trace(2,"javad EU not supported\n");
    
    return 0;
}
/* decode [WU] waas utc time parameters --------------------------------------*/
static int decode_WU(raw_t *raw)
{
    trace(2,"javad WU not supported\n");
    
    return 0;
}
/* decode [QU] qzss utc and gps time parameters ------------------------------*/
static int decode_QU(raw_t *raw)
{
    trace(2,"javad QU not supported\n");
    
    return 0;
}
/* decode [IO] ionospheric parameters ----------------------------------------*/
static int decode_IO(raw_t *raw)
{
    int i;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad IO checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<44) {
        trace(2,"javad IO length error: len=%d\n",raw->len);
        return -1;
    }
    p+=4+2;
    for (i=0;i<8;i++) {
        raw->nav.ion_gps[i]=R4(p); p+=4;
    }
    return 9;
}
/* decode L1 NAV data --------------------------------------------------------*/
static int decode_L1nav(unsigned char *buff, int len, int sat, raw_t *raw)
{
    eph_t eph={0};
    double ion[8]={0},utc[4]={0};
    unsigned char *subfrm,*p;
    unsigned int word;
    int i,j,sys,week,leaps=0,id=(U4((unsigned char*)buff+4)>>8)&7;
    
    if (id<1||5<id) {
        trace(2,"navigation subframe format error: id=%d\n",id);
        return 0;
    }
    subfrm=raw->subfrm[sat-1];
    
    for (i=0,p=subfrm+(id-1)*30;i<10;i++) {
        word=U4((unsigned char*)buff+i*4)>>6;
        for (j=16;j>=0;j-=8) {
            *p++=(word>>j)&0xFF;
        }
    }
    if (id==3) { /* ephemeris */
        eph.sat=sat;
        if (decode_frame(subfrm   ,&eph,NULL,NULL,NULL,NULL)!=1||
            decode_frame(subfrm+30,&eph,NULL,NULL,NULL,NULL)!=2||
            decode_frame(subfrm+60,&eph,NULL,NULL,NULL,NULL)!=3) {
            return 0;
        }
        if (!strstr(raw->opt,"-EPHALL")) {
            if (eph.iode==raw->nav.eph[sat-1].iode&&
                eph.iodc==raw->nav.eph[sat-1].iodc) return 0; /* unchanged */
        }
        raw->nav.eph[sat-1]=eph;
        raw->ephsat=sat;
        return 2;
    }
    if (id==4) { /* almanac or ion/utc parameters */
        if (decode_frame(subfrm+90,NULL,NULL,ion,utc,&leaps)!=4) {
            return 0;
        }
        if (norm(ion,8)==0.0||norm(utc,4)==0.0||raw->time.time==0) {
            return 0;
        }
        sys=satsys(sat,NULL);
        time2gpst(raw->time,&week);
        utc[3]+=floor((week-utc[3])/256.0+0.5)*256.0;
        
        if (sys==SYS_GPS) {
            for (i=0;i<8;i++) raw->nav.ion_gps[i]=ion[i];
            for (i=0;i<4;i++) raw->nav.utc_gps[i]=utc[i];
            raw->nav.leaps=leaps;
            return 9;
        }
        if (sys==SYS_QZS) {
            for (i=0;i<8;i++) raw->nav.ion_qzs[i]=ion[i];
            for (i=0;i<4;i++) raw->nav.utc_qzs[i]=utc[i];
            raw->nav.leaps=leaps;
            return 9;
        }
    }
    return 0;
}
/* decode raw L2C CNAV data --------------------------------------------------*/
static int decode_L2nav(unsigned char *buff, int len, int sat, raw_t *raw)
{
    unsigned char msg[1024]={0};
    int i,j,preamb,prn,msgid,tow,alert;
    
    trace(3,"decode_L2nav len=%2d sat=%2d L5 CNAV\n",len,sat);
    
    for (i=0;i<len;i++) for (j=0;j<4;j++) {
        msg[3-j+i*4]=buff[j+i*4];
    }
    i=0;
    preamb=getbitu(msg,i, 8); i+= 8;
    prn   =getbitu(msg,i, 6); i+= 6;
    msgid =getbitu(msg,i, 6); i+= 6;
    tow   =getbitu(msg,i,17); i+=17;
    alert =getbitu(msg,i, 1); i+= 1;
    
    if (preamb!=PREAMB_CNAV) {
        trace(2,"javad *d sat=%2d L2 CNAV preamble error preamb=%02X\n",preamb);
        return -1;
    }
    trace(3,"L2CNAV: sat=%2d prn=%2d msgid=%2d tow=%6d alert=%d\n",sat,prn,
          msgid,tow,alert);
    
    return 0;
}
/* decode raw L5 CNAV data ---------------------------------------------------*/
static int decode_L5nav(unsigned char *buff, int len, int sat, raw_t *raw)
{
    unsigned char msg[1024]={0};
    int i,j,preamb,prn,msgid,tow,alert;
    
    trace(3,"decode_L5nav len=%2d sat=%2d L5 CNAV\n",len,sat);
    
    for (i=0;i<len;i++) for (j=0;j<4;j++) {
        msg[3-j+i*4]=buff[j+i*4];
    }
    i=0;
    preamb=getbitu(msg,i, 8); i+= 8;
    prn   =getbitu(msg,i, 6); i+= 6;
    msgid =getbitu(msg,i, 6); i+= 6;
    tow   =getbitu(msg,i,17); i+=17;
    alert =getbitu(msg,i, 1); i+= 1;
    
    if (preamb!=PREAMB_CNAV) {
        trace(2,"javad *d sat=%2d L5 CNAV preamble error preamb=%02X\n",preamb);
        return -1;
    }
    trace(3,"L5CNAV: sat=%2d prn=%2d msgid=%2d tow=%6d alert=%d\n",sat,prn,
          msgid,tow,alert);
    
    return 0;
}
/* decode raw L1C CNAV2 data -------------------------------------------------*/
static int decode_L1Cnav(unsigned char *buff, int len, int sat, raw_t *raw)
{
    trace(2,"javad *d len=%2d sat=%2d L1C CNAV2 not supported\n",len,sat);
    
    return 0;
}
/* decode [*D] raw navigation data -------------------------------------------*/
static int decode_nD(raw_t *raw, int sys)
{
    int i,n,siz,sat,prn,stat=0;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad nD checksum error: sys=%d len=%d\n",sys,raw->len);
        return -1;
    }
    siz=U1(p); p+=1;
    n=(raw->len-7)/siz;
    
    if (n<=0) {
        trace(2,"javad nD length error: sys=%d len=%d\n",sys,raw->len);
        return -1;
    }
    for (i=0;i<n;i++,p+=siz) {
        trace(3,"decode_*D: sys=%2d prn=%3d\n",sys,U1(p));
        
        prn=U1(p);
        if (!(sat=satno(sys,prn))) {
            trace(2,"javad nD satellite error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        stat=decode_L1nav(p+2,0,sat,raw);
    }
    return stat;
}
/* decode [*d] raw navigation data -------------------------------------------*/
static int decode_nd(raw_t *raw, int sys)
{
    unsigned char *p=raw->buff+5;
    char *msg;
    int sat,prn,time,type,len;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad nd checksum error: sys=%d len=%d\n",sys,raw->len);
        return -1;
    }
    trace(3,"decode_*d: sys=%2d prn=%3d\n",sys,U1(p));
    
    prn =U1(p); p+=1;
    time=U4(p); p+=4;
    type=U1(p); p+=1;
    len =U1(p); p+=1;
    if (raw->len!=13+len*4) {
        trace(2,"javad nd length error: sys=%d len=%d\n",sys,raw->len);
        return -1;
    }
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%3d time=%7d type=%d",prn,time,type);
    }
    if (!(sat=satno(sys,prn))) {
        trace(2,"javad nd satellite error: sys=%d prn=%d\n",sys,prn);
        return 0;
    }
    trace(4,"sat=%2d time=%7d type=%d len=%3d\n",sat,time,type,len);
    
    switch (type) {
        case 0: return decode_L1nav (p,len,sat,raw); /* L1  NAV */
        case 1: return decode_L2nav (p,len,sat,raw); /* L2C CNAV */
        case 2: return decode_L5nav (p,len,sat,raw); /* L5  CNAV */
        case 3: return decode_L1Cnav(p,len,sat,raw); /* L1C CNAV2 */
        case 4: break;
    }
    return 0;
}
/* decode [LD] glonass raw navigation data -----------------------------------*/
static int decode_LD(raw_t *raw)
{
    trace(2,"javad LD not supported\n");
    
    return 0;
}
/* decode [lD] glonass raw navigation data -----------------------------------*/
static int decode_lD(raw_t *raw)
{
    geph_t geph={0};
    unsigned char *p=raw->buff+5;
    char *msg;
    int i,sat,prn,frq,time,type,len,id;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad lD checksum error: len=%d\n",raw->len);
        return -1;
    }
    trace(3,"decode_lD: prn=%3d\n",U1(p));
    
    prn =U1(p); p+=1;
    frq =I1(p); p+=1;
    time=U4(p); p+=4;
    type=U1(p); p+=1;
    len =U1(p); p+=1;
    
    if (raw->len!=14+len*4) {
        trace(2,"javad lD length error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%2d frq=%2d time=%7d type=%d",prn,frq,time,type);
    }
    if (!(sat=satno(SYS_GLO,prn))) {
        trace(2,"javad lD satellite error: prn=%d\n",prn);
        return 0;
    }
    if (type!=0) {
        trace(3,"javad lD type not supported: type=%d\n",type);
        return 0;
    }
    if ((id=(U4(p)>>20)&0xF)<1) return 0;
    
    /* get 77 bit (25x3+2) in frame without hamming and time mark */
    for (i=0;i<4;i++) {
        setbitu(raw->subfrm[sat-1]+(id-1)*10,i*25,i<3?25:2,
                U4(p+4*i)>>(i<3?0:23));
    }
    if (id!=4) return 0;
    
    /* decode glonass ephemeris strings */
    geph.tof=raw->time;
    if (!decode_glostr(raw->subfrm[sat-1],&geph)||geph.sat!=sat) return -1;
    geph.frq=frq;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode) return 0; /* unchanged */
    }
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    return 2;
}
/* decode [ED] galileo raw navigation data -----------------------------------*/
static int decode_ED(raw_t *raw)
{
    trace(2,"javad ED not supported\n");
    
    return 0;
}
/* decode [cd] beidou raw navigation data ------------------------------------*/
static int decode_cd(raw_t *raw)
{
    trace(2,"javad cd not supported\n");
    
    return 0;
}
/* decode [id] irnss raw navigation data -------------------------------------*/
static int decode_id(raw_t *raw)
{
    trace(2,"javad id not supported\n");
    
    return 0;
}
/* decode [WD] sbas raw navigation data --------------------------------------*/
static int decode_WD(raw_t *raw)
{
    int i,prn,tow,tow_p,week;
    char *msg;
    unsigned char *p=raw->buff+5;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad WD checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len<45) {
        trace(2,"javad WD length error: len=%d\n",raw->len);
        return -1;
    }
    trace(3,"decode_WD: prn=%3d\n",U1(p));
    
    prn=U1(p); p+=1;
    tow=U4(p); p+=4+2;
    
    if (raw->outtype) {
        msg=raw->msgtype+strlen(raw->msgtype);
        sprintf(msg," prn=%3d tow=%6d",prn,tow);
    }
    if ((prn<MINPRNSBS||MAXPRNSBS<prn)&&(prn<MINPRNQZS||MAXPRNQZS<prn)) {
        trace(2,"javad WD satellite error: prn=%d\n",prn);
        return 0;
    }
    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=tow;
    
    if (raw->time.time==0) {
        raw->sbsmsg.week=0;
    }
    else {
        tow_p=(int)time2gpst(raw->time,&week);
        if      (tow<tow_p-302400.0) week++;
        else if (tow>tow_p+302400.0) week--;
        raw->sbsmsg.week=week;
    }
    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=*p++;
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode [R*] pseudoranges --------------------------------------------------*/
static int decode_Rx(raw_t *raw, char code)
{
    double pr,prm;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad R%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*8+6) {
        trace(2,"javad R%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        pr=R8(p); p+=8; if (pr==0.0) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))) continue;
        
        prm=pr*CLIGHT;
        
        if (code=='C') raw->prCA[sat-1]=prm;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].P[j]=prm;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [r*] short pseudoranges --------------------------------------------*/
static int decode_rx(raw_t *raw, char code)
{
    double prm;
    int i,j,pr,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad r%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad r%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        pr=I4(p); p+=4;
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))) continue;
        
        if (pr==0x7FFFFFFF) {
            trace(2,"javad r%c value missing: sat=%2d\n",code,sat);
            continue;
        }
        /*                             Ksys  Asys */
        if      (sys==SYS_SBS) prm=(pr*1E-11+0.125)*CLIGHT; /* [6] */
        else if (sys==SYS_QZS) prm=(pr*2E-11+0.125)*CLIGHT; /* [3] */
        else if (sys==SYS_CMP) prm=(pr*2E-11+0.105)*CLIGHT; /* [4] */
        else if (sys==SYS_GAL) prm=(pr*2E-11+0.085)*CLIGHT; /* [7] */
        else if (sys==SYS_IRN) prm=(pr*2E-11+0.105)*CLIGHT; /* [6] */
        else                   prm=(pr*1E-11+0.075)*CLIGHT;
        
        if (code=='c') raw->prCA[sat-1]=prm;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].P[j]=prm;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [*R] relative pseudoranges -----------------------------------------*/
static int decode_xR(raw_t *raw, char code)
{
    float pr;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cR checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad %cR length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        pr=R4(p); p+=4; if (pr==0.0) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))||raw->prCA[sat-1]==0.0) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].P[j]=pr*CLIGHT+raw->prCA[sat-1];
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [*r] short relative pseudoranges -----------------------------------*/
static int decode_xr(raw_t *raw, char code)
{
    double prm;
    short pr;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cr checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*2+6) {
        trace(2,"javad %cR length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        pr=I2(p); p+=2; if (pr==(short)0x7FFF) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))||raw->prCA[sat-1]==0.0) continue;
        
        prm=(pr*1E-11+2E-7)*CLIGHT+raw->prCA[sat-1];
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].P[j]=prm;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [P*] carrier phases ------------------------------------------------*/
static int decode_Px(raw_t *raw, char code)
{
    double cp;
    int i,j,freq,type,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad P%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*8+6) {
        trace(2,"javad P%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        cp=R8(p); p+=8; if (cp==0.0) continue;
        
        if (!(sys=satsys(raw->obuf.data[i].sat,NULL))) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].L[j]=cp;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [p*] short carrier phases ------------------------------------------*/
static int decode_px(raw_t *raw, char code)
{
    unsigned int cp;
    int i,j,freq,type,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad p%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad p%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        cp=U4(p); p+=4; if (cp==0xFFFFFFFF) continue;
        
        if (!(sys=satsys(raw->obuf.data[i].sat,NULL))) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].L[j]=cp/1024.0;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [*P] short relative carrier phases ---------------------------------*/
static int decode_xP(raw_t *raw, char code)
{
    double cp,rcp,fn;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cP checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad %cP length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        rcp=R4(p); p+=4; if (rcp==0.0) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))||raw->prCA[sat-1]==0.0) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            
            fn=freq_sys(sys,freq,raw->freqn[i]);
            cp=(rcp+raw->prCA[sat-1]/CLIGHT)*fn;
            
            raw->obuf.data[i].L[j]=cp;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [*p] short relative carrier phases ---------------------------------*/
static int decode_xp(raw_t *raw, char code)
{
    double cp,fn;
    int i,j,rcp,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cp checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad %cp length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        rcp=I4(p); p+=4; if (rcp==0x7FFFFFFF) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))||raw->prCA[sat-1]==0.0) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            
            fn=freq_sys(sys,freq,raw->freqn[i]);
            cp=(rcp*P2_40+raw->prCA[sat-1]/CLIGHT)*fn;
            
            raw->obuf.data[i].L[j]=cp;
            raw->obuf.data[i].code[j]=type;
        }
    }
    return 0;
}
/* decode [D*] doppler -------------------------------------------------------*/
static int decode_Dx(raw_t *raw, char code)
{
    double dop;
    int i,j,dp,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad D%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*4+6) {
        trace(2,"javad D%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        dp=I4(p); p+=4; if (dp==0x7FFFFFFF) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))) continue;
        
        dop=-dp*1E-4;
        
        if (code=='C') raw->dpCA[sat-1]=dop;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].D[j]=(float)dop;
        }
    }
    return 0;
}
/* decode [*d] short relative doppler ----------------------------------------*/
static int decode_xd(raw_t *raw, char code)
{
    double dop,f1,fn;
    short rdp;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cd checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*2+6) {
        trace(2,"javad %cd length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        rdp=I2(p); p+=2; if (rdp==(short)0x7FFF) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))||raw->dpCA[sat-1]==0.0) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            f1=freq_sys(sys,0   ,raw->freqn[i]);
            fn=freq_sys(sys,freq,raw->freqn[i]);
            dop=(-rdp+raw->dpCA[sat-1]*1E4)*fn/f1*1E-4;
            
            raw->obuf.data[i].D[j]=(float)dop;
        }
    }
    return 0;
}
/* decode [E*] carrier to noise ratio ----------------------------------------*/
static int decode_Ex(raw_t *raw, char code)
{
    unsigned char cnr;
    int i,j,freq,type,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad E%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n+6) {
        trace(2,"javad E%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        cnr=U1(p); p+=1; if (cnr==255) continue;
        
        if (!(sys=satsys(raw->obuf.data[i].sat,NULL))) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].SNR[j]=(unsigned char)(cnr*4.0+0.5);
        }
    }
    return 0;
}
/* decode [*E] carrier to noise ratio x 4 ------------------------------------*/
static int decode_xE(raw_t *raw, char code)
{
    unsigned char cnr;
    int i,j,freq,type,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad %cE checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n+6) {
        trace(2,"javad %cE length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        cnr=U1(p); p+=1; if (cnr==255) continue;
        
        if (!(sys=satsys(raw->obuf.data[i].sat,NULL))) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
            raw->obuf.data[i].SNR[j]=cnr;
        }
    }
    return 0;
}
/* decode [F*] signal lock loop flags ----------------------------------------*/
static int decode_Fx(raw_t *raw, char code)
{
    unsigned short flags;
    int i,j,freq,type,sat,sys;
    unsigned char *p=raw->buff+5;
    
    if (!is_meas(code)||raw->tod<0||raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad F%c checksum error: len=%d\n",code,raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*2+6) {
        trace(2,"javad F%c length error: n=%d len=%d\n",code,raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        flags=U2(p); p+=1; if (flags==0xFFFF) continue;
        
        sat=raw->obuf.data[i].sat;
        if (!(sys=satsys(sat,NULL))) continue;
        
        if ((freq=tofreq(code,sys,&type))<0) continue;
        
        if ((j=checkpri(raw->opt,sys,type,freq))>=0) {
            if (!settag(raw->obuf.data+i,raw->time)) continue;
#if 0
            if (flags&0x20) { /* loss-of-lock potential */
                raw->obuf.data[i].LLI[j]|=1;
            }
            if (!(flags&0x40)||!(flags&0x100)) { /* integral indicator */
                raw->obuf.data[i].LLI[j]|=2;
            }
#endif
        }
    }
    return 0;
}
/* decode [TC] CA/L1 continuous tracking time --------------------------------*/
static int decode_TC(raw_t *raw)
{
    unsigned short tt,tt_p;
    int i,sat;
    unsigned char *p=raw->buff+5;
    
    if (raw->obuf.n==0) return 0;
    
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"javad TC checksum error: len=%d\n",raw->len);
        return -1;
    }
    if (raw->len!=raw->obuf.n*2+6) {
        trace(2,"javad TC length error: n=%d len=%d\n",raw->obuf.n,raw->len);
        return -1;
    }
    for (i=0;i<raw->obuf.n&&i<MAXOBS;i++) {
        tt=U2(p); p+=2; if (tt==0xFFFF) continue;
        
        if (!settag(raw->obuf.data+i,raw->time)) continue;
        
        sat=raw->obuf.data[i].sat;
        tt_p=(unsigned short)raw->lockt[sat-1][0];
        
        trace(4,"%s: sat=%2d tt=%6d->%6d\n",time_str(raw->time,3),sat,tt_p,tt);
        
        /* loss-of-lock detected by lock-time counter */
        if (tt==0||tt<tt_p) {
            trace(3,"decode_TC: loss-of-lock detected: t=%s sat=%2d tt=%6d->%6d\n",
                  time_str(raw->time,3),sat,tt_p,tt);
            raw->obuf.data[i].LLI[0]|=1;
        }
        raw->lockt[sat-1][0]=tt;
    }
    return 0;
}
/* decode javad raw message --------------------------------------------------*/
static int decode_javad(raw_t *raw)
{
    char *p=(char *)raw->buff;
    
    trace(3,"decode_javad: type=%2.2s len=%3d\n",p,raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"JAVAD %2.2s (%4d)",p,raw->len);
    }
    if (!strncmp(p,"~~",2)) return decode_RT(raw); /* receiver time */
    
    if (strstr(raw->opt,"-NOET")) {
        if (!strncmp(p,"::",2)) return decode_ET(raw); /* epoch time */
    }
    if (!strncmp(p,"RD",2)) return decode_RD(raw); /* receiver date */
    if (!strncmp(p,"SI",2)) return decode_SI(raw); /* satellite indices */
    if (!strncmp(p,"NN",2)) return decode_NN(raw); /* glonass slot numbers */
    
    if (!strncmp(p,"GA",2)) return decode_GA(raw); /* gps almanac */
    if (!strncmp(p,"NA",2)) return decode_NA(raw); /* glonass almanac */
    if (!strncmp(p,"EA",2)) return decode_EA(raw); /* galileo almanac */
    if (!strncmp(p,"WA",2)) return decode_WA(raw); /* sbas almanac */
    if (!strncmp(p,"QA",2)) return decode_QA(raw); /* qzss almanac */
    if (!strncmp(p,"CA",2)) return decode_CA(raw); /* beidou almanac */
    if (!strncmp(p,"IA",2)) return decode_IA(raw); /* irnss almanac */
    
    if (!strncmp(p,"GE",2)) return decode_GE(raw); /* gps ephemeris */
    if (!strncmp(p,"NE",2)) return decode_NE(raw); /* glonass ephemeris */
    if (!strncmp(p,"EN",2)) return decode_EN(raw); /* galileo ephemeris */
    if (!strncmp(p,"WE",2)) return decode_WE(raw); /* sbas ephemeris */
    if (!strncmp(p,"QE",2)) return decode_QE(raw); /* qzss ephemeris */
    if (!strncmp(p,"CN",2)) return decode_CN(raw); /* beidou ephemeris */
    if (!strncmp(p,"IE",2)) return decode_IE(raw); /* irnss ephemeris */
    
    if (!strncmp(p,"UO",2)) return decode_UO(raw); /* gps utc time parameters */
    if (!strncmp(p,"NU",2)) return decode_NU(raw); /* glonass utc and gps time par */
    if (!strncmp(p,"EU",2)) return decode_EU(raw); /* galileo utc and gps time par */
    if (!strncmp(p,"WU",2)) return decode_WU(raw); /* waas utc time parameters */
    if (!strncmp(p,"QU",2)) return decode_QU(raw); /* qzss utc and gps time par */
    if (!strncmp(p,"IO",2)) return decode_IO(raw); /* ionospheric parameters */
    
    if (!strncmp(p,"GD",2)) return decode_nD(raw,SYS_GPS); /* raw navigation data */
    if (!strncmp(p,"QD",2)) return decode_nD(raw,SYS_QZS); /* raw navigation data */
    if (!strncmp(p,"gd",2)) return decode_nd(raw,SYS_GPS); /* raw navigation data */
    if (!strncmp(p,"qd",2)) return decode_nd(raw,SYS_QZS); /* raw navigation data */
    if (!strncmp(p,"ED",2)) return decode_ED(raw); /* galileo raw navigation data */
    if (!strncmp(p,"cd",2)) return decode_cd(raw); /* beidou raw navigation data */
    if (!strncmp(p,"id",2)) return decode_id(raw); /* irnss raw navigation data */
    if (!strncmp(p,"LD",2)) return decode_LD(raw); /* glonass raw navigation data */
    if (!strncmp(p,"lD",2)) return decode_lD(raw); /* glonass raw navigation data */
    if (!strncmp(p,"WD",2)) return decode_WD(raw); /* sbas raw navigation data */
    
    if (!strncmp(p,"TC",2)) return decode_TC(raw); /* CA/L1 continuous track time */
    
    if (p[0]=='R') return decode_Rx(raw,p[1]); /* pseudoranges */
    if (p[0]=='r') return decode_rx(raw,p[1]); /* short pseudoranges */
    if (p[1]=='R') return decode_xR(raw,p[0]); /* relative pseudoranges */
    if (p[1]=='r') return decode_xr(raw,p[0]); /* short relative pseudoranges */
    if (p[0]=='P') return decode_Px(raw,p[1]); /* carrier phases */
    if (p[0]=='p') return decode_px(raw,p[1]); /* short carrier phases */
    if (p[1]=='P') return decode_xP(raw,p[0]); /* relative carrier phases */
    if (p[1]=='p') return decode_xp(raw,p[0]); /* relative carrier phases */
    if (p[0]=='D') return decode_Dx(raw,p[1]); /* doppler */
    if (p[1]=='d') return decode_xd(raw,p[0]); /* short relative doppler */
    if (p[0]=='E') return decode_Ex(raw,p[1]); /* carrier to noise ratio */
    if (p[1]=='E') return decode_xE(raw,p[0]); /* carrier to noise ratio x 4 */
    if (p[0]=='F') return decode_Fx(raw,p[1]); /* signal lock loop flags */
    
    return 0;
}
/* sync javad message --------------------------------------------------------*/
static int sync_javad(unsigned char *buff, unsigned char data)
{
    unsigned char p=buff[0];
    
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=buff[3]; buff[3]=buff[4];
    buff[4]=data;
    
    /* sync message header {\r|\n}IIHHH (II:id,HHH: hex length) */
    return (p=='\r'||p=='\n')&&ISTXT(buff[0])&&ISTXT(buff[1])&&
           ISHEX(buff[2])&&ISHEX(buff[3])&&ISHEX(buff[4]);
}
/* clear buffer --------------------------------------------------------------*/
static void clearbuff(raw_t *raw)
{
    int i;
    for (i=0;i<5;i++) raw->buff[i]=0;
    raw->len=raw->nbyte=0;
}
/* input javad raw message from stream -----------------------------------------
* fetch next javad raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL : input all ephemerides
*          -GL1W   : select 1W for GPS L1 (default 1C)
*          -GL1X   : select 1X for GPS L1 (default 1C)
*          -GL2X   : select 2X for GPS L2 (default 2W)
*          -RL1C   : select 1C for GLO L1 (default 1P)
*          -RL2C   : select 2C for GLO L2 (default 2P)
*          -JL1Z   : select 1Z for QZS L1 (default 1C)
*          -JL1X   : select 1X for QZS L1 (default 1C)
*          -NOET   : discard epoch time message ET (::)
*          -GALINAV: input only I/NAV for galileo ephemeris
*          -GALFNAV: input only F/NAV for galileo ephemeris
*
*-----------------------------------------------------------------------------*/
extern int input_javad(raw_t *raw, unsigned char data)
{
    int len,stat;
    
    trace(5,"input_javad: data=%02x\n",data);
    
    /* synchronize message */
    if (raw->nbyte==0) {
        if (!sync_javad(raw->buff,data)) return 0;
        if (!(len=decodelen(raw->buff+2))||len>MAXRAWLEN-5) {
            trace(2,"javad message length error: len=%d\n",len);
            clearbuff(raw);
            return -1;
        }
        raw->len=len+5;
        raw->nbyte=5;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte<raw->len) return 0;
    
    /* decode javad raw message */
    stat=decode_javad(raw);
    
    clearbuff(raw);
    return stat;
}
/* start input file ----------------------------------------------------------*/
static void startfile(raw_t *raw)
{
    raw->tod=-1;
    raw->obuf.n=0;
    raw->buff[4]='\n';
}
/* end input file ------------------------------------------------------------*/
static int endfile(raw_t *raw)
{
    /* flush observation data buffer */
    if (!flushobuf(raw)) return -2;
    raw->obuf.n=0;
    return 1;
}
/* input javad raw message from file -------------------------------------------
* fetch next javad raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_javadf(raw_t *raw, FILE *fp)
{
    int i,data,len,stat;
    
    trace(4,"input_javadf:\n");
    
    /* start input file */
    if (raw->flag) {
        startfile(raw);
        raw->flag=0;
    }
    /* synchronize message */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return endfile(raw);
            if (sync_javad(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (!(len=decodelen(raw->buff+2))||len>MAXRAWLEN-5) {
        trace(2,"javad message length error: len=%3.3s\n",raw->buff+2);
        clearbuff(raw);
        return -1;
    }
    raw->len=len+5;
    raw->nbyte=5;
    
    if (fread(raw->buff+5,1,raw->len-5,fp)<(size_t)(raw->len-5)) {
        return endfile(raw);
    }
    /* decode javad raw message */
    stat=decode_javad(raw);
    
    clearbuff(raw);
    return stat;
}
