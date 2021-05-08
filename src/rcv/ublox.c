/*------------------------------------------------------------------------------
* ublox.c : ublox receiver dependent functions
*
*          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
*          Copyright (C) 2014 by T.SUZUKI, All rights reserved.
*
* reference :
*     [1] ublox-AG, GPS.G3-X-03002-D, ANTARIS Positioning Engine NMEA and UBX
*         Protocol Specification, Version 5.00, 2003
*     [2] ublox-AG, UBX-13003221-R03, u-blox M8 Receiver Description including
*         Protocol Specification V5, Dec 20, 2013
*     [3] ublox-AG, UBX-13003221-R07, u-blox M8 Receiver Description including
*         Protocol Specification V15.00-17.00, Nov 3, 2014
*     [4] ublox-AG, UBX-13003221-R09, u-blox 8 /u-blox M8 Receiver Description
*         including Protocol Specification V15.00-18.00, January, 2016
*     [5] ublox-AG, UBX-18010854-R08, u-blox ZED-F9P Interface Description,
*         May, 2020
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2007/10/08 1.0  new
*           2008/06/16 1.1  separate common functions to rcvcmn.c
*           2009/04/01 1.2  add range check of prn number
*           2009/04/10 1.3  refactored
*           2009/09/25 1.4  add function gen_ubx()
*           2010/01/17 1.5  add time tag adjustment option -tadj sec
*           2010/10/31 1.6  fix bug on playback disabled for raw data (2.4.0_p9)
*           2011/05/27 1.7  add almanac decoding
*                           add -EPHALL option
*                           fix problem with ARM compiler
*           2013/02/23 1.8  fix memory access violation problem on arm
*                           change options -tadj to -TADJ, -invcp to -INVCP
*           2014/05/26 1.9  fix bug on message size of CFG-MSG
*                           fix bug on return code of decode_alm1()
*           2014/06/21 1.10 support message TRK-MEAS and TRK-SFRBX
*                           support message NAV-SOL and NAV-TIMEGPS to get time
*                           support message GFG-GNSS generation
*           2014/06/23 1.11 support message TRK-MEAS for beidou ephemeris
*           2014/08/11 1.12 fix bug on unable to read RXM-RAW
*                           fix problem on decoding glo ephemeris in TRK-SFRBX
*                           support message TRK-TRKD5
*           2014/08/31 1.13 suppress warning
*           2014/11/04 1.14 support message RXM-RAWX and RXM-SFRBX
*           2015/03/20 1.15 omit time adjustment for RXM-RAWX
*           2016/01/22 1.16 add time-tag in raw-message-type
*           2016/01/26 1.17 support galileo navigation data in RXM-SFRBX
*                           enable option -TADJ for RXM-RAWX
*           2016/05/25 1.18 fix bug on crc-buffer-overflow by decoding galileo
*                           navigation data
*           2016/07/04 1.19 add half-cycle vaild check for ubx-trk-meas
*           2016/07/29 1.20 support RXM-CFG-TMODE3 (0x06 0x71) for M8P
*                           crc24q() -> rtk_crc24q()
*                           check week number zero for ubx-rxm-raw and rawx
*           2016/08/20 1.21 add test of std-dev for carrier-phase valid
*           2016/08/26 1.22 add option -STD_SLIP to test slip by std-dev of cp
*                           fix on half-cyc valid for sbas in trkmeas
*           2017/04/11 1.23 (char *) -> (signed char *)
*                           fix bug on week handover in decode_trkmeas/trkd5()
*                           fix bug on prn for geo in decode_cnav()
*           2017/06/10 1.24 output half-cycle-subtracted flag
*           2018/10/09 1.25 support ZED-F9P according to [5]
*                           beidou C17 is handled as GEO (navigation D2).
*           2018/11/05 1.26 fix problem on missing QZSS L2C signal
*                           save signal in obs data by signal index
*                           suppress warning for cnav in ubx-rxm-sfrbx
*           2019/05/10 1.27 disable half-cyc-subtract flag on LLI for RXM-RAWX
*                           save galileo E5b data to obs index 2
*                           handle C17 as no-GEO (MEO/IGSO)
*           2020/11/30 1.28 update reference [5]
*                           support UBX-CFG-VALDEL,VALGET,VALSET
*                           support hex field format for ubx binary message
*                           add quality test for receiver time in decode_trkd5()
*                           add half cycle shift correction for BDS GEO
*                           delete receiver option -GALFNAV
*                           use API code2idx() and code2freq()
*                           support QZSS L1S (CODE_L1Z)
*                           CODE_L1I -> CODE_L2I for BDS B1I (RINEX 3.04)
*                           use integer types in stdint.h
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define UBXSYNC1    0xB5        /* ubx message sync code 1 */
#define UBXSYNC2    0x62        /* ubx message sync code 2 */
#define UBXCFG      0x06        /* ubx message cfg-??? */

#define PREAMB_CNAV 0x8B        /* cnav preamble */

#define ID_NAVSOL   0x0106      /* ubx message id: nav solution info */
#define ID_NAVTIME  0x0120      /* ubx message id: nav time gps */
#define ID_RXMRAW   0x0210      /* ubx message id: raw measurement data */
#define ID_RXMSFRB  0x0211      /* ubx message id: subframe buffer */
#define ID_RXMSFRBX 0x0213      /* ubx message id: raw subframe data */
#define ID_RXMRAWX  0x0215      /* ubx message id: multi-gnss raw meas data */
#define ID_TRKD5    0x030A      /* ubx message id: trace mesurement data */
#define ID_TRKMEAS  0x0310      /* ubx message id: trace mesurement data */
#define ID_TRKSFRBX 0x030F      /* ubx message id: trace subframe buffer */

#define FU1         1           /* ubx message field types */
#define FU2         2
#define FU4         3
#define FI1         4
#define FI2         5
#define FI4         6
#define FR4         7
#define FR8         8
#define FS32        9

#define P2_10       0.0009765625 /* 2^-10 */

#define CPSTD_VALID 5           /* std-dev threshold of carrier-phase valid */

#define ROUND(x)    (int)floor((x)+0.5)

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((uint8_t *)(p)))
#define I1(p) (*((int8_t  *)(p)))
static uint16_t U2(uint8_t *p) {uint16_t u; memcpy(&u,p,2); return u;}
static uint32_t U4(uint8_t *p) {uint32_t u; memcpy(&u,p,4); return u;}
static int32_t  I4(uint8_t *p) {int32_t  u; memcpy(&u,p,4); return u;}
static float    R4(uint8_t *p) {float    r; memcpy(&r,p,4); return r;}
static double   R8(uint8_t *p) {double   r; memcpy(&r,p,8); return r;}
static double   I8(uint8_t *p) {return I4(p+4)*4294967296.0+U4(p);}

/* set fields (little-endian) ------------------------------------------------*/
static void setU1(uint8_t *p, uint8_t  u) {*p=u;}
static void setU2(uint8_t *p, uint16_t u) {memcpy(p,&u,2);}
static void setU4(uint8_t *p, uint32_t u) {memcpy(p,&u,4);}
static void setI1(uint8_t *p, int8_t   i) {*p=(uint8_t)i;}
static void setI2(uint8_t *p, int16_t  i) {memcpy(p,&i,2);}
static void setI4(uint8_t *p, int32_t  i) {memcpy(p,&i,4);}
static void setR4(uint8_t *p, float    r) {memcpy(p,&r,4);}
static void setR8(uint8_t *p, double   r) {memcpy(p,&r,8);}

/* checksum ------------------------------------------------------------------*/
static int checksum(uint8_t *buff, int len)
{
    uint8_t cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    return cka==buff[len-2]&&ckb==buff[len-1];
}
static void setcs(uint8_t *buff, int len)
{
    uint8_t cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    buff[len-2]=cka;
    buff[len-1]=ckb;
}
/* UBX GNSSId to system (ref [2] 25) -----------------------------------------*/
static int ubx_sys(int gnssid)
{
    switch (gnssid) {
        case 0: return SYS_GPS;
        case 1: return SYS_SBS;
        case 2: return SYS_GAL;
        case 3: return SYS_CMP;
        case 5: return SYS_QZS;
        case 6: return SYS_GLO;
    }
    return 0;
}
/* UBX SigId to signal (ref [5] 1.5.4) ---------------------------------------*/
static int ubx_sig(int sys, int sigid)
{
    if (sys==SYS_GPS) {
        if (sigid==0) return CODE_L1C; /* L1C/A */
        if (sigid==3) return CODE_L2L; /* L2CL */
        if (sigid==4) return CODE_L2S; /* L2CM */
    }
    else if (sys==SYS_GLO) {
        if (sigid==0) return CODE_L1C; /* G1C/A (GLO L1 OF) */
        if (sigid==2) return CODE_L2C; /* G2C/A (GLO L2 OF) */
    }
    else if (sys==SYS_GAL) {
        if (sigid==0) return CODE_L1C; /* E1C */
        if (sigid==1) return CODE_L1B; /* E1B */
        if (sigid==5) return CODE_L7I; /* E5bI */
        if (sigid==6) return CODE_L7Q; /* E5bQ */
    }
    else if (sys==SYS_QZS) {
        if (sigid==0) return CODE_L1C; /* L1C/A */
        if (sigid==1) return CODE_L1Z; /* L1S */
        if (sigid==4) return CODE_L2S; /* L2CM */
        if (sigid==5) return CODE_L2L; /* L2CL */
    }
    else if (sys==SYS_CMP) {
        if (sigid==0) return CODE_L2I; /* B1I D1 */
        if (sigid==1) return CODE_L2I; /* B1I D2 */
        if (sigid==2) return CODE_L7I; /* B2I D1 */
        if (sigid==3) return CODE_L7I; /* B2I D2 */
    }
    else if (sys==SYS_SBS) {
        if (sigid==0) return CODE_L1C; /* L1C/A */
    }
    return CODE_NONE;
}
/* signal index in obs data --------------------------------------------------*/
static int sig_idx(int sys, uint8_t code)
{
    int idx=code2idx(sys,code),nex=NEXOBS;
    
    if (sys==SYS_GPS) {
        if (code==CODE_L2S) return (nex<1)?-1:NFREQ;   /* L2CM */
    }
    else if (sys==SYS_GAL) {
        if (code==CODE_L1B) return (nex<1)?-1:NFREQ;   /* E1B */
        if (code==CODE_L7I) return (nex<2)?-1:NFREQ+1; /* E5bI */
    }
    else if (sys==SYS_QZS) {
        if (code==CODE_L2S) return (nex<1)?-1:NFREQ;   /* L2CM */
        if (code==CODE_L1Z) return (nex<2)?-1:NFREQ+1; /* L1S */
    }
    return (idx<NFREQ)?idx:-1;
}
/* decode UBX-RXM-RAW: raw measurement data ----------------------------------*/
static int decode_rxmraw(raw_t *raw)
{
    uint8_t *p=raw->buff+6;
    gtime_t time;
    double tow,tt,tadj=0.0,toff=0.0,tn;
    int i,j,prn,sat,n=0,nsat,week;
    char *q;
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX RXM-RAW   (%4d): nsat=%d",raw->len,U1(p+6));
    }
    /* time tag adjustment option (-TADJ) */
    if ((q=strstr(raw->opt,"-TADJ="))) {
        sscanf(q,"-TADJ=%lf",&tadj);
    }
    nsat=U1(p+6);
    if (raw->len<12+24*nsat) {
        trace(2,"ubx rxmraw length error: len=%d nsat=%d\n",raw->len,nsat);
        return -1;
    }
    tow =U4(p  );
    week=U2(p+4);
    time=gpst2time(week,tow*0.001);
    
    if (week==0) {
        trace(3,"ubx rxmraw week=0 error: len=%d nsat=%d\n",raw->len,nsat);
        return 0;
    }
    /* time tag adjustment */
    if (tadj>0.0) {
        tn=time2gpst(time,&week)/tadj;
        toff=(tn-floor(tn+0.5))*tadj;
        time=timeadd(time,-toff);
    }
    tt=timediff(time,raw->time);
    
    for (i=0,p+=8;i<nsat&&i<MAXOBS;i++,p+=24) {
        raw->obs.data[n].time=time;
        raw->obs.data[n].L[0]  =R8(p   )-toff*FREQ1;
        raw->obs.data[n].P[0]  =R8(p+ 8)-toff*CLIGHT;
        raw->obs.data[n].D[0]  =R4(p+16);
        prn                    =U1(p+20);
        raw->obs.data[n].SNR[0]=(uint16_t)(I1(p+22)*1.0/SNR_UNIT+0.5);
        raw->obs.data[n].LLI[0]=U1(p+23);
        raw->obs.data[n].code[0]=CODE_L1C;
        
        /* phase polarity flip option (-INVCP) */
        if (strstr(raw->opt,"-INVCP")) {
            raw->obs.data[n].L[0]=-raw->obs.data[n].L[0];
        }
        if (!(sat=satno(MINPRNSBS<=prn?SYS_SBS:SYS_GPS,prn))) {
            trace(2,"ubx rxmraw sat number error: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].sat=sat;
        
        if (raw->obs.data[n].LLI[0]&1) raw->lockt[sat-1][0]=0.0;
        else if (tt<1.0||10.0<tt) raw->lockt[sat-1][0]=0.0;
        else raw->lockt[sat-1][0]+=tt;
        
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
/* decode UBX-RXM-RAWX: multi-GNSS raw measurement data (ref [3][4][5]) ------*/
static int decode_rxmrawx(raw_t *raw)
{
    uint8_t *p=raw->buff+6;
    gtime_t time;
    char *q,tstr[64];
    double tow,P,L,D,tn,tadj=0.0,toff=0.0;
    int i,j,k,idx,sys,prn,sat,code,slip,halfv,halfc,LLI,n=0,std_slip=0;
    int week,nmeas,ver,gnss,svid,sigid,frqid,lockt,cn0,cpstd,tstat;
    
    if (raw->len<24) {
        trace(2,"ubx rxmrawx length error: len=%d\n",raw->len);
        return -1;
    }
    tow  =R8(p   ); /* rcvTow (s) */
    week =U2(p+ 8); /* week */
    nmeas=U1(p+11); /* numMeas */
    ver  =U1(p+13); /* version ([5] 5.15.3.1) */
    
    if (raw->len<24+32*nmeas) {
        trace(2,"ubx rxmrawx length error: len=%d nmeas=%d\n",raw->len,nmeas);
        return -1;
    }
    if (week==0) {
        trace(3,"ubx rxmrawx week=0 error: len=%d nmeas=%d\n",raw->len,nmeas);
        return 0;
    }
    time=gpst2time(week,tow);
    
    if (raw->outtype) {
        time2str(time,tstr,2);
        sprintf(raw->msgtype,"UBX RXM-RAWX  (%4d): time=%s nmeas=%d ver=%d",
                raw->len,tstr,nmeas,ver);
    }
    /* time tag adjustment option (-TADJ) */
    if ((q=strstr(raw->opt,"-TADJ="))) {
        sscanf(q,"-TADJ=%lf",&tadj);
    }
    /* slip threshold of std-dev of carreir-phase (-STD_SLIP) */
    if ((q=strstr(raw->opt,"-STD_SLIP="))) {
        sscanf(q,"-STD_SLIP=%d",&std_slip);
    }
    /* time tag adjustment */
    if (tadj>0.0) {
        tn=time2gpst(time,&week)/tadj;
        toff=(tn-floor(tn+0.5))*tadj;
        time=timeadd(time,-toff);
    }
    for (i=0,p+=16;i<nmeas&&n<MAXOBS;i++,p+=32) {
        P    =R8(p   );    /* prMes (m) */
        L    =R8(p+ 8);    /* cpMes (cyc) */
        D    =R4(p+16);    /* doMes (hz) */
        gnss =U1(p+20);    /* gnssId */
        svid =U1(p+21);    /* svId */
        sigid=U1(p+22);    /* sigId ([5] 5.15.3.1) */
        frqid=U1(p+23);    /* freqId (fcn + 7) */
        lockt=U2(p+24);    /* locktime (ms) */
        cn0  =U1(p+26);    /* cn0 (dBHz) */
        cpstd=U1(p+28)&15; /* cpStdev (m) */
        tstat=U1(p+30);    /* trkStat */
        if (!(tstat&1)) P=0.0;
        if (!(tstat&2)||L==-0.5||cpstd>CPSTD_VALID) L=0.0;
        
        if (!(sys=ubx_sys(gnss))) {
            trace(2,"ubx rxmrawx: system error gnss=%d\n", gnss);
            continue;
        }
        prn=svid+(sys==SYS_QZS?192:0);
        if (!(sat=satno(sys,prn))) {
            if (sys==SYS_GLO&&prn==255) {
                continue; /* suppress warning for unknown glo satellite */
            }
            trace(2,"ubx rxmrawx sat number error: sys=%2d prn=%2d\n",sys,prn);
            continue;
        }
        if (sys==SYS_GLO&&!raw->nav.glo_fcn[prn-1]) {
            raw->nav.glo_fcn[prn-1]=frqid-7+8;
        }
        if (ver>=1) {
            code=ubx_sig(sys,sigid);
        }
        else {
            code=(sys==SYS_CMP)?CODE_L2I:((sys==SYS_GAL)?CODE_L1X:CODE_L1C);
        }
        /* signal index in obs data */
        if ((idx=sig_idx(sys,code))<0) {
            trace(2,"ubx rxmrawx signal error: sat=%2d sigid=%d\n",sat,sigid);
            continue;
        }
        /* offset by time tag adjustment */
        if (toff!=0.0) {
            P-=toff*CLIGHT;
            L-=toff*code2freq(sys,code,frqid-7);
        }
        /* half-cycle shift correction for BDS GEO */
        if (sys==SYS_CMP&&(prn<=5||prn>=59)&&L!=0.0) {
            L+=0.5;
        }
        halfv=(tstat&4)?1:0; /* half cycle valid */
        halfc=(tstat&8)?1:0; /* half cycle subtracted from phase */
        slip=lockt==0||lockt*1E-3<raw->lockt[sat-1][idx]||
             halfc!=raw->halfc[sat-1][idx]||(std_slip&&cpstd>=std_slip);
        raw->lockt[sat-1][idx]=lockt*1E-3;
        raw->halfc[sat-1][idx]=halfc;
        LLI=(slip?LLI_SLIP:0)|(!halfv?LLI_HALFC:0)|(halfc?LLI_HALFS:0);

        for (j=0;j<n;j++) {
            if (raw->obs.data[j].sat==sat) break;
        }
        if (j>=n) {
            raw->obs.data[n].time=time;
            raw->obs.data[n].sat=sat;
            raw->obs.data[n].rcv=0;
            for (k=0;k<NFREQ+NEXOBS;k++) {
                raw->obs.data[n].L[k]=raw->obs.data[n].P[k]=0.0;
                raw->obs.data[n].D[k]=0.0;
                raw->obs.data[n].SNR[k]=raw->obs.data[n].LLI[k]=0;
                raw->obs.data[n].code[k]=CODE_NONE;
            }
            n++;
        }
        raw->obs.data[j].L[idx]=L;
        raw->obs.data[j].P[idx]=P;
        raw->obs.data[j].D[idx]=(float)D;
        raw->obs.data[j].SNR[idx]=(uint16_t)(cn0*1.0/SNR_UNIT+0.5);
        raw->obs.data[j].LLI[idx]=(uint8_t)LLI;
        raw->obs.data[j].code[idx]=(uint8_t)code;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* decode UBX-NAV-SOL: navigation solution -----------------------------------*/
static int decode_navsol(raw_t *raw)
{
    uint8_t *p=raw->buff+6;
    int itow,ftow,week;
    
    trace(4,"decode_navsol: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX NAV-SOL   (%4d):",raw->len);
    }
    itow=U4(p);
    ftow=I4(p+4);
    week=U2(p+8);
    if ((U1(p+11)&0x0C)==0x0C) {
        raw->time=gpst2time(week,itow*1E-3+ftow*1E-9);
    }
    return 0;
}
/* decode UBX-NAV-TIMEGPS: GPS time solution ---------------------------------*/
static int decode_navtime(raw_t *raw)
{
    int itow,ftow,week;
    uint8_t *p=raw->buff+6;
    
    trace(4,"decode_navtime: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX NAV-TIME  (%4d):",raw->len);
    }
    itow=U4(p);
    ftow=I4(p+4);
    week=U2(p+8);
    if ((U1(p+11)&0x03)==0x03) {
        raw->time=gpst2time(week,itow*1E-3+ftow*1E-9);
    }
    return 0;
}
/* decode UBX-TRK-MEAS: trace measurement data (unofficial) ------------------*/
static int decode_trkmeas(raw_t *raw)
{
    static double adrs[MAXSAT]={0};
    uint8_t *p=raw->buff+6;
    gtime_t time;
    double ts,tr=-1.0,t,tau,utc_gpst,snr,adr,dop;
    int i,j,n=0,nch,sys,prn,sat,qi,frq,flag,lock1,lock2,week;
    
    trace(4,"decode_trkmeas: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX TRK-MEAS  (%4d):",raw->len);
    }
    if (!raw->time.time) return 0;
    
    /* number of channels */
    nch=U1(p+2);
    
    if (raw->len<112+nch*56) {
        trace(2,"decode_trkmeas: length error len=%d nch=%2d\n",raw->len,nch);
        return -1;
    }
    /* time-tag = max(transmission time + 0.08) rounded by 100 ms */
    for (i=0,p=raw->buff+110;i<nch;i++,p+=56) {
        if (U1(p+1)<4||ubx_sys(U1(p+4))!=SYS_GPS) continue;
        if ((t=I8(p+24)*P2_32/1000.0)>tr) tr=t;
    }
    if (tr<0.0) return 0;
    
    tr=ROUND((tr+0.08)/0.1)*0.1;
    
    /* adjust week handover */
    t=time2gpst(raw->time,&week);
    if      (tr<t-302400.0) week++;
    else if (tr>t+302400.0) week--;
    time=gpst2time(week,tr);
    
    utc_gpst=timediff(gpst2utc(time),time);
    
    for (i=0,p=raw->buff+110;i<nch;i++,p+=56) {
        
        /* quality indicator (0:idle,1:search,2:aquired,3:unusable, */
        /*                    4:code lock,5,6,7:code/carrier lock) */
        qi=U1(p+1);
        if (qi<4||7<qi) continue;
        
        /* system and satellite number */
        if (!(sys=ubx_sys(U1(p+4)))) {
            trace(2,"ubx trkmeas: system error\n");
            continue;
        }
        prn=U1(p+5)+(sys==SYS_QZS?192:0);
        if (!(sat=satno(sys,prn))) {
            trace(2,"ubx trkmeas sat number error: sys=%2d prn=%2d\n",sys,prn);
            continue;
        }
        /* transmission time */
        ts=I8(p+24)*P2_32/1000.0;
        if      (sys==SYS_CMP) ts+=14.0;             /* bdt  -> gpst */
        else if (sys==SYS_GLO) ts-=10800.0+utc_gpst; /* glot -> gpst */
        
        /* signal travel time */
        tau=tr-ts;
        if      (tau<-302400.0) tau+=604800.0;
        else if (tau> 302400.0) tau-=604800.0;
        
        frq  =U1(p+ 7)-7; /* frequency */
        flag =U1(p+ 8);   /* tracking status */
        lock1=U1(p+16);   /* code lock count */
        lock2=U1(p+17);   /* phase lock count */
        snr  =U2(p+20)/256.0;
        adr  =I8(p+32)*P2_32+(flag&0x40?0.5:0.0);
        dop  =I4(p+40)*P2_10*10.0;
        
        /* set slip flag */
        if (lock2==0||lock2<raw->lockt[sat-1][0]) raw->lockt[sat-1][1]=1.0;
        raw->lockt[sat-1][0]=lock2;
        
#if 0 /* for debug */
        trace(2,"[%2d] qi=%d sys=%d prn=%3d frq=%2d flag=%02X ?=%02X %02X "
              "%02X %02X %02X %02X %02X lock=%3d %3d ts=%10.3f snr=%4.1f "
              "dop=%9.3f adr=%13.3f %6.3f\n",U1(p),qi,U1(p+4),prn,frq,flag,
              U1(p+9),U1(p+10),U1(p+11),U1(p+12),U1(p+13),U1(p+14),U1(p+15),
              lock1,lock2,ts,snr,dop,adr,
              adrs[sat-1]==0.0||dop==0.0?0.0:(adr-adrs[sat-1])-dop);
#endif
        adrs[sat-1]=adr;
        
        /* check phase lock */
        if (!(flag&0x20)) continue;
        
        raw->obs.data[n].time=time;
        raw->obs.data[n].sat=sat;
        raw->obs.data[n].P[0]=tau*CLIGHT;
        raw->obs.data[n].L[0]=-adr;
        raw->obs.data[n].D[0]=(float)dop;
        raw->obs.data[n].SNR[0]=(uint16_t)(snr/SNR_UNIT+0.5);
        raw->obs.data[n].code[0]=sys==SYS_CMP?CODE_L2I:CODE_L1C;
        raw->obs.data[n].LLI[0]=raw->lockt[sat-1][1]>0.0?1:0;
        if (sys==SYS_SBS) { /* half-cycle valid */
            raw->obs.data[n].LLI[0]|=lock2>142?0:2;
        }
        else {
            raw->obs.data[n].LLI[0]|=flag&0x80?0:2;
        }
        raw->lockt[sat-1][1]=0.0;
        
        for (j=1;j<NFREQ+NEXOBS;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    if (n<=0) return 0;
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* decode UBX-TRKD5: trace measurement data (unofficial) ---------------------*/
static int decode_trkd5(raw_t *raw)
{
    static double adrs[MAXSAT]={0};
    gtime_t time;
    double ts,tr=-1.0,t,tau,adr,dop,snr,utc_gpst;
    int i,j,n=0,type,off,len,sys,prn,sat,qi,frq,flag,week;
    uint8_t *p=raw->buff+6;
    
    trace(4,"decode_trkd5: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX TRK-D5    (%4d):",raw->len);
    }
    if (!raw->time.time) return 0;
    
    utc_gpst=timediff(gpst2utc(raw->time),raw->time);
    
    switch ((type=U1(p))) {
        case 3 : off=86; len=56; break;
        case 6 : off=86; len=64; break; /* u-blox 7 */
        default: off=78; len=56; break;
    }
    for (i=0,p=raw->buff+off;p-raw->buff<raw->len-2;i++,p+=len) {
        qi=U1(p+41)&7;
        if (qi<4||7<qi) continue;
        t=I8(p)*P2_32/1000.0;
        if (ubx_sys(U1(p+56))==SYS_GLO) t-=10800.0+utc_gpst;
        if (t>tr) {tr=t; break;}
    }
    if (tr<0.0) return 0;
    
    tr=ROUND((tr+0.08)/0.1)*0.1;
    
    /* adjust week handover */
    t=time2gpst(raw->time,&week);
    if      (tr<t-302400.0) week++;
    else if (tr>t+302400.0) week--;
    time=gpst2time(week,tr);
    
    trace(4,"time=%s\n",time_str(time,0));
    
    for (i=0,p=raw->buff+off;p-raw->buff<raw->len-2;i++,p+=len) {
        
        /* quality indicator */
        qi =U1(p+41)&7;
        if (qi<4||7<qi) continue;
        
        if (type==6) {
            if (!(sys=ubx_sys(U1(p+56)))) {
                trace(2,"ubx trkd5: system error\n");
                continue;
            }
            prn=U1(p+57)+(sys==SYS_QZS?192:0);
            frq=U1(p+59)-7;
        }
        else {
            prn=U1(p+34);
            sys=prn<MINPRNSBS?SYS_GPS:SYS_SBS;
        }
        if (!(sat=satno(sys,prn))) {
            trace(2,"ubx trkd5 sat number error: sys=%2d prn=%2d\n",sys,prn);
            continue;
        }
        /* transmission time */
        ts=I8(p)*P2_32/1000.0;
        if (sys==SYS_GLO) ts-=10800.0+utc_gpst; /* glot -> gpst */
        
        /* signal travel time */
        tau=tr-ts;
        if      (tau<-302400.0) tau+=604800.0;
        else if (tau> 302400.0) tau-=604800.0;
        
        flag=U1(p+54);   /* tracking status */
        adr=qi<6?0.0:I8(p+8)*P2_32+(flag&0x01?0.5:0.0);
        dop=I4(p+16)*P2_10/4.0;
        snr=U2(p+32)/256.0;
        
        if (snr<=10.0) raw->lockt[sat-1][1]=1.0;
        
#if 0 /* for debug */
        trace(2,"[%2d] qi=%d sys=%d prn=%3d frq=%2d flag=%02X ts=%1.3f "
              "snr=%4.1f dop=%9.3f adr=%13.3f %6.3f\n",U1(p+35),qi,U1(p+56),
              prn,frq,flag,ts,snr,dop,adr,
              adrs[sat-1]==0.0||dop==0.0?0.0:(adr-adrs[sat-1])-dop);
#endif
        adrs[sat-1]=adr;
        
        /* check phase lock */
        if (!(flag&0x08)) continue;
        
        raw->obs.data[n].time=time;
        raw->obs.data[n].sat=sat;
        raw->obs.data[n].P[0]=tau*CLIGHT;
        raw->obs.data[n].L[0]=-adr;
        raw->obs.data[n].D[0]=(float)dop;
        raw->obs.data[n].SNR[0]=(uint16_t)(snr/SNR_UNIT+0.5);
        raw->obs.data[n].code[0]=sys==SYS_CMP?CODE_L2I:CODE_L1C;
        raw->obs.data[n].LLI[0]=raw->lockt[sat-1][1]>0.0?1:0;
        raw->lockt[sat-1][1]=0.0;
        
        for (j=1;j<NFREQ+NEXOBS;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    if (n<=0) return 0;
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* UTC 8-bit week -> full week -----------------------------------------------*/
static void adj_utcweek(gtime_t time, double *utc)
{
    int week;
    
    time2gpst(time,&week);
    utc[3]+=week/256*256;
    if      (utc[3]<week-127) utc[3]+=256.0;
    else if (utc[3]>week+127) utc[3]-=256.0;
    utc[5]+=utc[3]/256*256;
    if      (utc[5]<utc[3]-127) utc[5]+=256.0;
    else if (utc[5]>utc[3]+127) utc[5]-=256.0;
}
/* decode GPS/QZSS ephemeris -------------------------------------------------*/
static int decode_eph(raw_t *raw, int sat)
{
    eph_t eph={0};
    
    if (!decode_frame(raw->subfrm[sat-1],&eph,NULL,NULL,NULL)) return 0;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode&&
            eph.iodc==raw->nav.eph[sat-1].iodc&&
            timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0&&
            timediff(eph.toc,raw->nav.eph[sat-1].toc)==0.0) return 0;
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode GPS/QZSS ION/UTC parameters ----------------------------------------*/
static int decode_ionutc(raw_t *raw, int sat)
{
    double ion[8],utc[8];
    int sys=satsys(sat,NULL);
    
    if (!decode_frame(raw->subfrm[sat-1],NULL,NULL,ion,utc)) return 0;
    
    adj_utcweek(raw->time,utc);
    if (sys==SYS_QZS) {
        matcpy(raw->nav.ion_qzs,ion,8,1);
        matcpy(raw->nav.utc_qzs,utc,8,1);
    }
    else {
        matcpy(raw->nav.ion_gps,ion,8,1);
        matcpy(raw->nav.utc_gps,utc,8,1);
    }
    return 9;
}
/* decode GPS/QZSS navigation data -------------------------------------------*/
static int decode_nav(raw_t *raw, int sat, int off)
{
    uint8_t *p=raw->buff+6+off,buff[30];
    int i,id,ret;
    
    if (raw->len<48+off) {
        trace(2,"ubx rxmsfrbx nav length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }
    if ((U4(p)>>24)==PREAMB_CNAV) {
        trace(3,"ubx rxmsfrbx nav unsupported sat=%d len=%d\n",sat,raw->len);
        return 0;
    }
    for (i=0;i<10;i++,p+=4) { /* 24 x 10 bits w/o parity */
        setbitu(buff,24*i,24,U4(p)>>6);
    }
    id=getbitu(buff,43,3);
    if (id<1||id>5) {
        trace(2,"ubx rxmsfrbx nav subframe id error: sat=%d id=%d\n",sat,id);
        return -1;
    }
    memcpy(raw->subfrm[sat-1]+(id-1)*30,buff,30);
    
    if (id==3) {
        return decode_eph(raw,sat);
    }
    if (id==4||id==5) {
        ret=decode_ionutc(raw,sat);
        memset(raw->subfrm[sat-1]+(id-1)*30,0,30);
        return ret;
    }
    return 0;
}
/* decode Galileo I/NAV navigation data --------------------------------------*/
static int decode_enav(raw_t *raw, int sat, int off)
{
    eph_t eph={0};
    double ion[4]={0},utc[8]={0};
    uint8_t *p=raw->buff+6+off,buff[32],crc_buff[26]={0};
    int i,j,part1,page1,part2,page2,type;
    
    if (raw->len<40+off) {
        trace(2,"ubx rxmsfrbx enav length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }
    if (raw->len<44+off) return 0; /* E5b I/NAV */
    
    for (i=0;i<8;i++,p+=4) {
        setbitu(buff,32*i,32,U4(p));
    }
    part1=getbitu(buff,  0,1);
    page1=getbitu(buff,  1,1);
    part2=getbitu(buff,128,1);
    page2=getbitu(buff,129,1);
    
    if (part1!=0||part2!=1) {
        trace(3,"ubx rxmsfrbx enav page even/odd error: sat=%d\n",sat);
        return -1;
    }
    if (page1==1||page2==1) return 0; /* alert page */
    
    /* test crc (4(pad) + 114 + 82 bits) */
    for (i=0,j=  4;i<15;i++,j+=8) setbitu(crc_buff,j,8,getbitu(buff,i*8    ,8));
    for (i=0,j=118;i<11;i++,j+=8) setbitu(crc_buff,j,8,getbitu(buff,i*8+128,8));
    if (rtk_crc24q(crc_buff,25)!=getbitu(buff,128+82,24)) {
        trace(2,"ubx rxmsfrbx enav crc error: sat=%d\n",sat);
        return -1;
    }
    type=getbitu(buff,2,6); /* word type */
    
    if (type>6) return 0;
    
    /* save 128 (112:even+16:odd) bits word */
    for (i=0,j=2;i<14;i++,j+=8) {
        raw->subfrm[sat-1][type*16+i]=getbitu(buff,j,8);
    }
    for (i=14,j=130;i<16;i++,j+=8) {
        raw->subfrm[sat-1][type*16+i]=getbitu(buff,j,8);
    }
    if (type!=5) return 0;
    if (!decode_gal_inav(raw->subfrm[sat-1],&eph,ion,utc)) return 0;
        
    if (eph.sat!=sat) {
        trace(2,"ubx rxmsfrbx enav satellite error: sat=%d %d\n",sat,eph.sat);
        return -1;
    }
    eph.code|=(1<<0); /* data source: E1 */
    
    adj_utcweek(raw->time,utc);
    matcpy(raw->nav.ion_gal,ion,4,1);
    matcpy(raw->nav.utc_gal,utc,8,1);
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode&&
            timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0&&
            timediff(eph.toc,raw->nav.eph[sat-1].toc)==0.0) return 0;
    }
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0; /* 0:I/NAV */
    return 2;
}
/* decode BDS navigation data ------------------------------------------------*/
static int decode_cnav(raw_t *raw, int sat, int off)
{
    eph_t eph={0};
    double ion[8],utc[8];
    uint8_t *p=raw->buff+6+off,buff[38]={0};
    int i,id,pgn,prn;
    
    if (raw->len<48+off) {
        trace(2,"ubx rxmsfrbx cnav length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }
    for (i=0;i<10;i++,p+=4) {
        setbitu(buff,30*i,30,U4(p));
    }
    id=getbitu(buff,15,3); /* subframe ID */
    if (id<1||5<id) {
        trace(2,"ubx rxmsfrbx cnav subframe id error: sat=%2d\n",sat);
        return -1;
    }
    satsys(sat,&prn);
    
    if (prn>=6&&prn<=58) { /* IGSO/MEO */
        memcpy(raw->subfrm[sat-1]+(id-1)*38,buff,38);
        
        if (id==3) {
            if (!decode_bds_d1(raw->subfrm[sat-1],&eph,NULL,NULL)) return 0;
        }
        else if (id==5) {
            if (!decode_bds_d1(raw->subfrm[sat-1],NULL,ion,utc)) return 0;
            matcpy(raw->nav.ion_cmp,ion,8,1);
            matcpy(raw->nav.utc_cmp,utc,8,1);
            return 9;
        }
        else return 0;
    }
    else { /* GEO */
        pgn=getbitu(buff,42,4); /* page numuber */
        
        if (id==1&&pgn>=1&&pgn<=10) {
            memcpy(raw->subfrm[sat-1]+(pgn-1)*38,buff,38);
            if (pgn!=10) return 0;
            if (!decode_bds_d2(raw->subfrm[sat-1],&eph,NULL)) return 0;
        }
        else if (id==5&&pgn==102) {
            memcpy(raw->subfrm[sat-1]+10*38,buff,38);
            if (!decode_bds_d2(raw->subfrm[sat-1],NULL,utc)) return 0;
            matcpy(raw->nav.utc_cmp,utc,8,1);
            return 9;
        }
        else return 0;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0) return 0;
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode GLONASS navigation data --------------------------------------------*/
static int decode_gnav(raw_t *raw, int sat, int off, int frq)
{
    geph_t geph={0};
    double utc_glo[8]={0};
    int i,j,k,m,prn;
    uint8_t *p=raw->buff+6+off,buff[64],*fid;
    
    satsys(sat,&prn);
    
    if (raw->len<24+off) {
        trace(2,"ubx rxmsfrbx gnav length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=k=0;i<4;i++,p+=4) for (j=0;j<4;j++) {
        buff[k++]=p[3-j];
    }
    /* test hamming of GLONASS string */
    if (!test_glostr(buff)) {
        trace(2,"ubx rxmsfrbx gnav hamming error: sat=%2d\n",sat);
        return -1;
    }
    m=getbitu(buff,1,4);
    if (m<1||15<m) {
        trace(2,"ubx rxmsfrbx gnav string no error: sat=%2d\n",sat);
        return -1;
    }
    /* flush frame buffer if frame-ID changed */
    fid=raw->subfrm[sat-1]+150;
    if (fid[0]!=buff[12]||fid[1]!=buff[13]) {
        for (i=0;i<4;i++) memset(raw->subfrm[sat-1]+i*10,0,10);
        memcpy(fid,buff+12,2); /* save frame-id */
    }
    memcpy(raw->subfrm[sat-1]+(m-1)*10,buff,10);
    
    if (m==4) {
        /* decode GLONASS ephemeris strings */
        geph.tof=raw->time;
        if (!decode_glostr(raw->subfrm[sat-1],&geph,NULL)||geph.sat!=sat) {
            return 0;
        }
        geph.frq=frq-7;
        
        if (!strstr(raw->opt,"-EPHALL")) {
            if (geph.iode==raw->nav.geph[prn-1].iode) return 0;
        }
        raw->nav.geph[prn-1]=geph;
        raw->ephsat=sat;
        raw->ephset=0;
        return 2;
    }
    else if (m==5) {
        if (!decode_glostr(raw->subfrm[sat-1],NULL,utc_glo)) return 0;
        matcpy(raw->nav.utc_glo,utc_glo,8,1);
        return 9;
    }
    return 0;
}
/* decode SBAS navigation data -----------------------------------------------*/
static int decode_snav(raw_t *raw, int prn, int off)
{
    int i,tow,week;
    uint8_t *p=raw->buff+6+off,buff[32];
    
    if (raw->len<40+off) {
        trace(2,"ubx rxmsfrbx snav length error: len=%d\n",raw->len);
        return -1;
    }
    tow=(int)time2gpst(timeadd(raw->time,-1.0),&week);
    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=tow;
    raw->sbsmsg.week=week;
    for (i=0;i<8;i++,p+=4) {
        setbitu(buff,32*i,32,U4(p));
    }
    memcpy(raw->sbsmsg.msg,buff,29);
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode UBX-RXM-SFRBX: raw subframe data (ref [3][4][5]) -------------------*/
static int decode_rxmsfrbx(raw_t *raw)
{
    uint8_t *p=raw->buff+6;
    int prn,sat,sys;
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX RXM-SFRBX (%4d): sys=%d prn=%3d",raw->len,
                U1(p),U1(p+1));
    }
    if (!(sys=ubx_sys(U1(p)))) {
        trace(2,"ubx rxmsfrbx sys id error: sys=%d\n",U1(p));
        return -1;
    }
    prn=U1(p+1)+((sys==SYS_QZS)?192:0);
    if (!(sat=satno(sys,prn))) {
        if (sys==SYS_GLO&&prn==255) {
            return 0; /* suppress error for unknown GLONASS satellite */
        }
        trace(2,"ubx rxmsfrbx sat number error: sys=%d prn=%d\n",sys,prn);
        return -1;
    }
    if (sys==SYS_QZS&&raw->len==52) { /* QZSS L1S */
        sys=SYS_SBS;
        prn-=10;
    }
    switch (sys) {
        case SYS_GPS: return decode_nav (raw,sat,8);
        case SYS_QZS: return decode_nav (raw,sat,8);
        case SYS_GAL: return decode_enav(raw,sat,8);
        case SYS_CMP: return decode_cnav(raw,sat,8);
        case SYS_GLO: return decode_gnav(raw,sat,8,U1(p+3));
        case SYS_SBS: return decode_snav(raw,prn,8);
    }
    return 0;
}
/* decode UBX-TRK-SFRBX: subframe buffer extension (unoffitial) --------------*/
static int decode_trksfrbx(raw_t *raw)
{
    uint8_t *p=raw->buff+6;
    int prn,sat,sys;
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX TRK-SFRBX (%4d): sys=%d prn=%3d",raw->len,
                U1(p+1),U1(p+2));
    }
    if (!(sys=ubx_sys(U1(p+1)))) {
        trace(2,"ubx trksfrbx sys id error: sys=%d\n",U1(p+1));
        return -1;
    }
    prn=U1(p+2)+(sys==SYS_QZS?192:0);
    if (!(sat=satno(sys,prn))) {
        trace(2,"ubx trksfrbx sat number error: sys=%d prn=%d\n",sys,prn);
        return -1;
    }
    switch (sys) {
        case SYS_GPS: return decode_nav (raw,sat,13);
        case SYS_QZS: return decode_nav (raw,sat,13);
        case SYS_GAL: return decode_enav(raw,sat,13);
        case SYS_CMP: return decode_cnav(raw,sat,13);
        case SYS_GLO: return decode_gnav(raw,sat,13,U1(p+4));
        case SYS_SBS: return decode_snav(raw,sat,13);
    }
    return 0;
}
/* decode UBX-RXM-SFRB: subframe buffer (GPS/SBAS) ---------------------------*/
static int decode_rxmsfrb(raw_t *raw)
{
    uint32_t words[10];
    uint8_t *p=raw->buff+6,buff[30];
    int i,sys,prn,sat,id;
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX RXM-SFRB  (%4d): prn=%2d",raw->len,U1(p+1));
    }
    if (raw->len<42) {
        trace(2,"ubx rxmsfrb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U1(p+1);
    sys=(prn>=MINPRNSBS)?SYS_SBS:SYS_GPS;
    
    if (!(sat=satno(sys,prn))) {
        trace(2,"ubx rxmsfrb satellite error: prn=%d\n",prn);
        return -1;
    }
    if (sys==SYS_GPS) {
        for (i=0,p+=2;i<10;i++,p+=4) setbitu(buff,24*i,24,U4(p));
        id=getbitu(buff,43,3);
        if (id>=1&&id<=5) {
            memcpy(raw->subfrm[sat-1]+(id-1)*30,buff,30);
            if      (id==3) return decode_eph   (raw,sat);
            else if (id==4) return decode_ionutc(raw,sat);
        }
    }
    else {
        for (i=0,p+=2;i<10;i++,p+=4) words[i]=U4(p);
        if (!sbsdecodemsg(raw->time,prn,words,&raw->sbsmsg)) return 0;
        return 3;
    }
    return 0;
}
/* decode ublox raw message --------------------------------------------------*/
static int decode_ubx(raw_t *raw)
{
    int type=(U1(raw->buff+2)<<8)+U1(raw->buff+3);
    
    trace(3,"decode_ubx: type=%04x len=%d\n",type,raw->len);
    
    /* checksum */
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"ubx checksum error: type=%04x len=%d\n",type,raw->len);
        return -1;
    }
    switch (type) {
        case ID_RXMRAW  : return decode_rxmraw  (raw);
        case ID_RXMRAWX : return decode_rxmrawx (raw);
        case ID_RXMSFRB : return decode_rxmsfrb (raw);
        case ID_RXMSFRBX: return decode_rxmsfrbx(raw);
        case ID_NAVSOL  : return decode_navsol  (raw);
        case ID_NAVTIME : return decode_navtime (raw);
        case ID_TRKMEAS : return decode_trkmeas (raw);
        case ID_TRKD5   : return decode_trkd5   (raw);
        case ID_TRKSFRBX: return decode_trksfrbx(raw);
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX 0x%02X 0x%02X (%4d)",type>>8,type&0xF,
                raw->len);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_ubx(uint8_t *buff, uint8_t data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==UBXSYNC1&&buff[1]==UBXSYNC2;
}
/* input ublox raw message from stream -----------------------------------------
* fetch next ublox raw data and input a mesasge from stream
* args   : raw_t *raw       IO  receiver raw data control struct
*          uint8_t data     I   stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL    : input all ephemerides
*          -INVCP     : invert polarity of carrier-phase
*          -TADJ=tint : adjust time tags to multiples of tint (sec)
*          -STD_SLIP=std: slip by std-dev of carrier phase under std
*
*          The supported messages are as follows.
*
*          UBX-RXM-RAW  : raw measurement data
*          UBX-RXM-RAWX : multi-gnss measurement data
*          UBX-RXM-SFRB : subframe buffer
*          UBX-RXM-SFRBX: subframe buffer extension
*
*          UBX-TRK-MEAS and UBX-TRK-SFRBX are based on NEO-M8N (F/W 2.01).
*          UBX-TRK-D5 is based on NEO-7N (F/W 1.00). They are not formally
*          documented and not supported by u-blox.
*          Users can use these messages by their own risk.
*-----------------------------------------------------------------------------*/
extern int input_ubx(raw_t *raw, uint8_t data)
{
    trace(5,"input_ubx: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_ubx(raw->buff,data)) return 0;
        raw->nbyte=2;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==6) {
        if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
            trace(2,"ubx length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
    if (raw->nbyte<6||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode ublox raw message */
    return decode_ubx(raw);
}
/* input ublox raw message from file -------------------------------------------
* fetch next ublox raw data and input a message from file
* args   : raw_t  *raw      IO  receiver raw data control struct
*          FILE   *fp       I   file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_ubxf(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_ubxf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_ubx(raw->buff,(uint8_t)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+2,1,4,fp)<4) return -2;
    raw->nbyte=6;
    
    if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
        trace(2,"ubx length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+6,1,raw->len-6,fp)<(size_t)(raw->len-6)) return -2;
    raw->nbyte=0;
    
    /* decode ubx raw message */
    return decode_ubx(raw);
}
/* convert string to integer -------------------------------------------------*/
static int stoi(const char *s)
{
    uint32_t n;
    if (sscanf(s,"0x%X",&n)==1) return (int)n; /* hex (0xXXXX) */
    return atoi(s);
}
/* generate ublox binary message -----------------------------------------------
* generate ublox binary message from message string
* args   : char  *msg   IO     message string 
*            "CFG-PRT   portid res0 res1 mode baudrate inmask outmask flags"
*            "CFG-USB   vendid prodid res1 res2 power flags vstr pstr serino"
*            "CFG-MSG   msgid rate0 rate1 rate2 rate3 rate4 rate5 rate6"
*            "CFG-NMEA  filter version numsv flags"
*            "CFG-RATE  meas nav time"
*            "CFG-CFG   clear_mask save_mask load_mask [dev_mask]"
*            "CFG-TP    interval length status time_ref res adelay rdelay udelay"
*            "CFG-NAV2  ..."
*            "CFG-DAT   maja flat dx dy dz rotx roty rotz scale"
*            "CFG-INF   protocolid res0 res1 res2 mask0 mask1 mask2 ... mask5"
*            "CFG-RST   navbbr reset res"
*            "CFG-RXM   gpsmode lpmode"
*            "CFG-ANT   flags pins"
*            "CFG-FXN   flags treacq tacq treacqoff tacqoff ton toff res basetow"
*            "CFG-SBAS  mode usage maxsbas res scanmode"
*            "CFG-LIC   key0 key1 key2 key3 key4 key5"
*            "CFG-TM    intid rate flags"
*            "CFG-TM2   ch res0 res1 rate flags"
*            "CFG-TMODE tmode posx posy posz posvar svinmindur svinvarlimit"
*            "CFG-EKF   ..."
*            "CFG-GNSS  ..."
*            "CFG-ITFM  conf conf2"
*            "CFG-LOGFILTER ver flag min_int time_thr speed_thr pos_thr"
*            "CFG-NAV5  ..."
*            "CFG-NAVX5 ..."
*            "CFG-ODO   ..."
*            "CFG-PM2   ..."
*            "CFG-PWR   ver rsv1 rsv2 rsv3 state"
*            "CFG-RINV  flag data ..."
*            "CFG-SMGR  ..."
*            "CFG-TMODE2 ..."
*            "CFG-TMODE3 ..."
*            "CFG-TPS   ..."
*            "CFG-TXSLOT ..."
*            "CFG-VALDEL ver layer res0 res1 key [key ...]"
*            "CFG-VALGET ver layer pos key [key ...]"
*            "CFG-VALSET ver layer res0 res1 key value [key value ...]"
*          uint8_t *buff O binary message
* return : length of binary message (0: error)
* note   : see reference [1][3][5] for details.
*          the following messages are not supported:
*             CFG-DOSC,CFG-ESRC
*-----------------------------------------------------------------------------*/
extern int gen_ubx(const char *msg, uint8_t *buff)
{
    const char *cmd[]={
        "PRT","USB","MSG","NMEA","RATE","CFG","TP","NAV2","DAT","INF",
        "RST","RXM","ANT","FXN","SBAS","LIC","TM","TM2","TMODE","EKF",
        "GNSS","ITFM","LOGFILTER","NAV5","NAVX5","ODO","PM2","PWR","RINV","SMGR",
        "TMODE2","TMODE3","TPS","TXSLOT",
        "VALDEL","VALGET","VALSET",""
    };
    const uint8_t id[]={
        0x00,0x1B,0x01,0x17,0x08,0x09,0x07,0x1A,0x06,0x02,
        0x04,0x11,0x13,0x0E,0x16,0x80,0x10,0x19,0x1D,0x12,
        0x3E,0x39,0x47,0x24,0x23,0x1E,0x3B,0x57,0x34,0x62,
        0x36,0x71,0x31,0x53,
        0x8c,0x8b,0x8a
    };
    const int prm[][32]={
        {FU1,FU1,FU2,FU4,FU4,FU2,FU2,FU2,FU2},    /* PRT */
        {FU2,FU2,FU2,FU2,FU2,FU2,FS32,FS32,FS32}, /* USB */
        {FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1},        /* MSG */
        {FU1,FU1,FU1,FU1},                        /* NMEA */
        {FU2,FU2,FU2},                            /* RATE */
        {FU4,FU4,FU4,FU1},                        /* CFG */
        {FU4,FU4,FI1,FU1,FU2,FI2,FI2,FI4},        /* TP */
        {FU1,FU1,FU2,FU1,FU1,FU1,FU1,FI4,FU1,FU1,FU1,FU1,FU1,FU1,FU2,FU2,FU2,FU2,
         FU2,FU1,FU1,FU2,FU4,FU4},                /* NAV2 */
        {FR8,FR8,FR4,FR4,FR4,FR4,FR4,FR4,FR4},    /* DAT */
        {FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1}, /* INF */
        {FU2,FU1,FU1},                            /* RST */
        {FU1,FU1},                                /* RXM */
        {FU2,FU2},                                /* ANT */
        {FU4,FU4,FU4,FU4,FU4,FU4,FU4,FU4},        /* FXN */
        {FU1,FU1,FU1,FU1,FU4},                    /* SBAS */
        {FU2,FU2,FU2,FU2,FU2,FU2},                /* LIC */
        {FU4,FU4,FU4},                            /* TM */
        {FU1,FU1,FU2,FU4,FU4},                    /* TM2 */
        {FU4,FI4,FI4,FI4,FU4,FU4,FU4},            /* TMODE */
        {FU1,FU1,FU1,FU1,FU4,FU2,FU2,FU1,FU1,FU2}, /* EKF */
        {FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU4},    /* GNSS */
        {FU4,FU4},                                /* ITFM */
        {FU1,FU1,FU2,FU2,FU2,FU4},                /* LOGFILTER */
        {FU2,FU1,FU1,FI4,FU4,FI1,FU1,FU2,FU2,FU2,FU2,FU1,FU1,FU1,FU1,FU1,FU1,FU2,
         FU1,FU1,FU1,FU1,FU1,FU1},                /* NAV5 */
        {FU2,FU2,FU4,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU2,FU1,FU1,FU1,FU1,
         FU1,FU1,FU1,FU1,FU1,FU1,FU2},            /* NAVX5 */
        {FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1,FU1},    /* ODO */
        {FU1,FU1,FU1,FU1,FU4,FU4,FU4,FU4,FU2,FU2}, /* PM2 */
        {FU1,FU1,FU1,FU1,FU4},                    /* PWR */
        {FU1,FU1},                                /* RINV */
        {FU1,FU1,FU2,FU2,FU1,FU1,FU2,FU2,FU2,FU2,FU4}, /* SMGR */
        {FU1,FU1,FU2,FI4,FI4,FI4,FU4,FU4,FU4},    /* TMODE2 */
        {FU1,FU1,FU2,FI4,FI4,FI4,FU4,FU4,FU4},    /* TMODE3 */
        {FU1,FU1,FU1,FU1,FI2,FI2,FU4,FU4,FU4,FU4,FI4,FU4}, /* TPS */
        {FU1,FU1,FU1,FU1,FU4,FU4,FU4,FU4,FU4},    /* TXSLOT */
        {FU1,FU1,FU1,FU1},                        /* VALDEL */
        {FU1,FU1,FU2},                            /* VALGET */
        {FU1,FU1,FU1,FU1}                         /* VALSET */
    };
    uint8_t *q=buff;
    char mbuff[1024],*args[32],*p;
    int i,j,n,narg=0;
    
    trace(4,"gen_ubxf: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    if (narg<1||strncmp(args[0],"CFG-",4)) return 0;
    
    for (i=0;*cmd[i];i++) {
        if (!strcmp(args[0]+4,cmd[i])) break;
    }
    if (!*cmd[i]) return 0;
    
    *q++=UBXSYNC1;
    *q++=UBXSYNC2;
    *q++=UBXCFG;
    *q++=id[i];
    q+=2;
    for (j=1;prm[i][j-1]||j<narg;j++) {
        switch (prm[i][j-1]) {
            case FU1 : setU1(q,j<narg?(uint8_t )stoi(args[j]):0); q+=1; break;
            case FU2 : setU2(q,j<narg?(uint16_t)stoi(args[j]):0); q+=2; break;
            case FU4 : setU4(q,j<narg?(uint32_t)stoi(args[j]):0); q+=4; break;
            case FI1 : setI1(q,j<narg?(int8_t  )stoi(args[j]):0); q+=1; break;
            case FI2 : setI2(q,j<narg?(int16_t )stoi(args[j]):0); q+=2; break;
            case FI4 : setI4(q,j<narg?(int32_t )stoi(args[j]):0); q+=4; break;
            case FR4 : setR4(q,j<narg?(float   )atof(args[j]):0); q+=4; break;
            case FR8 : setR8(q,j<narg?(double  )atof(args[j]):0); q+=8; break;
            case FS32: sprintf((char *)q,"%-32.32s",j<narg?args[j]:""); q+=32; break;
            default  : setU1(q,j<narg?(uint8_t )stoi(args[j]):0); q+=1; break;
        }
    }
    n=(int)(q-buff)+2;
    setU2(buff+4,(uint16_t)(n-8)); /* length */
    setcs(buff,n);
    
    trace(5,"gen_ubx: buff=\n"); traceb(5,buff,n);
    return n;
}
