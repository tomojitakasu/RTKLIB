/*------------------------------------------------------------------------------
* septentrio.c : Septentrio Binary Format (SBF) decoder
*
*          Copyright (C) 2020 by Tomoji TAKASU
*
* reference :
*     [1] Septentrio, mosaic-X5 reference guide applicable to version 4.8.0 of
*         the firmware, June 4, 2020
*
* version : $Revision:$
*
* history : 2013/07/17  1.0  begin writing
*           2013/10/24  1.1  GPS L1 working
*           2013/11/02  1.2  modified by TTAKASU
*           2015/01/26  1.3  fix some problems by Jens Reimann
*           2016/02/04  1.4  by Jens Reimann
*                           - added more sanity checks
*                           - added galileon raw decoding
*                           - added usage of decoded SBAS messages for testing
*                           - add QZSS and Compass/Beidou navigation messages
*                           - fixed code and Doppler for 2nd and following frequency
*                           - fixed bug in glonass ephemeris
*                           - fixed decoding of galileo ephemeris
*                           - fixed lost lock indicator
*                           - fixed sbas decoding
*                           - cleanups
*           2016/03/03  1.5 - fixed TOW in SBAS messages
*           2016/03/12  1.6 - respect code priorities
*                           - fixed bug in carrier phase calculation of type2 data
*                           - unify frequency determination
*                           - improve lock handling
*                           - various bug fixes
*           2016/05/25  1.7  rtk_crc24q() -> crc24q() by T.T
*           2016/07/29  1.8  crc24q() -> rtk_crc24q() by T.T
*           2017/04/11  1.9  (char *) -> (signed char *) by T.T
*           2017/09/01  1.10 suppress warnings
*
*           2020/11/30  1.11 rewritten from scratch to support mosaic-X5 [1]
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define SBF_SYNC1       0x24    /* SBF block header 1 */
#define SBF_SYNC2       0x40    /* SBF block header 2 */
#define SBF_MAXSIG      36      /* SBF max signal number */

#define SBF_MEASEPOCH   4027    /* SBF GNSS measurements */
#define SBF_MEASEXTRA   4000    /* SBF GNSS measurements extra info */
#define SBF_GPSRAWCA    4017    /* SBF GPS C/A subframe */
#define SBF_GLORAWCA    4026    /* SBF GLONASS L1CA or L2CA navigation string */
#define SBF_GALRAWFNAV  4022    /* SBF Galileo F/NAV navigation page */
#define SBF_GALRAWINAV  4023    /* SBF Galileo I/NAV navigation page */
#define SBF_GEORAWL1    4020    /* SBF SBAS L1 navigation frame */
#define SBF_BDSRAW      4047    /* SBF BDS navigation page */
#define SBF_QZSRAWL1CA  4066    /* SBF QZSS C/A subframe */
#define SBF_NAVICRAW    4093    /* SBF NavIC/IRNSS subframe */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((uint8_t *)(p)))
#define I1(p) (*((int8_t  *)(p)))
static uint16_t U2(uint8_t *p) {uint16_t a; memcpy(&a,p,2); return a;}
static uint32_t U4(uint8_t *p) {uint32_t a; memcpy(&a,p,4); return a;}
static int32_t  I4(uint8_t *p) {int32_t  a; memcpy(&a,p,4); return a;}

/* svid to satellite number ([1] 4.1.9) --------------------------------------*/
static int svid2sat(int svid)
{
    if (svid<= 37) return satno(SYS_GPS,svid);
    if (svid<= 61) return satno(SYS_GLO,svid-37);
    if (svid<= 62) return 0; /* glonass unknown slot */
    if (svid<= 68) return satno(SYS_GLO,svid-38);
    if (svid<= 70) return 0;
    if (svid<=106) return satno(SYS_GAL,svid-70);
    if (svid<=119) return 0;
    if (svid<=140) return satno(SYS_SBS,svid);
    if (svid<=180) return satno(SYS_CMP,svid-140);
    if (svid<=187) return satno(SYS_QZS,svid-180+192);
    if (svid<=190) return 0;
    if (svid<=197) return satno(SYS_IRN,svid-190);
    if (svid<=215) return satno(SYS_SBS,svid-57);
    if (svid<=222) return satno(SYS_IRN,svid-208);
    if (svid<=245) return satno(SYS_CMP,svid-182);
    return 0; /* error */
}
/* signal number table ([1] 4.1.10) ------------------------------------------*/
static uint8_t sig_tbl[SBF_MAXSIG+1][2]={ /* system, obs-code */
    {SYS_GPS, CODE_L1C}, /*  0: GPS L1C/A */
    {SYS_GPS, CODE_L1W}, /*  1: GPS L1P */
    {SYS_GPS, CODE_L2W}, /*  2: GPS L2P */
    {SYS_GPS, CODE_L2L}, /*  3: GPS L2C */
    {SYS_GPS, CODE_L5Q}, /*  4: GPS L5 */
    {SYS_GPS, CODE_L1L}, /*  5: GPS L1C */
    {SYS_QZS, CODE_L1C}, /*  6: QZS L1C/A */
    {SYS_QZS, CODE_L2L}, /*  7: QZS L2C */
    {SYS_GLO, CODE_L1C}, /*  8: GLO L1C/A */
    {SYS_GLO, CODE_L1P}, /*  9: GLO L1P */
    {SYS_GLO, CODE_L2P}, /* 10: GLO L2P */
    {SYS_GLO, CODE_L2C}, /* 11: GLO L2C/A */
    {SYS_GLO, CODE_L3Q}, /* 12: GLO L3 */
    {SYS_CMP, CODE_L1P}, /* 13: BDS B1C */
    {SYS_CMP, CODE_L5P}, /* 14: BDS B2a */
    {SYS_IRN, CODE_L5A}, /* 15: IRN L5 */
    {      0,        0}, /* 16: reserved */
    {SYS_GAL, CODE_L1C}, /* 17: GAL E1(L1BC) */
    {      0,        0}, /* 18: reserved */
    {SYS_GAL, CODE_L6C}, /* 19: GAL E6(E6BC) */
    {SYS_GAL, CODE_L5Q}, /* 20: GAL E5a */
    {SYS_GAL, CODE_L7Q}, /* 21: GAL E5b */
    {SYS_GAL, CODE_L8Q}, /* 22: GAL E5 AltBoc */
    {      0,        0}, /* 23: LBand */
    {SYS_SBS, CODE_L1C}, /* 24: SBS L1C/A */
    {SYS_SBS, CODE_L5I}, /* 25: SBS L5 */
    {SYS_QZS, CODE_L5Q}, /* 26: QZS L5 */
    {SYS_QZS, CODE_L6L}, /* 27: QZS L6 */
    {SYS_CMP, CODE_L2I}, /* 28: BDS B1I */
    {SYS_CMP, CODE_L7I}, /* 29: BDS B2I */
    {SYS_CMP, CODE_L6I}, /* 30: BDS B3I */
    {      0,        0}, /* 31: reserved */
    {SYS_QZS, CODE_L1L}, /* 32: QZS L1C */
    {SYS_QZS, CODE_L1Z}, /* 33: QZS L1S */
    {SYS_CMP, CODE_L7D}, /* 34: BDS B2b */
    {      0,        0}, /* 35: reserved */
    {SYS_IRN, CODE_L9A}  /* 36: IRN S */
};
/* signal number to freq-index and code --------------------------------------*/
static int sig2idx(int sat, int sig, const char *opt, uint8_t *code)
{
    int idx,sys=satsys(sat,NULL),nex=NEXOBS;
    
    if (sig<0||sig>SBF_MAXSIG||sig_tbl[sig][0]!=sys) return -1;
    *code=sig_tbl[sig][1];
    idx=code2idx(sys,*code);
    
    /* resolve code priority in a freq-index */
    if (sys==SYS_GPS) {
        if (strstr(opt,"-GL1W")&&idx==0) return (*code==CODE_L1W)?0:-1;
        if (strstr(opt,"-GL1L")&&idx==0) return (*code==CODE_L1L)?0:-1;
        if (strstr(opt,"-GL2L")&&idx==1) return (*code==CODE_L2L)?1:-1;
        if (*code==CODE_L1W) return (nex<1)?-1:NFREQ;
        if (*code==CODE_L2L) return (nex<2)?-1:NFREQ+1;
        if (*code==CODE_L1L) return (nex<3)?-1:NFREQ+2;
    }
    else if (sys==SYS_GLO) {
        if (strstr(opt,"-RL1P")&&idx==0) return (*code==CODE_L1P)?0:-1;
        if (strstr(opt,"-RL2C")&&idx==1) return (*code==CODE_L2C)?1:-1;
        if (*code==CODE_L1P) return (nex<1)?-1:NFREQ;
        if (*code==CODE_L2C) return (nex<2)?-1:NFREQ+1;
    }
    else if (sys==SYS_QZS) {
        if (strstr(opt,"-JL1L")&&idx==0) return (*code==CODE_L1L)?0:-1;
        if (strstr(opt,"-JL1Z")&&idx==0) return (*code==CODE_L1Z)?0:-1;
        if (*code==CODE_L1L) return (nex<1)?-1:NFREQ;
        if (*code==CODE_L1Z) return (nex<2)?-1:NFREQ+1;
    }
    else if (sys==SYS_CMP) {
        if (strstr(opt,"-CL1P")&&idx==0) return (*code==CODE_L1P)?0:-1;
        if (*code==CODE_L1P) return (nex<1)?-1:NFREQ;
    }
    return (idx<NFREQ)?idx:-1;
}
/* initialize obs data fields ------------------------------------------------*/
static void init_obsd(gtime_t time, int sat, obsd_t *data)
{
    int i;
    
    data->time=time;
    data->sat=(uint8_t)sat;
    
    for (i=0;i<NFREQ+NEXOBS;i++) {
        data->L[i]=data->P[i]=0.0;
        data->D[i]=0.0f;
        data->SNR[i]=(uint16_t)0;
        data->LLI[i]=(uint8_t)0;
        data->code[i]=CODE_NONE;
    }
}
/* decode SBF GNSS measurements ----------------------------------------------*/
static int decode_measepoch(raw_t *raw)
{
    uint8_t *p=raw->buff+14,code;
    double P1,P2,L1,L2,D1,D2,S1,S2,freq1,freq2;
    int i,j,idx,n,n1,n2,len1,len2,sig,ant,svid,info,sat,sys,lock,fcn,LLI;
    int ant_sel=0; /* antenna selection (0:main) */
    
    if      (strstr(raw->opt,"-AUX1")) ant_sel=1;
    else if (strstr(raw->opt,"-AUX2")) ant_sel=2;
    
    if (raw->len<20) {
        trace(2,"sbf measepoch length error: len=%d\n",raw->len);
        return -1;
    }
    n1  =U1(p);
    len1=U1(p+1);
    len2=U1(p+2);
    
    if (U1(p+3)&0x80) {
        trace(2,"sbf measepoch scrambled\n");
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," nsat=%d",n1);
    }
    for (i=n=0,p+=6;i<n1&&n<MAXOBS&&p+20<=raw->buff+raw->len;i++) {
        svid=U1(p+2);
        ant =U1(p+1)>>5;
        sig =U1(p+1)&0x1f;
        info=U1(p+18);
        n2  =U1(p+19);
        fcn =0;
        if (sig==31) sig+=(info>>3)*32;
        else if (sig>=8&&sig<=11) fcn=(info>>3)-8;
        
        if (ant!=ant_sel) {
            trace(3,"sbf measepoch ant error: svid=%d ant=%d\n",svid,ant);
            p+=len1+len2*n2;
            continue;
        }
        if (!(sat=svid2sat(svid))) {
            trace(3,"sbf measepoch svid error: svid=%d\n",svid);
            p+=len1+len2*n2;
            continue;
        }
        if ((idx=sig2idx(sat,sig,raw->opt,&code))<0) {
            trace(2,"sbf measepoch sig error: sat=%d sig=%d\n",sat,sig);
            p+=len1+len2*n2;
            continue;
        }
        init_obsd(raw->time,sat,raw->obs.data+n);
        P1=D1=0.0;
        sys=satsys(sat,NULL);
        freq1=code2freq(sys,code,fcn);
        
        if ((U1(p+3)&0x1f)!=0||U4(p+4)!=0) {
            P1=(U1(p+3)&0x0f)*4294967.296+U4(p+4)*0.001;
            raw->obs.data[n].P[idx]=P1;
        }
        if (I4(p+8)!=-2147483648) {
            D1=I4(p+8)*0.0001;
            raw->obs.data[n].D[idx]=(float)D1;
        }
        lock=U2(p+16);
        if (P1!=0.0&&freq1>0.0&&lock!=65535&&(I1(p+14)!=-128||U2(p+12)!=0)) {
            L1=I1(p+14)*65.536+U2(p+12)*0.001;
            raw->obs.data[n].L[idx]=P1*freq1/CLIGHT+L1;
            LLI=(lock<raw->lockt[sat-1][idx]?1:0)+((info&(1<<2))?2:0);
            raw->obs.data[n].LLI[idx]=(uint8_t)LLI;
            raw->lockt[sat-1][idx]=lock;
        }
        if (U1(p+15)!=255) {
            S1=U1(p+15)*0.25+((sig==1||sig==2)?0.0:10.0);
            raw->obs.data[n].SNR[idx]=(uint16_t)(S1/SNR_UNIT+0.5);
        }
        raw->obs.data[n].code[idx]=code;
        
        for (j=0,p+=len1;j<n2&&p+12<=raw->buff+raw->len;j++,p+=len2) {
            sig =U1(p)&0x1f;
            ant =U1(p)>>5;
            info=U1(p+5);
            if (sig==31) sig+=(info>>3)*32;
            
            if (ant!=ant_sel) {
                trace(3,"sbf measepoch ant error: sat=%d ant=%d\n",sat,ant);
                continue;
            }
            if ((idx=sig2idx(sat,sig,raw->opt,&code))<0) {
                trace(3,"sbf measepoch sig error: sat=%d sig=%d\n",sat,sig);
                continue;
            }
            P2=0.0;
            freq2=code2freq(sys,code,fcn);
            
            if (P1!=0.0&&(getbits(p+3,5,3)!=-4||U2(p+6)!=0)) {
                P2=P1+getbits(p+3,5,3)*65.536+U2(p+6)*0.001;
                raw->obs.data[n].P[idx]=P2;
            }
            if (P2!=0.0&&freq2>0.0&&(I1(p+4)!=-128||U2(p+8)!=0)) {
                L2=I1(p+4)*65.536+U2(p+8)*0.001;
                raw->obs.data[n].L[idx]=P2*freq2/CLIGHT+L2;
            }
            if (D1!=0.0&&freq1>0.0&&freq2>0.0&&
                (getbits(p+3,0,5)!=-16||U2(p+10)!=0)) {
                D2=getbits(p+3,0,5)*6.5536+U2(p+10)*0.0001;
                raw->obs.data[n].D[idx]=(float)(D1*freq2/freq1)+D2;
            }
            lock=U1(p+1);
            if (lock!=255) {
                LLI=(lock<raw->lockt[sat-1][idx]?1:0)+((info&(1<<2))?2:0);
                raw->obs.data[n].LLI[idx]=(uint8_t)LLI;
                raw->lockt[sat-1][idx]=lock;
            }
            if (U1(p+2)!=255) {
                S2=U1(p+2)*0.25+((sig==1||sig==2)?0.0:10.0);
                raw->obs.data[n].SNR[idx]=(uint16_t)(S2/SNR_UNIT+0.5);
            }
            raw->obs.data[n].code[idx]=code;
        }
        n++;
    }
    raw->obs.n=n;
    return 1;
}
/* decode SBF GNSS measurements extra info -----------------------------------*/
static int decode_measextra(raw_t *raw)
{
    /* not yet supported */
    return 0;
}
/* decode ephemeris ----------------------------------------------------------*/
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
/* decode ION/UTC parameters -------------------------------------------------*/
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
    return 1;
}
/* decode SBF raw C/A subframe -----------------------------------------------*/
static int decode_rawca(raw_t *raw, int sys)
{
    uint8_t *p=raw->buff+14,buff[30];
    int i,svid,sat,prn,id,ret;
    
    if (raw->len<60) {
        trace(2,"sbf rawca length error: sys=%d len=%d\n",sys,raw->len);
        return -1;
    }
    svid=U1(p);
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=sys) {
        trace(2,"sbf rawca svid error: sys=%d svid=%d\n",sys,svid);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf rawca parity/crc error: sys=%d prn=%d\n",sys,prn);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    for (i=0,p+=6;i<10;i++,p+=4) { /* 24 x 10 bits w/o parity */
        setbitu(buff,24*i,24,U4(p)>>6);
    }
    id=getbitu(buff,43,3);

    if (id<1||id>5) {
        trace(2,"sbf rawca subframe id error: sys=%d prn=%d id=%d\n",sys,prn,id);
        return -1;
    }
    memcpy(raw->subfrm[sat-1]+(id-1)*30,buff,30);

    if (id==3) {
        return decode_eph(raw,sat);
    }
    if (id==4||id==5) {
        ret=decode_ionutc(raw,sat);
        memset(raw->subfrm[sat-1]+id*30,0,30);
        return ret;
    }
    return 0;
}
/* decode SBF GPS C/A subframe -----------------------------------------------*/
static int decode_gpsrawca(raw_t *raw)
{
    return decode_rawca(raw,SYS_GPS);
}
/* decode SBF GLONASS L1CA or L2CA navigation string -------------------------*/
static int decode_glorawca(raw_t *raw)
{
    geph_t geph={0};
    gtime_t *time;
    double utc[8]={0};
    uint8_t *p=raw->buff+14,buff[12];
    int i,svid,sat,prn,m;
    
    if (raw->len<32) {
        trace(2,"sbf glorawca length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_GLO) {
        trace(3,"sbf glorawca svid error: svid=%d\n",svid);
        return (svid==62)?0:-1; /* svid=62: slot unknown */
    }
    if (!U1(p+1)) {
        trace(3,"sbf glorawca parity/crc error: prn=%d\n",prn);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    for (i=0;i<3;i++) {
        setbitu(buff,32*i,32,U4(p+6+4*i)); /* 85 bits */
    }
    m=getbitu(buff,1,4);
    if (m<1||m>15) {
        trace(2,"sbf glorawca string number error: prn=%d m=%d\n",prn,m);
        return -1;
    }
    time=(gtime_t *)(raw->subfrm[sat-1]+150);
    if (fabs(timediff(raw->time,*time))>30.0) {
        memset(raw->subfrm[sat-1],0,40);
        memcpy(time,&raw->time,sizeof(gtime_t));
    }
    memcpy(raw->subfrm[sat-1]+(m-1)*10,buff,10);
    if (m!=4) return 0;
    
    geph.tof=raw->time;
    if (!decode_glostr(raw->subfrm[sat-1],&geph,utc)) return 0;
    
    matcpy(raw->nav.utc_glo,utc,8,1);

    if (geph.sat!=sat) {
        trace(2,"sbf glorawca satellite error: sat=%d %d\n",sat,geph.sat);
        return -1;
    }
    geph.frq=(int)U1(p+4)-8;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode&&
            timediff(geph.toe,raw->nav.geph[prn-1].toe)==0.0) return 0;
    }
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    raw->ephset=0;
    return 2;
}
/* decode SBF Galileo F/NAV navigation page ----------------------------------*/
static int decode_galrawfnav(raw_t *raw)
{
    eph_t eph={0};
    double ion[4]={0},utc[8]={0};
    uint8_t *p=raw->buff+14,buff[32];
    int i,svid,src,sat,prn,type;
    
    if (strstr(raw->opt,"-GALINAV")) return 0;
    
    if (raw->len<52) {
        trace(2,"sbf galrawfnav length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    src =U1(p+3)&0x1f;
    
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_GAL) {
        trace(2,"sbf galrawfnav svid error: svid=%d src=%d\n",svid,src);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf galrawfnav parity/crc error: prn=%d src=%d\n",prn,src);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d src=%d",prn,src);
    }
    if (src!=20&&src!=22) { /* E5a or E5 AltBOC */
        trace(2,"sbf galrawfnav source error: prn=%d src=%d\n",prn,src);
        return -1;
    }
    for (i=0;i<8;i++) {
        setbitu(buff,32*i,32,U4(p+6+4*i)); /* 244 bits page */
    }
    type=getbitu(buff,0,6); /* page type */
    
    if (type==63) return 0; /* dummy page */
    if (type<1||type>6) {
        trace(2,"sbf galrawfnav page type error: prn=%d type=%d\n",prn,type);
        return -1;
    }
    /* save 244 bits page (31 bytes * 6 page) */
    memcpy(raw->subfrm[sat-1]+128+(type-1)*31,buff,31);
    
    if (type!=4) return 0;
    if (!decode_gal_fnav(raw->subfrm[sat-1]+128,&eph,ion,utc)) return 0;
    
    if (eph.sat!=sat) {
        trace(2,"sbf galrawfnav satellite error: sat=%d %d\n",sat,eph.sat);
        return -1;
    }
    eph.code|=(1<<1); /* data source: E5a */
    
    adj_utcweek(raw->time,utc);
    matcpy(raw->nav.ion_gal,ion,4,1);
    matcpy(raw->nav.utc_gal,utc,8,1);
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1+MAXSAT].iode&&
            timediff(eph.toe,raw->nav.eph[sat-1+MAXSAT].toe)==0.0&&
            timediff(eph.toc,raw->nav.eph[sat-1+MAXSAT].toc)==0.0) return 0;
    }
    raw->nav.eph[sat-1+MAXSAT]=eph;
    raw->ephsat=sat;
    raw->ephset=1; /* 1:F/NAV */
    return 2;
}
/* decode SBF Galileo I/NAV navigation page ----------------------------------*/
static int decode_galrawinav(raw_t *raw)
{
    eph_t eph={0};
    double ion[4]={0},utc[8]={0};
    uint8_t *p=raw->buff+14,buff[32],type,part1,part2,page1,page2;
    int i,j,svid,src,sat,prn;
    
    if (strstr(raw->opt,"-GALFNAV")) return 0;
    
    if (raw->len<52) {
        trace(2,"sbf galrawinav length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    src =U1(p+3)&0x1f;
    
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_GAL) {
        trace(2,"sbf galrawinav svid error: svid=%d src=%d\n",svid,src);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf galrawinav parity/crc error: prn=%d src=%d\n",prn,src);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d src=%d",prn,src);
    }
    if (src!=17&&src!=21&&src!=22) { /* E1, E5b or E5 AltBOC */
        trace(2,"sbf galrawinav source error: prn=%d src=%d\n",prn,src);
        return -1;
    }
    for (i=0,p+=6;i<8;i++,p+=4) {
        setbitu(buff,32*i,32,U4(p)); /* 114(even) + 120(odd) bits */
    }
    part1=getbitu(buff,  0,1);
    page1=getbitu(buff,  1,1);
    part2=getbitu(buff,114,1);
    page2=getbitu(buff,115,1);

    if (part1!=0||part2!=1) {
        trace(3,"sbf galrawinav part error: prn=%d even/odd=%d %d\n",prn,part1,
              part2);
        return -1;
    }
    if (page1==1||page2==1) return 0; /* alert page */
    
    type=getbitu(buff,2,6); /* word type */
    
    if (type>6) return 0;
    
    /* save 128 (112:even+16:odd) bits word (16 bytes * 7 word) */
    for (i=0,j=2;i<14;i++,j+=8) {
        raw->subfrm[sat-1][type*16+i]=getbitu(buff,j,8);
    }
    for (i=14,j=116;i<16;i++,j+=8) {
        raw->subfrm[sat-1][type*16+i]=getbitu(buff,j,8);
    }
    if (type!=5) return 0;
    if (!decode_gal_inav(raw->subfrm[sat-1],&eph,ion,utc)) return 0;
    
    if (eph.sat!=sat) {
        trace(2,"sbf galrawinav satellite error: sat=%d %d\n",sat,eph.sat);
        return -1;
    }
    eph.code|=(src==17)?(1<<0):(1<<2); /* data source: E1 or E5b */
    
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
/* decode SBF SBAS L1 navigation frame ---------------------------------------*/
static int decode_georawl1(raw_t *raw)
{
    uint8_t *p=raw->buff+14,buff[32];
    int i,svid,sat,prn;
    
    if (raw->len<52) {
        trace(2,"sbf georawl1 length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_SBS) {
        trace(2,"sbf georawl1 svid error: svid=%d\n",svid);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf georawl1 parity/crc error: prn=%d err=%d\n",prn,U1(p+2));
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    raw->sbsmsg.tow=(int)time2gpst(raw->time,&raw->sbsmsg.week);
    raw->sbsmsg.prn=prn;
    
    for (i=0;i<8;i++) {
        setbitu(buff,32*i,32,U4(p+6+4*i));
    }
    memcpy(raw->sbsmsg.msg,buff,29); /* 226 bits w/o CRC */
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode SBF BDS navigation frame -------------------------------------------*/
static int decode_bdsraw(raw_t *raw)
{
    eph_t eph={0};
    double ion[8],utc[8];
    uint8_t *p=raw->buff+14,buff[40];
    int i,id,svid,sat,prn,pgn;
    
    if (raw->len<52) {
        trace(2,"sbf bdsraw length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_CMP) {
        trace(2,"sbf bdsraw svid error: svid=%d\n",svid);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf bdsraw parity/crc error: prn=%d\n",prn);
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    for (i=0,p+=6;i<10;i++,p+=4) {
        setbitu(buff,32*i,32,U4(p));
    }
    id=getbitu(buff,15,3); /* subframe ID */
    if (id<1||id>5) {
        trace(2,"sbf bdsraw id error: prn=%d id=%d\n",prn,id);
        return -1;
    }
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
        pgn=getbitu(buff,42,4); /* page number */
        
        if (id==1&&pgn>=1&&pgn<=10) {
            memcpy(raw->subfrm[sat-1]+(pgn-1)*38,buff,38);
            if (pgn!=10) return 0;
            if (!decode_bds_d2(raw->subfrm[sat-1],&eph,NULL)) return 0;
        }
        else if (id==1&&pgn==102) {
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
/* decode SBF QZS C/A subframe -----------------------------------------------*/
static int decode_qzsrawl1ca(raw_t *raw)
{
    return decode_rawca(raw,SYS_QZS);
}
/* decode SBF NavIC/IRNSS subframe -------------------------------------------*/
static int decode_navicraw(raw_t *raw)
{
    eph_t eph={0};
    double ion[8],utc[9];
    uint8_t *p=raw->buff+14,buff[40];
    int i,id,svid,sat,prn,ret=0;
    
    if (raw->len<52) {
        trace(2,"sbf navicraw length error: len=%d\n",raw->len);
        return -1;
    }
    svid=U1(p);
    if (!(sat=svid2sat(svid))||satsys(sat,&prn)!=SYS_IRN) {
        trace(2,"sbf navicraw svid error: svid=%d\n",svid);
        return -1;
    }
    if (!U1(p+1)) {
        trace(3,"sbf navicraw parity/crc error: prn=%d err=%d\n",prn,U1(p+2));
        return 0;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype+strlen(raw->msgtype)," prn=%d",prn);
    }
    for (i=0,p+=6;i<10;i++,p+=4) {
        setbitu(buff,32*i,32,U4(p));
    }
    id=getbitu(buff,27,2); /* subframe ID (0-3) */
    
    memcpy(raw->subfrm[sat-1]+id*37,buff,37);
    
    if (id==1) { /* subframe 2 */
        if (!decode_irn_nav(raw->subfrm[sat-1],&eph,NULL,NULL)) return 0;
        
        if (!strstr(raw->opt,"-EPHALL")) {
            if (eph.iode==raw->nav.eph[sat-1].iode&&
                timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0) {
                return 0;
            }
        }
        eph.sat=sat;
        raw->nav.eph[sat-1]=eph;
        raw->ephsat=sat;
        raw->ephset=0;
        return 2;
    }
    else if (id==2||id==3) { /* subframe 3 or 4 */
        if (decode_irn_nav(raw->subfrm[sat-1],NULL,ion,NULL)) {
            matcpy(raw->nav.ion_irn,ion,8,1);
            ret=9;
        }
        if (decode_irn_nav(raw->subfrm[sat-1],NULL,NULL,utc)) {
            adj_utcweek(raw->time,utc);
            matcpy(raw->nav.utc_irn,utc,9,1);
            ret=9;
        }
        memset(raw->subfrm[sat-1]+id*37,0,37);
        return ret;
    }
    return 0;
}
/* decode SBF block ----------------------------------------------------------*/
static int decode_sbf(raw_t *raw)
{
    uint8_t *p=raw->buff;
    uint32_t week,tow;
    char tstr[32];
    int type=U2(p+4)&0x1fff;
    
    if (rtk_crc16(p+4,raw->len-4)!=U2(p+2)) {
        trace(2,"sbf crc error: type=%d len=%d\n",type,raw->len);
        return -1;
    }
    if (raw->len<14) {
        trace(2,"sbf length error: type=%d len=%d\n",type,raw->len);
        return -1;
    }
    tow =U4(p+8);
    week=U2(p+12);
    if (tow==4294967295u||week==65535u) {
        trace(2,"sbf tow/week error: type=%d len=%d\n",type,raw->len);
        return -1;
    }
    raw->time=gpst2time(week,tow*0.001);
    
    if (raw->outtype) {
        time2str(raw->time,tstr,2);
        sprintf(raw->msgtype,"SBF %4d (%4d): %s",type,raw->len,tstr);
    }
    switch (type) {
        case SBF_MEASEPOCH : return decode_measepoch (raw);
        case SBF_MEASEXTRA : return decode_measextra (raw);
        case SBF_GPSRAWCA  : return decode_gpsrawca  (raw);
        case SBF_GLORAWCA  : return decode_glorawca  (raw);
        case SBF_GALRAWFNAV: return decode_galrawfnav(raw);
        case SBF_GALRAWINAV: return decode_galrawinav(raw);
        case SBF_GEORAWL1  : return decode_georawl1  (raw);
        case SBF_BDSRAW    : return decode_bdsraw    (raw);
        case SBF_QZSRAWL1CA: return decode_qzsrawl1ca(raw);
        case SBF_NAVICRAW  : return decode_navicraw  (raw);
    }
    trace(3,"sbf unsupported message: type=%d\n",type);
    return 0;
}
/* synchronize SBF block header ----------------------------------------------*/
static int sync_sbf(uint8_t *buff, uint8_t data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==SBF_SYNC1&&buff[1]==SBF_SYNC2;
}
/* input SBF raw data from stream ----------------------------------------------
* fetch next SBF raw data and input a mesasge from stream
* args   : raw_t *raw       IO  receiver raw data control struct
*          uint_t data      I   stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : supported SBF block (block ID):
*
*           MEASEPOCH(4027), GPSRAWCA(4017), GLORAWCA(4026), GALRAWFNAV(4022),
*           GALRAWINAV(4023), GEORAWL1(4020), BDSRAW(4047), QZSRAWL1CA(4066),
*           NAVICRAW(4093)
*
*          to specify input options for sbf, set raw->opt to the following
*          option strings separated by spaces.
*
*          -EPHALL : input all ephemerides
*          -AUX1   : select antenna Aux1  (default: main)
*          -AUX2   : select antenna Aux2  (default: main)
*          -GL1W   : select 1W for GPS L1 (default: 1C)
*          -GL1L   : select 1L for GPS L1 (default: 1C)
*          -GL2L   : select 2L for GPS L2 (default: 2W)
*          -RL1P   : select 1P for GLO G1 (default: 1C)
*          -RL2C   : select 2C for GLO G2 (default: 2P)
*          -JL1L   : select 1L for QZS L1 (default: 1C)
*          -JL1Z   : select 1Z for QZS L1 (default: 1C)
*          -CL1P   : select 1P for BDS B1 (default: 2I)
*          -GALINAV: select I/NAV for Galileo ephemeris (default: all)
*          -GALFNAV: select F/NAV for Galileo ephemeris (default: all)
*-----------------------------------------------------------------------------*/
extern int input_sbf(raw_t *raw, uint8_t data)
{
    trace(5,"input_sbf: data=%02x\n",data);
    
    if (raw->nbyte==0) {
        if (sync_sbf(raw->buff,data)) raw->nbyte=2;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    if (raw->nbyte<8) return 0;
    
    if ((raw->len=U2(raw->buff+6))>MAXRAWLEN) {
        trace(2,"sbf length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode SBF block */
    return decode_sbf(raw);
}
/* input SBF raw data from file ------------------------------------------------
* fetch next SBF raw data and input a message from file
* args   : raw_t *raw       IO  receiver raw data control struct
*          FILE  *fp        I   file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_sbff(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_sbff:\n");
    
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_sbf(raw->buff,(uint8_t)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+2,1,6,fp)<6) return -2;
    raw->nbyte=8;
    
    if ((raw->len=U2(raw->buff+6))>MAXRAWLEN) {
        trace(2,"sbf length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+8,raw->len-8,1,fp)<1) return -2;
    raw->nbyte=0;
    
    /* decode SBF block */
    return decode_sbf(raw);
}

