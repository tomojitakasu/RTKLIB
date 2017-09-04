/*------------------------------------------------------------------------------
* nvs.c : NVS receiver dependent functions
*
*    Copyright (C) 2012-2016 by M.BAVARO and T.TAKASU, All rights reserved.
*    Copyright (C) 2014 by T.TAKASU, All rights reserved.
*
*     [1] Description of BINR messages which is used by RC program for RINEX
*         files accumulation, NVS
*     [2] NAVIS Navis Standard Interface Protocol BINR, NVS
*
* version : $Revision:$ $Date:$
* history : 2012/01/30 1.0  first version by M.BAVARO
*           2012/11/08 1.1  modified by T.TAKASU
*           2013/02/23 1.2  fix memory access violation problem on arm
*           2013/04/24 1.3  fix bug on cycle-slip detection
*                           add range check of gps ephemeris week
*           2013/09/01 1.4  add check error of week, time jump, obs data range
*           2014/08/26 1.5  fix bug on iode in glonass ephemeris
*           2016/01/26 1.6  fix bug on unrecognized meas data (#130)
*           2017/04/11 1.7  (char *) -> (signed char *)
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define NVSSYNC     0x10        /* nvs message sync code 1 */
#define NVSENDMSG   0x03        /* nvs message sync code 1 */
#define NVSCFG      0x06        /* nvs message cfg-??? */

#define ID_XF5RAW   0xf5        /* nvs msg id: raw measurement data */
#define ID_X4AIONO  0x4a        /* nvs msg id: gps ionospheric data */
#define ID_X4BTIME  0x4b        /* nvs msg id: GPS/GLONASS/UTC timescale data */
#define ID_XF7EPH   0xf7        /* nvs msg id: subframe buffer */
#define ID_XE5BIT   0xe5        /* nvs msg id: bit information */

#define ID_XD7ADVANCED 0xd7     /* */
#define ID_X02RATEPVT  0x02     /* */
#define ID_XF4RATERAW  0xf4     /* */
#define ID_XD7SMOOTH   0xd7     /* */
#define ID_XD5BIT      0xd5     /* */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((signed char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

/* ura values (ref [3] 20.3.3.3.1.1) -----------------------------------------*/
static const double ura_eph[]={
    2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
    3072.0,6144.0,0.0
};
/* ura value (m) to ura index ------------------------------------------------*/
static int uraindex(double value)
{
    int i;
    for (i=0;i<15;i++) if (ura_eph[i]>=value) break;
    return i;
}
/* decode NVS xf5-raw: raw measurement data ----------------------------------*/
static int decode_xf5raw(raw_t *raw)
{
    gtime_t time;
    double tadj=0.0,toff=0.0,tn;
    int dTowInt;
    double dTowUTC, dTowGPS, dTowFrac, L1, P1, D1;
    double gpsutcTimescale;
    unsigned char rcvTimeScaleCorr, sys, carrNo;
    int i,j,prn,sat,n=0,nsat,week;
    unsigned char *p=raw->buff+2;
    char *q,tstr[32],flag;
    
    trace(4,"decode_xf5raw: len=%d\n",raw->len);
    
    /* time tag adjustment option (-TADJ) */
    if ((q=strstr(raw->opt,"-tadj"))) {
        sscanf(q,"-TADJ=%lf",&tadj);
    }
    dTowUTC =R8(p);
    week = U2(p+8);
    gpsutcTimescale = R8(p+10);
    /* glonassutcTimescale = R8(p+18); */
    rcvTimeScaleCorr = I1(p+26);
    
    /* check gps week range */
    if (week>=4096) {
        trace(2,"nvs xf5raw obs week error: week=%d\n",week);
        return -1;
    }
    week=adjgpsweek(week);
    
    if ((raw->len - 31)%30) {
        
        /* Message length is not correct: there could be an error in the stream */
        trace(2,"nvs xf5raw len=%d seems not be correct\n",raw->len);
        return -1;
    }
    nsat = (raw->len - 31)/30;
    
    dTowGPS = dTowUTC + gpsutcTimescale;
    
    /* Tweak pseudoranges to allow Rinex to represent the NVS time of measure */
    dTowInt  = 10.0*floor((dTowGPS/10.0)+0.5);
    dTowFrac = dTowGPS - (double) dTowInt;
    time=gpst2time(week, dTowInt*0.001);
    
    /* time tag adjustment */
    if (tadj>0.0) {
        tn=time2gpst(time,&week)/tadj;
        toff=(tn-floor(tn+0.5))*tadj;
        time=timeadd(time,-toff);
    }
    /* check time tag jump and output warning */
    if (raw->time.time&&fabs(timediff(time,raw->time))>86400.0) {
        time2str(time,tstr,3);
        trace(2,"nvs xf5raw time tag jump warning: time=%s\n",tstr);
    }
    if (fabs(timediff(time,raw->time))<=1e-3) {
        time2str(time,tstr,3);
        trace(2,"nvs xf5raw time tag duplicated: time=%s\n",tstr);
        return 0;
    }
    for (i=0,p+=27;(i<nsat) && (n<MAXOBS); i++,p+=30) {
        raw->obs.data[n].time  = time;
        sys = (U1(p)==1)?SYS_GLO:((U1(p)==2)?SYS_GPS:((U1(p)==4)?SYS_SBS:SYS_NONE));
        prn = U1(p+1);
        if (sys == SYS_SBS) prn += 120; /* Correct this */
        if (!(sat=satno(sys,prn))) {
            trace(2,"nvs xf5raw satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        carrNo = I1(p+2);
        L1 = R8(p+ 4);
        P1 = R8(p+12);
        D1 = R8(p+20);
        
        /* check range error */
        if (L1<-1E10||L1>1E10||P1<-1E10||P1>1E10||D1<-1E5||D1>1E5) {
            trace(2,"nvs xf5raw obs range error: sat=%2d L1=%12.5e P1=%12.5e D1=%12.5e\n",
                  sat,L1,P1,D1);
            continue;
        }
        raw->obs.data[n].SNR[0]=(unsigned char)(I1(p+3)*4.0+0.5);
        if (sys==SYS_GLO) {
            raw->obs.data[n].L[0]  =  L1 - toff*(FREQ1_GLO+DFRQ1_GLO*carrNo);
        } else {
            raw->obs.data[n].L[0]  =  L1 - toff*FREQ1;
        }
        raw->obs.data[n].P[0]    = (P1-dTowFrac)*CLIGHT*0.001 - toff*CLIGHT; /* in ms, needs to be converted */
        raw->obs.data[n].D[0]    =  (float)D1;
        
        /* set LLI if meas flag 4 (carrier phase present) off -> on */
        flag=U1(p+28);
        raw->obs.data[n].LLI[0]=(flag&0x08)&&!(raw->halfc[sat-1][0]&0x08)?1:0;
        raw->halfc[sat-1][0]=flag;
        
#if 0
        if (raw->obs.data[n].SNR[0] > 160) {
            time2str(time,tstr,3);
            trace(2,"%s, obs.data[%d]: SNR=%.3f  LLI=0x%02x\n",  tstr,
                n, (raw->obs.data[n].SNR[0])/4.0, U1(p+28) );
        }
#endif
        raw->obs.data[n].code[0] = CODE_L1C;
        raw->obs.data[n].sat = sat;
        
        for (j=1;j<NFREQ+NEXOBS;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_gpsephem(int sat, raw_t *raw)
{
    eph_t eph={0};
    unsigned char *puiTmp = (raw->buff)+2;
    unsigned short week;
    double toc;
    
    trace(4,"decode_ephem: sat=%2d\n",sat);
    
    eph.crs    = R4(&puiTmp[  2]);
    eph.deln   = R4(&puiTmp[  6]) * 1e+3;
    eph.M0     = R8(&puiTmp[ 10]);
    eph.cuc    = R4(&puiTmp[ 18]);
    eph.e      = R8(&puiTmp[ 22]);
    eph.cus    = R4(&puiTmp[ 30]);
    eph.A      = pow(R8(&puiTmp[ 34]), 2);
    eph.toes   = R8(&puiTmp[ 42]) * 1e-3;
    eph.cic    = R4(&puiTmp[ 50]);
    eph.OMG0   = R8(&puiTmp[ 54]);
    eph.cis    = R4(&puiTmp[ 62]);
    eph.i0     = R8(&puiTmp[ 66]);
    eph.crc    = R4(&puiTmp[ 74]);
    eph.omg    = R8(&puiTmp[ 78]);
    eph.OMGd   = R8(&puiTmp[ 86]) * 1e+3;
    eph.idot   = R8(&puiTmp[ 94]) * 1e+3;
    eph.tgd[0] = R4(&puiTmp[102]) * 1e-3;
    toc        = R8(&puiTmp[106]) * 1e-3;
    eph.f2     = R4(&puiTmp[114]) * 1e+3;
    eph.f1     = R4(&puiTmp[118]);
    eph.f0     = R4(&puiTmp[122]) * 1e-3;
    eph.sva    = uraindex(I2(&puiTmp[126]));
    eph.iode   = I2(&puiTmp[128]);
    eph.iodc   = I2(&puiTmp[130]);
    eph.code   = I2(&puiTmp[132]);
    eph.flag   = I2(&puiTmp[134]);
    week       = I2(&puiTmp[136]);
    eph.fit    = 0;
    
    if (week>=4096) {
        trace(2,"nvs gps ephemeris week error: sat=%2d week=%d\n",sat,week);
        return -1;
    }
    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
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
/* decode gloephem -----------------------------------------------------------*/
static int decode_gloephem(int sat, raw_t *raw)
{
    geph_t geph={0};
    unsigned char *p=(raw->buff)+2;
    int prn,tk,tb;
    
    if (raw->len>=93) {
        prn        =I1(p+ 1);
        geph.frq   =I1(p+ 2);
        geph.pos[0]=R8(p+ 3);
        geph.pos[1]=R8(p+11);
        geph.pos[2]=R8(p+19);
        geph.vel[0]=R8(p+27) * 1e+3;
        geph.vel[1]=R8(p+35) * 1e+3;
        geph.vel[2]=R8(p+43) * 1e+3;
        geph.acc[0]=R8(p+51) * 1e+6;
        geph.acc[1]=R8(p+59) * 1e+6;
        geph.acc[2]=R8(p+67) * 1e+6;
        tb = R8(p+75) * 1e-3;
        tk = tb;
        geph.gamn  =R4(p+83);
        geph.taun  =R4(p+87) * 1e-3;
        geph.age   =I2(p+91);
    }
    else {
        trace(2,"nvs NE length error: len=%d\n",raw->len);
        return -1;
    }
    if (!(geph.sat=satno(SYS_GLO,prn))) {
        trace(2,"nvs NE satellite error: prn=%d\n",prn);
        return -1;
    }
    if (raw->time.time==0) return 0;
    
    geph.iode=(tb/900)&0x7F;
    geph.toe=utc2gpst(adjday(raw->time,tb-10800.0));
    geph.tof=utc2gpst(adjday(raw->time,tk-10800.0));
#if 0
    /* check illegal ephemeris by toe */
    tt=timediff(raw->time,geph.toe);
    if (fabs(tt)>3600.0) {
        trace(3,"nvs NE illegal toe: prn=%2d tt=%6.0f\n",prn,tt);
        return 0;
    }
#endif
#if 0
    /* check illegal ephemeris by frequency number consistency */
    if (raw->nav.geph[prn-MINPRNGLO].toe.time&&
        geph.frq!=raw->nav.geph[prn-MINPRNGLO].frq) {
        trace(2,"nvs NE illegal freq change: prn=%2d frq=%2d->%2d\n",prn,
              raw->nav.geph[prn-MINPRNGLO].frq,geph.frq);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(geph.toe,raw->nav.geph[prn-MINPRNGLO].toe))<1.0&&
            geph.svh==raw->nav.geph[prn-MINPRNGLO].svh) return 0;
    }
#endif
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=geph.sat;
    
    return 2;
}
/* decode NVS epehemerides in clear ------------------------------------------*/
static int decode_xf7eph(raw_t *raw)
{
    int prn,sat,sys;
    unsigned char *p=raw->buff;
    
    trace(4,"decode_xf7eph: len=%d\n",raw->len);
    
    if ((raw->len)<93) {
        trace(2,"nvs xf7eph length error: len=%d\n",raw->len);
        return -1;
    }
    sys = (U1(p+2)==1)?SYS_GPS:((U1(p+2)==2)?SYS_GLO:SYS_NONE);
    prn = U1(p+3);
    if (!(sat=satno(sys==1?SYS_GPS:SYS_GLO,prn))) {
        trace(2,"nvs xf7eph satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (sys==SYS_GPS) {
        return decode_gpsephem(sat,raw);
    }
    else if (sys==SYS_GLO) {
        return decode_gloephem(sat,raw);
    }
    return 0;
}
/* decode NVS rxm-sfrb: subframe buffer --------------------------------------*/
static int decode_xe5bit(raw_t *raw)
{
    int prn;
    int iBlkStartIdx, iExpLen, iIdx;
    unsigned int words[10];
    unsigned char uiDataBlocks, uiDataType;
    unsigned char *p=raw->buff;
    
    trace(4,"decode_xe5bit: len=%d\n",raw->len);
    
    p += 2;         /* Discard preamble and message identifier */
    uiDataBlocks = U1(p);
    
    if (uiDataBlocks>=16) {
        trace(2,"nvs xf5bit message error: data blocks %u\n", uiDataBlocks);
        return -1;
    }
    iBlkStartIdx = 1;
    for (iIdx = 0; iIdx < uiDataBlocks; iIdx++) {
        iExpLen = (iBlkStartIdx+10);
        if ((raw->len) < iExpLen) {
            trace(2,"nvs xf5bit message too short (expected at least %d)\n", iExpLen);
            return -1;
        }
        uiDataType = U1(p+iBlkStartIdx+1);
        
        switch (uiDataType) {
            case 1: /* Glonass */
                iBlkStartIdx += 19;
                break;
            case 2: /* GPS */
                iBlkStartIdx += 47;
                break;
            case 4: /* SBAS */
                prn = U1(p+(iBlkStartIdx+2)) + 120;
                
                /* sat = satno(SYS_SBS, prn); */
                /* sys = satsys(sat,&prn); */
                memset(words, 0, 10*sizeof(unsigned int));
                for (iIdx=0, iBlkStartIdx+=7; iIdx<10; iIdx++, iBlkStartIdx+=4) {
                    words[iIdx]=U4(p+iBlkStartIdx);
                }
                words[7] >>= 6;
                return sbsdecodemsg(raw->time,prn,words,&raw->sbsmsg) ? 3 : 0;
            default:
                trace(2,"nvs xf5bit SNS type unknown (got %d)\n", uiDataType);
                return -1;
        }
    }
    return 0;
}
/* decode NVS x4aiono --------------------------------------------------------*/
static int decode_x4aiono(raw_t *raw)
{
    unsigned char *p=raw->buff+2;
    
    trace(4,"decode_x4aiono: len=%d\n", raw->len);
    
    raw->nav.ion_gps[0] = R4(p   );
    raw->nav.ion_gps[1] = R4(p+ 4);
    raw->nav.ion_gps[2] = R4(p+ 8);
    raw->nav.ion_gps[3] = R4(p+12);
    raw->nav.ion_gps[4] = R4(p+16);
    raw->nav.ion_gps[5] = R4(p+20);
    raw->nav.ion_gps[6] = R4(p+24);
    raw->nav.ion_gps[7] = R4(p+28);
    
    return 9;
}
/* decode NVS x4btime --------------------------------------------------------*/
static int decode_x4btime(raw_t *raw)
{
    unsigned char *p=raw->buff+2;
    
    trace(4,"decode_x4btime: len=%d\n", raw->len);
    
    raw->nav.utc_gps[1] = R8(p   );
    raw->nav.utc_gps[0] = R8(p+ 8);
    raw->nav.utc_gps[2] = I4(p+16);
    raw->nav.utc_gps[3] = I2(p+20);
    raw->nav.leaps = I1(p+22);
    
    return 9;
}
/* decode NVS raw message ----------------------------------------------------*/
static int decode_nvs(raw_t *raw)
{
    int type=U1(raw->buff+1);
    
    trace(3,"decode_nvs: type=%02x len=%d\n",type,raw->len);
    
    sprintf(raw->msgtype,"NVS: type=%2d len=%3d",type,raw->len);
    
    switch (type) {
        case ID_XF5RAW:  return decode_xf5raw (raw);
        case ID_XF7EPH:  return decode_xf7eph (raw);
        case ID_XE5BIT:  return decode_xe5bit (raw);
        case ID_X4AIONO: return decode_x4aiono(raw);
        case ID_X4BTIME: return decode_x4btime(raw);
        default: break;
    }
    return 0;
}
/* input NVS raw message from stream -------------------------------------------
* fetch next NVS raw data and input a message from stream
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
*          -TADJ=tint : adjust time tags to multiples of tint (sec)
*
*-----------------------------------------------------------------------------*/
extern int input_nvs(raw_t *raw, unsigned char data)
{
    trace(5,"input_nvs: data=%02x\n",data);
    
    /* synchronize frame */
    if ((raw->nbyte==0) && (data==NVSSYNC)) {
        
        /* Search a 0x10 */
        raw->buff[0] = data;
        raw->nbyte=1;
        return 0;
    }
    if ((raw->nbyte==1) && (data != NVSSYNC) && (data != NVSENDMSG)) {
        
        /* Discard double 0x10 and 0x10 0x03 at beginning of frame */
        raw->buff[1]=data;
        raw->nbyte=2;
        raw->flag=0;
        return 0;
    }
    /* This is all done to discard a double 0x10 */
    if (data==NVSSYNC) raw->flag = (raw->flag +1) % 2;
    if ((data!=NVSSYNC) || (raw->flag)) {
        
        /* Store the new byte */
        raw->buff[(raw->nbyte++)] = data;
    }
    /* Detect ending sequence */
    if ((data==NVSENDMSG) && (raw->flag)) {
        raw->len   = raw->nbyte;
        raw->nbyte = 0;
        
        /* Decode NVS raw message */
        return decode_nvs(raw);
    }
    if (raw->nbyte == MAXRAWLEN) {
        trace(2,"nvs message size error: len=%d\n",raw->nbyte);
        raw->nbyte=0;
        return -1;
    }
    return 0;
}
/* input NVS raw message from file ---------------------------------------------
* fetch next NVS raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_nvsf(raw_t *raw, FILE *fp)
{
    int i,data, odd=0;
    
    trace(4,"input_nvsf:\n");
    
    /* synchronize frame */
    for (i=0;;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        
        /* Search a 0x10 */
        if (data==NVSSYNC) {
            
            /* Store the frame begin */
            raw->buff[0] = data;
            if ((data=fgetc(fp))==EOF) return -2;
            
            /* Discard double 0x10 and 0x10 0x03 */
            if ((data != NVSSYNC) && (data != NVSENDMSG)) {
                raw->buff[1]=data;
                break;
            }
        }
        if (i>=4096) return 0;
    }
    raw->nbyte = 2;
    for (i=0;;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if (data==NVSSYNC) odd=(odd+1)%2;
        if ((data!=NVSSYNC) || odd) {
            
            /* Store the new byte */
            raw->buff[(raw->nbyte++)] = data;
        }
        /* Detect ending sequence */
        if ((data==NVSENDMSG) && odd) break;
        if (i>=4096) return 0;
    }
    raw->len = raw->nbyte;
    if ((raw->len) > MAXRAWLEN) {
        trace(2,"nvs length error: len=%d\n",raw->len);
        return -1;
    }
    /* decode nvs raw message */
    return decode_nvs(raw);
}
/* generate NVS binary message -------------------------------------------------
* generate NVS binary message from message string
* args   : char  *msg   I      message string
*            "RESTART  [arg...]" system reset
*            "CFG-SERI [arg...]" configure serial port property
*            "CFG-FMT  [arg...]" configure output message format
*            "CFG-RATE [arg...]" configure binary measurement output rates
*          unsigned char *buff O binary message
* return : length of binary message (0: error)
* note   : see reference [1][2] for details.
*-----------------------------------------------------------------------------*/
extern int gen_nvs(const char *msg, unsigned char *buff)
{
    unsigned char *q=buff;
    char mbuff[1024],*args[32],*p;
    unsigned int byte;
    int iRate,n,narg=0;
    unsigned char ui100Ms;
    
    trace(4,"gen_nvs: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    *q++=NVSSYNC; /* DLE */
    
    if (!strcmp(args[0],"CFG-PVTRATE")) {
        *q++=ID_XD7ADVANCED;
        *q++=ID_X02RATEPVT;
        if (narg>1) {
            iRate = atoi(args[1]);
            *q++ = (unsigned char) iRate;
        }
    }
    else if (!strcmp(args[0],"CFG-RAWRATE")) {
        *q++=ID_XF4RATERAW;
        if (narg>1) {
            iRate = atoi(args[1]);
            switch(iRate) {
                case 2:  ui100Ms =  5; break;
                case 5:  ui100Ms =  2; break;
                case 10: ui100Ms =  1; break;
                default: ui100Ms = 10; break;
            }
            *q++ = ui100Ms;
        }
    }
    else if (!strcmp(args[0],"CFG-SMOOTH")) {
        *q++=ID_XD7SMOOTH;
        *q++ = 0x03;
        *q++ = 0x01;
        *q++ = 0x00;
    }
    else if (!strcmp(args[0],"CFG-BINR")) {
        for (n=1;(n<narg);n++) {
            if (sscanf(args[n], "%2x",&byte)) *q++=(unsigned char)byte;
        }
    }
    else return 0;
    
    n=(int)(q-buff);
    
    *q++=0x10; /* ETX */
    *q=0x03;   /* DLE */
    return n+2;
}
