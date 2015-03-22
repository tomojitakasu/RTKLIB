/*------------------------------------------------------------------------------
* rinex.c : rinex functions
*
*          Copyright (C) 2007-2014 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 2.11, December 10, 2007
*     [2] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 3.00, November 28, 2007
*     [3] IS-GPS-200D, Navstar GPS Space Segment/Navigation User Interfaces,
*         7 March, 2006
*     [4] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 2.12, June 23, 2009
*     [5] W.Gurtner and L.Estey, RINEX The Receiver Independent Exchange Format
*         Version 3.01, June 22, 2009
*     [6] J.Ray and W.Gurtner, RINEX extentions to handle clock information
*         version 3.02, September 2, 2010
*     [7] RINEX The Receiver Independent Exchange Format Version 3.02,
*         International GNSS Service (IGS), RINEX Working Group and Radio
*         Technical Commission for Maritime Services Special Committee 104
*         (RTCM-SC104), December 10, 2012
*
* version : $Revision:$
* history : 2006/01/16 1.0  new
*           2007/03/14 1.1  read P1 if no obstype of C1
*           2007/04/27 1.2  add readrnxt() function
*           2007/05/25 1.3  add support of file path with wild-card (*)
*                           add support of compressed files
*           2007/11/02 1.4  support sbas/geo satellite
*                           support doppler observables
*                           support rinex bug of week handover
*                           add rinex obs/nav output functions
*           2008/06/16 1.5  export readrnxf(), add compress()
*                           separate sortobs(), uniqeph(), screent()
*           2008/10/28 1.6  fix bug on reading rinex obs header types of observ
*           2009/04/09 1.7  support rinex 2.11
*                           change api of outrnxobsh(),outrnxobsb(),outrnxnavb()
*           2009/06/02 1.8  add api outrnxgnavb()
*           2009/08/15 1.9  support glonass
*                           add slip save/restore functions
*           2010/03/03 1.10 fix bug of array access by disabled satellite
*           2010/07/21 1.11 support rinex ver.2.12, 3.00
*                           support rinex extension for qzss
*                           support geo navigation messages
*                           added api:
*                               setrnxcodepri(),outrnxhnavh(),outrnxhnavb(),
*                           changed api:
*                               readrnx(),readrnxt(),outrnxnavh(),outrnxgnavh()
*           2010/05/29 1.12 fix bug on skipping invalid satellite data
*                           fix bug on frequency number overflow
*                           output P1 instead of C1 if rnxopt.rcvopt=-L1P
*                           output C2 instead of P2 if rnxopt.rcvopt=-L2C
*                           change api:
*                               outrnxgnavh(),outrnxhnavh(),readrnx(),
*                               readrnxt()
*                           add api:
*                               outrnxlnavh(), outrnxqnav()
*                           move uniqeph(),uniqgeph,uniqseph()
*           2010/08/19 1.13 suppress warning
*           2012/03/01 1.14 add function to read cnes widelane fcb in rnxclk
*                           support compass rinex nav
*                           change api: setcodepri()
*           2012/10/17 1.15 support ver.2.12, ver.3.01
*                           add api init_rnxctr(),free_rnxctr(),open_rnxctr(),
*                           input_rnxctr()
*                           change api readrnxt(),readrnx()
*                           delete api setrnxcodepri()
*                           fix bug on message frama time in v.3 glonass nav
*           2013/02/09 1.16 add reading geph.iode derived from toe
*           2013/02/23 1.17 support rinex 3.02 (ref [7])
*                           change api outrnxobsh()
*                           add api outrnxcnavh()
*                           fix bug on output of fit interval
*           2013/05/08 1.18 fix bug on reading glo and geo nav in rinex 3
*           2013/09/01 1.19 fix bug on reading galileo "C1" in rinex 2.12
*           2013/12/16 1.20 reject C1 for 2.12
*           2014/05/26 1.21 fix bug on reading gps "C2" in rinex 2.11 or 2.12
*                           fix problem on type imcompatibility
*                           support beidou
*           2014/08/29 1.22 fix bug on reading gps "C2" in rinex 2.11 or 2.12
*           2014/10/20 1.23 recognize "C2" in 2.12 as "C2W" instead of "C2D"
*           2014/12/07 1.24 add read rinex option -SYS=...
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* constants/macros ----------------------------------------------------------*/

#define SQR(x)      ((x)*(x))

#define NUMSYS      6                   /* number of systems */
#define MAXRNXLEN   (16*MAXOBSTYPE+4)   /* max rinex record length */
#define MAXPOSHEAD  1024                /* max head line position */
#define MINFREQ_GLO -7                  /* min frequency number glonass */
#define MAXFREQ_GLO 13                  /* max frequency number glonass */
#define NINCOBS     262144              /* inclimental number of obs data */

static const int navsys[]={             /* satellite systems */
    SYS_GPS,SYS_GLO,SYS_GAL,SYS_QZS,SYS_SBS,SYS_CMP,0
};
static const char syscodes[]="GREJSC";  /* satellite system codes */

static const char obscodes[]="CLDS";    /* obs type codes */

static const char frqcodes[]="125678";  /* frequency codes */

static const double ura_eph[]={         /* ura values (ref [3] 20.3.3.3.1.1) */
    2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
    3072.0,6144.0,0.0
};
/* type definition -----------------------------------------------------------*/
typedef struct {                        /* signal index type */
    int n;                              /* number of index */
    int frq[MAXOBSTYPE];                /* signal frequency (1:L1,2:L2,...) */
    int pos[MAXOBSTYPE];                /* signal index in obs data (-1:no) */
    unsigned char pri [MAXOBSTYPE];     /* signal priority (15-0) */
    unsigned char type[MAXOBSTYPE];     /* type (0:C,1:L,2:D,3:S) */
    unsigned char code[MAXOBSTYPE];     /* obs code (CODE_L??) */
    double shift[MAXOBSTYPE];           /* phase shift (cycle) */
} sigind_t;

/* set string without tail space ---------------------------------------------*/
static void setstr(char *dst, const char *src, int n)
{
    char *p=dst;
    const char *q=src;
    while (*q&&q<src+n) *p++=*q++;
    *p--='\0';
    while (p>=dst&&*p==' ') *p--='\0';
}
/* adjust time considering week handover -------------------------------------*/
static gtime_t adjweek(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-302400.0) return timeadd(t, 604800.0);
    if (tt> 302400.0) return timeadd(t,-604800.0);
    return t;
}
/* adjust time considering week handover -------------------------------------*/
static gtime_t adjday(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-43200.0) return timeadd(t, 86400.0);
    if (tt> 43200.0) return timeadd(t,-86400.0);
    return t;
}
/* time string for ver.3 (yyyymmdd hhmmss UTC) -------------------------------*/
static void timestr_rnx(char *str)
{
    gtime_t time;
    double ep[6];
    time=timeget();
    time.sec=0.0;
    time2epoch(time,ep);
    sprintf(str,"%04.0f%02.0f%02.0f %02.0f%02.0f%02.0f UTC",ep[0],ep[1],ep[2],
            ep[3],ep[4],ep[5]);
}
/* satellite to satellite code -----------------------------------------------*/
static int sat2code(int sat, char *code)
{
    int prn;
    switch (satsys(sat,&prn)) {
        case SYS_GPS: sprintf(code,"G%2d",prn-MINPRNGPS+1); break;
        case SYS_GLO: sprintf(code,"R%2d",prn-MINPRNGLO+1); break;
        case SYS_GAL: sprintf(code,"E%2d",prn-MINPRNGAL+1); break;
        case SYS_SBS: sprintf(code,"S%2d",prn-100); break;
        case SYS_QZS: sprintf(code,"J%2d",prn-MINPRNQZS+1); break;
        case SYS_CMP: sprintf(code,"C%2d",prn-MINPRNCMP+1); break;
        default: return 0;
    }
    return 1;
}
/* ura index to ura value (m) ------------------------------------------------*/
static double uravalue(int sva)
{
    return 0<=sva&&sva<15?ura_eph[sva]:32767.0;
}
/* ura value (m) to ura index ------------------------------------------------*/
static int uraindex(double value)
{
    int i;
    for (i=0;i<15;i++) if (ura_eph[i]>=value) break;
    return i;
}
/* initialize station parameter ----------------------------------------------*/
static void init_sta(sta_t *sta)
{
    int i;
    *sta->name   ='\0';
    *sta->marker ='\0';
    *sta->antdes ='\0';
    *sta->antsno ='\0';
    *sta->rectype='\0';
    *sta->recver ='\0';
    *sta->recsno ='\0';
    sta->antsetup=sta->itrf=sta->deltype=0;
    for (i=0;i<3;i++) sta->pos[i]=0.0;
    for (i=0;i<3;i++) sta->del[i]=0.0;
    sta->hgt=0.0;
}
/*------------------------------------------------------------------------------
* input rinex functions
*-----------------------------------------------------------------------------*/

/* convert rinex obs type ver.2 -> ver.3 -------------------------------------*/
static void convcode(double ver, int sys, const char *str, char *type)
{
    strcpy(type,"   ");
    
    if      (!strcmp(str,"P1")) { /* ver.2.11 GPS L1PY,GLO L2P */
        if      (sys==SYS_GPS) sprintf(type,"%c1W",'C');
        else if (sys==SYS_GLO) sprintf(type,"%c1P",'C');
    }
    else if (!strcmp(str,"P2")) { /* ver.2.11 GPS L2PY,GLO L2P */
        if      (sys==SYS_GPS) sprintf(type,"%c2W",'C');
        else if (sys==SYS_GLO) sprintf(type,"%c2P",'C');
    }
    else if (!strcmp(str,"C1")) { /* ver.2.11 GPS L1C,GLO L1C/A */
        if      (ver>=2.12) ; /* reject C1 for 2.12 */
        else if (sys==SYS_GPS) sprintf(type,"%c1C",'C');
        else if (sys==SYS_GLO) sprintf(type,"%c1C",'C');
        else if (sys==SYS_GAL) sprintf(type,"%c1X",'C'); /* ver.2.12 */
        else if (sys==SYS_QZS) sprintf(type,"%c1C",'C');
        else if (sys==SYS_SBS) sprintf(type,"%c1C",'C');
    }
    else if (!strcmp(str,"C2")) {
        if (sys==SYS_GPS) {
            if (ver>=2.12) sprintf(type,"%c2W",'C'); /* L2P(Y) */
            else           sprintf(type,"%c2X",'C'); /* L2C */
        }
        else if (sys==SYS_GLO) sprintf(type,"%c2C",'C');
        else if (sys==SYS_QZS) sprintf(type,"%c2X",'C');
        else if (sys==SYS_CMP) sprintf(type,"%c1X",'C'); /* ver.2.12 B1 */
    }
    else if (ver>=2.12&&str[1]=='A') { /* ver.2.12 L1C/A */
        if      (sys==SYS_GPS) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_GLO) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_SBS) sprintf(type,"%c1C",str[0]);
    }
    else if (ver>=2.12&&str[1]=='B') { /* ver.2.12 GPS L1C */
        if      (sys==SYS_GPS) sprintf(type,"%c1X",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c1X",str[0]);
    }
    else if (ver>=2.12&&str[1]=='C') { /* ver.2.12 GPS L2C */
        if      (sys==SYS_GPS) sprintf(type,"%c2X",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c2X",str[0]);
    }
    else if (ver>=2.12&&str[1]=='D') { /* ver.2.12 GLO L2C/A */
        if      (sys==SYS_GLO) sprintf(type,"%c2C",str[0]);
    }
    else if (ver>=2.12&&str[1]=='1') { /* ver.2.12 GPS L1PY,GLO L1P */
        if      (sys==SYS_GPS) sprintf(type,"%c1W",str[0]);
        else if (sys==SYS_GLO) sprintf(type,"%c1P",str[0]);
        else if (sys==SYS_GAL) sprintf(type,"%c1X",str[0]); /* tentative */
        else if (sys==SYS_CMP) sprintf(type,"%c1X",str[0]); /* extension */
    }
    else if (ver<2.12&&str[1]=='1') {
        if      (sys==SYS_GPS) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_GLO) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_GAL) sprintf(type,"%c1X",str[0]); /* tentative */
        else if (sys==SYS_QZS) sprintf(type,"%c1C",str[0]);
        else if (sys==SYS_SBS) sprintf(type,"%c1C",str[0]);
    }
    else if (str[1]=='2') {
        if      (sys==SYS_GPS) sprintf(type,"%c2W",str[0]);
        else if (sys==SYS_GLO) sprintf(type,"%c2P",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c2X",str[0]);
        else if (sys==SYS_CMP) sprintf(type,"%c1X",str[0]); /* ver.2.12 B1 */
    }
    else if (str[1]=='5') {
        if      (sys==SYS_GPS) sprintf(type,"%c5X",str[0]);
        else if (sys==SYS_GAL) sprintf(type,"%c5X",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c5X",str[0]);
        else if (sys==SYS_SBS) sprintf(type,"%c5X",str[0]);
    }
    else if (str[1]=='6') {
        if      (sys==SYS_GAL) sprintf(type,"%c6X",str[0]);
        else if (sys==SYS_QZS) sprintf(type,"%c6X",str[0]);
        else if (sys==SYS_CMP) sprintf(type,"%c6X",str[0]); /* ver.2.12 B3 */
    }
    else if (str[1]=='7') {
        if      (sys==SYS_GAL) sprintf(type,"%c7X",str[0]);
        else if (sys==SYS_CMP) sprintf(type,"%c7X",str[0]); /* ver.2.12 B2 */
    }
    else if (str[1]=='8') {
        if      (sys==SYS_GAL) sprintf(type,"%c8X",str[0]);
    }
    trace(3,"convcode: ver=%.2f sys=%2d type= %s -> %s\n",ver,sys,str,type);
}
/* decode obs header ---------------------------------------------------------*/
static void decode_obsh(FILE *fp, char *buff, double ver, int *tsys,
                        char tobs[][MAXOBSTYPE][4], nav_t *nav, sta_t *sta)
{
    /* default codes for unknown code */
    const char *defcodes[]={
        "CWX   ",   /* GPS: L125___ */
        "CC    ",   /* GLO: L12____ */
        "X XXXX",   /* GAL: L1_5678 */
        "CXXX  ",   /* QZS: L1256__ */
        "C X   ",   /* SBS: L1_5___ */
        "X  XX "    /* BDS: L1__67_ */
    };
    double del[3];
    int i,j,k,n,nt,prn,fcn;
    const char *p;
    char *label=buff+60,str[4];
    
    trace(4,"decode_obsh: ver=%.2f\n",ver);
    
    if      (strstr(label,"MARKER NAME"         )) {
        if (sta) setstr(sta->name,buff,60);
    }
    else if (strstr(label,"MARKER NUMBER"       )) { /* opt */
        if (sta) setstr(sta->marker,buff,20);
    }
    else if (strstr(label,"MARKER TYPE"         )) ; /* ver.3 */
    else if (strstr(label,"OBSERVER / AGENCY"   )) ;
    else if (strstr(label,"REC # / TYPE / VERS" )) {
        if (sta) {
            setstr(sta->recsno, buff,   20);
            setstr(sta->rectype,buff+20,20);
            setstr(sta->recver, buff+40,20);
        }
    }
    else if (strstr(label,"ANT # / TYPE"        )) {
        if (sta) {
            setstr(sta->antsno,buff   ,20);
            setstr(sta->antdes,buff+20,20);
        }
    }
    else if (strstr(label,"APPROX POSITION XYZ" )) {
        if (sta) {
            for (i=0,j=0;i<3;i++,j+=14) sta->pos[i]=str2num(buff,j,14);
        }
    }
    else if (strstr(label,"ANTENNA: DELTA H/E/N")) {
        if (sta) {
            for (i=0,j=0;i<3;i++,j+=14) del[i]=str2num(buff,j,14);
            sta->del[2]=del[0]; /* h */
            sta->del[0]=del[1]; /* e */
            sta->del[1]=del[2]; /* n */
        }
    }
    else if (strstr(label,"ANTENNA: DELTA X/Y/Z")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: PHASECENTER")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: B.SIGHT XYZ")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: ZERODIR AZI")) ; /* opt ver.3 */
    else if (strstr(label,"ANTENNA: ZERODIR XYZ")) ; /* opt ver.3 */
    else if (strstr(label,"CENTER OF MASS: XYZ" )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / # / OBS TYPES" )) { /* ver.3 */
        if (!(p=strchr(syscodes,buff[0]))) {
            trace(2,"invalid system code: sys=%c\n",buff[0]);
            return;
        }
        i=(int)(p-syscodes);
        n=(int)str2num(buff,3,3);
        for (j=nt=0,k=7;j<n;j++,k+=4) {
            if (k>58) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                k=7;
            }
            if (nt<MAXOBSTYPE-1) setstr(tobs[i][nt++],buff+k,3);
        }
        *tobs[i][nt]='\0';
        
        /* change beidou B1 code: 3.02 draft -> 3.02 */
        if (i==5) {
            for (j=0;j<nt;j++) if (tobs[i][j][1]=='2') tobs[i][j][1]='1';
        }
        /* if unknown code in ver.3, set default code */
        for (j=0;j<nt;j++) {
            if (tobs[i][j][2]) continue;
            if (!(p=strchr(frqcodes,tobs[i][j][1]))) continue;
            tobs[i][j][2]=defcodes[i][(int)(p-frqcodes)];
            trace(2,"set default for unknown code: sys=%c code=%s\n",buff[0],
                  tobs[i][j]);
        }
    }
    else if (strstr(label,"WAVELENGTH FACT L1/2")) ; /* opt ver.2 */
    else if (strstr(label,"# / TYPES OF OBSERV" )) { /* ver.2 */
        n=(int)str2num(buff,0,6);
        for (i=nt=0,j=10;i<n;i++,j+=6) {
            if (j>58) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                j=10;
            }
            if (nt>=MAXOBSTYPE-1) continue;
            if (ver<=2.99) {
                setstr(str,buff+j,2);
                convcode(ver,SYS_GPS,str,tobs[0][nt]);
                convcode(ver,SYS_GLO,str,tobs[1][nt]);
                convcode(ver,SYS_GAL,str,tobs[2][nt]);
                convcode(ver,SYS_QZS,str,tobs[3][nt]);
                convcode(ver,SYS_SBS,str,tobs[4][nt]);
                convcode(ver,SYS_CMP,str,tobs[5][nt]);
            }
            nt++;
        }
        *tobs[0][nt]='\0';
    }
    else if (strstr(label,"SIGNAL STRENGTH UNIT")) ; /* opt ver.3 */
    else if (strstr(label,"INTERVAL"            )) ; /* opt */
    else if (strstr(label,"TIME OF FIRST OBS"   )) {
        if      (!strncmp(buff+48,"GPS",3)) *tsys=TSYS_GPS;
        else if (!strncmp(buff+48,"GLO",3)) *tsys=TSYS_UTC;
        else if (!strncmp(buff+48,"GAL",3)) *tsys=TSYS_GAL;
        else if (!strncmp(buff+48,"QZS",3)) *tsys=TSYS_QZS; /* ver.3.02 */
        else if (!strncmp(buff+48,"BDT",3)) *tsys=TSYS_CMP; /* ver.3.02 */
    }
    else if (strstr(label,"TIME OF LAST OBS"    )) ; /* opt */
    else if (strstr(label,"RCV CLOCK OFFS APPL" )) ; /* opt */
    else if (strstr(label,"SYS / DCBS APPLIED"  )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / PCVS APPLIED"  )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / SCALE FACTOR"  )) ; /* opt ver.3 */
    else if (strstr(label,"SYS / PHASE SHIFTS"  )) ; /* ver.3.01 */
    else if (strstr(label,"GLONASS SLOT / FRQ #")) { /* ver.3.02 */
        if (nav) {
            for (i=0,p=buff+4;i<8;i++,p+=8) {
                if (sscanf(p,"R%2d %2d",&prn,&fcn)<2) continue;
                if (1<=prn&&prn<=MAXPRNGLO) nav->glo_fcn[prn-1]=fcn+8;
            }
        }
    }
    else if (strstr(label,"GLONASS COD/PHS/BIS" )) { /* ver.3.02 */
        if (nav) {
            for (i=0,p=buff;i<4;i++,p+=13) {
                if      (strncmp(p+1,"C1C",3)) nav->glo_cpbias[0]=str2num(p,5,8);
                else if (strncmp(p+1,"C1P",3)) nav->glo_cpbias[1]=str2num(p,5,8);
                else if (strncmp(p+1,"C2C",3)) nav->glo_cpbias[2]=str2num(p,5,8);
                else if (strncmp(p+1,"C2P",3)) nav->glo_cpbias[3]=str2num(p,5,8);
            }
        }
    }
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
    else if (strstr(label,"# OF SALTELLITES"    )) ; /* opt */
    else if (strstr(label,"PRN / # OF OBS"      )) ; /* opt */
}
/* decode nav header ---------------------------------------------------------*/
static void decode_navh(char *buff, nav_t *nav)
{
    int i,j;
    char *label=buff+60;
    
    trace(4,"decode_navh:\n");
    
    if      (strstr(label,"ION ALPHA"           )) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=2;i<4;i++,j+=12) nav->ion_gps[i]=str2num(buff,j,12);
        }
    }
    else if (strstr(label,"ION BETA"            )) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=2;i<4;i++,j+=12) nav->ion_gps[i+4]=str2num(buff,j,12);
        }
    }
    else if (strstr(label,"DELTA-UTC: A0,A1,T,W")) { /* opt ver.2 */
        if (nav) {
            for (i=0,j=3;i<2;i++,j+=19) nav->utc_gps[i]=str2num(buff,j,19);
            for (;i<4;i++,j+=9) nav->utc_gps[i]=str2num(buff,j,9);
        }
    }
    else if (strstr(label,"IONOSPHERIC CORR"    )) { /* opt ver.3 */
        if (nav) {
            if (!strncmp(buff,"GPSA",4)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gps[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"GPSB",4)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gps[i+4]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"GAL",3)) {
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_gal[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"QZSA",4)) { /* v.3.02 */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_qzs[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"QZSB",4)) { /* v.3.02 */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_qzs[i+4]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"BDSA",4)) { /* v.3.02 */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_cmp[i]=str2num(buff,j,12);
            }
            else if (!strncmp(buff,"BDSB",4)) { /* v.3.02 */
                for (i=0,j=5;i<4;i++,j+=12) nav->ion_cmp[i+4]=str2num(buff,j,12);
            }
        }
    }
    else if (strstr(label,"TIME SYSTEM CORR"    )) { /* opt ver.3 */
        if (nav) {
            if (!strncmp(buff,"GPUT",4)) {
                nav->utc_gps[0]=str2num(buff, 5,17);
                nav->utc_gps[1]=str2num(buff,22,16);
                nav->utc_gps[2]=str2num(buff,38, 7);
                nav->utc_gps[3]=str2num(buff,45, 5);
            }
            else if (!strncmp(buff,"GLUT",4)) {
                nav->utc_glo[0]=str2num(buff, 5,17);
                nav->utc_glo[1]=str2num(buff,22,16);
            }
            else if (!strncmp(buff,"GAUT",4)) { /* v.3.02 */
                nav->utc_gal[0]=str2num(buff, 5,17);
                nav->utc_gal[1]=str2num(buff,22,16);
                nav->utc_gal[2]=str2num(buff,38, 7);
                nav->utc_gal[3]=str2num(buff,45, 5);
            }
            else if (!strncmp(buff,"QZUT",4)) { /* v.3.02 */
                nav->utc_qzs[0]=str2num(buff, 5,17);
                nav->utc_qzs[1]=str2num(buff,22,16);
                nav->utc_qzs[2]=str2num(buff,38, 7);
                nav->utc_qzs[3]=str2num(buff,45, 5);
            }
            else if (!strncmp(buff,"BDUT",4)) { /* v.3.02 */
                nav->utc_cmp[0]=str2num(buff, 5,17);
                nav->utc_cmp[1]=str2num(buff,22,16);
                nav->utc_cmp[2]=str2num(buff,38, 7);
                nav->utc_cmp[3]=str2num(buff,45, 5);
            }
            else if (!strncmp(buff,"SBUT",4)) { /* v.3.02 */
                nav->utc_cmp[0]=str2num(buff, 5,17);
                nav->utc_cmp[1]=str2num(buff,22,16);
                nav->utc_cmp[2]=str2num(buff,38, 7);
                nav->utc_cmp[3]=str2num(buff,45, 5);
            }
        }
    }
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* decode gnav header --------------------------------------------------------*/
static void decode_gnavh(char *buff, nav_t *nav)
{
    char *label=buff+60;
    
    trace(4,"decode_gnavh:\n");
    
    if      (strstr(label,"CORR TO SYTEM TIME"  )) ; /* opt */
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* decode geo nav header -----------------------------------------------------*/
static void decode_hnavh(char *buff, nav_t *nav)
{
    char *label=buff+60;
    
    trace(4,"decode_hnavh:\n");
    
    if      (strstr(label,"CORR TO SYTEM TIME"  )) ; /* opt */
    else if (strstr(label,"D-UTC A0,A1,T,W,S,U" )) ; /* opt */
    else if (strstr(label,"LEAP SECONDS"        )) { /* opt */
        if (nav) nav->leaps=(int)str2num(buff,0,6);
    }
}
/* read rinex header ---------------------------------------------------------*/
static int readrnxh(FILE *fp, double *ver, char *type, int *sys, int *tsys,
                    char tobs[][MAXOBSTYPE][4], nav_t *nav, sta_t *sta)
{
    double bias;
    char buff[MAXRNXLEN],*label=buff+60;
    int i=0,block=0,sat;
    
    trace(3,"readrnxh:\n");
    
    *ver=2.10; *type=' '; *sys=SYS_GPS; *tsys=TSYS_GPS;
    
    while (fgets(buff,MAXRNXLEN,fp)) {
        
        if (strlen(buff)<=60) continue;
        
        else if (strstr(label,"RINEX VERSION / TYPE")) {
            *ver=str2num(buff,0,9);
            *type=*(buff+20);
            
            /* satellite system */
            switch (*(buff+40)) {
                case ' ':
                case 'G': *sys=SYS_GPS;  *tsys=TSYS_GPS; break;
                case 'R': *sys=SYS_GLO;  *tsys=TSYS_UTC; break;
                case 'E': *sys=SYS_GAL;  *tsys=TSYS_GAL; break; /* v.2.12 */
                case 'S': *sys=SYS_SBS;  *tsys=TSYS_GPS; break;
                case 'J': *sys=SYS_QZS;  *tsys=TSYS_QZS; break; /* v.3.02 */
                case 'C': *sys=SYS_CMP;  *tsys=TSYS_CMP; break; /* v.2.12 */
                case 'M': *sys=SYS_NONE; *tsys=TSYS_GPS; break; /* mixed */
                default :
                    trace(2,"not supported satellite system: %c\n",*(buff+40));
                    break;
            }
            continue;
        }
        else if (strstr(label,"PGM / RUN BY / DATE")) continue;
        else if (strstr(label,"COMMENT")) { /* opt */
            
            /* read cnes wl satellite fractional bias */
            if (strstr(buff,"WIDELANE SATELLITE FRACTIONAL BIASES")||
                strstr(buff,"WIDELANE SATELLITE FRACTIONNAL BIASES")) {
                block=1;
            }
            else if (block) {
                /* cnes/cls grg clock */
                if (!strncmp(buff,"WL",2)&&(sat=satid2no(buff+3))&&
                    sscanf(buff+40,"%lf",&bias)==1) {
                    nav->wlbias[sat-1]=bias;
                }
                /* cnes ppp-wizard clock */
                else if ((sat=satid2no(buff+1))&&sscanf(buff+6,"%lf",&bias)==1) {
                    nav->wlbias[sat-1]=bias;
                }
            }
            continue; 
        }
        /* file type */
        switch (*type) {
            case 'O': decode_obsh(fp,buff,*ver,tsys,tobs,nav,sta); break;
            case 'N': decode_navh (buff,nav); break;
            case 'G': decode_gnavh(buff,nav); break;
            case 'H': decode_hnavh(buff,nav); break;
            case 'J': decode_navh (buff,nav); break; /* extension */
            case 'L': decode_navh (buff,nav); break; /* extension */
        }
        if (strstr(label,"END OF HEADER")) return 1;
        
        if (++i>=MAXPOSHEAD&&*type==' ') break; /* no rinex file */
    }
    return 0;
}
/* decode obs epoch ----------------------------------------------------------*/
static int decode_obsepoch(FILE *fp, char *buff, double ver, gtime_t *time,
                           int *flag, int *sats)
{
    int i,j,n;
    char satid[8]="";
    
    trace(4,"decode_obsepoch: ver=%.2f\n",ver);
    
    if (ver<=2.99) { /* ver.2 */
        if ((n=(int)str2num(buff,29,3))<=0) return 0;
        
        /* epoch flag: 3:new site,4:header info,5:external event */
        *flag=(int)str2num(buff,28,1);
        
        if (3<=*flag&&*flag<=5) return n;
        
        if (str2time(buff,0,26,time)) {
            trace(2,"rinex obs invalid epoch: epoch=%26.26s\n",buff);
            return 0;
        }
        for (i=0,j=32;i<n;i++,j+=3) {
            if (j>=68) {
                if (!fgets(buff,MAXRNXLEN,fp)) break;
                j=32;
            }
            if (i<MAXOBS) {
                strncpy(satid,buff+j,3);
                sats[i]=satid2no(satid);
            }
        }
    }
    else { /* ver.3 */
        if ((n=(int)str2num(buff,32,3))<=0) return 0;
        
        *flag=(int)str2num(buff,31,1);
        
        if (3<=*flag&&*flag<=5) return n;
        
        if (buff[0]!='>'||str2time(buff,1,28,time)) {
            trace(2,"rinex obs invalid epoch: epoch=%29.29s\n",buff);
            return 0;
        }
    }
    trace(4,"decode_obsepoch: time=%s flag=%d\n",time_str(*time,3),*flag);
    return n;
}
/* decode obs data -----------------------------------------------------------*/
static int decode_obsdata(FILE *fp, char *buff, double ver, int mask,
                          sigind_t *index, obsd_t *obs)
{
    sigind_t *ind;
    double val[MAXOBSTYPE]={0};
    unsigned char lli[MAXOBSTYPE]={0};
    char satid[8]="";
    int i,j,n,m,stat=1,p[MAXOBSTYPE],k[16],l[16];
    
    trace(4,"decode_obsdata: ver=%.2f\n",ver);
    
    if (ver>2.99) { /* ver.3 */
        strncpy(satid,buff,3);
        obs->sat=(unsigned char)satid2no(satid);
    }
    if (!obs->sat) {
        trace(4,"decode_obsdata: unsupported sat sat=%s\n",satid);
        stat=0;
    }
    else if (!(satsys(obs->sat,NULL)&mask)) {
        stat=0;
    }
    /* read obs data fields */
    switch (satsys(obs->sat,NULL)) {
        case SYS_GLO: ind=index+1; break;
        case SYS_GAL: ind=index+2; break;
        case SYS_QZS: ind=index+3; break;
        case SYS_SBS: ind=index+4; break;
        case SYS_CMP: ind=index+5; break;
        default:      ind=index  ; break;
    }
    for (i=0,j=ver<=2.99?0:3;i<ind->n;i++,j+=16) {
        
        if (ver<=2.99&&j>=80) { /* ver.2 */
            if (!fgets(buff,MAXRNXLEN,fp)) break;
            j=0;
        }
        if (stat) {
            val[i]=str2num(buff,j,14)+ind->shift[i];
            lli[i]=(unsigned char)str2num(buff,j+14,1)&3;
        }
    }
    if (!stat) return 0;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        obs->P[i]=obs->L[i]=0.0; obs->D[i]=0.0f;
        obs->SNR[i]=obs->LLI[i]=obs->code[i]=0;
    }
    /* assign position in obs data */
    for (i=n=m=0;i<ind->n;i++) {
        
        p[i]=ver<=2.11?ind->frq[i]-1:ind->pos[i];
        
        if (ind->type[i]==0&&p[i]==0) k[n++]=i; /* C1? index */
        if (ind->type[i]==0&&p[i]==1) l[m++]=i; /* C2? index */
    }
    if (ver<=2.11) {
        
        /* if multiple codes (C1/P1,C2/P2), select higher priority */
        if (n>=2) {
            if (val[k[0]]==0.0&&val[k[1]]==0.0) {
                p[k[0]]=-1; p[k[1]]=-1;
            }
            else if (val[k[0]]!=0.0&&val[k[1]]==0.0) {
                p[k[0]]=0; p[k[1]]=-1;
            }
            else if (val[k[0]]==0.0&&val[k[1]]!=0.0) {
                p[k[0]]=-1; p[k[1]]=0;
            }
            else if (ind->pri[k[1]]>ind->pri[k[0]]) {
                p[k[1]]=0; p[k[0]]=NEXOBS<1?-1:NFREQ;
            }
            else {
                p[k[0]]=0; p[k[1]]=NEXOBS<1?-1:NFREQ;
            }
        }
        if (m>=2) {
            if (val[l[0]]==0.0&&val[l[1]]==0.0) {
                p[l[0]]=-1; p[l[1]]=-1;
            }
            else if (val[l[0]]!=0.0&&val[l[1]]==0.0) {
                p[l[0]]=1; p[l[1]]=-1;
            }
            else if (val[l[0]]==0.0&&val[l[1]]!=0.0) {
                p[l[0]]=-1; p[l[1]]=1; 
            }
            else if (ind->pri[l[1]]>ind->pri[l[0]]) {
                p[l[1]]=1; p[l[0]]=NEXOBS<2?-1:NFREQ+1;
            }
            else {
                p[l[0]]=1; p[l[1]]=NEXOBS<2?-1:NFREQ+1;
            }
        }
    }
    /* save obs data */
    for (i=0;i<ind->n;i++) {
        if (p[i]<0||val[i]==0.0) continue;
        switch (ind->type[i]) {
            case 0: obs->P[p[i]]=val[i]; obs->code[p[i]]=ind->code[i]; break;
            case 1: obs->L[p[i]]=val[i]; obs->LLI [p[i]]=lli[i];       break;
            case 2: obs->D[p[i]]=(float)val[i];                        break;
            case 3: obs->SNR[p[i]]=(unsigned char)(val[i]*4.0+0.5);    break;
        }
    }
    trace(4,"decode_obsdata: time=%s sat=%2d\n",time_str(obs->time,0),obs->sat);
    return 1;
}
/* save slips ----------------------------------------------------------------*/
static void saveslips(unsigned char slips[][NFREQ], obsd_t *data)
{
    int i;
    for (i=0;i<NFREQ;i++) {
        if (data->LLI[i]&1) slips[data->sat-1][i]|=1;
    }
}
/* restore slips -------------------------------------------------------------*/
static void restslips(unsigned char slips[][NFREQ], obsd_t *data)
{
    int i;
    for (i=0;i<NFREQ;i++) {
        if (slips[data->sat-1][i]&1) data->LLI[i]|=1;
        slips[data->sat-1][i]=0;
    }
}
/* add obs data --------------------------------------------------------------*/
static int addobsdata(obs_t *obs, const obsd_t *data)
{
    obsd_t *obs_data;
    
    if (obs->nmax<=obs->n) {
        if (obs->nmax<=0) obs->nmax=NINCOBS; else obs->nmax*=2;
        if (!(obs_data=(obsd_t *)realloc(obs->data,sizeof(obsd_t)*obs->nmax))) {
            trace(1,"addobsdata: memalloc error n=%dx%d\n",sizeof(obsd_t),obs->nmax);
            free(obs->data); obs->data=NULL; obs->n=obs->nmax=0;
            return -1;
        }
        obs->data=obs_data;
    }
    obs->data[obs->n++]=*data;
    return 1;
}
/* set system mask -----------------------------------------------------------*/
static int set_sysmask(const char *opt)
{
    const char *p;
    int mask=SYS_NONE;
    
    if (!(p=strstr(opt,"-SYS="))) return SYS_ALL;
    
    for (p+=5;*p&&*p!=' ';p++) {
        switch (*p) {
            case 'G': mask|=SYS_GPS; break;
            case 'R': mask|=SYS_GLO; break;
            case 'E': mask|=SYS_GAL; break;
            case 'J': mask|=SYS_QZS; break;
            case 'C': mask|=SYS_CMP; break;
            case 'S': mask|=SYS_SBS; break;
        }
    }
    return mask;
}
/* set signal index ----------------------------------------------------------*/
static void set_index(double ver, int sys, const char *opt,
                      char tobs[MAXOBSTYPE][4], sigind_t *ind)
{
    const char *p;
    char str[8],*optstr="";
    double shift;
    int i,j,k,n;
    
    for (i=n=0;*tobs[i];i++,n++) {
        ind->code[i]=obs2code(tobs[i]+1,ind->frq+i);
        ind->type[i]=(p=strchr(obscodes,tobs[i][0]))?(int)(p-obscodes):0;
        ind->pri[i]=getcodepri(sys,ind->code[i],opt);
        ind->pos[i]=-1;
        
        /* frequency index for beidou */
        if (sys==SYS_CMP) {
            if      (ind->frq[i]==5) ind->frq[i]=2; /* B2 */
            else if (ind->frq[i]==4) ind->frq[i]=3; /* B3 */
        }
    }
    /* parse phase shift options */
    switch (sys) {
        case SYS_GPS: optstr="-GL%2s=%lf"; break;
        case SYS_GLO: optstr="-RL%2s=%lf"; break;
        case SYS_GAL: optstr="-EL%2s=%lf"; break;
        case SYS_QZS: optstr="-JL%2s=%lf"; break;
        case SYS_SBS: optstr="-SL%2s=%lf"; break;
        case SYS_CMP: optstr="-CL%2s=%lf"; break;
    }
    for (p=opt;p&&(p=strchr(p,'-'));p++) {
        if (sscanf(p,optstr,str,&shift)<2) continue;
        for (i=0;i<n;i++) {
            if (strcmp(code2obs(ind->code[i],NULL),str)) continue;
            ind->shift[i]=shift;
            trace(2,"phase shift: sys=%2d tobs=%s shift=%.3f\n",sys,
                  tobs[i],shift);
        }
    }
    /* assign index for highest priority code */
    for (i=0;i<NFREQ;i++) {
        for (j=0,k=-1;j<n;j++) {
            if (ind->frq[j]==i+1&&ind->pri[j]&&(k<0||ind->pri[j]>ind->pri[k])) {
                k=j;
            }
        }
        if (k<0) continue;
        
        for (j=0;j<n;j++) {
            if (ind->code[j]==ind->code[k]) ind->pos[j]=i;
        }
    }
    /* assign index of extended obs data */
    for (i=0;i<NEXOBS;i++) {
        for (j=0;j<n;j++) {
            if (ind->code[j]&&ind->pri[j]&&ind->pos[j]<0) break;
        }
        if (j>=n) break;
        
        for (k=0;k<n;k++) {
            if (ind->code[k]==ind->code[j]) ind->pos[k]=NFREQ+i;
        }
    }
    for (i=0;i<n;i++) {
        if (!ind->code[i]||!ind->pri[i]||ind->pos[i]>=0) continue;
        trace(4,"reject obs type: sys=%2d, obs=%s\n",sys,tobs[i]);
    }
    ind->n=n;
    
#if 0 /* for debug */
    for (i=0;i<n;i++) {
        trace(2,"set_index: sys=%2d,tobs=%s code=%2d pri=%2d frq=%d pos=%d shift=%5.2f\n",
              sys,tobs[i],ind->code[i],ind->pri[i],ind->frq[i],ind->pos[i],
              ind->shift[i]);
    }
#endif
}
/* read rinex obs data body --------------------------------------------------*/
static int readrnxobsb(FILE *fp, const char *opt, double ver,
                       char tobs[][MAXOBSTYPE][4], int *flag, obsd_t *data)
{
    gtime_t time={0};
    sigind_t index[6]={{0}};
    char buff[MAXRNXLEN];
    int i=0,n=0,nsat=0,sats[MAXOBS]={0},mask;
    
    /* set system mask */
    mask=set_sysmask(opt);
    
    /* set signal index */
    set_index(ver,SYS_GPS,opt,tobs[0],index  );
    set_index(ver,SYS_GLO,opt,tobs[1],index+1);
    set_index(ver,SYS_GAL,opt,tobs[2],index+2);
    set_index(ver,SYS_QZS,opt,tobs[3],index+3);
    set_index(ver,SYS_SBS,opt,tobs[4],index+4);
    set_index(ver,SYS_CMP,opt,tobs[5],index+5);
    
    /* read record */
    while (fgets(buff,MAXRNXLEN,fp)) {
        
        /* decode obs epoch */
        if (i==0) {
            if ((nsat=decode_obsepoch(fp,buff,ver,&time,flag,sats))<=0) {
                continue;
            }
        }
        else if (*flag<=2||*flag==6) {
            
            data[n].time=time;
            data[n].sat=(unsigned char)sats[i-1];
            
            /* decode obs data */
            if (decode_obsdata(fp,buff,ver,mask,index,data+n)&&n<MAXOBS) n++;
        }
        if (++i>nsat) return n;
    }
    return -1;
}
/* read rinex obs ------------------------------------------------------------*/
static int readrnxobs(FILE *fp, gtime_t ts, gtime_t te, double tint,
                      const char *opt, int rcv, double ver, int tsys,
                      char tobs[][MAXOBSTYPE][4], obs_t *obs)
{
    obsd_t *data;
    unsigned char slips[MAXSAT][NFREQ]={{0}};
    int i,n,flag=0,stat=0;
    
    trace(4,"readrnxobs: rcv=%d ver=%.2f tsys=%d\n",rcv,ver,tsys);
    
    if (!obs||rcv>MAXRCV) return 0;
    
    if (!(data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))) return 0;
    
    /* read rinex obs data body */
    while ((n=readrnxobsb(fp,opt,ver,tobs,&flag,data))>=0&&stat>=0) {
        
        for (i=0;i<n;i++) {
            
            /* utc -> gpst */
            if (tsys==TSYS_UTC) data[i].time=utc2gpst(data[i].time);
            
            /* save cycle-slip */
            saveslips(slips,data+i);
        }
        /* screen data by time */
        if (n>0&&!screent(data[0].time,ts,te,tint)) continue;
        
        for (i=0;i<n;i++) {
            
            /* restore cycle-slip */
            restslips(slips,data+i);
            
            data[i].rcv=(unsigned char)rcv;
            
            /* save obs data */
            if ((stat=addobsdata(obs,data+i))<0) break;
        }
    }
    trace(4,"readrnxobs: nobs=%d stat=%d\n",obs->n,stat);
    
    free(data);
    
    return stat;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_eph(double ver, int sat, gtime_t toc, const double *data,
                      eph_t *eph)
{
    eph_t eph0={0};
    int sys;
    
    trace(4,"decode_eph: ver=%.2f sat=%2d\n",ver,sat);
    
    sys=satsys(sat,NULL);
    
    if (!(sys&(SYS_GPS|SYS_GAL|SYS_QZS|SYS_CMP))) {
        trace(2,"ephemeris error: invalid satellite sat=%2d\n",sat);
        return 0;
    }
    *eph=eph0;
    
    eph->sat=sat;
    eph->toc=toc;
    
    eph->f0=data[0];
    eph->f1=data[1];
    eph->f2=data[2];
    
    eph->A=SQR(data[10]); eph->e=data[ 8]; eph->i0  =data[15]; eph->OMG0=data[13];
    eph->omg =data[17]; eph->M0 =data[ 6]; eph->deln=data[ 5]; eph->OMGd=data[18];
    eph->idot=data[19]; eph->crc=data[16]; eph->crs =data[ 4]; eph->cuc =data[ 7];
    eph->cus =data[ 9]; eph->cic=data[12]; eph->cis =data[14];
    
    if (sys==SYS_GPS||sys==SYS_QZS) {
        eph->iode=(int)data[ 3];      /* IODE */
        eph->iodc=(int)data[26];      /* IODC */
        eph->toes=     data[11];      /* toe (s) in gps week */
        eph->week=(int)data[21];      /* gps week */
        eph->toe=adjweek(gpst2time(eph->week,data[11]),toc);
        eph->ttr=adjweek(gpst2time(eph->week,data[27]),toc);
        
        eph->code=(int)data[20];      /* GPS: codes on L2 ch */
        eph->svh =(int)data[24];      /* sv health */
        eph->sva=uraindex(data[23]);  /* ura (m->index) */
        eph->flag=(int)data[22];      /* GPS: L2 P data flag */
        
        eph->tgd[0]=   data[25];      /* TGD */
        eph->fit   =   data[28];      /* fit interval */
    }
    else if (sys==SYS_GAL) { /* GAL ver.3 */
        eph->iode=(int)data[ 3];      /* IODnav */
        eph->toes=     data[11];      /* toe (s) in galileo week */
        eph->week=(int)data[21];      /* gal week = gps week */
        eph->toe=adjweek(gpst2time(eph->week,data[11]),toc);
        eph->ttr=adjweek(gpst2time(eph->week,data[27]),toc);
        
        eph->code=(int)data[20];      /* data sources */
                                      /* bit 0 set: I/NAV E1-B */
                                      /* bit 1 set: F/NAV E5a-I */
                                      /* bit 2 set: F/NAV E5b-I */
                                      /* bit 8 set: af0-af2 toc are for E5a.E1 */
                                      /* bit 9 set: af0-af2 toc are for E5b.E1 */
        eph->svh =(int)data[24];      /* sv health */
                                      /* bit     0: E1B DVS */
                                      /* bit   1-2: E1B HS */
                                      /* bit     3: E5a DVS */
                                      /* bit   4-5: E5a HS */
                                      /* bit     6: E5b DVS */
                                      /* bit   7-8: E5b HS */
        eph->sva =uraindex(data[23]); /* ura (m->index) */
        
        eph->tgd[0]=   data[25];      /* BGD E5a/E1 */
        eph->tgd[1]=   data[26];      /* BGD E5b/E1 */
    }
    else if (sys==SYS_CMP) { /* BeiDou v.3.02 */
        eph->toc=bdt2gpst(eph->toc);  /* bdt -> gpst */
        eph->iode=(int)data[ 3];      /* AODE */
        eph->iodc=(int)data[28];      /* AODC */
        eph->toes=     data[11];      /* toe (s) in bdt week */
        eph->week=(int)data[21];      /* bdt week */
        eph->toe=bdt2gpst(bdt2time(eph->week,data[11])); /* bdt -> gpst */
        eph->ttr=bdt2gpst(bdt2time(eph->week,data[27])); /* bdt -> gpst */
        eph->toe=adjweek(eph->toe,toc);
        eph->ttr=adjweek(eph->ttr,toc);
        
        eph->svh =(int)data[24];      /* satH1 */
        eph->sva=uraindex(data[23]);  /* ura (m->index) */
        
        eph->tgd[0]=   data[25];      /* TGD1 B1/B3 */
        eph->tgd[1]=   data[26];      /* TGD2 B2/B3 */
    }
    if (eph->iode<0||1023<eph->iode) {
        trace(2,"rinex nav invalid: sat=%2d iode=%d\n",sat,eph->iode);
    }
    if (eph->iodc<0||1023<eph->iodc) {
        trace(2,"rinex nav invalid: sat=%2d iodc=%d\n",sat,eph->iodc);
    }
    return 1;
}
/* decode glonass ephemeris --------------------------------------------------*/
static int decode_geph(double ver, int sat, gtime_t toc, double *data,
                       geph_t *geph)
{
    geph_t geph0={0};
    gtime_t tof;
    double tow,tod;
    int week,dow;
    
    trace(4,"decode_geph: ver=%.2f sat=%2d\n",ver,sat);
    
    if (satsys(sat,NULL)!=SYS_GLO) {
        trace(2,"glonass ephemeris error: invalid satellite sat=%2d\n",sat);
        return 0;
    }
    *geph=geph0;
    
    geph->sat=sat;
    
    /* toc rounded by 15 min in utc */
    tow=time2gpst(toc,&week);
    toc=gpst2time(week,floor((tow+450.0)/900.0)*900);
    dow=(int)floor(tow/86400.0);
    
    /* time of frame in utc */
    tod=ver<=2.99?data[2]:fmod(data[2],86400.0); /* tod (v.2), tow (v.3) in utc */
    tof=gpst2time(week,tod+dow*86400.0);
    tof=adjday(tof,toc);
    
    geph->toe=utc2gpst(toc);   /* toc (gpst) */
    geph->tof=utc2gpst(tof);   /* tof (gpst) */
    
    /* iode = tb (7bit), tb =index of UTC+3H within current day */
    geph->iode=(int)(fmod(tow+10800.0,86400.0)/900.0+0.5);
    
    geph->taun=-data[0];       /* -taun */
    geph->gamn= data[1];       /* +gamman */
    
    geph->pos[0]=data[3]*1E3; geph->pos[1]=data[7]*1E3; geph->pos[2]=data[11]*1E3;
    geph->vel[0]=data[4]*1E3; geph->vel[1]=data[8]*1E3; geph->vel[2]=data[12]*1E3;
    geph->acc[0]=data[5]*1E3; geph->acc[1]=data[9]*1E3; geph->acc[2]=data[13]*1E3;
    
    geph->svh=(int)data[ 6];
    geph->frq=(int)data[10];
    geph->age=(int)data[14];
    
    /* some receiver output >128 for minus frequency number */
    if (geph->frq>128) geph->frq-=256;
    
    if (geph->frq<MINFREQ_GLO||MAXFREQ_GLO<geph->frq) {
        trace(2,"rinex gnav invalid freq: sat=%2d fn=%d\n",sat,geph->frq);
    }
    return 1;
}
/* decode geo ephemeris ------------------------------------------------------*/
static int decode_seph(double ver, int sat, gtime_t toc, double *data,
                       seph_t *seph)
{
    seph_t seph0={0};
    int week;
    
    trace(4,"decode_seph: ver=%.2f sat=%2d\n",ver,sat);
    
    if (satsys(sat,NULL)!=SYS_SBS) {
        trace(2,"geo ephemeris error: invalid satellite sat=%2d\n",sat);
        return 0;
    }
    *seph=seph0;
    
    seph->sat=sat;
    seph->t0 =toc;
    
    time2gpst(toc,&week);
    seph->tof=adjweek(gpst2time(week,data[2]),toc);
    
    seph->af0=data[0];
    seph->af1=data[1];
    
    seph->pos[0]=data[3]*1E3; seph->pos[1]=data[7]*1E3; seph->pos[2]=data[11]*1E3;
    seph->vel[0]=data[4]*1E3; seph->vel[1]=data[8]*1E3; seph->vel[2]=data[12]*1E3;
    seph->acc[0]=data[5]*1E3; seph->acc[1]=data[9]*1E3; seph->acc[2]=data[13]*1E3;
    
    seph->svh=(int)data[6];
    seph->sva=uraindex(data[10]);
    
    return 1;
}
/* read rinex navigation data body -------------------------------------------*/
static int readrnxnavb(FILE *fp, const char *opt, double ver, int sys,
                       int *type, eph_t *eph, geph_t *geph, seph_t *seph)
{
    gtime_t toc;
    double data[64];
    int i=0,j,prn,sat=0,sp=3,mask;
    char buff[MAXRNXLEN],id[8]="",*p;
    
    trace(4,"readrnxnavb: ver=%.2f sys=%d\n",ver,sys);
    
    /* set system mask */
    mask=set_sysmask(opt);
    
    while (fgets(buff,MAXRNXLEN,fp)) {
        
        if (i==0) {
            
            /* decode satellite field */
            if (ver>=3.0||sys==SYS_GAL||sys==SYS_QZS) { /* ver.3 or GAL/QZS */
                strncpy(id,buff,3);
                sat=satid2no(id);
                sp=4;
                if (ver>=3.0) sys=satsys(sat,NULL);
            }
            else {
                prn=(int)str2num(buff,0,2);
                
                if (sys==SYS_SBS) {
                    sat=satno(SYS_SBS,prn+100);
                }
                else if (sys==SYS_GLO) {
                    sat=satno(SYS_GLO,prn);
                }
                else if (93<=prn&&prn<=97) { /* extension */
                    sat=satno(SYS_QZS,prn+100);
                }
                else sat=satno(SYS_GPS,prn);
            }
            /* decode toc field */
            if (str2time(buff+sp,0,19,&toc)) {
                trace(2,"rinex nav toc error: %23.23s\n",buff);
                return 0;
            }
            /* decode data fields */
            for (j=0,p=buff+sp+19;j<3;j++,p+=19) {
                data[i++]=str2num(p,0,19);
            }
        }
        else {
            /* decode data fields */
            for (j=0,p=buff+sp;j<4;j++,p+=19) {
                data[i++]=str2num(p,0,19);
            }
            /* decode ephemeris */
            if (sys==SYS_GLO&&i>=15) {
                if (!(mask&sys)) return 0;
                *type=1;
                return decode_geph(ver,sat,toc,data,geph);
            }
            else if (sys==SYS_SBS&&i>=15) {
                if (!(mask&sys)) return 0;
                *type=2;
                return decode_seph(ver,sat,toc,data,seph);
            }
            else if (i>=31) {
                if (!(mask&sys)) return 0;
                *type=0;
                return decode_eph(ver,sat,toc,data,eph);
            }
        }
    }
    return -1;
}
/* add ephemeris to navigation data ------------------------------------------*/
static int add_eph(nav_t *nav, const eph_t *eph)
{
    eph_t *nav_eph;
    
    if (nav->nmax<=nav->n) {
        nav->nmax+=1024;
        if (!(nav_eph=(eph_t *)realloc(nav->eph,sizeof(eph_t)*nav->nmax))) {
            trace(1,"decode_eph malloc error: n=%d\n",nav->nmax);
            free(nav->eph); nav->eph=NULL; nav->n=nav->nmax=0;
            return 0;
        }
        nav->eph=nav_eph;
    }
    nav->eph[nav->n++]=*eph;
    return 1;
}
static int add_geph(nav_t *nav, const geph_t *geph)
{
    geph_t *nav_geph;
    
    if (nav->ngmax<=nav->ng) {
        nav->ngmax+=1024;
        if (!(nav_geph=(geph_t *)realloc(nav->geph,sizeof(geph_t)*nav->ngmax))) {
            trace(1,"decode_geph malloc error: n=%d\n",nav->ngmax);
            free(nav->geph); nav->geph=NULL; nav->ng=nav->ngmax=0;
            return 0;
        }
        nav->geph=nav_geph;
    }
    nav->geph[nav->ng++]=*geph;
    return 1;
}
static int add_seph(nav_t *nav, const seph_t *seph)
{
    seph_t *nav_seph;
    
    if (nav->nsmax<=nav->ns) {
        nav->nsmax+=1024;
        if (!(nav_seph=(seph_t *)realloc(nav->seph,sizeof(seph_t)*nav->nsmax))) {
            trace(1,"decode_seph malloc error: n=%d\n",nav->nsmax);
            free(nav->seph); nav->seph=NULL; nav->ns=nav->nsmax=0;
            return 0;
        }
        nav->seph=nav_seph;
    }
    nav->seph[nav->ns++]=*seph;
    return 1;
}
/* read rinex nav/gnav/geo nav -----------------------------------------------*/
static int readrnxnav(FILE *fp, const char *opt, double ver, int sys,
                      nav_t *nav)
{
    eph_t eph;
    geph_t geph;
    seph_t seph;
    int stat,type;
    
    trace(3,"readrnxnav: ver=%.2f sys=%d\n",ver,sys);
    
    if (!nav) return 0;
    
    /* read rinex navigation data body */
    while ((stat=readrnxnavb(fp,opt,ver,sys,&type,&eph,&geph,&seph))>=0) {
        
        /* add ephemeris to navigation data */
        if (stat) {
            switch (type) {
                case 1 : stat=add_geph(nav,&geph); break;
                case 2 : stat=add_seph(nav,&seph); break;
                default: stat=add_eph (nav,&eph ); break;
            }
            if (!stat) return 0;
        }
    }
    return nav->n>0||nav->ng>0||nav->ns>0;
}
/* read rinex clock ----------------------------------------------------------*/
static int readrnxclk(FILE *fp, const char *opt, int index, nav_t *nav)
{
    pclk_t *nav_pclk;
    gtime_t time;
    double data[2];
    int i,j,sat,mask;
    char buff[MAXRNXLEN],satid[8]="";
    
    trace(3,"readrnxclk: index=%d\n", index);
    
    if (!nav) return 0;
    
    /* set system mask */
    mask=set_sysmask(opt);
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (str2time(buff,8,26,&time)) {
            trace(2,"rinex clk invalid epoch: %34.34s\n",buff);
            continue;
        }
        strncpy(satid,buff+3,4);
        
        /* only read AS (satellite clock) record */
        if (strncmp(buff,"AS",2)||!(sat=satid2no(satid))) continue;
        
        if (!(satsys(sat,NULL)&mask)) continue;
        
        for (i=0,j=40;i<2;i++,j+=20) data[i]=str2num(buff,j,19);
        
        if (nav->nc>=nav->ncmax) {
            nav->ncmax+=1024;
            if (!(nav_pclk=(pclk_t *)realloc(nav->pclk,sizeof(pclk_t)*(nav->ncmax)))) {
                trace(1,"readrnxclk malloc error: nmax=%d\n",nav->ncmax);
                free(nav->pclk); nav->pclk=NULL; nav->nc=nav->ncmax=0;
                return -1;
            }
            nav->pclk=nav_pclk;
        }
        if (nav->nc<=0||fabs(timediff(time,nav->pclk[nav->nc-1].time))>1E-9) {
            nav->nc++;
            nav->pclk[nav->nc-1].time =time;
            nav->pclk[nav->nc-1].index=index;
            for (i=0;i<MAXSAT;i++) {
                nav->pclk[nav->nc-1].clk[i][0]=0.0;
                nav->pclk[nav->nc-1].std[i][0]=0.0f;
            }
        }
        nav->pclk[nav->nc-1].clk[sat-1][0]=data[0];
        nav->pclk[nav->nc-1].std[sat-1][0]=(float)data[1];
    }
    return nav->nc>0;
}
/* read rinex file -----------------------------------------------------------*/
static int readrnxfp(FILE *fp, gtime_t ts, gtime_t te, double tint,
                     const char *opt, int flag, int index, char *type,
                     obs_t *obs, nav_t *nav, sta_t *sta)
{
    double ver;
    int sys,tsys;
    char tobs[NUMSYS][MAXOBSTYPE][4]={{""}};
    
    trace(3,"readrnxfp: flag=%d index=%d\n",flag,index);
    
    /* read rinex header */
    if (!readrnxh(fp,&ver,type,&sys,&tsys,tobs,nav,sta)) return 0;
    
    /* flag=0:except for clock,1:clock */
    if ((!flag&&*type=='C')||(flag&&*type!='C')) return 0;
    
    /* read rinex body */
    switch (*type) {
        case 'O': return readrnxobs(fp,ts,te,tint,opt,index,ver,tsys,tobs,obs);
        case 'N': return readrnxnav(fp,opt,ver,sys    ,nav);
        case 'G': return readrnxnav(fp,opt,ver,SYS_GLO,nav);
        case 'H': return readrnxnav(fp,opt,ver,SYS_SBS,nav);
        case 'J': return readrnxnav(fp,opt,ver,SYS_QZS,nav); /* extension */
        case 'L': return readrnxnav(fp,opt,ver,SYS_GAL,nav); /* extension */
        case 'C': return readrnxclk(fp,opt,index,nav);
    }
    trace(2,"unsupported rinex type ver=%.2f type=%c\n",ver,*type);
    return 0;
}
/* uncompress and read rinex file --------------------------------------------*/
static int readrnxfile(const char *file, gtime_t ts, gtime_t te, double tint,
                       const char *opt, int flag, int index, char *type,
                       obs_t *obs, nav_t *nav, sta_t *sta)
{
    FILE *fp;
    int cstat,stat;
    char tmpfile[1024];
    
    trace(3,"readrnxfile: file=%s flag=%d index=%d\n",file,flag,index);
    
    if (sta) init_sta(sta);
    
    /* uncompress file */
    if ((cstat=uncompress(file,tmpfile))<0) {
        trace(2,"rinex file uncompact error: %s\n",file);
        return 0;
    }
    if (!(fp=fopen(cstat?tmpfile:file,"r"))) {
        trace(2,"rinex file open error: %s\n",cstat?tmpfile:file);
        return 0;
    }
    /* read rinex file */
    stat=readrnxfp(fp,ts,te,tint,opt,flag,index,type,obs,nav,sta);
    
    fclose(fp);
    
    /* delete temporary file */
    if (cstat) remove(tmpfile);
    
    return stat;
}
/* read rinex obs and nav files ------------------------------------------------
* read rinex obs and nav files
* args   : char *file    I      file (wild-card * expanded) ("": stdin)
*          int   rcv     I      receiver number for obs data
*         (gtime_t ts)   I      observation time start (ts.time==0: no limit)
*         (gtime_t te)   I      observation time end   (te.time==0: no limit)
*         (double tint)  I      observation time interval (s) (0:all)
*          char  *opt    I      rinex options (see below,"": no option)
*          obs_t *obs    IO     observation data   (NULL: no input)
*          nav_t *nav    IO     navigation data    (NULL: no input)
*          sta_t *sta    IO     station parameters (NULL: no input)
* return : status (1:ok,0:no data,-1:error)
* notes  : read data are appended to obs and nav struct
*          before calling the function, obs and nav should be initialized.
*          observation data and navigation data are not sorted.
*          navigation data may be duplicated.
*          call sortobs() or uniqnav() to sort data or delete duplicated eph.
*
*          read rinex options (separated by spaces) :
*
*            -GLss[=shift]: select GPS signal ss (ss: RINEX 3 code, "1C","2W"...)
*            -RLss[=shift]: select GLO signal ss
*            -ELss[=shift]: select GAL signal ss
*            -JLss[=shift]: select QZS signal ss
*            -CLss[=shift]: select BDS signal ss
*            -SLss[=shift]: select SBS signal ss
*
*                 shift: carrier phase shift to be added (cycle)
*            
*            -SYS=sys[,sys...]: select navi systems
*                               (sys=G:GPS,R:GLO,E:GAL,J:QZS,C:BDS,S:SBS)
*
*-----------------------------------------------------------------------------*/
extern int readrnxt(const char *file, int rcv, gtime_t ts, gtime_t te,
                    double tint, const char *opt, obs_t *obs, nav_t *nav,
                    sta_t *sta)
{
    int i,n,stat=0;
    const char *p;
    char type=' ',*files[MAXEXFILE]={0};
    
    trace(3,"readrnxt: file=%s rcv=%d\n",file,rcv);
    
    if (!*file) {
        return readrnxfp(stdin,ts,te,tint,opt,0,1,&type,obs,nav,sta);
    }
    for (i=0;i<MAXEXFILE;i++) {
        if (!(files[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(files[i]);
            return -1;
        }
    }
    /* expand wild-card */
    if ((n=expath(file,files,MAXEXFILE))<=0) {
        for (i=0;i<MAXEXFILE;i++) free(files[i]);
        return 0;
    }
    /* read rinex files */
    for (i=0;i<n&&stat>=0;i++) {
        stat=readrnxfile(files[i],ts,te,tint,opt,0,rcv,&type,obs,nav,sta);
    }
    /* if station name empty, set 4-char name from file head */
    if (type=='O'&&sta) {
        if (!(p=strrchr(file,FILEPATHSEP))) p=file-1;
        if (!*sta->name) setstr(sta->name,p+1,4);
    }
    for (i=0;i<MAXEXFILE;i++) free(files[i]);
    
    return stat;
}
extern int readrnx(const char *file, int rcv, const char *opt, obs_t *obs,
                   nav_t *nav, sta_t *sta)
{
    gtime_t t={0};
    
    trace(3,"readrnx : file=%s rcv=%d\n",file,rcv);
    
    return readrnxt(file,rcv,t,t,0.0,opt,obs,nav,sta);
}
/* compare precise clock -----------------------------------------------------*/
static int cmppclk(const void *p1, const void *p2)
{
    pclk_t *q1=(pclk_t *)p1,*q2=(pclk_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-1E-9?-1:(tt>1E-9?1:q1->index-q2->index);
}
/* combine precise clock -----------------------------------------------------*/
static void combpclk(nav_t *nav)
{
    pclk_t *nav_pclk;
    int i,j,k;
    
    trace(3,"combpclk: nc=%d\n",nav->nc);
    
    if (nav->nc<=0) return;
    
    qsort(nav->pclk,nav->nc,sizeof(pclk_t),cmppclk);
    
    for (i=0,j=1;j<nav->nc;j++) {
        if (fabs(timediff(nav->pclk[i].time,nav->pclk[j].time))<1E-9) {
            for (k=0;k<MAXSAT;k++) {
                if (nav->pclk[j].clk[k][0]==0.0) continue;
                nav->pclk[i].clk[k][0]=nav->pclk[j].clk[k][0];
                nav->pclk[i].std[k][0]=nav->pclk[j].std[k][0];
            }
        }
        else if (++i<j) nav->pclk[i]=nav->pclk[j];
    }
    nav->nc=i+1;
    
    if (!(nav_pclk=(pclk_t *)realloc(nav->pclk,sizeof(pclk_t)*nav->nc))) {
        free(nav->pclk); nav->pclk=NULL; nav->nc=nav->ncmax=0;
        trace(1,"combpclk malloc error nc=%d\n",nav->nc);
        return;
    }
    nav->pclk=nav_pclk;
    nav->ncmax=nav->nc;
    
    trace(4,"combpclk: nc=%d\n",nav->nc);
}
/* read rinex clock files ------------------------------------------------------
* read rinex clock files
* args   : char *file    I      file (wild-card * expanded)
*          nav_t *nav    IO     navigation data    (NULL: no input)
* return : number of precise clock
*-----------------------------------------------------------------------------*/
extern int readrnxc(const char *file, nav_t *nav)
{
    gtime_t t={0};
    int i,n,index=0,stat=1;
    char *files[MAXEXFILE]={0},type;
    
    trace(3,"readrnxc: file=%s\n",file);
    
    for (i=0;i<MAXEXFILE;i++) {
        if (!(files[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(files[i]); return 0;
        }
    }
    /* expand wild-card */
    n=expath(file,files,MAXEXFILE);
    
    /* read rinex clock files */
    for (i=0;i<n;i++) {
        if (readrnxfile(files[i],t,t,0.0,"",1,index++,&type,NULL,nav,NULL)) {
            continue;
        }
        stat=0;
        break;
    }
    for (i=0;i<MAXEXFILE;i++) free(files[i]);
    
    if (!stat) return 0;
    
    /* unique and combine ephemeris and precise clock */
    combpclk(nav);
    
    return nav->nc;
}
/* initialize rinex control ----------------------------------------------------
* initialize rinex control struct and reallocate memory for observation and
* ephemeris buffer in rinex control struct
* args   : rnxctr_t *rnx IO     rinex control struct
* return : status (1:ok,0:memory allocation error)
*-----------------------------------------------------------------------------*/
extern int init_rnxctr(rnxctr_t *rnx)
{
    gtime_t time0={0};
    obsd_t data0={{0}};
    eph_t  eph0={0,-1,-1};
    geph_t geph0={0,-1};
    seph_t seph0={0};
    int i,j;
    
    trace(3,"init_rnxctr:\n");
    
    rnx->obs.data=NULL;
    rnx->nav.eph =NULL;
    rnx->nav.geph=NULL;
    rnx->nav.seph=NULL;
    
    if (!(rnx->obs.data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS ))||
        !(rnx->nav.eph =(eph_t  *)malloc(sizeof(eph_t )*MAXSAT ))||
        !(rnx->nav.geph=(geph_t *)malloc(sizeof(geph_t)*NSATGLO))||
        !(rnx->nav.seph=(seph_t *)malloc(sizeof(seph_t)*NSATSBS))) {
        free_rnxctr(rnx);
        return 0;
    }
    rnx->time=time0;
    rnx->ver=0.0;
    rnx->sys=rnx->tsys=0;
    for (i=0;i<6;i++) for (j=0;j<MAXOBSTYPE;j++) rnx->tobs[i][j][0]='\0';
    rnx->obs.n=0;
    rnx->nav.n=MAXSAT;
    rnx->nav.ng=NSATGLO;
    rnx->nav.ns=NSATSBS;
    for (i=0;i<MAXOBS ;i++) rnx->obs.data[i]=data0;
    for (i=0;i<MAXSAT ;i++) rnx->nav.eph [i]=eph0;
    for (i=0;i<NSATGLO;i++) rnx->nav.geph[i]=geph0;
    for (i=0;i<NSATSBS;i++) rnx->nav.seph[i]=seph0;
    rnx->ephsat=0;
    rnx->opt[0]='\0';
    
    return 1;
}
/* free rinex control ----------------------------------------------------------
* free observation and ephemris buffer in rinex control struct
* args   : rnxctr_t *rnx IO  rinex control struct
* return : none
*-----------------------------------------------------------------------------*/
extern void free_rnxctr(rnxctr_t *rnx)
{
    trace(3,"free_rnxctr:\n");
    
    free(rnx->obs.data); rnx->obs.data=NULL; rnx->obs.n =0;
    free(rnx->nav.eph ); rnx->nav.eph =NULL; rnx->nav.n =0;
    free(rnx->nav.geph); rnx->nav.geph=NULL; rnx->nav.ng=0;
    free(rnx->nav.seph); rnx->nav.seph=NULL; rnx->nav.ns=0;
}
/* open rinex data -------------------------------------------------------------
* fetch next rinex message and input a messsage from file
* args   : rnxctr_t *rnx IO  rinex control struct
*          FILE  *fp    I    file pointer
* return : status (-2: end of file, 0: no message, 1: input observation data,
*                   2: input navigation data)
*-----------------------------------------------------------------------------*/
extern int open_rnxctr(rnxctr_t *rnx, FILE *fp)
{
    const char *rnxtypes="ONGLJHC";
    double ver;
    char type,tobs[6][MAXOBSTYPE][4]={{""}};
    int i,j,sys,tsys;
    
    trace(3,"open_rnxctr:\n");
    
    /* read rinex header from file */
    if (!readrnxh(fp,&ver,&type,&sys,&tsys,tobs,&rnx->nav,&rnx->sta)) {
        trace(2,"open_rnxctr: rinex header read error\n");
        return 0;
    }
    if (!strchr(rnxtypes,type)) {
        trace(2,"open_rnxctr: not supported rinex type type=%c\n",type);
        return 0;
    }
    rnx->ver=ver;
    rnx->type=type;
    rnx->sys=sys;
    rnx->tsys=tsys;
    for (i=0;i<6;i++) for (j=0;j<MAXOBSTYPE&&*tobs[i][j];j++) {
        strcpy(rnx->tobs[i][j],tobs[i][j]);
    }
    rnx->ephsat=0;
    return 1;
}
/* input rinex control ---------------------------------------------------------
* fetch next rinex message and input a messsage from file
* args   : rnxctr_t *rnx IO  rinex control struct
*          FILE  *fp    I    file pointer
* return : status (-2: end of file, 0: no message, 1: input observation data,
*                   2: input navigation data)
*-----------------------------------------------------------------------------*/
extern int input_rnxctr(rnxctr_t *rnx, FILE *fp)
{
    eph_t eph={0};
    geph_t geph={0};
    seph_t seph={0};
    int n,sys,stat,flag,prn,type;
    
    trace(4,"input_rnxctr:\n");
    
    /* read rinex obs data */
    if (rnx->type=='O') {
        if ((n=readrnxobsb(fp,rnx->opt,rnx->ver,rnx->tobs,&flag,
                           rnx->obs.data))<=0) {
            rnx->obs.n=0;
            return n<0?-2:0;
        }
        rnx->time=rnx->obs.data[0].time;
        rnx->obs.n=n;
        return 1;
    }
    /* read rinex nav data */
    switch (rnx->type) {
        case 'N': sys=SYS_NONE; break;
        case 'G': sys=SYS_GLO ; break;
        case 'H': sys=SYS_SBS ; break;
        case 'L': sys=SYS_GAL ; break; /* extension */
        case 'J': sys=SYS_QZS ; break; /* extension */
        default: return 0;
    }
    if ((stat=readrnxnavb(fp,rnx->opt,rnx->ver,sys,&type,&eph,&geph,&seph))<=0) {
        return stat<0?-2:0;
    }
    if (type==1) {
        sys=satsys(geph.sat,&prn);
        rnx->nav.geph[prn-1]=geph;
        rnx->time=geph.tof;
        rnx->ephsat=geph.sat;
    }
    else if (type==2) {
        sys=satsys(seph.sat,&prn);
        rnx->nav.seph[prn-MINPRNSBS]=seph;
        rnx->time=seph.tof;
        rnx->ephsat=seph.sat;
    }
    else {
        rnx->nav.eph[eph.sat-1]=eph;
        rnx->time=eph.ttr;
        rnx->ephsat=eph.sat;
    }
    return 2;
}
/*------------------------------------------------------------------------------
* output rinex functions
*-----------------------------------------------------------------------------*/

/* output obs types ver.2 ----------------------------------------------------*/
static void outobstype_ver2(FILE *fp, const rnxopt_t *opt)
{
    const char label[]="# / TYPES OF OBSERV";
    int i;
    
    trace(3,"outobstype_ver2:\n");
    
    fprintf(fp,"%6d",opt->nobs[0]);
    
    for (i=0;i<opt->nobs[0];i++) {
        if (i>0&&i%9==0) fprintf(fp,"      ");
        
        fprintf(fp,"%6s",opt->tobs[0][i]);
        
        if (i%9==8) fprintf(fp,"%-20s\n",label);
    }
    if (opt->nobs[0]==0||i%9>0) {
        fprintf(fp,"%*s%-20s\n",(9-i%9)*6,"",label);
    }
}
/* output obs types ver.3 ----------------------------------------------------*/
static void outobstype_ver3(FILE *fp, const rnxopt_t *opt)
{
    const char label[]="SYS / # / OBS TYPES";
    int i,j;
    
    trace(3,"outobstype_ver3:\n");
    
    for (i=0;navsys[i];i++) {
        if (!(navsys[i]&opt->navsys)||!opt->nobs[i]) continue;
        
        fprintf(fp,"%c  %3d",syscodes[i],opt->nobs[i]);
        
        for (j=0;j<opt->nobs[i];j++) {
            if (j>0&&j%13==0) fprintf(fp,"      ");
            
            fprintf(fp," %3s",opt->tobs[i][j]);
            
            if (j%13==12) fprintf(fp,"  %-20s\n",label);
        }
        if (j%13>0) {
            fprintf(fp,"%*s  %-20s\n",(13-j%13)*4,"",label);
        }
    }
}
/* output rinex obs header -----------------------------------------------------
* output rinex obd file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  *nav      I   navigation data
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxobsh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    const char *glo_codes[]={"C1C","C1P","C2C","C2P"};
    double ep[6],pos[3]={0},del[3]={0};
    int i,j,k,n,prn[MAXPRNGLO];
    char date[32],*sys,*tsys="GPS";
    
    trace(3,"outrnxobsh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        sys=opt->navsys==SYS_GPS?"G (GPS)":"M (MIXED)";
    }
    else { /* ver.3 */
        if      (opt->navsys==SYS_GPS) sys="G: GPS";
        else if (opt->navsys==SYS_GLO) sys="R: GLONASS";
        else if (opt->navsys==SYS_GAL) sys="E: Galielo";
        else if (opt->navsys==SYS_QZS) sys="J: QZSS";   /* ver.3.02 */
        else if (opt->navsys==SYS_CMP) sys="C: BeiDou"; /* ver.3.02 */
        else if (opt->navsys==SYS_SBS) sys="S: SBAS Payload";
        else sys="M: Mixed";
    }
    fprintf(fp,"%9.2f%-11s%-20s%-20s%-20s\n",opt->rnxver,"","OBSERVATION DATA",
            sys,"RINEX VERSION / TYPE");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    fprintf(fp,"%-60.60s%-20s\n",opt->marker,"MARKER NAME");
    fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->markerno,"","MARKER NUMBER");
    
    if (opt->rnxver>2.99) {
        fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->markertype,"","MARKER TYPE");
    }
    fprintf(fp,"%-20.20s%-40.40s%-20s\n",opt->name[0],opt->name[1],
            "OBSERVER / AGENCY");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->rec[0],opt->rec[1],
            opt->rec[2],"REC # / TYPE / VERS");
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->ant[0],opt->ant[1],
            opt->ant[2],"ANT # / TYPE");
    
    for (i=0;i<3;i++) if (fabs(opt->apppos[i])<1E8) pos[i]=opt->apppos[i];
    for (i=0;i<3;i++) if (fabs(opt->antdel[i])<1E8) del[i]=opt->antdel[i];
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",pos[0],pos[1],pos[2],"",
            "APPROX POSITION XYZ");
    fprintf(fp,"%14.4f%14.4f%14.4f%-18s%-20s\n",del[0],del[1],del[2],"",
            "ANTENNA: DELTA H/E/N");
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%6d%6d%-48s%-20s\n",1,1,"","WAVELENGTH FACT L1/2");
        outobstype_ver2(fp,opt);
    }
    else { /* ver.3 */
        outobstype_ver3(fp,opt);
    }
    if (opt->tint>0.0) {
        fprintf(fp,"%10.3f%50s%-20s\n",opt->tint,"","INTERVAL");
    }
    time2epoch(opt->tstart,ep);
    fprintf(fp,"  %04.0f%6.0f%6.0f%6.0f%6.0f%13.7f     %-12s%-20s\n",ep[0],
            ep[1],ep[2],ep[3],ep[4],ep[5],tsys,"TIME OF FIRST OBS");
    
    time2epoch(opt->tend,ep);
    fprintf(fp,"  %04.0f%6.0f%6.0f%6.0f%6.0f%13.7f     %-12s%-20s\n",ep[0],
            ep[1],ep[2],ep[3],ep[4],ep[5],tsys,"TIME OF LAST OBS");
    
    if (opt->rnxver>=3.01) { /* ver.3.01 */
        for (i=0;navsys[i];i++) {
            if (!(navsys[i]&opt->navsys)||!opt->nobs[i]) continue;
            fprintf(fp,"%c %-58s%-20s\n",syscodes[i],"","SYS / PHASE SHIFT");
        }
    }
    if (opt->rnxver>=3.02) { /* ver.3.02 */
        for (i=n=0;i<MAXPRNGLO;i++) {
            if (nav->glo_fcn[i]>=1) prn[n++]=i+1;
        }
        for (i=j=0;i<(n<=0?1:(n-1)/8+1);i++) {
            if (i==0) fprintf(fp,"%3d",n); else fprintf(fp,"   ");
            for (k=0;k<8;k++,j++) {
                if (j<n) {
                    fprintf(fp," R%02d %2d",prn[j],nav->glo_fcn[prn[j]-1]-8);
                }
                else {
                    fprintf(fp," %6s","");
                }
            }
            fprintf(fp," %-20s\n","GLONASS SLOT / FRQ #");
        }
    }
    if (opt->rnxver>=3.02) { /* ver.3.02 */
        for (i=0;i<4;i++) fprintf(fp," %3s %8.3f",glo_codes[i],0.0);
        fprintf(fp,"%8s%-20s\n","","GLONASS COD/PHS/BIS");
    }
    return fprintf(fp,"%-60.60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output obs data field -----------------------------------------------------*/
static void outrnxobsf(FILE *fp, double obs, int lli)
{
    if (obs==0.0||obs<=-1E9||obs>=1E9) fprintf(fp,"              ");
    else fprintf(fp,"%14.3f",obs);
    if (lli<=0) fprintf(fp,"  "); else fprintf(fp,"%1.1d ",lli);
}
/* search obs data index -----------------------------------------------------*/
static int obsindex(double ver, int sys, const unsigned char *code,
                    const char *tobs, const char *mask)
{
    char *id;
    int i;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        
        /* signal mask */
        if (mask[code[i]-1]=='0') continue;
        
        if (ver<=2.99) { /* ver.2 */
            if (!strcmp(tobs,"C1")&&(sys==SYS_GPS||sys==SYS_GLO||sys==SYS_QZS||
                sys==SYS_SBS||sys==SYS_CMP)) {
                if (code[i]==CODE_L1C) return i;
            }
            else if (!strcmp(tobs,"P1")) {
                if (code[i]==CODE_L1P||code[i]==CODE_L1W||code[i]==CODE_L1Y||
                    code[i]==CODE_L1N) return i;
            }
            else if (!strcmp(tobs,"C2")&&(sys==SYS_GPS||sys==SYS_QZS)) {
                if (code[i]==CODE_L2S||code[i]==CODE_L2L||code[i]==CODE_L2X)
                    return i;
            }
            else if (!strcmp(tobs,"C2")&&sys==SYS_GLO) {
                if (code[i]==CODE_L2C) return i;
            }
            else if (!strcmp(tobs,"P2")) {
                if (code[i]==CODE_L2P||code[i]==CODE_L2W||code[i]==CODE_L2Y||
                    code[i]==CODE_L2N||code[i]==CODE_L2D) return i;
            }
            else if (ver>=2.12&&tobs[1]=='A') { /* L1C/A */
                if (code[i]==CODE_L1C) return i;
            }
            else if (ver>=2.12&&tobs[1]=='B') { /* L1C */
                if (code[i]==CODE_L1S||code[i]==CODE_L1L||code[i]==CODE_L1X)
                    return i;
            }
            else if (ver>=2.12&&tobs[1]=='C') { /* L2C */
                if (code[i]==CODE_L2S||code[i]==CODE_L2L||code[i]==CODE_L2X)
                    return i;
            }
            else if (ver>=2.12&&tobs[1]=='D'&&sys==SYS_GLO) { /* GLO L2C/A */
                if (code[i]==CODE_L2C) return i;
            }
            else if (tobs[1]=='2'&&sys==SYS_CMP) { /* BDS B1 */
                if (code[i]==CODE_L1I||code[i]==CODE_L1Q||code[i]==CODE_L1X)
                    return i;
            }
            else {
                id=code2obs(code[i],NULL);
                if (id[0]==tobs[1]) return i;
            }
        }
        else { /* ver.3 */
            id=code2obs(code[i],NULL);
            if (!strcmp(id,tobs+1)) return i;
        }
    }
    return -1;
}
/* output rinex obs body -------------------------------------------------------
* output rinex obs body
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          obsd_t *obs      I   observation data
*          int    n         I   number of observation data
*          int    flag      I   epoch flag (0:ok,1:power failure,>1:event flag)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxobsb(FILE *fp, const rnxopt_t *opt, const obsd_t *obs, int n,
                      int flag)
{
    const char *mask;
    double ep[6];
    char sats[MAXOBS][4]={""};
    int i,j,k,m,ns,sys,ind[MAXOBS],s[MAXOBS]={0};
    
    trace(3,"outrnxobsb: n=%d\n",n);
    
    time2epoch(obs[0].time,ep);
    
    for (i=ns=0;i<n&&ns<MAXOBS;i++) {
        sys=satsys(obs[i].sat,NULL);
        if (!(sys&opt->navsys)||opt->exsats[obs[i].sat-1]) continue;
        if (!sat2code(obs[i].sat,sats[ns])) continue;
        switch (sys) {
            case SYS_GPS: s[ns]=0; break;
            case SYS_GLO: s[ns]=1; break;
            case SYS_GAL: s[ns]=2; break;
            case SYS_QZS: s[ns]=3; break;
            case SYS_SBS: s[ns]=4; break;
            case SYS_CMP: s[ns]=5; break;
        }
        if (!opt->nobs[opt->rnxver<=2.99?0:s[ns]]) continue;
        ind[ns++]=i;
    }
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp," %02d %2.0f %2.0f %2.0f %2.0f%11.7f  %d%3d",
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5],flag,ns);
        for (i=0;i<ns;i++) {
            if (i>0&&i%12==0) fprintf(fp,"\n%32s","");
            fprintf(fp,"%-3s",sats[i]);
        }
    }
    else { /* ver.3 */
        fprintf(fp,"> %04.0f %2.0f %2.0f %2.0f %2.0f%11.7f  %d%3d%21s\n",
                ep[0],ep[1],ep[2],ep[3],ep[4],ep[5],flag,ns,"");
    }
    for (i=0;i<ns;i++) {
        sys=satsys(obs[ind[i]].sat,NULL);
        
        if (opt->rnxver<=2.99) { /* ver.2 */
            m=0;
            mask=opt->mask[s[i]];
        }
        else { /* ver.3 */
            fprintf(fp,"%-3s",sats[i]);
            m=s[i];
            mask=opt->mask[s[i]];
        }
        for (j=0;j<opt->nobs[m];j++) {
            
            if (opt->rnxver<=2.99) { /* ver.2 */
                if (j%5==0) fprintf(fp,"\n");
            }
            /* search obs data index */
            if ((k=obsindex(opt->rnxver,sys,obs[ind[i]].code,opt->tobs[m][j],
                            mask))<0) {
                outrnxobsf(fp,0.0,-1);
                continue;
            }
            /* output field */
            switch (opt->tobs[m][j][0]) {
                case 'C':
                case 'P': outrnxobsf(fp,obs[ind[i]].P[k],-1); break;
                case 'L': outrnxobsf(fp,obs[ind[i]].L[k],obs[ind[i]].LLI[k]); break;
                case 'D': outrnxobsf(fp,obs[ind[i]].D[k],-1); break;
                case 'S': outrnxobsf(fp,obs[ind[i]].SNR[k]*0.25,-1); break;
            }
        }
        if (opt->rnxver>2.99&&fprintf(fp,"\n")==EOF) return 0;
    }
    if (opt->rnxver>2.99) return 1;
    
    return fprintf(fp,"\n")!=EOF;
}
/* output nav member by rinex nav format -------------------------------------*/
static void outnavf(FILE *fp, double value)
{
    double e=fabs(value)<1E-99?0.0:floor(log10(fabs(value))+1.0);
    fprintf(fp," %s.%012.0fE%+03.0f",value<0.0?"-":" ",fabs(value)/pow(10.0,e-12.0),e);
}
/* output rinex nav header -----------------------------------------------------
* output rinex nav file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64],*sys;
    
    trace(3,"outrnxnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "N: GPS NAV DATA","","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        if      (opt->navsys==SYS_GPS) sys="G: GPS";
        else if (opt->navsys==SYS_GLO) sys="R: GLONASS";
        else if (opt->navsys==SYS_GAL) sys="E: Galileo";
        else if (opt->navsys==SYS_QZS) sys="J: QZSS";   /* v.3.02 */
        else if (opt->navsys==SYS_CMP) sys="C: BeiDou"; /* v.3.02 */
        else if (opt->navsys==SYS_SBS) sys="S: SBAS Payload";
        else sys="M: Mixed";
        
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "N: GNSS NAV DATA",sys,"RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    if (opt->rnxver<=2.99) { /* ver.2 */
        if (opt->outiono) {
            fprintf(fp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",nav->ion_gps[0],
                    nav->ion_gps[1],nav->ion_gps[2],nav->ion_gps[3],"","ION ALPHA");
            fprintf(fp,"  %12.4E%12.4E%12.4E%12.4E%10s%-20s\n",nav->ion_gps[4],
                    nav->ion_gps[5],nav->ion_gps[6],nav->ion_gps[7],"","ION BETA");
        }
        if (opt->outtime) {
            fprintf(fp,"   ");
            outnavf(fp,nav->utc_gps[0]);
            outnavf(fp,nav->utc_gps[1]);
            fprintf(fp,"%9.0f%9.0f %-20s\n",nav->utc_gps[2],nav->utc_gps[3],
                    "DELTA-UTC: A0,A1,T,W");
        }
    }
    else { /* ver.3 */
        if (opt->navsys&SYS_GPS) {
            if (opt->outiono) {
                fprintf(fp,"GPSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",
                        nav->ion_gps[0],nav->ion_gps[1],nav->ion_gps[2],
                        nav->ion_gps[3],"","IONOSPHERIC CORR");
                fprintf(fp,"GPSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",
                        nav->ion_gps[4],nav->ion_gps[5],nav->ion_gps[6],
                        nav->ion_gps[7],"","IONOSPHERIC CORR");
            }
        }
        if (opt->navsys&SYS_GAL) {
            if (opt->outiono) {
                fprintf(fp,"GAL  %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",
                        nav->ion_gal[0],nav->ion_gal[1],nav->ion_gal[2],0.0,"",
                        "IONOSPHERIC CORR");
            }
        }
        if (opt->navsys&SYS_QZS) {
            if (opt->outiono) {
                fprintf(fp,"QZSA %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",
                        nav->ion_qzs[0],nav->ion_qzs[1],nav->ion_qzs[2],
                        nav->ion_qzs[3],"","IONOSPHERIC CORR");
                fprintf(fp,"QZSB %12.4E%12.4E%12.4E%12.4E%7s%-20s\n",
                        nav->ion_qzs[4],nav->ion_qzs[5],nav->ion_qzs[6],
                        nav->ion_qzs[7],"","IONOSPHERIC CORR");
            }
        }
        if (opt->navsys&SYS_GPS) {
            if (opt->outtime) {
                fprintf(fp,"GPUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\n",
                        nav->utc_gps[0],nav->utc_gps[1],nav->utc_gps[2],
                        nav->utc_gps[3],"","","TIME SYSTEM CORR");
            }
        }
        if (opt->navsys&SYS_GAL) {
            if (opt->outtime) {
                fprintf(fp,"GAUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\n",
                        nav->utc_gal[0],nav->utc_gal[1],nav->utc_gal[2],
                        nav->utc_gal[3],"","","TIME SYSTEM CORR");
            }
        }
        if (opt->navsys&SYS_QZS) { /* ver.3.02 */
            if (opt->outtime) {
                fprintf(fp,"QZUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\n",
                        nav->utc_qzs[0],nav->utc_qzs[1],nav->utc_qzs[2],
                        nav->utc_qzs[3],"","","TIME SYSTEM CORR");
            }
        }
        if (opt->navsys&SYS_CMP) { /* ver.3.02 */
            if (opt->outtime) {
                fprintf(fp,"BDUT %17.10E%16.9E%7.0f%5.0f %-5s %-2s %-20s\n",
                        nav->utc_cmp[0],nav->utc_cmp[1],nav->utc_cmp[2],
                        nav->utc_cmp[3],"","","TIME SYSTEM CORR");
            }
        }
    }
    if (opt->outleaps) {
        fprintf(fp,"%6d%54s%-20s\n",nav->leaps,"","LEAP SECONDS");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex nav body -------------------------------------------------------
* output rinex nav file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          eph_t  *eph      I   ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxnavb(FILE *fp, const rnxopt_t *opt, const eph_t *eph)
{
    double ep[6],ttr;
    int week,sys,prn;
    char code[32],*sep;
    
    trace(3,"outrnxgnavb: sat=%2d\n",eph->sat);
    
    if (!(sys=satsys(eph->sat,&prn))||!(sys&opt->navsys)) return 0;
    
    if (sys!=SYS_CMP) {
        time2epoch(eph->toc,ep);
    }
    else {
        time2epoch(gpst2bdt(eph->toc),ep); /* gpst -> bdt */
    }
    if (opt->rnxver>2.99||sys==SYS_GAL||sys==SYS_CMP) { /* ver.3 or ver.2 GAL */
        if (!sat2code(eph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    else if (sys==SYS_QZS) { /* ver.2 or ver.3.02 QZS */
        if (!sat2code(eph->sat,code)) return 0;
        fprintf(fp,"%-3s %02d %2.0f %2.0f %2.0f %2.0f %4.1f",code,
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    else {
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn,
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    outnavf(fp,eph->f0     );
    outnavf(fp,eph->f1     );
    outnavf(fp,eph->f2     );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->iode   ); /* GPS/QZS: IODE, GAL: IODnav, BDS: AODE */
    outnavf(fp,eph->crs    );
    outnavf(fp,eph->deln   );
    outnavf(fp,eph->M0     );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->cuc    );
    outnavf(fp,eph->e      );
    outnavf(fp,eph->cus    );
    outnavf(fp,sqrt(eph->A));
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->toes   );
    outnavf(fp,eph->cic    );
    outnavf(fp,eph->OMG0   );
    outnavf(fp,eph->cis    );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->i0     );
    outnavf(fp,eph->crc    );
    outnavf(fp,eph->omg    );
    outnavf(fp,eph->OMGd   );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,eph->idot   );
    outnavf(fp,eph->code   );
    outnavf(fp,eph->week   ); /* GPS/QZS: GPS week, GAL: GAL week, BDS: BDT week */
    outnavf(fp,eph->flag   );
    fprintf(fp,"\n%s",sep  );
    
    outnavf(fp,uravalue(eph->sva));
    outnavf(fp,eph->svh    );
    outnavf(fp,eph->tgd[0] ); /* GPS/QZS:TGD, GAL:BGD E5a/E1, BDS: TGD1 B1/B3 */
    if (sys==SYS_GAL||sys==SYS_CMP) {
        outnavf(fp,eph->tgd[1]); /* GAL:BGD E5b/E1, BDS: TGD2 B2/B3 */
    }
    else {
        outnavf(fp,eph->iodc);   /* GPS/QZS:IODC */
    }
    fprintf(fp,"\n%s",sep  );
    
    if (sys!=SYS_CMP) {
        ttr=time2gpst(eph->ttr,&week);
    }
    else {
        ttr=time2bdt(gpst2bdt(eph->ttr),&week); /* gpst -> bdt */
    }
    outnavf(fp,ttr+(week-eph->week)*604800.0);
    
    if (sys==SYS_GPS||sys==SYS_QZS) {
        outnavf(fp,eph->fit);
    }
    else if (sys==SYS_CMP) {
        outnavf(fp,eph->iodc); /* AODC */
    }
    else {
        outnavf(fp,0.0); /* spare */
    }
    return fprintf(fp,"\n")!=EOF;
}
/* output rinex gnav header ----------------------------------------------------
* output rinex gnav (glonass navigation) file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxgnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64];
    
    trace(3,"outrnxgnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "GLONASS NAV DATA","","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "N: GNSS NAV DATA","R: GLONASS","RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex gnav body ------------------------------------------------------
* output rinex gnav (glonass navigation) file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          geph_t  *geph    I   glonass ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxgnavb(FILE *fp, const rnxopt_t *opt, const geph_t *geph)
{
    gtime_t toe;
    double ep[6],tof;
    int prn;
    char code[32],*sep;
    
    trace(3,"outrnxgnavb: sat=%2d\n",geph->sat);
    
    if ((satsys(geph->sat,&prn)&opt->navsys)!=SYS_GLO) return 0;
    
    tof=time2gpst(gpst2utc(geph->tof),NULL);      /* v.3: tow in utc */
    if (opt->rnxver<=2.99) tof=fmod(tof,86400.0); /* v.2: tod in utc */
    
    toe=gpst2utc(geph->toe); /* gpst -> utc */
    time2epoch(toe,ep);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn,(int)ep[0]%100,
                ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    else { /* ver.3 */
        if (!sat2code(geph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    outnavf(fp,-geph->taun     );
    outnavf(fp,geph->gamn      );
    outnavf(fp,tof             );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[0]/1E3);
    outnavf(fp,geph->vel[0]/1E3);
    outnavf(fp,geph->acc[0]/1E3);
    outnavf(fp,geph->svh       );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[1]/1E3);
    outnavf(fp,geph->vel[1]/1E3);
    outnavf(fp,geph->acc[1]/1E3);
    outnavf(fp,geph->frq       );
    fprintf(fp,"\n%s",sep      );
    
    outnavf(fp,geph->pos[2]/1E3);
    outnavf(fp,geph->vel[2]/1E3);
    outnavf(fp,geph->acc[2]/1E3);
    outnavf(fp,geph->age       );
    
    return fprintf(fp,"\n")!=EOF;
}
/* output rinex geo nav header -------------------------------------------------
* output rinex geo nav file header
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxhnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64];
    
    trace(3,"outrnxhnavh:\n");
    
    timestr_rnx(date);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "H: GEO NAV MSG DATA","","RINEX VERSION / TYPE");
    }
    else { /* ver.3 */
        fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
                "N: GNSS NAV DATA","S: SBAS Payload","RINEX VERSION / TYPE");
    }
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex geo nav body ---------------------------------------------------
* output rinex geo nav file body record
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          seph_t  *seph    I   sbas ephemeris
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxhnavb(FILE *fp, const rnxopt_t *opt, const seph_t *seph)
{
    double ep[6];
    int prn;
    char code[32],*sep;
    
    trace(3,"outrnxhnavb: sat=%2d\n",seph->sat);
    
    if ((satsys(seph->sat,&prn)&opt->navsys)!=SYS_SBS) return 0;
    
    time2epoch(seph->t0,ep);
    
    if (opt->rnxver<=2.99) { /* ver.2 */
        fprintf(fp,"%2d %02d %2.0f %2.0f %2.0f %2.0f %4.1f",prn-100,
                (int)ep[0]%100,ep[1],ep[2],ep[3],ep[4],ep[5]);
        sep="   ";
    }
    else { /* ver.3 */
        if (!sat2code(seph->sat,code)) return 0;
        fprintf(fp,"%-3s %04.0f %2.0f %2.0f %2.0f %2.0f %2.0f",code,ep[0],ep[1],
                ep[2],ep[3],ep[4],ep[5]);
        sep="    ";
    }
    outnavf(fp,seph->af0          );
    outnavf(fp,seph->af1          );
    outnavf(fp,time2gpst(seph->tof,NULL));
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[0]/1E3   );
    outnavf(fp,seph->vel[0]/1E3   );
    outnavf(fp,seph->acc[0]/1E3   );
    outnavf(fp,seph->svh          );
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[1]/1E3   );
    outnavf(fp,seph->vel[1]/1E3   );
    outnavf(fp,seph->acc[1]/1E3   );
    outnavf(fp,uravalue(seph->sva));
    fprintf(fp,"\n%s",sep         );
    
    outnavf(fp,seph->pos[2]/1E3   );
    outnavf(fp,seph->vel[2]/1E3   );
    outnavf(fp,seph->acc[2]/1E3   );
    outnavf(fp,0                  );
    
    return fprintf(fp,"\n")!=EOF;
}
/* output rinex galileo nav header ---------------------------------------------
* output rinex galileo nav file header (2.12)
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxlnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64];
    
    trace(3,"outrnxlnavh:\n");
    
    timestr_rnx(date);
    
    fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
            "N: GNSS NAV DATA","E: Galileo","RINEX VERSION / TYPE");
    
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex qzss nav header ------------------------------------------------
* output rinex qzss nav file header (2.12 extention and 3.02)
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxqnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64];
    
    trace(3,"outrnxqnavh:\n");
    
    timestr_rnx(date);
    
    fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
            "N: GNSS NAV DATA","J: QZSS","RINEX VERSION / TYPE");
    
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
/* output rinex beidou nav header ----------------------------------------------
* output rinex beidou nav file header (2.12 extention and 3.02)
* args   : FILE   *fp       I   output file pointer
*          rnxopt_t *opt    I   rinex options
*          nav_t  nav       I   navigation data (NULL: no input)
* return : status (1:ok, 0:output error)
*-----------------------------------------------------------------------------*/
extern int outrnxcnavh(FILE *fp, const rnxopt_t *opt, const nav_t *nav)
{
    int i;
    char date[64];
    
    trace(3,"outrnxcnavh:\n");
    
    timestr_rnx(date);
    
    fprintf(fp,"%9.2f           %-20s%-20s%-20s\n",opt->rnxver,
            "N: GNSS NAV DATA","C: BeiDou","RINEX VERSION / TYPE");
    
    fprintf(fp,"%-20.20s%-20.20s%-20.20s%-20s\n",opt->prog,opt->runby,date,
            "PGM / RUN BY / DATE");
    
    for (i=0;i<MAXCOMMENT;i++) {
        if (!*opt->comment[i]) continue;
        fprintf(fp,"%-60.60s%-20s\n",opt->comment[i],"COMMENT");
    }
    return fprintf(fp,"%60s%-20s\n","","END OF HEADER")!=EOF;
}
