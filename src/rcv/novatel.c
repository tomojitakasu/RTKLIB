/*------------------------------------------------------------------------------
* notvatel.c : NovAtel OEM7/OEM6/OEM5/OEM4/OEM3 receiver functions
*
*          Copyright (C) 2007-2020 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] NovAtel, OM-20000094 Rev6 OEMV Family Firmware Reference Manual, 2008
*     [2] NovAtel, OM-20000053 Rev2 MiLLennium GPSCard Software Versions 4.503
*         and 4.52 Command Descriptions Manual, 2001
*     [3] NovAtel, OM-20000129 Rev2 OEM6 Family Firmware Reference Manual, 2011
*     [4] NovAtel, OM-20000127 Rev1 OEMStar Firmware Reference Manual, 2009
*     [5] NovAtel, OM-20000129 Rev6 OEM6 Family Firmware Reference Manual, 2014
*     [6] NovAtel, OM-20000169 v15C OEM7 Commands and Logs Reference Manual,
*         June 2020
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2007/10/08 1.0 new
*           2008/05/09 1.1 fix bug lli flag outage
*           2008/06/16 1.2 separate common functions to rcvcmn.c
*           2009/04/01 1.3 add prn number check for raw obs data
*           2009/04/10 1.4 refactored
*                          add oem3, oem4 rangeb support
*           2009/06/06 1.5 fix bug on numerical exception with illegal snr
*                          support oem3 regd message
*           2009/12/09 1.6 support oem4 gloephemerisb message
*                          invalid if parity unknown in GLONASS range
*                          fix bug of dopper polarity inversion for oem3 regd
*           2010/04/29 1.7 add tod field in geph_t
*           2011/05/27 1.8 support RAWALM for oem4/v
*                          add almanac decoding
*                          add -EPHALL option
*                          fix problem on ARM compiler
*           2012/05/02 1.9 support OEM6,L5,QZSS
*           2012/10/18 1.10 change obs codes
*                           support Galileo
*                           support rawsbasframeb,galephemerisb,galalmanacb,
*                           galclockb,galionob
*           2012/11/08 1.11 support galfnavrawpageb, galinavrawword
*           2012/11/19 1.12 fix bug on decodeing rangeb
*           2013/02/23 1.13 fix memory access violation problem on arm
*           2013/03/28 1.14 fix invalid phase if glonass wavelen unavailable
*           2013/06/02 1.15 fix bug on reading galephemrisb,galalmanacb,
*                           galclockb,galionob
*                           fix bug on decoding rawwaasframeb for qzss-saif
*           2014/05/24 1.16 support beidou
*           2014/07/01 1.17 fix problem on decoding of bdsephemerisb
*                           fix bug on beidou tracking codes
*           2014/10/20 1.11 fix bug on receiver option -GL*,-RL*,-EL*
*           2016/01/28 1.12 precede I/NAV for galileo ephemeris
*                           add option -GALINAV and -GALFNAV
*           2016/07/31 1.13 add week number check to decode oem4 messages
*           2017/04/11 1.14 (char *) -> (signed char *)
*                           improve unchange-test of beidou ephemeris
*           2017/06/15 1.15 add output half-cycle-ambiguity status to LLI
*                           improve slip-detection by lock-time rollback
*           2018/10/10 1.16 fix problem on data souce for galileo ephemeris
*                           output L2W instead of L2D for L2Pcodeless
*                           test toc difference to output beidou ephemeris
*           2019/05/10 1.17 save galileo E5b data to obs index 2
*           2020/11/30 1.18 support OEM7 receiver (ref [6])
*                           support NavIC/IRNSS
*                           support GPS/QZS L1C, GLO L3, GAL E6, QZS L6, BDS B3,
*                            B1C, B2a, B2b
*                           support message NAVICEPHEMERISB
*                           support QZS L1S in RANGEB and RANGECMPB
*                           no support message GALALMANACB
*                           add receiver option -GL1L,-GL2S,-GL2P,-EL6B,-JL1L,
*                            -JL1Z,-CL1P,-CL7D,-GLOBIAS=bias
*                           delete receiver option -GL1P,-GL2X,EL2C
*                           fix bug on reading SVH in GLOEPHEMERISB
*                           add reading of dtaun field in GLOEPHEMERISB
*                           output GAL I/NAV or F/NAV to seperated ephem sets
*                           use API sat2freq() to get carrier-frequency
*                           use API code2idx() to get freq-index
*                           use integer types in stdint.h
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define OEM4SYNC1       0xAA    /* oem7/6/4 message start sync code 1 */
#define OEM4SYNC2       0x44    /* oem7/6/4 message start sync code 2 */
#define OEM4SYNC3       0x12    /* oem7/6/4 message start sync code 3 */
#define OEM3SYNC1       0xAA    /* oem3 message start sync code 1 */
#define OEM3SYNC2       0x44    /* oem3 message start sync code 2 */
#define OEM3SYNC3       0x11    /* oem3 message start sync code 3 */
#define OEM4HLEN        28      /* oem7/6/4 message header length (bytes) */
#define OEM3HLEN        12      /* oem3 message header length (bytes) */

/* message IDs */
#define ID_RANGECMP     140     /* oem7/6/4 range compressed */
#define ID_RANGE        43      /* oem7/6/4 range measurement */
#define ID_RAWEPHEM     41      /* oem7/6/4 raw ephemeris */
#define ID_IONUTC       8       /* oem7/6/4 iono and utc data */
#define ID_RAWWAASFRAME 287     /* oem7/6/4 raw waas frame */
#define ID_RAWSBASFRAME 973     /* oem7/6 raw sbas frame */
#define ID_GLOEPHEMERIS 723     /* oem7/6/4 glonass ephemeris */
#define ID_GALEPHEMERIS 1122    /* oem7/6 decoded galileo ephemeris */
#define ID_GALIONO      1127    /* oem7/6 decoded galileo iono corrections */
#define ID_GALCLOCK     1121    /* oem7/6 galileo clock information */
#define ID_QZSSRAWEPHEM 1331    /* oem7/6 qzss raw ephemeris */
#define ID_QZSSRAWSUBFRAME 1330 /* oem7/6 qzss raw subframe */
#define ID_QZSSIONUTC   1347    /* oem7/6 qzss ion/utc parameters */
#define ID_BDSEPHEMERIS 1696    /* oem7/6 decoded bds ephemeris */
#define ID_NAVICEPHEMERIS 2123  /* oem7 decoded navic ephemeris */

#define ID_ALMB         18      /* oem3 decoded almanac */
#define ID_IONB         16      /* oem3 iono parameters */
#define ID_UTCB         17      /* oem3 utc parameters */
#define ID_FRMB         54      /* oem3 framed raw navigation data */
#define ID_RALB         15      /* oem3 raw almanac */
#define ID_RASB         66      /* oem3 raw almanac set */
#define ID_REPB         14      /* oem3 raw ephemeris */
#define ID_RGEB         32      /* oem3 range measurement */
#define ID_RGED         65      /* oem3 range compressed */

#define WL1             0.1902936727984
#define WL2             0.2442102134246
#define MAXVAL          8388608.0
#define OFF_FRQNO       -7      /* F/W ver.3.620 */

#define SQR(x)          ((x)*(x))

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((uint8_t *)(p)))
#define I1(p) (*((int8_t  *)(p)))
static uint16_t U2(uint8_t *p) {uint16_t u; memcpy(&u,p,2); return u;}
static uint32_t U4(uint8_t *p) {uint32_t u; memcpy(&u,p,4); return u;}
static int32_t  I4(uint8_t *p) {int32_t  i; memcpy(&i,p,4); return i;}
static float    R4(uint8_t *p) {float    r; memcpy(&r,p,4); return r;}
static double   R8(uint8_t *p) {double   r; memcpy(&r,p,8); return r;}

/* extend sign ---------------------------------------------------------------*/
static int32_t exsign(uint32_t v, int bits)
{
    return (int32_t)(v&(1<<(bits-1))?v|(~0u<<bits):v);
}
/* checksum ------------------------------------------------------------------*/
static uint8_t chksum(const uint8_t *buff, int len)
{
    uint8_t sum=0;
    int i;
    for (i=0;i<len;i++) sum^=buff[i];
    return sum;
}
/* adjust weekly rollover of GPS time ----------------------------------------*/
static gtime_t adjweek(gtime_t time, double tow)
{
    double tow_p;
    int week;
    tow_p=time2gpst(time,&week);
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    return gpst2time(week,tow);
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
/* get observation data index ------------------------------------------------*/
static int obsindex(obs_t *obs, gtime_t time, int sat)
{
    int i,j;
    
    if (obs->n>=MAXOBS) return -1;
    for (i=0;i<obs->n;i++) {
        if (obs->data[i].sat==sat) return i;
    }
    obs->data[i].time=time;
    obs->data[i].sat=sat;
    for (j=0;j<NFREQ+NEXOBS;j++) {
        obs->data[i].L[j]=obs->data[i].P[j]=0.0;
        obs->data[i].D[j]=0.0;
        obs->data[i].SNR[j]=obs->data[i].LLI[j]=0;
        obs->data[i].code[j]=CODE_NONE;
    }
    obs->n++;
    return i;
}
/* URA value (m) to URA index ------------------------------------------------*/
static int uraindex(double value)
{
    static const double ura_eph[]={
        2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
        3072.0,6144.0,0.0
    };
    int i;
    for (i=0;i<15;i++) if (ura_eph[i]>=value) break;
    return i;
}
/* signal type to obs code ---------------------------------------------------*/
static int sig2code(int sys, int sigtype)
{
    if (sys==SYS_GPS) {
        switch (sigtype) {
            case  0: return CODE_L1C; /* L1C/A */
            case  5: return CODE_L2P; /* L2P    (OEM7) */
            case  9: return CODE_L2W; /* L2P(Y),semi-codeless */
            case 14: return CODE_L5Q; /* L5Q    (OEM6) */
            case 16: return CODE_L1L; /* L1C(P) (OEM7) */
            case 17: return CODE_L2S; /* L2C(M) (OEM7) */
        }
    }
    else if (sys==SYS_GLO) {
        switch (sigtype) {
            case  0: return CODE_L1C; /* L1C/A */
            case  1: return CODE_L2C; /* L2C/A (OEM6) */
            case  5: return CODE_L2P; /* L2P */
            case  6: return CODE_L3Q; /* L3Q   (OEM7) */
        }
    }
    else if (sys==SYS_GAL) {
        switch (sigtype) {
            case  2: return CODE_L1C; /* E1C  (OEM6) */
            case  6: return CODE_L6B; /* E6B  (OEM7) */
            case  7: return CODE_L6C; /* E6C  (OEM7) */
            case 12: return CODE_L5Q; /* E5aQ (OEM6) */
            case 17: return CODE_L7Q; /* E5bQ (OEM6) */
            case 20: return CODE_L8Q; /* AltBOCQ (OEM6) */
        }
    }
    else if (sys==SYS_QZS) {
        switch (sigtype) {
            case  0: return CODE_L1C; /* L1C/A */
            case 14: return CODE_L5Q; /* L5Q    (OEM6) */
            case 16: return CODE_L1L; /* L1C(P) (OEM7) */
            case 17: return CODE_L2S; /* L2C(M) (OEM7) */
            case 27: return CODE_L6L; /* L6P    (OEM7) */
        }
    }
    else if (sys==SYS_CMP) {
        switch (sigtype) {
            case  0: return CODE_L2I; /* B1I with D1 (OEM6) */
            case  1: return CODE_L7I; /* B2I with D1 (OEM6) */
            case  2: return CODE_L6I; /* B3I with D1 (OEM7) */
            case  4: return CODE_L2I; /* B1I with D2 (OEM6) */
            case  5: return CODE_L7I; /* B2I with D2 (OEM6) */
            case  6: return CODE_L6I; /* B3I with D2 (OEM7) */
            case  7: return CODE_L1P; /* B1C(P) (OEM7) */
            case  9: return CODE_L5P; /* B2a(P) (OEM7) */
            case 11: return CODE_L7D; /* B2b(I) (OEM7,F/W 7.08) */
        }
    }
    else if (sys==SYS_IRN) {
        switch (sigtype) {
            case  0: return CODE_L5A; /* L5 (OEM7) */
        }
    }
    else if (sys==SYS_SBS) {
        switch (sigtype) {
            case  0: return CODE_L1C; /* L1C/A */
            case  6: return CODE_L5I; /* L5I (OEM6) */
        }
    }
    return 0;
}
/* decode receiver tracking status ---------------------------------------------
* decode receiver tracking status
* args   : uint32_t stat I  tracking status field
*          int    *sys   O      system (SYS_???)
*          int    *code  O      signal code (CODE_L??)
*          int    *track O      tracking state
*                         (OEM4/5)
*                         0=L1 idle                   8=L2 idle
*                         1=L1 sky search             9=L2 p-code align
*                         2=L1 wide freq pull-in     10=L2 search
*                         3=L1 narrow freq pull-in   11=L2 pll
*                         4=L1 pll                   12=L2 steering
*                         5=L1 reacq
*                         6=L1 steering
*                         7=L1 fll
*                         (OEM6/7)
*                         0=idle                      7=freq-lock loop
*                         2=wide freq band pull-in    9=channel alignment
*                         3=narrow freq band pull-in 10=code search
*                         4=phase lock loop          11=aided phase lock loop
*          int    *plock O      phase-lock flag   (0=not locked, 1=locked)
*          int    *clock O      code-lock flag    (0=not locked, 1=locked)
*          int    *parity O     parity known flag (0=not known,  1=known)
*          int    *halfc O      phase measurement (0=half-cycle not added,
*                                                  1=added)
* return : freq-index (-1:error)
* notes  : refer [1][3]
*-----------------------------------------------------------------------------*/
static int decode_track_stat(uint32_t stat, int *sys, int *code, int *track,
                             int *plock, int *clock, int *parity, int *halfc)
{
    int satsys,sigtype,idx=-1;
    
    *code=CODE_NONE;
    *track =stat&0x1F;
    *plock =(stat>>10)&1;
    *parity=(stat>>11)&1;
    *clock =(stat>>12)&1;
    satsys =(stat>>16)&7;
    *halfc =(stat>>28)&1;
    sigtype=(stat>>21)&0x1F;
    
    switch (satsys) {
        case 0: *sys=SYS_GPS; break;
        case 1: *sys=SYS_GLO; break;
        case 2: *sys=SYS_SBS; break;
        case 3: *sys=SYS_GAL; break; /* OEM6 */
        case 4: *sys=SYS_CMP; break; /* OEM6 F/W 6.400 */
        case 5: *sys=SYS_QZS; break; /* OEM6 */
        case 6: *sys=SYS_IRN; break; /* OEM7 */
        default:
            trace(2,"oem4 unknown system: sys=%d\n",satsys);
            return -1;
    }
    if (!(*code=sig2code(*sys,sigtype))||(idx=code2idx(*sys,*code))<0) {
        trace(2,"oem4 signal type error: sys=%d sigtype=%d\n",*sys,sigtype);
        return -1;
    }
    return idx;
}
/* check code priority and return freq-index ---------------------------------*/
static int checkpri(const char *opt, int sys, int code, int idx)
{
    int nex=NEXOBS;
    
    if (sys==SYS_GPS) {
        if (strstr(opt,"-GL1L")&&idx==0) return (code==CODE_L1L)?0:-1;
        if (strstr(opt,"-GL2S")&&idx==1) return (code==CODE_L2X)?1:-1;
        if (strstr(opt,"-GL2P")&&idx==1) return (code==CODE_L2P)?1:-1;
        if (code==CODE_L1L) return (nex<1)?-1:NFREQ;
        if (code==CODE_L2S) return (nex<2)?-1:NFREQ+1;
        if (code==CODE_L2P) return (nex<3)?-1:NFREQ+2;
    }
    else if (sys==SYS_GLO) {
        if (strstr(opt,"-RL2C")&&idx==1) return (code==CODE_L2C)?1:-1;
        if (code==CODE_L2C) return (nex<1)?-1:NFREQ;
    }
    else if (sys==SYS_GAL) {
        if (strstr(opt,"-EL6B")&&idx==3) return (code==CODE_L6B)?3:-1;
        if (code==CODE_L6B) return (nex<2)?-1:NFREQ;
    }
    else if (sys==SYS_QZS) {
        if (strstr(opt,"-JL1L")&&idx==0) return (code==CODE_L1L)?0:-1;
        if (strstr(opt,"-JL1Z")&&idx==0) return (code==CODE_L1Z)?0:-1;
        if (code==CODE_L1L) return (nex<1)?-1:NFREQ;
        if (code==CODE_L1Z) return (nex<2)?-1:NFREQ+1;
    }
    else if (sys==SYS_CMP) {
        if (strstr(opt,"-CL1P")&&idx==0) return (code==CODE_L1P)?0:-1;
        if (strstr(opt,"-CL7D")&&idx==0) return (code==CODE_L7D)?0:-1;
        if (code==CODE_L1P) return (nex<1)?-1:NFREQ;
        if (code==CODE_L7D) return (nex<2)?-1:NFREQ+1;
    }
    return idx<NFREQ?idx:-1;
}
/* decode RANGECMPB ----------------------------------------------------------*/
static int decode_rangecmpb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    char *q;
    double psr,adr,adr_rolls,lockt,tt,dop,snr,freq,glo_bias=0.0;
    int i,index,nobs,prn,sat,sys,code,idx,track,plock,clock,parity,halfc,lli;
    
    if ((q=strstr(raw->opt,"-GLOBIAS="))) sscanf(q,"-GLOBIAS=%lf",&glo_bias);
    
    nobs=U4(p);
    if (raw->len<OEM4HLEN+4+nobs*24) {
        trace(2,"oem4 rangecmpb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," nobs=%d",nobs);
    }
    for (i=0,p+=4;i<nobs;i++,p+=24) {
        if ((idx=decode_track_stat(U4(p),&sys,&code,&track,&plock,&clock,
                                   &parity,&halfc))<0) {
            continue;
        }
        prn=U1(p+17);
        if (sys==SYS_GLO) prn-=37;
        if (sys==SYS_SBS&&prn>=MINPRNQZS_S&&prn<=MAXPRNQZS_S&&code==CODE_L1C) {
            sys=SYS_QZS;
            prn+=10;
            code=CODE_L1Z; /* QZS L1S */
        }
        if (!(sat=satno(sys,prn))) {
            trace(3,"oem4 rangecmpb satellite number error: sys=%d,prn=%d\n",sys,prn);
            continue;
        }
        if (sys==SYS_GLO&&!parity) continue; /* invalid if GLO parity unknown */
        
        if ((idx=checkpri(raw->opt,sys,code,idx))<0) continue;
        
        dop=exsign(U4(p+4)&0xFFFFFFF,28)/256.0;
        psr=(U4(p+7)>>4)/128.0+U1(p+11)*2097152.0;
        
        if ((freq=sat2freq(sat,(uint8_t)code,&raw->nav))!=0.0) {
            adr=I4(p+12)/256.0;
            adr_rolls=(psr*freq/CLIGHT+adr)/MAXVAL;
            adr=-adr+MAXVAL*floor(adr_rolls+(adr_rolls<=0?-0.5:0.5));
            if (sys==SYS_GLO) adr+=glo_bias*freq/CLIGHT;
        }
        else {
            adr=1e-9;
        }
        lockt=(U4(p+18)&0x1FFFFF)/32.0; /* lock time */
        
        if (raw->tobs[sat-1][idx].time!=0) {
            tt=timediff(raw->time,raw->tobs[sat-1][idx]);
            lli=(lockt<65535.968&&lockt-raw->lockt[sat-1][idx]+0.05<=tt)?LLI_SLIP:0;
        }
        else {
            lli=0;
        }
        if (!parity) lli|=LLI_HALFC;
        if (halfc  ) lli|=LLI_HALFA;
        raw->tobs [sat-1][idx]=raw->time;
        raw->lockt[sat-1][idx]=lockt;
        raw->halfc[sat-1][idx]=halfc;
        
        snr=((U2(p+20)&0x3FF)>>5)+20.0;
        if (!clock) psr=0.0;     /* code unlock */
        if (!plock) adr=dop=0.0; /* phase unlock */
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [idx]=adr;
            raw->obs.data[index].P  [idx]=psr;
            raw->obs.data[index].D  [idx]=(float)dop;
            raw->obs.data[index].SNR[idx]=(uint16_t)(snr/SNR_UNIT+0.5);
            raw->obs.data[index].LLI[idx]=(uint8_t)lli;
            raw->obs.data[index].code[idx]=(uint8_t)code;
        }
    }
    return 1;
}
/* decode RANGEB -------------------------------------------------------------*/
static int decode_rangeb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    char *q;
    double psr,adr,dop,snr,lockt,tt,freq,glo_bias=0.0;
    int i,index,nobs,prn,sat,sys,code,idx,track,plock,clock,parity,halfc,lli;
    int gfrq;
    
    if ((q=strstr(raw->opt,"-GLOBIAS="))) sscanf(q,"-GLOBIAS=%lf",&glo_bias);
    
    nobs=U4(p);
    if (raw->len<OEM4HLEN+4+nobs*44) {
        trace(2,"oem4 rangeb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," nobs=%d",nobs);
    }
    for (i=0,p+=4;i<nobs;i++,p+=44) {
        if ((idx=decode_track_stat(U4(p+40),&sys,&code,&track,&plock,&clock,
                                   &parity,&halfc))<0) {
            continue;
        }
        prn=U2(p);
        if (sys==SYS_GLO) prn-=37;
        if (sys==SYS_SBS&&prn>=MINPRNQZS_S&&prn<=MAXPRNQZS_S&&code==CODE_L1C) {
            sys=SYS_QZS;
            prn+=10;
            code=CODE_L1Z; /* QZS L1S */
        }
        if (!(sat=satno(sys,prn))) {
            trace(3,"oem4 rangeb satellite number error: sys=%d,prn=%d\n",sys,prn);
            continue;
        }
        if (sys==SYS_GLO&&!parity) continue;
        
        if ((idx=checkpri(raw->opt,sys,code,idx))<0) continue;
        
        gfrq =U2(p+ 2); /* GLONASS FCN+8 */
        psr  =R8(p+ 4);
        adr  =R8(p+16);
        dop  =R4(p+28);
        snr  =R4(p+32);
        lockt=R4(p+36);
        
        if (sys==SYS_GLO) {
            freq=sat2freq(sat,(uint8_t)code,&raw->nav);
            adr-=glo_bias*freq/CLIGHT;
            if (!raw->nav.glo_fcn[prn-1]) {
                raw->nav.glo_fcn[prn-1]=gfrq; /* fcn+8 */
            }
        }
        if (raw->tobs[sat-1][idx].time!=0) {
            tt=timediff(raw->time,raw->tobs[sat-1][idx]);
            lli=lockt-raw->lockt[sat-1][idx]+0.05<=tt?LLI_SLIP:0;
        }
        else {
            lli=0;
        }
        if (!parity) lli|=LLI_HALFC;
        if (halfc  ) lli|=LLI_HALFA;
        raw->tobs [sat-1][idx]=raw->time;
        raw->lockt[sat-1][idx]=lockt;
        raw->halfc[sat-1][idx]=halfc;
        
        if (!clock) psr=0.0;     /* code unlock */
        if (!plock) adr=dop=0.0; /* phase unlock */
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [idx]=-adr;
            raw->obs.data[index].P  [idx]=psr;
            raw->obs.data[index].D  [idx]=(float)dop;
            raw->obs.data[index].SNR[idx]=(uint16_t)(snr/SNR_UNIT+0.5);
            raw->obs.data[index].LLI[idx]=(uint8_t)lli;
            raw->obs.data[index].code[idx]=(uint8_t)code;
        }
    }
    return 1;
}
/* decode RAWEPHEMB ----------------------------------------------------------*/
static int decode_rawephemb(raw_t *raw)
{
    eph_t eph={0};
    uint8_t *p=raw->buff+OEM4HLEN,subframe[30*5]={0};
    int prn,sat;
    
    if (raw->len<OEM4HLEN+102) {
        trace(2,"oem4 rawephemb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"oem4 rawephemb satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    memcpy(subframe,p+12,30*3); /* subframe 1-3 */
    
    if (!decode_frame(subframe,&eph,NULL,NULL,NULL)) {
        trace(2,"oem4 rawephemb subframe error: prn=%d\n",prn);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode&&
            eph.iodc==raw->nav.eph[sat-1].iodc) return 0;
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode IONUTCB ------------------------------------------------------------*/
static int decode_ionutcb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    int i;
    
    if (raw->len<OEM4HLEN+108) {
        trace(2,"oem4 ionutcb length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_gps[i]=R8(p+i*8);
    raw->nav.utc_gps[0]=R8(p+ 72); /* A0 */
    raw->nav.utc_gps[1]=R8(p+ 80); /* A1 */
    raw->nav.utc_gps[2]=U4(p+ 68); /* tot */
    raw->nav.utc_gps[3]=U4(p+ 64); /* WNt */
    raw->nav.utc_gps[4]=I4(p+ 96); /* dt_LS */
    raw->nav.utc_gps[5]=U4(p+ 88); /* WN_LSF */
    raw->nav.utc_gps[6]=U4(p+ 92); /* DN */
    raw->nav.utc_gps[7]=I4(p+100); /* dt_LSF */
    return 9;
}
/* decode RAWWAASFRAMEB ------------------------------------------------------*/
static int decode_rawwaasframeb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    int prn;
    
    if (raw->len<OEM4HLEN+48) {
        trace(2,"oem4 rawwaasframeb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p+4);
    if ((prn<MINPRNSBS||prn>MAXPRNSBS)&&(prn<MINPRNQZS_S||prn>MAXPRNQZS_S)) {
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    raw->sbsmsg.tow=(int)time2gpst(raw->time,&raw->sbsmsg.week);
    raw->sbsmsg.prn=prn;
    memcpy(raw->sbsmsg.msg,p+12,29);
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode RAWSBASFRAMEB ------------------------------------------------------*/
static int decode_rawsbasframeb(raw_t *raw)
{
    return decode_rawwaasframeb(raw);
}
/* decode GLOEPHEMERISB ------------------------------------------------------*/
static int decode_gloephemerisb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    geph_t geph={0};
    double tow,tof,toff;
    int prn,sat,week;
    
    if (raw->len<OEM4HLEN+144) {
        trace(2,"oem4 gloephemerisb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U2(p)-37;
    
    if (!(sat=satno(SYS_GLO,prn))) {
        trace(2,"oem4 gloephemerisb prn error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    geph.frq   =U2(p+  2)+OFF_FRQNO;
    week       =U2(p+  6);
    tow        =floor(U4(p+8)/1000.0+0.5); /* rounded to integer sec */
    toff       =U4(p+ 12);
    geph.iode  =U4(p+ 20)&0x7F;
    geph.svh   =(U4(p+24)<4)?0:1; /* 0:healthy,1:unhealthy */
    geph.pos[0]=R8(p+ 28);
    geph.pos[1]=R8(p+ 36);
    geph.pos[2]=R8(p+ 44);
    geph.vel[0]=R8(p+ 52);
    geph.vel[1]=R8(p+ 60);
    geph.vel[2]=R8(p+ 68);
    geph.acc[0]=R8(p+ 76);
    geph.acc[1]=R8(p+ 84);
    geph.acc[2]=R8(p+ 92);
    geph.taun  =R8(p+100);
    geph.dtaun =R8(p+108);
    geph.gamn  =R8(p+116);
    tof        =U4(p+124)-toff; /* glonasst->gpst */
    geph.age   =U4(p+136);
    geph.toe=gpst2time(week,tow);
    tof+=floor(tow/86400.0)*86400;
    if      (tof<tow-43200.0) tof+=86400.0;
    else if (tof>tow+43200.0) tof-=86400.0;
    geph.tof=gpst2time(week,tof);
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(geph.toe,raw->nav.geph[prn-1].toe))<1.0&&
            geph.svh==raw->nav.geph[prn-1].svh) return 0; /* unchanged */
    }
    geph.sat=sat;
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode QZSSRAWEPHEMB ------------------------------------------------------*/
static int decode_qzssrawephemb(raw_t *raw)
{
    eph_t eph={0};
    uint8_t *p=raw->buff+OEM4HLEN,subfrm[90];
    int prn,sat;
    
    if (raw->len<OEM4HLEN+106) {
        trace(2,"oem4 qzssrawephemb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    if (!(sat=satno(SYS_QZS,prn))) {
        trace(2,"oem4 qzssrawephemb satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    memcpy(subfrm,p+12,90);
    
    if (!decode_frame(subfrm,&eph,NULL,NULL,NULL)) {
        trace(3,"oem4 qzssrawephemb ephemeris error: prn=%d\n",prn);
        return 0;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iodc==raw->nav.eph[sat-1].iodc&&
            eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode QZSSRAWSUBFRAMEB ---------------------------------------------------*/
static int decode_qzssrawsubframeb(raw_t *raw)
{
    eph_t eph={0};
    double ion[8],utc[8];
    uint8_t *p=raw->buff+OEM4HLEN;
    int prn,sat,id;
    
    if (raw->len<OEM4HLEN+44) {
        trace(2,"oem4 qzssrawsubframeb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    id =U4(p+4);
    if (!(sat=satno(SYS_QZS,prn))) {
        trace(2,"oem4 qzssrawsubframeb satellite error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d id=%d",prn,id);
    }
    if (id<1||id>5) {
        trace(2,"oem4 qzssrawsubframeb subfrm id error: prn=%d id=%d\n",prn,id);
        return -1;
    }
    memcpy(raw->subfrm[sat-1]+30*(id-1),p+8,30);
    
    if (id==3) {
        if (!decode_frame(raw->subfrm[sat-1],&eph,NULL,NULL,NULL)) return 0;
        if (!strstr(raw->opt,"-EPHALL")) {
            if (eph.iodc==raw->nav.eph[sat-1].iodc&&
                eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
        }
        eph.sat=sat;
        raw->nav.eph[sat-1]=eph;
        raw->ephsat=sat;
        raw->ephset=0;
        return 2;
    }
    else if (id==4||id==5) {
        if (!decode_frame(raw->subfrm[sat-1],NULL,NULL,ion,utc)) return 0;
        adj_utcweek(raw->time,utc);
        matcpy(raw->nav.ion_qzs,ion,8,1);
        matcpy(raw->nav.utc_qzs,utc,8,1);
        return 9;
    }
    return 0;
}
/* decode QZSSIONUTCB --------------------------------------------------------*/
static int decode_qzssionutcb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    int i;
    
    if (raw->len<OEM4HLEN+108) {
        trace(2,"oem4 qzssionutcb length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_qzs[i]=R8(p+i*8);
    raw->nav.utc_qzs[0]=R8(p+72);
    raw->nav.utc_qzs[1]=R8(p+80);
    raw->nav.utc_qzs[2]=U4(p+68);
    raw->nav.utc_qzs[3]=U4(p+64);
    raw->nav.utc_qzs[4]=I4(p+96);
    return 9;
}
/* decode GALEPHEMERISB ------------------------------------------------------*/
static int decode_galephemerisb(raw_t *raw)
{
    eph_t eph={0};
    uint8_t *p=raw->buff+OEM4HLEN;
    double tow,sqrtA,af0_fnav,af1_fnav,af2_fnav,af0_inav,af1_inav,af2_inav,tt;
    int prn,sat,week,rcv_fnav,rcv_inav,svh_e1b,svh_e5a,svh_e5b,dvs_e1b,dvs_e5a;
    int dvs_e5b,toc_fnav,toc_inav,set,sel_eph=3; /* 1:I/NAV+2:F/NAV */
    
    if (strstr(raw->opt,"-GALINAV")) sel_eph=1;
    if (strstr(raw->opt,"-GALFNAV")) sel_eph=2;
    
    if (raw->len<OEM4HLEN+220) {
        trace(2,"oem4 galephemrisb length error: len=%d\n",raw->len);
        return -1;
    }
    prn       =U4(p);   p+=4;
    rcv_fnav  =U4(p)&1; p+=4;
    rcv_inav  =U4(p)&1; p+=4;
    svh_e1b   =U1(p)&3; p+=1;
    svh_e5a   =U1(p)&3; p+=1;
    svh_e5b   =U1(p)&3; p+=1;
    dvs_e1b   =U1(p)&1; p+=1;
    dvs_e5a   =U1(p)&1; p+=1;
    dvs_e5b   =U1(p)&1; p+=1;
    eph.sva   =U1(p);   p+=1+1; /* SISA index */
    eph.iode  =U4(p);   p+=4;   /* IODNav */
    eph.toes  =U4(p);   p+=4;
    sqrtA     =R8(p);   p+=8;
    eph.deln  =R8(p);   p+=8;
    eph.M0    =R8(p);   p+=8;
    eph.e     =R8(p);   p+=8;
    eph.omg   =R8(p);   p+=8;
    eph.cuc   =R8(p);   p+=8;
    eph.cus   =R8(p);   p+=8;
    eph.crc   =R8(p);   p+=8;
    eph.crs   =R8(p);   p+=8;
    eph.cic   =R8(p);   p+=8;
    eph.cis   =R8(p);   p+=8;
    eph.i0    =R8(p);   p+=8;
    eph.idot  =R8(p);   p+=8;
    eph.OMG0  =R8(p);   p+=8;
    eph.OMGd  =R8(p);   p+=8;
    toc_fnav  =U4(p);   p+=4;
    af0_fnav  =R8(p);   p+=8;
    af1_fnav  =R8(p);   p+=8;
    af2_fnav  =R8(p);   p+=8;
    toc_inav  =U4(p);   p+=4;
    af0_inav  =R8(p);   p+=8;
    af1_inav  =R8(p);   p+=8;
    af2_inav  =R8(p);   p+=8;
    eph.tgd[0]=R8(p);   p+=8; /* BGD: E5A-E1 (s) */
    eph.tgd[1]=R8(p);         /* BGD: E5B-E1 (s) */
    
    if (!(sat=satno(SYS_GAL,prn))) {
        trace(2,"oemv galephemeris satellite error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    set=rcv_fnav?1:0; /* 0:I/NAV,1:F/NAV */
    if (!(sel_eph&1)&&set==0) return 0;
    if (!(sel_eph&2)&&set==1) return 0;
    
    eph.sat =sat;
    eph.A   =SQR(sqrtA);
    eph.f0  =set?af0_fnav:af0_inav;
    eph.f1  =set?af1_fnav:af1_inav;
    eph.f2  =set?af2_fnav:af2_inav;
    eph.svh =((svh_e5b<<7)|(dvs_e5b<<6)|(svh_e5a<<4)|(dvs_e5a<<3)|
             (svh_e1b<<1)|dvs_e1b);
    eph.code=set?((1<<1)+(1<<8)):((1<<0)+(1<<2)+(1<<9));
    eph.iodc=eph.iode;
    tow=time2gpst(raw->time,&week);
    eph.week=week; /* gps-week = gal-week */
    eph.toe=gpst2time(eph.week,eph.toes);
    
    tt=timediff(eph.toe,raw->time);
    if      (tt<-302400.0) eph.week++;
    else if (tt> 302400.0) eph.week--;
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=adjweek(raw->time,set?toc_fnav:toc_inav);
    eph.ttr=raw->time;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1+MAXSAT*set].iode&&
            timediff(eph.toe,raw->nav.eph[sat-1+MAXSAT*set].toe)==0.0&&
            timediff(eph.toc,raw->nav.eph[sat-1+MAXSAT*set].toc)==0.0) {
            return 0; /* unchanged */
        }
    }
    raw->nav.eph[sat-1+MAXSAT*set]=eph;
    raw->ephsat=sat;
    raw->ephset=set;
    return 2;
}
/* decode GALCLOCKB ----------------------------------------------------------*/
static int decode_galclockb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    double a0,a1,a0g,a1g;
    int dtls,tot,wnt,wnlsf,dn,dtlsf,t0g,wn0g;
    
    if (raw->len<OEM4HLEN+64) {
        trace(2,"oem4 galclockb length error: len=%d\n",raw->len);
        return -1;
    }
    a0   =R8(p); p+=8;
    a1   =R8(p); p+=8;
    dtls =I4(p); p+=4;
    tot  =U4(p); p+=4;
    wnt  =U4(p); p+=4;
    wnlsf=U4(p); p+=4;
    dn   =U4(p); p+=4;
    dtlsf=U4(p); p+=4;
    a0g  =R8(p); p+=8;
    a1g  =R8(p); p+=8;
    t0g  =U4(p); p+=4;
    wn0g =U4(p);
    raw->nav.utc_gal[0]=a0;
    raw->nav.utc_gal[1]=a1;
    raw->nav.utc_gal[2]=tot;
    raw->nav.utc_gal[3]=wnt;
    raw->nav.utc_gal[4]=dtls;
    raw->nav.utc_gal[5]=wnlsf;
    raw->nav.utc_gal[6]=dn;
    raw->nav.utc_gal[7]=dtlsf;
    return 9;
}
/* decode GALIONOB -----------------------------------------------------------*/
static int decode_galionob(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM4HLEN;
    double ai[3];
    int i,sf[5];
    
    if (raw->len<OEM4HLEN+29) {
        trace(2,"oem4 galionob length error: len=%d\n",raw->len);
        return -1;
    }
    ai[0]=R8(p); p+=8;
    ai[1]=R8(p); p+=8;
    ai[2]=R8(p); p+=8;
    sf[0]=U1(p); p+=1;
    sf[1]=U1(p); p+=1;
    sf[2]=U1(p); p+=1;
    sf[3]=U1(p); p+=1;
    sf[4]=U1(p);
    
    for (i=0;i<3;i++) raw->nav.ion_gal[i]=ai[i];
    return 9;
}
/* decode BDSEPHEMERISB ------------------------------------------------------*/
static int decode_bdsephemerisb(raw_t *raw)
{
    eph_t eph={0};
    uint8_t *p=raw->buff+OEM4HLEN;
    double ura,sqrtA;
    int prn,sat,toc;
    
    if (raw->len<OEM4HLEN+196) {
        trace(2,"oem4 bdsephemrisb length error: len=%d\n",raw->len);
        return -1;
    }
    prn       =U4(p);   p+=4;
    eph.week  =U4(p);   p+=4;
    ura       =R8(p);   p+=8;
    eph.svh   =U4(p)&1; p+=4;
    eph.tgd[0]=R8(p);   p+=8; /* TGD1 for B1 (s) */
    eph.tgd[1]=R8(p);   p+=8; /* TGD2 for B2 (s) */
    eph.iodc  =U4(p);   p+=4; /* AODC */
    toc       =U4(p);   p+=4;
    eph.f0    =R8(p);   p+=8;
    eph.f1    =R8(p);   p+=8;
    eph.f2    =R8(p);   p+=8;
    eph.iode  =U4(p);   p+=4; /* AODE */
    eph.toes  =U4(p);   p+=4;
    sqrtA     =R8(p);   p+=8;
    eph.e     =R8(p);   p+=8;
    eph.omg   =R8(p);   p+=8;
    eph.deln  =R8(p);   p+=8;
    eph.M0    =R8(p);   p+=8;
    eph.OMG0  =R8(p);   p+=8;
    eph.OMGd  =R8(p);   p+=8;
    eph.i0    =R8(p);   p+=8;
    eph.idot  =R8(p);   p+=8;
    eph.cuc   =R8(p);   p+=8;
    eph.cus   =R8(p);   p+=8;
    eph.crc   =R8(p);   p+=8;
    eph.crs   =R8(p);   p+=8;
    eph.cic   =R8(p);   p+=8;
    eph.cis   =R8(p);
    
    if (!(sat=satno(SYS_CMP,prn))) {
        trace(2,"oemv bdsephemeris satellite error: prn=%d\n",prn);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    eph.sat=sat;
    eph.A  =SQR(sqrtA);
    eph.sva=uraindex(ura);
    eph.toe=bdt2gpst(bdt2time(eph.week,eph.toes)); /* bdt -> gpst */
    eph.toc=bdt2gpst(bdt2time(eph.week,toc));      /* bdt -> gpst */
    eph.ttr=raw->time;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(raw->nav.eph[sat-1].toe,eph.toe)==0.0&&
            timediff(raw->nav.eph[sat-1].toc,eph.toc)==0.0) return 0;
    }
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode NAVICEPHEMERISB ----------------------------------------------------*/
static int decode_navicephemerisb(raw_t *raw)
{
    eph_t eph={0};
    uint8_t *p=raw->buff+OEM4HLEN;
    double sqrtA;
    int prn,sat,toc,rsv,l5_health,s_health,alert,autonav;
    
    if (raw->len<OEM4HLEN+204) {
        trace(2,"oem4 navicephemrisb length error: len=%d\n",raw->len);
        return -1;
    }
    prn       =U4(p);   p+=4;
    eph.week  =U4(p);   p+=4;
    eph.f0    =R8(p);   p+=8;
    eph.f1    =R8(p);   p+=8;
    eph.f2    =R8(p);   p+=8;
    eph.sva   =U4(p);   p+=4; /* URA index */
    toc       =U4(p);   p+=4;
    eph.tgd[0]=R8(p);   p+=8; /* TGD */
    eph.deln  =R8(p);   p+=8;
    eph.iode  =U4(p);   p+=4; /* IODEC */
    rsv       =U4(p);   p+=4;
    l5_health =U4(p)&1; p+=4;
    s_health  =U4(p)&1; p+=4;
    eph.cuc   =R8(p);   p+=8;
    eph.cus   =R8(p);   p+=8;
    eph.cic   =R8(p);   p+=8;
    eph.cis   =R8(p);   p+=8;
    eph.crc   =R8(p);   p+=8;
    eph.crs   =R8(p);   p+=8;
    eph.idot  =R8(p);   p+=8;
    rsv       =U4(p);   p+=4;
    eph.M0    =R8(p);   p+=8;
    eph.toes  =U4(p);   p+=4;
    eph.e     =R8(p);   p+=8;
    sqrtA     =R8(p);   p+=8;
    eph.OMG0  =R8(p);   p+=8;
    eph.omg   =R8(p);   p+=8;
    eph.OMGd  =R8(p);   p+=8;
    eph.i0    =R8(p);   p+=8;
    rsv       =U4(p);   p+=4;
    alert     =U4(p);   p+=4;
    autonav   =U4(p);
    
    if (toc!=eph.toes) { /* toe and toc should be matched */
        trace(2,"oem4 navicephemrisb toe and toc unmatch prn=%d\n",prn);
        return -1;
    }
    if (!(sat=satno(SYS_IRN,prn))) {
        trace(2,"oemv navicephemeris satellite error: prn=%d\n",prn);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    eph.sat =sat;
    eph.A   =SQR(sqrtA);
    eph.svh =(l5_health<<1)|s_health;
    eph.iodc=eph.iode;
    eph.week+=1024; /* irnss-week -> gps-week */
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;
    eph.tgd[1]=0.0;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(raw->nav.eph[sat-1].toe,eph.toe)==0.0&&
            raw->nav.eph[sat-1].iode==eph.iode) return 0; /* unchanged */
    }
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode RGEB ---------------------------------------------------------------*/
static int decode_rgeb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM3HLEN;
    double tow,psr,adr,tt,lockt,dop,snr;
    int i,week,nobs,prn,sat,stat,sys,parity,lli,index,freq;
    
    week=adjgpsweek(U4(p));
    tow =R8(p+ 4);
    nobs=U4(p+12);
    raw->time=gpst2time(week,tow);
    
    if (raw->len!=OEM3HLEN+20+nobs*44) {
        trace(2,"oem3 regb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=20;i<nobs;i++,p+=44) {
        prn   =U4(p   );
        psr   =R8(p+ 4);
        adr   =R8(p+16);
        dop   =R4(p+28);
        snr   =R4(p+32);
        lockt =R4(p+36);     /* lock time (s) */
        stat  =I4(p+40);     /* tracking status */
        freq  =(stat>>20)&1; /* L1:0,L2:1 */
        sys   =(stat>>15)&7; /* satellite sys (0:GPS,1:GLONASS,2:WAAS) */
        parity=(stat>>10)&1; /* parity known */
        if (!(sat=satno(sys==1?SYS_GLO:(sys==2?SYS_SBS:SYS_GPS),prn))) {
            trace(2,"oem3 regb satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        if (raw->tobs[sat-1][freq].time!=0) {
            tt=timediff(raw->time,raw->tobs[sat-1][freq]);
            lli=lockt-raw->lockt[sat-1][freq]+0.05<tt||
                parity!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->tobs [sat-1][freq]=raw->time;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=parity;
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }        
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=-adr; /* flip sign */
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=
                0.0<=snr&&snr<255.0?(uint16_t)(snr/SNR_UNIT+0.5):0;
            raw->obs.data[index].LLI[freq]=(uint8_t)lli;
            raw->obs.data[index].code[freq]=freq==0?CODE_L1C:CODE_L2P;
        }
    }
    return 1;
}
/* decode RGED ---------------------------------------------------------------*/
static int decode_rged(raw_t *raw)
{
    uint32_t word;
    uint8_t *p=raw->buff+OEM3HLEN;
    double tow,psrh,psrl,psr,adr,adr_rolls,tt,lockt,dop;
    int i,week,nobs,prn,sat,stat,sys,parity,lli,index,freq,snr;
    
    nobs=U2(p);
    week=adjgpsweek(U2(p+2));
    tow =U4(p+4)/100.0;
    raw->time=gpst2time(week,tow);
    if (raw->len!=OEM3HLEN+12+nobs*20) {
        trace(2,"oem3 regd length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=12;i<nobs;i++,p+=20) {
        word  =U4(p);
        prn   =word&0x3F;
        snr   =((word>>6)&0x1F)+20;
        lockt =(word>>11)/32.0;
        adr   =-I4(p+4)/256.0;
        word  =U4(p+8);
        psrh  =word&0xF;
        dop   =exsign(word>>4,28)/256.0;
        psrl  =U4(p+12);
        stat  =U4(p+16)>>8;
        freq  =(stat>>20)&1; /* L1:0,L2:1 */
        sys   =(stat>>15)&7; /* satellite sys (0:GPS,1:GLONASS,2:WAAS) */
        parity=(stat>>10)&1; /* parity known */
        if (!(sat=satno(sys==1?SYS_GLO:(sys==2?SYS_SBS:SYS_GPS),prn))) {
            trace(2,"oem3 regd satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        psr=(psrh*4294967296.0+psrl)/128.0;
        adr_rolls=floor((psr/(freq==0?WL1:WL2)-adr)/MAXVAL+0.5);
        adr=adr+MAXVAL*adr_rolls;
        
        if (raw->tobs[sat-1][freq].time!=0) {
            tt=timediff(raw->time,raw->tobs[sat-1][freq]);
            lli=lockt-raw->lockt[sat-1][freq]+0.05<tt||
                parity!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->tobs [sat-1][freq]=raw->time;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=parity;
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=adr;
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=(uint16_t)(snr/SNR_UNIT+0.5);
            raw->obs.data[index].LLI[freq]=(uint8_t)lli;
            raw->obs.data[index].code[freq]=freq==0?CODE_L1C:CODE_L2P;
        }
    }
    return 1;
}
/* decode REPB ---------------------------------------------------------------*/
static int decode_repb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM3HLEN;
    eph_t eph={0};
    int prn,sat;
    
    if (raw->len!=OEM3HLEN+96) {
        trace(2,"oem3 repb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"oem3 repb satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (!decode_frame(p+4,&eph,NULL,NULL,NULL)) {
        trace(2,"oem3 repb subframe error: prn=%d\n",prn);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode FRMB --------------------------------------------------------------*/
static int decode_frmb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM3HLEN;
    double tow;
    int i,week,prn,nbit;
    
    trace(3,"decode_frmb: len=%d\n",raw->len);
    
    week=adjgpsweek(U4(p));
    tow =R8(p+ 4);
    prn =U4(p+12);
    nbit=U4(p+20);
    raw->time=gpst2time(week,tow);
    if (nbit!=250) return 0;
    if (prn<MINPRNSBS||MAXPRNSBS<prn) {
        trace(2,"oem3 frmb satellite number error: prn=%d\n",prn);
        return -1;
    }
    raw->sbsmsg.week=week;
    raw->sbsmsg.tow=(int)tow;
    raw->sbsmsg.prn=prn;
    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=p[24+i];
    return 3;
}
/* decode IONB ---------------------------------------------------------------*/
static int decode_ionb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM3HLEN;
    int i;
    
    if (raw->len!=64+OEM3HLEN) {
        trace(2,"oem3 ionb length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_gps[i]=R8(p+i*8);
    return 9;
}
/* decode UTCB ---------------------------------------------------------------*/
static int decode_utcb(raw_t *raw)
{
    uint8_t *p=raw->buff+OEM3HLEN;
    
    if (raw->len!=40+OEM3HLEN) {
        trace(2,"oem3 utcb length error: len=%d\n",raw->len);
        return -1;
    }
    raw->nav.utc_gps[0]=R8(p   );
    raw->nav.utc_gps[1]=R8(p+ 8);
    raw->nav.utc_gps[2]=U4(p+16);
    raw->nav.utc_gps[3]=adjgpsweek(U4(p+20));
    raw->nav.utc_gps[4]=I4(p+28);
    return 9;
}
/* decode NovAtel OEM4/V/6/7 message -----------------------------------------*/
static int decode_oem4(raw_t *raw)
{
    double tow;
    char tstr[32];
    int msg,stat,week,type=U2(raw->buff+4);
    
    trace(3,"decode_oem4: type=%3d len=%d\n",type,raw->len);
    
    /* check crc32 */
    if (rtk_crc32(raw->buff,raw->len)!=U4(raw->buff+raw->len)) {
        trace(2,"oem4 crc error: type=%3d len=%d\n",type,raw->len);
        return -1;
    }
    msg =(U1(raw->buff+6)>>4)&0x3; /* message type: 0=binary,1=ascii */
    stat=U1(raw->buff+13);
    week=U2(raw->buff+14);
    
    if (stat==20||week==0) {
        trace(3,"oem4 time error: type=%3d msg=%d stat=%d week=%d\n",type,msg,
              stat,week);
        return 0;
    }
    week=adjgpsweek(week);
    tow =U4(raw->buff+16)*0.001;
    raw->time=gpst2time(week,tow);
    if (msg!=0) return 0;
    
    if (raw->outtype) {
        time2str(gpst2time(week,tow),tstr,2);
        sprintf(raw->msgtype,"OEM4 %4d (%4d): %s",type,raw->len,tstr);
    }
    switch (type) {
        case ID_RANGECMP       : return decode_rangecmpb       (raw);
        case ID_RANGE          : return decode_rangeb          (raw);
        case ID_RAWEPHEM       : return decode_rawephemb       (raw);
        case ID_IONUTC         : return decode_ionutcb         (raw);
        case ID_RAWWAASFRAME   : return decode_rawwaasframeb   (raw);
        case ID_RAWSBASFRAME   : return decode_rawsbasframeb   (raw);
        case ID_GLOEPHEMERIS   : return decode_gloephemerisb   (raw);
        case ID_GALEPHEMERIS   : return decode_galephemerisb   (raw);
        case ID_GALIONO        : return decode_galionob        (raw);
        case ID_GALCLOCK       : return decode_galclockb       (raw);
        case ID_QZSSRAWEPHEM   : return decode_qzssrawephemb   (raw);
        case ID_QZSSRAWSUBFRAME: return decode_qzssrawsubframeb(raw);
        case ID_QZSSIONUTC     : return decode_qzssionutcb     (raw);
        case ID_BDSEPHEMERIS   : return decode_bdsephemerisb   (raw);
        case ID_NAVICEPHEMERIS : return decode_navicephemerisb (raw);
    }
    return 0;
}
/* decode NovAtel OEM3 message -----------------------------------------------*/
static int decode_oem3(raw_t *raw)
{
    int type=U4(raw->buff+4);
    
    trace(3,"decode_oem3: type=%3d len=%d\n",type,raw->len);
    
    /* checksum */
    if (chksum(raw->buff,raw->len)) {
        trace(2,"oem3 checksum error: type=%3d len=%d\n",type,raw->len);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"OEM3 %4d (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_RGEB: return decode_rgeb(raw);
        case ID_RGED: return decode_rged(raw);
        case ID_REPB: return decode_repb(raw);
        case ID_FRMB: return decode_frmb(raw);
        case ID_IONB: return decode_ionb(raw);
        case ID_UTCB: return decode_utcb(raw);
    }
    return 0;
}
/* sync header ---------------------------------------------------------------*/
static int sync_oem4(uint8_t *buff, uint8_t data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==OEM4SYNC1&&buff[1]==OEM4SYNC2&&buff[2]==OEM4SYNC3;
}
static int sync_oem3(uint8_t *buff, uint8_t data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==OEM3SYNC1&&buff[1]==OEM3SYNC2&&buff[2]==OEM3SYNC3;
}
/* input NovAtel OEM4/V/6/7 raw data from stream -------------------------------
* fetch next NovAtel OEM4/V/6/7 raw data and input a mesasge from stream
* args   : raw_t *raw       IO  receiver raw data control struct
*          uint8_t data     I   stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options for oem4, set raw->opt to the following
*          option strings separated by spaces.
*
*          -EPHALL : input all ephemerides
*          -GL1L   : select 1L for GPS L1 (default 1C)
*          -GL2S   : select 2S for GPS L2 (default 2W)
*          -GL2P   : select 2P for GPS L2 (default 2W)
*          -RL2C   : select 2C for GLO G2 (default 2P)
*          -EL6B   : select 6B for GAL E6 (default 6C)
*          -JL1L   : select 1L for QZS L1 (default 1C)
*          -JL1Z   : select 1Z for QZS L1 (default 1C)
*          -CL1P   : select 1P for BDS B1 (default 2I)
*          -CL7D   : select 7D for BDS B2 (default 7I)
*          -GALINAV: select I/NAV for Galileo ephemeris (default: all)
*          -GALFNAV: select F/NAV for Galileo ephemeris (default: all)
*          -GLOBIAS=bias: GLONASS code-phase bias (m)
*-----------------------------------------------------------------------------*/
extern int input_oem4(raw_t *raw, uint8_t data)
{
    trace(5,"input_oem4: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (sync_oem4(raw->buff,data)) raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==10&&(raw->len=U2(raw->buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
        trace(2,"oem4 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<10||raw->nbyte<raw->len+4) return 0;
    raw->nbyte=0;
    
    /* decode oem7/6/4 message */
    return decode_oem4(raw);
}
/* input NovAtel OEM3 raw data from stream -------------------------------------
* fetch next NovAtel OEM3 raw data and input a mesasge from stream
* args   : raw_t *raw       IO  receiver raw data control struct
*          uint8_t data     I   stream data (1 byte)
* return : same as above
*-----------------------------------------------------------------------------*/
extern int input_oem3(raw_t *raw, uint8_t data)
{
    trace(5,"input_oem3: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (sync_oem3(raw->buff,data)) raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==12&&(raw->len=U4(raw->buff+8))>MAXRAWLEN) {
        trace(2,"oem3 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<12||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode oem3 message */
    return decode_oem3(raw);
}
/* input NovAtel OEM4/V/6/7 raw data from file ---------------------------------
* fetch next NovAtel OEM4/V/6/7 raw data and input a message from file
* args   : raw_t  *raw      IO  receiver raw data control struct
*          FILE   *fp       I   file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_oem4f(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_oem4f:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_oem4(raw->buff,(uint8_t)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,7,1,fp)<1) return -2;
    raw->nbyte=10;
    
    if ((raw->len=U2(raw->buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
        trace(2,"oem4 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+10,raw->len-6,1,fp)<1) return -2;
    raw->nbyte=0;
    
    /* decode NovAtel OEM4/V/6/7 message */
    return decode_oem4(raw);
}
/* input NovAtel OEM3 raw data from file ---------------------------------------
* fetch next NovAtel OEM3 raw data and input a message from file
* args   : raw_t  *raw      IO  receiver raw data control struct
*          FILE   *fp       I   file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_oem3f(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_oem3f:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_oem3(raw->buff,(uint8_t)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,1,9,fp)<9) return -2;
    raw->nbyte=12;
    
    if ((raw->len=U4(raw->buff+8))>MAXRAWLEN) {
        trace(2,"oem3 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+12,1,raw->len-12,fp)<(size_t)(raw->len-12)) return -2;
    raw->nbyte=0;
    
    /* decode oem3 message */
    return decode_oem3(raw);
}
