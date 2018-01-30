/*------------------------------------------------------------------------------
* rtcm3e.c : rtcm ver.3 message encoder functions
*
*          Copyright (C) 2012-2018 by T.TAKASU, All rights reserved.
*
* references :
*     see rtcm.c
*
* version : $Revision:$ $Date:$
* history : 2012/12/05 1.0  new
*           2012/12/16 1.1  fix bug on ssr high rate clock correction
*           2012/12/24 1.2  fix bug on msm carrier-phase offset correction
*                           fix bug on SBAS sat id in 1001-1004
*                           fix bug on carrier-phase in 1001-1004,1009-1012
*           2012/12/28 1.3  fix bug on compass carrier wave length
*           2013/01/18 1.4  fix bug on ssr message generation
*           2013/05/11 1.5  change type of arg value of setbig()
*           2013/05/19 1.5  gpst -> bdt of time-tag in beidou msm message
*           2013/04/27 1.7  comply with rtcm 3.2 with amendment 1/2 (ref[15])
*                           delete MT 1046 according to ref [15]
*           2014/05/15 1.8  set NT field in MT 1020 glonass ephemeris
*           2014/12/06 1.9  support SBAS/BeiDou SSR messages (ref [16])
*                           fix bug on invalid staid in qzss ssr messages
*           2015/03/22 1.9  add handling of iodcrc for beidou/sbas ssr messages
*           2018/01/29 1.10 add support of MT1042, MT1046, MT63
*                           delete support of MT1047
*                           delete compile option SSR_QZSS_DRAFT_V05
*                           update SSR signal and tracking mode IDs
*                           change session time field to reserved in MSM header
*                           fix range overflow in MT1001-4,1009-12 (##133)
*                           fix bug on SSR 3 message generation (#321)
*                           fix bug on segmentation fault by generating MSM1
*                           fix bug on Galileo week rollover in MT1045
*                           fix bug on lock time in MSM messages
*                           fix bug on generation of BDS MSM
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* constants and macros ------------------------------------------------------*/

#define PRUNIT_GPS  299792.458          /* rtcm 3 unit of gps pseudorange (m) */
#define PRUNIT_GLO  599584.916          /* rtcm 3 unit of glo pseudorange (m) */
#define RANGE_MS    (CLIGHT*0.001)      /* range in 1 ms */
#define P2_10       0.0009765625          /* 2^-10 */
#define P2_34       5.820766091346740E-11 /* 2^-34 */
#define P2_46       1.421085471520200E-14 /* 2^-46 */
#define P2_59       1.734723475976810E-18 /* 2^-59 */
#define P2_66       1.355252715606880E-20 /* 2^-66 */

#define ROUND(x)    ((int)floor((x)+0.5))
#define ROUND_U(x)  ((unsigned int)floor((x)+0.5))
#define MIN(x,y)    ((x)<(y)?(x):(y))

/* msm signal id table -------------------------------------------------------*/
extern const char *msm_sig_gps[32];
extern const char *msm_sig_glo[32];
extern const char *msm_sig_gal[32];
extern const char *msm_sig_qzs[32];
extern const char *msm_sig_sbs[32];
extern const char *msm_sig_cmp[32];

/* ssr update intervals ------------------------------------------------------*/
static const double ssrudint[16]={
    1,2,5,10,15,30,60,120,240,300,600,900,1800,3600,7200,10800
};
/* set sign-magnitude bits ---------------------------------------------------*/
static void setbitg(unsigned char *buff, int pos, int len, int value)
{
    setbitu(buff,pos,1,value<0?1:0);
    setbitu(buff,pos+1,len-1,value<0?-value:value);
}
/* set signed 38 bit field ---------------------------------------------------*/
static void set38bits(unsigned char *buff, int pos, double value)
{
    int word_h=(int)floor(value/64.0);
    unsigned int word_l=(unsigned int)(value-word_h*64.0);
    setbits(buff,pos  ,32,word_h);
    setbitu(buff,pos+32,6,word_l);
}
/* lock time -----------------------------------------------------------------*/
static int locktime(gtime_t time, gtime_t *lltime, unsigned char LLI)
{
    if (!lltime->time||(LLI&1)) *lltime=time;
    return (int)timediff(time,*lltime);
}
/* lock time in double -------------------------------------------------------*/
static double locktime_d(gtime_t time, gtime_t *lltime, unsigned char LLI)
{
    if (!lltime->time||(LLI&1)) *lltime=time;
    return timediff(time,*lltime);
}
/* glonass frequency channel number in rtcm (0-13,-1:error) ------------------*/
static int fcn_glo(int sat, rtcm_t *rtcm)
{
    int prn;
    if (satsys(sat,&prn)!=SYS_GLO||rtcm->nav.geph[prn-1].sat!=sat) return -1;
    return rtcm->nav.geph[prn-1].frq+7;
}
/* lock time indicator (ref [2] table 3.4-2) ---------------------------------*/
static int to_lock(int lock)
{
    if (lock<0  ) return 0;
    if (lock<24 ) return lock;
    if (lock<72 ) return (lock+24  )/2;
    if (lock<168) return (lock+120 )/4;
    if (lock<360) return (lock+408 )/8;
    if (lock<744) return (lock+1176)/16;
    if (lock<937) return (lock+3096)/32;
    return 127;
}
/* msm lock time indicator (ref [17] table 3.5-74) ---------------------------*/
static int to_msm_lock(double lock)
{
    if (lock<0.032  ) return 0;
    if (lock<0.064  ) return 1;
    if (lock<0.128  ) return 2;
    if (lock<0.256  ) return 3;
    if (lock<0.512  ) return 4;
    if (lock<1.024  ) return 5;
    if (lock<2.048  ) return 6;
    if (lock<4.096  ) return 7;
    if (lock<8.192  ) return 8;
    if (lock<16.384 ) return 9;
    if (lock<32.768 ) return 10;
    if (lock<65.536 ) return 11;
    if (lock<131.072) return 12;
    if (lock<262.144) return 13;
    if (lock<524.288) return 14;
    return 15;
}
/* msm lock time indicator with extended-resolution (ref [17] table 3.5-76) --*/
static int to_msm_lock_ex(double lock)
{
    int lock_ms = (int)(lock * 1000.0);
    
    if (lock<0.0      ) return 0;
    if (lock<0.064    ) return lock_ms;
    if (lock<0.128    ) return (lock_ms+64       )/2;
    if (lock<0.256    ) return (lock_ms+256      )/4;
    if (lock<0.512    ) return (lock_ms+768      )/8;
    if (lock<1.024    ) return (lock_ms+2048     )/16;
    if (lock<2.048    ) return (lock_ms+5120     )/32;
    if (lock<4.096    ) return (lock_ms+12288    )/64;
    if (lock<8.192    ) return (lock_ms+28672    )/128;
    if (lock<16.384   ) return (lock_ms+65536    )/256;
    if (lock<32.768   ) return (lock_ms+147456   )/512;
    if (lock<65.536   ) return (lock_ms+327680   )/1024;
    if (lock<131.072  ) return (lock_ms+720896   )/2048;
    if (lock<262.144  ) return (lock_ms+1572864  )/4096;
    if (lock<524.288  ) return (lock_ms+3407872  )/8192;
    if (lock<1048.576 ) return (lock_ms+7340032  )/16384;
    if (lock<2097.152 ) return (lock_ms+15728640 )/32768;
    if (lock<4194.304 ) return (lock_ms+33554432 )/65536;
    if (lock<8388.608 ) return (lock_ms+71303168 )/131072;
    if (lock<16777.216) return (lock_ms+150994944)/262144;
    if (lock<33554.432) return (lock_ms+318767104)/524288;
    if (lock<67108.864) return (lock_ms+671088640)/1048576;
    return 704;
}
/* L1 code indicator gps -----------------------------------------------------*/
static int to_code1_gps(unsigned char code)
{
    switch (code) {
        case CODE_L1C: return 0; /* L1 C/A */
        case CODE_L1P:
        case CODE_L1W:
        case CODE_L1Y:
        case CODE_L1N: return 1; /* L1 P(Y) direct */
    }
    return 0;
}
/* L2 code indicator gps -----------------------------------------------------*/
static int to_code2_gps(unsigned char code)
{
    switch (code) {
        case CODE_L2C:
        case CODE_L2S:
        case CODE_L2L:
        case CODE_L2X: return 0; /* L2 C/A or L2C */
        case CODE_L2P:
        case CODE_L2Y: return 1; /* L2 P(Y) direct */
        case CODE_L2D: return 2; /* L2 P(Y) cross-correlated */
        case CODE_L2W:
        case CODE_L2N: return 3; /* L2 correlated P/Y */
    }
    return 0;
}
/* L1 code indicator glonass -------------------------------------------------*/
static int to_code1_glo(unsigned char code)
{
    switch (code) {
        case CODE_L1C: return 0; /* L1 C/A */
        case CODE_L1P: return 1; /* L1 P */
    }
    return 0;
}
/* L2 code indicator glonass -------------------------------------------------*/
static int to_code2_glo(unsigned char code)
{
    switch (code) {
        case CODE_L2C: return 0; /* L2 C/A */
        case CODE_L2P: return 1; /* L2 P */
    }
    return 0;
}
/* carrier-phase - pseudorange in cycle --------------------------------------*/
static double cp_pr(double cp, double pr_cyc)
{
    return fmod(cp-pr_cyc+750.0,1500.0)-750.0;
}
/* generate obs field data gps -----------------------------------------------*/
static void gen_obs_gps(rtcm_t *rtcm, const obsd_t *data, int *code1, int *pr1,
                        int *ppr1, int *lock1, int *amb, int *cnr1, int *code2,
                        int *pr21, int *ppr2, int *lock2, int *cnr2)
{
    double lam1,lam2,pr1c=0.0,ppr;
    int lt1,lt2;
    
    lam1=CLIGHT/FREQ1;
    lam2=CLIGHT/FREQ2;
    *pr1=*amb=0;
    if (ppr1) *ppr1=0xFFF80000; /* invalid values */
    if (pr21) *pr21=0xFFFFE000;
    if (ppr2) *ppr2=0xFFF80000;
    
    /* L1 peudorange */
    if (data->P[0]!=0.0&&data->code[0]) {
        *amb=(int)floor(data->P[0]/PRUNIT_GPS);
        *pr1=ROUND((data->P[0]-*amb*PRUNIT_GPS)/0.02);
        pr1c=*pr1*0.02+*amb*PRUNIT_GPS;
    }
    /* L1 phaserange - L1 pseudorange */
    if (data->P[0]!=0.0&&data->L[0]!=0.0&&data->code[0]) {
        ppr=cp_pr(data->L[0],pr1c/lam1);
        if (ppr1) *ppr1=ROUND(ppr*lam1/0.0005);
    }
    /* L2 -L1 pseudorange */
    if (data->P[0]!=0.0&&data->P[1]!=0.0&&data->code[0]&&data->code[1]&&
        fabs(data->P[1]-pr1c)<=163.82) {
        if (pr21) *pr21=ROUND((data->P[1]-pr1c)/0.02);
    }
    /* L2 phaserange - L1 pseudorange */
    if (data->P[0]!=0.0&&data->L[1]!=0.0&&data->code[0]&&data->code[1]) {
        ppr=cp_pr(data->L[1],pr1c/lam2);
        if (ppr2) *ppr2=ROUND(ppr*lam2/0.0005);
    }
    lt1=locktime(data->time,rtcm->lltime[data->sat-1]  ,data->LLI[0]);
    lt2=locktime(data->time,rtcm->lltime[data->sat-1]+1,data->LLI[1]);
    
    if (lock1) *lock1=to_lock(lt1);
    if (lock2) *lock2=to_lock(lt2);
    if (cnr1 ) *cnr1=data->SNR[0];
    if (cnr2 ) *cnr2=data->SNR[1];
    if (code1) *code1=to_code1_gps(data->code[0]);
    if (code2) *code2=to_code2_gps(data->code[1]);
}
/* generate obs field data glonass -------------------------------------------*/
static void gen_obs_glo(rtcm_t *rtcm, const obsd_t *data, int fcn, int *code1,
                        int *pr1, int *ppr1, int *lock1, int *amb, int *cnr1,
                        int *code2, int *pr21, int *ppr2, int *lock2, int *cnr2)
{
    double lam1=0.0,lam2=0.0,pr1c=0.0,ppr;
    int lt1,lt2;
    
    if (fcn>=0) {
        lam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(fcn-7));
        lam2=CLIGHT/(FREQ2_GLO+DFRQ2_GLO*(fcn-7));
    }
    *pr1=*amb=0;
    if (ppr1) *ppr1=0xFFF80000; /* invalid values */
    if (pr21) *pr21=0xFFFFE000;
    if (ppr2) *ppr2=0xFFF80000;
    
    /* L1 peudorange */
    if (data->P[0]!=0.0) {
        *amb=(int)floor(data->P[0]/PRUNIT_GLO);
        *pr1=ROUND((data->P[0]-*amb*PRUNIT_GLO)/0.02);
        pr1c=*pr1*0.02+*amb*PRUNIT_GLO;
    }
    /* L1 phaserange - L1 pseudorange */
    if (data->P[0]!=0.0&&data->L[0]!=0.0&&data->code[0]&&lam1>0.0) {
        ppr=cp_pr(data->L[0],pr1c/lam1);
        if (ppr1) *ppr1=ROUND(ppr*lam1/0.0005);
    }
    /* L2 -L1 pseudorange */
    if (data->P[0]!=0.0&&data->P[1]!=0.0&&data->code[0]&&data->code[1]&&
        fabs(data->P[1]-pr1c)<=163.82) {
        if (pr21) *pr21=ROUND((data->P[1]-pr1c)/0.02);
    }
    /* L2 phaserange - L1 pseudorange */
    if (data->P[0]!=0.0&&data->L[1]!=0.0&&data->code[0]&&data->code[1]&&
        lam2>0.0) {
        ppr=cp_pr(data->L[1],pr1c/lam2);
        if (ppr2) *ppr2=ROUND(ppr*lam2/0.0005);
    }
    lt1=locktime(data->time,rtcm->lltime[data->sat-1]  ,data->LLI[0]);
    lt2=locktime(data->time,rtcm->lltime[data->sat-1]+1,data->LLI[1]);
    
    if (lock1) *lock1=to_lock(lt1);
    if (lock2) *lock2=to_lock(lt2);
    if (cnr1 ) *cnr1=data->SNR[0];
    if (cnr2 ) *cnr2=data->SNR[1];
    if (code1) *code1=to_code1_glo(data->code[0]);
    if (code2) *code2=to_code2_glo(data->code[1]);
}
/* encode rtcm header --------------------------------------------------------*/
static int encode_head(int type, rtcm_t *rtcm, int sys, int sync, int nsat)
{
    double tow;
    int i=24,week,epoch;
    
    trace(4,"encode_head: type=%d sync=%d sys=%d nsat=%d\n",type,sync,sys,nsat);
    
    setbitu(rtcm->buff,i,12,type       ); i+=12; /* message no */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* ref station id */
    
    if (sys==SYS_GLO) {
        tow=time2gpst(timeadd(gpst2utc(rtcm->time),10800.0),&week);
        epoch=ROUND(fmod(tow,86400.0)/0.001);
        setbitu(rtcm->buff,i,27,epoch); i+=27; /* glonass epoch time */
    }
    else {
        tow=time2gpst(rtcm->time,&week);
        epoch=ROUND(tow/0.001);
        setbitu(rtcm->buff,i,30,epoch); i+=30; /* gps epoch time */
    }
    setbitu(rtcm->buff,i, 1,sync); i+= 1; /* synchronous gnss flag */
    setbitu(rtcm->buff,i, 5,nsat); i+= 5; /* no of satellites */
    setbitu(rtcm->buff,i, 1,0   ); i+= 1; /* smoothing indicator */
    setbitu(rtcm->buff,i, 3,0   ); i+= 3; /* smoothing interval */
    return i;
}
/* encode type 1001: basic L1-only gps rtk observables -----------------------*/
static int encode_type1001(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sys,prn;
    int code1,pr1,ppr1,lock1,amb;
    
    trace(3,"encode_type1001: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1001,rtcm,SYS_GPS,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        
        if (sys==SYS_SBS) prn-=80; /* 40-58: sbas 120-138 */
        
        /* generate obs field data gps */
        gen_obs_gps(rtcm,rtcm->obs.data+j,&code1,&pr1,&ppr1,&lock1,&amb,NULL,
                    NULL,NULL,NULL,NULL,NULL);
        
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i,24,pr1  ); i+=24;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1002: extended L1-only gps rtk observables --------------------*/
static int encode_type1002(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sys,prn;
    int code1,pr1,ppr1,lock1,amb,cnr1;
    
    trace(3,"encode_type1002: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1002,rtcm,SYS_GPS,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        
        if (sys==SYS_SBS) prn-=80; /* 40-58: sbas 120-138 */
        
        /* generate obs field data gps */
        gen_obs_gps(rtcm,rtcm->obs.data+j,&code1,&pr1,&ppr1,&lock1,&amb,&cnr1,
                    NULL,NULL,NULL,NULL,NULL);
        
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i,24,pr1  ); i+=24;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 8,amb  ); i+= 8;
        setbitu(rtcm->buff,i, 8,cnr1 ); i+= 8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1003: basic L1&L2 gps rtk observables -------------------------*/
static int encode_type1003(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sys,prn;
    int code1,pr1,ppr1,lock1,amb,code2,pr21,ppr2,lock2;
    
    trace(3,"encode_type1003: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1003,rtcm,SYS_GPS,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        
        if (sys==SYS_SBS) prn-=80; /* 40-58: sbas 120-138 */
        
        /* generate obs field data gps */
        gen_obs_gps(rtcm,rtcm->obs.data+j,&code1,&pr1,&ppr1,&lock1,&amb,
                    NULL,&code2,&pr21,&ppr2,&lock2,NULL);
        
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i,24,pr1  ); i+=24;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 2,code2); i+= 2;
        setbits(rtcm->buff,i,14,pr21 ); i+=14;
        setbits(rtcm->buff,i,20,ppr2 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock2); i+= 7;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1004: extended L1&L2 gps rtk observables ----------------------*/
static int encode_type1004(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sys,prn;
    int code1,pr1,ppr1,lock1,amb,cnr1,code2,pr21,ppr2,lock2,cnr2;
    
    trace(3,"encode_type1004: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1004,rtcm,SYS_GPS,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sys=satsys(rtcm->obs.data[j].sat,&prn);
        if (!(sys&(SYS_GPS|SYS_SBS))) continue;
        
        if (sys==SYS_SBS) prn-=80; /* 40-58: sbas 120-138 */
        
        /* generate obs field data gps */
        gen_obs_gps(rtcm,rtcm->obs.data+j,&code1,&pr1,&ppr1,&lock1,&amb,
                    &cnr1,&code2,&pr21,&ppr2,&lock2,&cnr2);
        
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i,24,pr1  ); i+=24;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 8,amb  ); i+= 8;
        setbitu(rtcm->buff,i, 8,cnr1 ); i+= 8;
        setbitu(rtcm->buff,i, 2,code2); i+= 2;
        setbits(rtcm->buff,i,14,pr21 ); i+=14;
        setbits(rtcm->buff,i,20,ppr2 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock2); i+= 7;
        setbitu(rtcm->buff,i, 8,cnr2 ); i+= 8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1005: stationary rtk reference station arp --------------------*/
static int encode_type1005(rtcm_t *rtcm, int sync)
{
    double *p=rtcm->sta.pos;
    int i=24;
    
    trace(3,"encode_type1005: sync=%d\n",sync);
    
    setbitu(rtcm->buff,i,12,1005       ); i+=12; /* message no */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* ref station id */
    setbitu(rtcm->buff,i, 6,0          ); i+= 6; /* itrf realization year */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* gps indicator */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* glonass indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* galileo indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* ref station indicator */
    set38bits(rtcm->buff,i,p[0]/0.0001 ); i+=38; /* antenna ref point ecef-x */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* oscillator indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* reserved */
    set38bits(rtcm->buff,i,p[1]/0.0001 ); i+=38; /* antenna ref point ecef-y */
    setbitu(rtcm->buff,i, 2,0          ); i+= 2; /* quarter cycle indicator */
    set38bits(rtcm->buff,i,p[2]/0.0001 ); i+=38; /* antenna ref point ecef-z */
    rtcm->nbit=i;
    return 1;
}
/* encode type 1006: stationary rtk reference station arp with height --------*/
static int encode_type1006(rtcm_t *rtcm, int sync)
{
    double *p=rtcm->sta.pos;
    int i=24,hgt=0;
    
    trace(3,"encode_type1006: sync=%d\n",sync);
    
    if (0.0<=rtcm->sta.hgt&&rtcm->sta.hgt<=6.5535) {
        hgt=ROUND(rtcm->sta.hgt/0.0001);
    }
    else {
        trace(2,"antenna height error: h=%.4f\n",rtcm->sta.hgt);
    }
    setbitu(rtcm->buff,i,12,1006       ); i+=12; /* message no */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* ref station id */
    setbitu(rtcm->buff,i, 6,0          ); i+= 6; /* itrf realization year */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* gps indicator */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* glonass indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* galileo indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* ref station indicator */
    set38bits(rtcm->buff,i,p[0]/0.0001 ); i+=38; /* antenna ref point ecef-x */
    setbitu(rtcm->buff,i, 1,1          ); i+= 1; /* oscillator indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* reserved */
    set38bits(rtcm->buff,i,p[1]/0.0001 ); i+=38; /* antenna ref point ecef-y */
    setbitu(rtcm->buff,i, 2,0          ); i+= 2; /* quarter cycle indicator */
    set38bits(rtcm->buff,i,p[2]/0.0001 ); i+=38; /* antenna ref point ecef-z */
    setbitu(rtcm->buff,i,16,hgt        ); i+=16; /* antenna height */
    rtcm->nbit=i;
    return 1;
}
/* encode type 1007: antenna descriptor --------------------------------------*/
static int encode_type1007(rtcm_t *rtcm, int sync)
{
    int i=24,j,antsetup=rtcm->sta.antsetup;
    int n=MIN(strlen(rtcm->sta.antdes),31);
    
    trace(3,"encode_type1007: sync=%d\n",sync);
    
    setbitu(rtcm->buff,i,12,1007       ); i+=12; /* message no */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* ref station id */
    
    /* antenna descriptor */
    setbitu(rtcm->buff,i,8,n); i+=8;
    for (j=0;j<n;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.antdes[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,antsetup); i+=8; /* antetnna setup id */
    rtcm->nbit=i;
    return 1;
}
/* encode type 1008: antenna descriptor & serial number ----------------------*/
static int encode_type1008(rtcm_t *rtcm, int sync)
{
    int i=24,j,antsetup=rtcm->sta.antsetup;
    int n=MIN(strlen(rtcm->sta.antdes),31);
    int m=MIN(strlen(rtcm->sta.antsno),31);
    
    trace(3,"encode_type1008: sync=%d\n",sync);
    
    setbitu(rtcm->buff,i,12,1008       ); i+=12; /* message no */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* ref station id */
    
    /* antenna descriptor */
    setbitu(rtcm->buff,i,8,n); i+=8;
    for (j=0;j<n;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.antdes[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,antsetup); i+=8; /* antenna setup id */
    
    /* antenna serial number */
    setbitu(rtcm->buff,i,8,m); i+=8;
    for (j=0;j<m;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.antsno[j]); i+=8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1009: basic L1-only glonass rtk observables -------------------*/
static int encode_type1009(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sat,prn,fcn;
    int code1,pr1,ppr1,lock1,amb;
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1009,rtcm,SYS_GLO,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        fcn=fcn_glo(sat,rtcm);
        
        /* generate obs field data glonass */
        gen_obs_glo(rtcm,rtcm->obs.data+j,fcn,&code1,&pr1,&ppr1,&lock1,&amb,
                    NULL,NULL,NULL,NULL,NULL,NULL);
        
        if (fcn<0) fcn=0;
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i, 5,fcn  ); i+= 5;
        setbitu(rtcm->buff,i,25,pr1  ); i+=25;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1010: extended L1-only glonass rtk observables ----------------*/
static int encode_type1010(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sat,prn,fcn;
    int code1,pr1,ppr1,lock1,amb,cnr1;
    
    trace(3,"encode_type1010: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1010,rtcm,SYS_GLO,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        fcn=fcn_glo(sat,rtcm);
        
        /* generate obs field data glonass */
        gen_obs_glo(rtcm,rtcm->obs.data+j,fcn,&code1,&pr1,&ppr1,&lock1,&amb,
                    &cnr1,NULL,NULL,NULL,NULL,NULL);
        
        if (fcn<0) fcn=0;
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i, 5,fcn  ); i+= 5;
        setbitu(rtcm->buff,i,25,pr1  ); i+=25;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 7,amb  ); i+= 7;
        setbitu(rtcm->buff,i, 8,cnr1 ); i+= 8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1011: basic  L1&L2 glonass rtk observables --------------------*/
static int encode_type1011(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sat,prn,fcn;
    int code1,pr1,ppr1,lock1,amb,code2,pr21,ppr2,lock2;
    
    trace(3,"encode_type1011: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1011,rtcm,SYS_GLO,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        fcn=fcn_glo(sat,rtcm);
        
        /* generate obs field data glonass */
        gen_obs_glo(rtcm,rtcm->obs.data+j,fcn,&code1,&pr1,&ppr1,&lock1,&amb,
                    NULL,&code2,&pr21,&ppr2,&lock2,NULL);
        
        if (fcn<0) fcn=0;
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i, 5,fcn  ); i+= 5;
        setbitu(rtcm->buff,i,25,pr1  ); i+=25;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 2,code2); i+= 2;
        setbits(rtcm->buff,i,14,pr21 ); i+=14;
        setbits(rtcm->buff,i,20,ppr2 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock2); i+= 7;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1012: extended L1&L2 glonass rtk observables ------------------*/
static int encode_type1012(rtcm_t *rtcm, int sync)
{
    int i,j,nsat=0,sat,prn,fcn;
    int code1,pr1,ppr1,lock1,amb,cnr1,code2,pr21,ppr2,lock2,cnr2;
    
    trace(3,"encode_type1012: sync=%d\n",sync);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        nsat++;
    }
    /* encode header */
    i=encode_head(1012,rtcm,SYS_GLO,sync,nsat);
    
    for (j=0;j<rtcm->obs.n&&nsat<MAXOBS;j++) {
        sat=rtcm->obs.data[j].sat;
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        fcn=fcn_glo(sat,rtcm);
        
        /* generate obs field data glonass */
        gen_obs_glo(rtcm,rtcm->obs.data+j,fcn,&code1,&pr1,&ppr1,&lock1,&amb,
                    &cnr1,&code2,&pr21,&ppr2,&lock2,&cnr2);
        
        if (fcn<0) fcn=0;
        setbitu(rtcm->buff,i, 6,prn  ); i+= 6;
        setbitu(rtcm->buff,i, 1,code1); i+= 1;
        setbitu(rtcm->buff,i, 5,fcn  ); i+= 5;
        setbitu(rtcm->buff,i,25,pr1  ); i+=25;
        setbits(rtcm->buff,i,20,ppr1 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock1); i+= 7;
        setbitu(rtcm->buff,i, 7,amb  ); i+= 7;
        setbitu(rtcm->buff,i, 8,cnr1 ); i+= 8;
        setbitu(rtcm->buff,i, 2,code2); i+= 2;
        setbits(rtcm->buff,i,14,pr21 ); i+=14;
        setbits(rtcm->buff,i,20,ppr2 ); i+=20;
        setbitu(rtcm->buff,i, 7,lock2); i+= 7;
        setbitu(rtcm->buff,i, 8,cnr2 ); i+= 8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1019: gps ephemerides -----------------------------------------*/
static int encode_type1019(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,tgd;
    
    trace(3,"encode_type1019: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_GPS) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week=eph->week%1024;
    toe  =ROUND(eph->toes/16.0);
    toc  =ROUND(time2gpst(eph->toc,NULL)/16.0);
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_5 );
    crc  =ROUND(eph->crc/P2_5 );
    cus  =ROUND(eph->cus/P2_29);
    cuc  =ROUND(eph->cuc/P2_29);
    cis  =ROUND(eph->cis/P2_29);
    cic  =ROUND(eph->cic/P2_29);
    af0  =ROUND(eph->f0 /P2_31);
    af1  =ROUND(eph->f1 /P2_43);
    af2  =ROUND(eph->f2 /P2_55);
    tgd  =ROUND(eph->tgd[0]/P2_31);
    
    setbitu(rtcm->buff,i,12,1019     ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i,10,week     ); i+=10;
    setbitu(rtcm->buff,i, 4,eph->sva ); i+= 4;
    setbitu(rtcm->buff,i, 2,eph->code); i+= 2;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i, 8,eph->iode); i+= 8;
    setbitu(rtcm->buff,i,16,toc      ); i+=16;
    setbits(rtcm->buff,i, 8,af2      ); i+= 8;
    setbits(rtcm->buff,i,16,af1      ); i+=16;
    setbits(rtcm->buff,i,22,af0      ); i+=22;
    setbitu(rtcm->buff,i,10,eph->iodc); i+=10;
    setbits(rtcm->buff,i,16,crs      ); i+=16;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,16,cuc      ); i+=16;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,16,cus      ); i+=16;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,16,toe      ); i+=16;
    setbits(rtcm->buff,i,16,cic      ); i+=16;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,16,cis      ); i+=16;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,16,crc      ); i+=16;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i, 8,tgd      ); i+= 8;
    setbitu(rtcm->buff,i, 6,eph->svh ); i+= 6;
    setbitu(rtcm->buff,i, 1,eph->flag); i+= 1;
    setbitu(rtcm->buff,i, 1,eph->fit>0.0?0:1); i+=1;
    rtcm->nbit=i;
    return 1;
}
/* encode type 1020: glonass ephemerides -------------------------------------*/
static int encode_type1020(rtcm_t *rtcm, int sync)
{
    geph_t *geph;
    gtime_t time;
    double ep[6];
    int i=24,j,prn,tk_h,tk_m,tk_s,tb,pos[3],vel[3],acc[3],gamn,taun,dtaun;
    int fcn,NT;
    
    trace(3,"encode_type1020: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_GLO) return 0;
    geph=rtcm->nav.geph+prn-1;
    if (geph->sat!=rtcm->ephsat) return 0;
    fcn=geph->frq+7;
    
    /* time of frame within day (utc(su) + 3 hr) */
    time=timeadd(gpst2utc(geph->tof),10800.0);
    time2epoch(time,ep);
    tk_h=(int)ep[3];
    tk_m=(int)ep[4];
    tk_s=ROUND(ep[5]/30.0);
    
    /* # of days since jan 1 in leap year */
    ep[0]=floor(ep[0]/4.0)*4.0; ep[1]=ep[2]=1.0;
    ep[3]=ep[4]=ep[5]=0.0;
    NT=(int)floor(timediff(time,epoch2time(ep))/86400.+1.0);
    
    /* index of time interval within day (utc(su) + 3 hr) */
    time=timeadd(gpst2utc(geph->toe),10800.0);
    time2epoch(time,ep);
    tb=ROUND((ep[3]*3600.0+ep[4]*60.0+ep[5])/900.0);
    
    for (j=0;j<3;j++) {
        pos[j]=ROUND(geph->pos[j]/P2_11/1E3);
        vel[j]=ROUND(geph->vel[j]/P2_20/1E3);
        acc[j]=ROUND(geph->acc[j]/P2_30/1E3);
    }
    gamn =ROUND(geph->gamn /P2_40);
    taun =ROUND(geph->taun /P2_30);
    dtaun=ROUND(geph->dtaun/P2_30);
    
    setbitu(rtcm->buff,i,12,1020     ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i, 5,fcn      ); i+= 5;
    setbitu(rtcm->buff,i, 4,0        ); i+= 4; /* almanac health,P1 */
    setbitu(rtcm->buff,i, 5,tk_h     ); i+= 5;
    setbitu(rtcm->buff,i, 6,tk_m     ); i+= 6;
    setbitu(rtcm->buff,i, 1,tk_s     ); i+= 1;
    setbitu(rtcm->buff,i, 1,geph->svh); i+= 1; /* Bn */
    setbitu(rtcm->buff,i, 1,0        ); i+= 1; /* P2 */
    setbitu(rtcm->buff,i, 7,tb       ); i+= 7;
    setbitg(rtcm->buff,i,24,vel[0]   ); i+=24;
    setbitg(rtcm->buff,i,27,pos[0]   ); i+=27;
    setbitg(rtcm->buff,i, 5,acc[0]   ); i+= 5;
    setbitg(rtcm->buff,i,24,vel[1]   ); i+=24;
    setbitg(rtcm->buff,i,27,pos[1]   ); i+=27;
    setbitg(rtcm->buff,i, 5,acc[1]   ); i+= 5;
    setbitg(rtcm->buff,i,24,vel[2]   ); i+=24;
    setbitg(rtcm->buff,i,27,pos[2]   ); i+=27;
    setbitg(rtcm->buff,i, 5,acc[2]   ); i+= 5;
    setbitu(rtcm->buff,i, 1,0        ); i+= 1; /* P3 */
    setbitg(rtcm->buff,i,11,gamn     ); i+=11;
    setbitu(rtcm->buff,i, 3,0        ); i+= 3; /* P,ln */
    setbitg(rtcm->buff,i,22,taun     ); i+=22;
    setbitu(rtcm->buff,i, 5,dtaun    ); i+= 5;
    setbitu(rtcm->buff,i, 5,geph->age); i+= 5; /* En */
    setbitu(rtcm->buff,i, 1,0        ); i+= 1; /* P4 */
    setbitu(rtcm->buff,i, 4,0        ); i+= 4; /* FT */
    setbitu(rtcm->buff,i,11,NT       ); i+=11;
    setbitu(rtcm->buff,i, 2,0        ); i+= 2; /* M */
    setbitu(rtcm->buff,i, 1,0        ); i+= 1; /* flag for addtional data */
    setbitu(rtcm->buff,i,11,0        ); i+=11; /* NA */
    setbitu(rtcm->buff,i,32,0        ); i+=32; /* tauc */
    setbitu(rtcm->buff,i, 5,0        ); i+= 5; /* N4 */
    setbitu(rtcm->buff,i,22,0        ); i+=22; /* taugps */
    setbitu(rtcm->buff,i, 1,0        ); i+= 1; /* ln */
    setbitu(rtcm->buff,i, 7,0        ); i+= 7;
    rtcm->nbit=i;
    return 1;
}
/* encode type 1033: receiver and antenna descriptor -------------------------*/
static int encode_type1033(rtcm_t *rtcm, int sync)
{
    int i=24,j,antsetup=rtcm->sta.antsetup;
    int n=MIN(strlen(rtcm->sta.antdes ),31);
    int m=MIN(strlen(rtcm->sta.antsno ),31);
    int I=MIN(strlen(rtcm->sta.rectype),31);
    int J=MIN(strlen(rtcm->sta.recver ),31);
    int K=MIN(strlen(rtcm->sta.recsno ),31);
    
    trace(3,"encode_type1033: sync=%d\n",sync);
    
    setbitu(rtcm->buff,i,12,1033       ); i+=12;
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12;
    
    setbitu(rtcm->buff,i,8,n); i+= 8;
    for (j=0;j<n;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.antdes[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,antsetup); i+= 8;
    
    setbitu(rtcm->buff,i,8,m); i+= 8;
    for (j=0;j<m;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.antsno[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,I); i+= 8;
    for (j=0;j<I;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.rectype[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,J); i+= 8;
    for (j=0;j<J;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.recver[j]); i+=8;
    }
    setbitu(rtcm->buff,i,8,K); i+= 8;
    for (j=0;j<K;j++) {
        setbitu(rtcm->buff,i,8,rtcm->sta.recsno[j]); i+=8;
    }
    rtcm->nbit=i;
    return 1;
}
/* encode type 1044: qzss ephemerides (ref [15]) -----------------------------*/
static int encode_type1044(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,tgd;
    
    trace(3,"encode_type1044: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_QZS) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week=eph->week%1024;
    toe  =ROUND(eph->toes/16.0);
    toc  =ROUND(time2gpst(eph->toc,NULL)/16.0);
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_5 );
    crc  =ROUND(eph->crc/P2_5 );
    cus  =ROUND(eph->cus/P2_29);
    cuc  =ROUND(eph->cuc/P2_29);
    cis  =ROUND(eph->cis/P2_29);
    cic  =ROUND(eph->cic/P2_29);
    af0  =ROUND(eph->f0 /P2_31);
    af1  =ROUND(eph->f1 /P2_43);
    af2  =ROUND(eph->f2 /P2_55);
    tgd  =ROUND(eph->tgd[0]/P2_31);
    
    setbitu(rtcm->buff,i,12,1044     ); i+=12;
    setbitu(rtcm->buff,i, 4,prn-192  ); i+= 4;
    setbitu(rtcm->buff,i,16,toc      ); i+=16;
    setbits(rtcm->buff,i, 8,af2      ); i+= 8;
    setbits(rtcm->buff,i,16,af1      ); i+=16;
    setbits(rtcm->buff,i,22,af0      ); i+=22;
    setbitu(rtcm->buff,i, 8,eph->iode); i+= 8;
    setbits(rtcm->buff,i,16,crs      ); i+=16;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,16,cuc      ); i+=16;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,16,cus      ); i+=16;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,16,toe      ); i+=16;
    setbits(rtcm->buff,i,16,cic      ); i+=16;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,16,cis      ); i+=16;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,16,crc      ); i+=16;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i, 2,eph->code); i+= 2;
    setbitu(rtcm->buff,i,10,week     ); i+=10;
    setbitu(rtcm->buff,i, 4,eph->sva ); i+= 4;
    setbitu(rtcm->buff,i, 6,eph->svh ); i+= 6;
    setbits(rtcm->buff,i, 8,tgd      ); i+= 8;
    setbitu(rtcm->buff,i,10,eph->iodc); i+=10;
    setbitu(rtcm->buff,i, 1,eph->fit==2.0?0:1); i+=1;
    rtcm->nbit=i;
    return 1;
}
/* encode type 1045: galileo F/NAV satellite ephemerides (ref [15]) ----------*/
static int encode_type1045(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,bgd1,bgd2,oshs,osdvs;
    
    trace(3,"encode_type1045: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_GAL) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week=(eph->week-1024)%4096; /* gst-week = gal-week - 1024 */
    toe  =ROUND(eph->toes/60.0);
    toc  =ROUND(time2gpst(eph->toc,NULL)/60.0);
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_5 );
    crc  =ROUND(eph->crc/P2_5 );
    cus  =ROUND(eph->cus/P2_29);
    cuc  =ROUND(eph->cuc/P2_29);
    cis  =ROUND(eph->cis/P2_29);
    cic  =ROUND(eph->cic/P2_29);
    af0  =ROUND(eph->f0 /P2_34);
    af1  =ROUND(eph->f1 /P2_46);
    af2  =ROUND(eph->f2 /P2_59);
    bgd1 =ROUND(eph->tgd[0]/P2_32); /* E5a/E1 */
    bgd2 =ROUND(eph->tgd[1]/P2_32); /* E5b/E1 */
    oshs =(eph->svh>>4)&3;          /* E5a SVH */
    osdvs=(eph->svh>>3)&1;          /* E5a DVS */
    setbitu(rtcm->buff,i,12,1045     ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i,12,week     ); i+=12;
    setbitu(rtcm->buff,i,10,eph->iode); i+=10;
    setbitu(rtcm->buff,i, 8,eph->sva ); i+= 8;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i,14,toc      ); i+=14;
    setbits(rtcm->buff,i, 6,af2      ); i+= 6;
    setbits(rtcm->buff,i,21,af1      ); i+=21;
    setbits(rtcm->buff,i,31,af0      ); i+=31;
    setbits(rtcm->buff,i,16,crs      ); i+=16;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,16,cuc      ); i+=16;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,16,cus      ); i+=16;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,14,toe      ); i+=14;
    setbits(rtcm->buff,i,16,cic      ); i+=16;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,16,cis      ); i+=16;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,16,crc      ); i+=16;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i,10,bgd1     ); i+=10;
    setbitu(rtcm->buff,i, 2,oshs     ); i+= 2; /* E5a SVH */
    setbitu(rtcm->buff,i, 1,osdvs    ); i+= 1; /* E5a DVS */
    setbitu(rtcm->buff,i, 7,0        ); i+= 7; /* reserved */
    rtcm->nbit=i;
    return 1;
}
/* encode type 1046: galileo I/NAV satellite ephemerides ---------------------*/
static int encode_type1046(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,bgd1,bgd2,oshs1,osdvs1,oshs2,osdvs2;
    
    trace(3,"encode_type1046: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_GAL) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week=(eph->week-1024)%4096; /* gst-week = gal-week - 1024 */
    toe  =ROUND(eph->toes/60.0);
    toc  =ROUND(time2gpst(eph->toc,NULL)/60.0);
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_5 );
    crc  =ROUND(eph->crc/P2_5 );
    cus  =ROUND(eph->cus/P2_29);
    cuc  =ROUND(eph->cuc/P2_29);
    cis  =ROUND(eph->cis/P2_29);
    cic  =ROUND(eph->cic/P2_29);
    af0  =ROUND(eph->f0 /P2_34);
    af1  =ROUND(eph->f1 /P2_46);
    af2  =ROUND(eph->f2 /P2_59);
    bgd1 =ROUND(eph->tgd[0]/P2_32); /* E5a/E1 */
    bgd2 =ROUND(eph->tgd[1]/P2_32); /* E5b/E1 */
    oshs1 =(eph->svh>>7)&3;         /* E5b SVH */
    osdvs1=(eph->svh>>6)&1;         /* E5b DVS */
    oshs2 =(eph->svh>>1)&3;         /* E1 SVH */
    osdvs2=(eph->svh>>0)&1;         /* E1 DVS */
    setbitu(rtcm->buff,i,12,1046     ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i,12,week     ); i+=12;
    setbitu(rtcm->buff,i,10,eph->iode); i+=10;
    setbitu(rtcm->buff,i, 8,eph->sva ); i+= 8;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i,14,toc      ); i+=14;
    setbits(rtcm->buff,i, 6,af2      ); i+= 6;
    setbits(rtcm->buff,i,21,af1      ); i+=21;
    setbits(rtcm->buff,i,31,af0      ); i+=31;
    setbits(rtcm->buff,i,16,crs      ); i+=16;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,16,cuc      ); i+=16;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,16,cus      ); i+=16;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,14,toe      ); i+=14;
    setbits(rtcm->buff,i,16,cic      ); i+=16;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,16,cis      ); i+=16;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,16,crc      ); i+=16;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i,10,bgd1     ); i+=10;
    setbits(rtcm->buff,i,10,bgd2     ); i+=10;
    setbitu(rtcm->buff,i, 2,oshs1    ); i+= 2; /* E5b SVH */
    setbitu(rtcm->buff,i, 1,osdvs1   ); i+= 1; /* E5b DVS */
    setbitu(rtcm->buff,i, 2,oshs2    ); i+= 2; /* E1 SVH */
    setbitu(rtcm->buff,i, 1,osdvs2   ); i+= 1; /* E1 DVS */
    rtcm->nbit=i;
    return 1;
}
/* encode type 1042: beidou ephemerides --------------------------------------*/
static int encode_type1042(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,tgd1,tgd2;
    
    trace(3,"encode_type1042: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_CMP) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week =eph->week%8192;
    toe  =ROUND(eph->toes/8.0);
    toc  =ROUND(time2bdt(gpst2bdt(eph->toc),NULL)/8.0); /* gpst -> bdt */
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_6 );
    crc  =ROUND(eph->crc/P2_6 );
    cus  =ROUND(eph->cus/P2_31);
    cuc  =ROUND(eph->cuc/P2_31);
    cis  =ROUND(eph->cis/P2_31);
    cic  =ROUND(eph->cic/P2_31);
    af0  =ROUND(eph->f0 /P2_33);
    af1  =ROUND(eph->f1 /P2_50);
    af2  =ROUND(eph->f2 /P2_66);
    tgd1 =ROUND(eph->tgd[0]/1E-10);
    tgd2 =ROUND(eph->tgd[1]/1E-10);
    
    setbitu(rtcm->buff,i,12,1042     ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i,13,week     ); i+=13;
    setbitu(rtcm->buff,i, 4,eph->sva ); i+= 4;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i, 5,eph->iode); i+= 5;
    setbitu(rtcm->buff,i,17,toc      ); i+=17;
    setbits(rtcm->buff,i,11,af2      ); i+=11;
    setbits(rtcm->buff,i,22,af1      ); i+=22;
    setbits(rtcm->buff,i,24,af0      ); i+=24;
    setbitu(rtcm->buff,i, 5,eph->iodc); i+= 5;
    setbits(rtcm->buff,i,18,crs      ); i+=18;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,18,cuc      ); i+=18;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,18,cus      ); i+=18;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,17,toe      ); i+=17;
    setbits(rtcm->buff,i,18,cic      ); i+=18;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,18,cis      ); i+=18;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,18,crc      ); i+=18;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i,10,tgd1     ); i+=10;
    setbits(rtcm->buff,i,10,tgd2     ); i+=10;
    setbitu(rtcm->buff,i, 1,eph->svh ); i+= 1;
    rtcm->nbit=i;
    return 1;
}
/* encode type 63: beidou ephemerides (RTCM draft) ---------------------------*/
static int encode_type63(rtcm_t *rtcm, int sync)
{
    eph_t *eph;
    unsigned int sqrtA,e;
    int i=24,prn,week,toe,toc,i0,OMG0,omg,M0,deln,idot,OMGd,crs,crc;
    int cus,cuc,cis,cic,af0,af1,af2,tgd1,tgd2;
    
    trace(3,"encode_type63: sync=%d\n",sync);
    
    if (satsys(rtcm->ephsat,&prn)!=SYS_CMP) return 0;
    eph=rtcm->nav.eph+rtcm->ephsat-1;
    if (eph->sat!=rtcm->ephsat) return 0;
    week =eph->week%8192;
    toe  =ROUND(eph->toes/8.0);
    toc  =ROUND(time2bdt(gpst2bdt(eph->toc),NULL)/8.0); /* gpst -> bdt */
    sqrtA=ROUND_U(sqrt(eph->A)/P2_19);
    e    =ROUND_U(eph->e/P2_33);
    i0   =ROUND(eph->i0  /P2_31/SC2RAD);
    OMG0 =ROUND(eph->OMG0/P2_31/SC2RAD);
    omg  =ROUND(eph->omg /P2_31/SC2RAD);
    M0   =ROUND(eph->M0  /P2_31/SC2RAD);
    deln =ROUND(eph->deln/P2_43/SC2RAD);
    idot =ROUND(eph->idot/P2_43/SC2RAD);
    OMGd =ROUND(eph->OMGd/P2_43/SC2RAD);
    crs  =ROUND(eph->crs/P2_6 );
    crc  =ROUND(eph->crc/P2_6 );
    cus  =ROUND(eph->cus/P2_31);
    cuc  =ROUND(eph->cuc/P2_31);
    cis  =ROUND(eph->cis/P2_31);
    cic  =ROUND(eph->cic/P2_31);
    af0  =ROUND(eph->f0 /P2_33);
    af1  =ROUND(eph->f1 /P2_50);
    af2  =ROUND(eph->f2 /P2_66);
    tgd1 =ROUND(eph->tgd[0]/1E-10);
    tgd2 =ROUND(eph->tgd[1]/1E-10);
    
    setbitu(rtcm->buff,i,12,63       ); i+=12;
    setbitu(rtcm->buff,i, 6,prn      ); i+= 6;
    setbitu(rtcm->buff,i,13,week     ); i+=13;
    setbitu(rtcm->buff,i, 4,eph->sva ); i+= 4;
    setbits(rtcm->buff,i,14,idot     ); i+=14;
    setbitu(rtcm->buff,i, 5,eph->iode); i+= 5;
    setbitu(rtcm->buff,i,17,toc      ); i+=17;
    setbits(rtcm->buff,i,11,af2      ); i+=11;
    setbits(rtcm->buff,i,22,af1      ); i+=22;
    setbits(rtcm->buff,i,24,af0      ); i+=24;
    setbitu(rtcm->buff,i, 5,eph->iodc); i+= 5;
    setbits(rtcm->buff,i,18,crs      ); i+=18;
    setbits(rtcm->buff,i,16,deln     ); i+=16;
    setbits(rtcm->buff,i,32,M0       ); i+=32;
    setbits(rtcm->buff,i,18,cuc      ); i+=18;
    setbitu(rtcm->buff,i,32,e        ); i+=32;
    setbits(rtcm->buff,i,18,cus      ); i+=18;
    setbitu(rtcm->buff,i,32,sqrtA    ); i+=32;
    setbitu(rtcm->buff,i,17,toe      ); i+=17;
    setbits(rtcm->buff,i,18,cic      ); i+=18;
    setbits(rtcm->buff,i,32,OMG0     ); i+=32;
    setbits(rtcm->buff,i,18,cis      ); i+=18;
    setbits(rtcm->buff,i,32,i0       ); i+=32;
    setbits(rtcm->buff,i,18,crc      ); i+=18;
    setbits(rtcm->buff,i,32,omg      ); i+=32;
    setbits(rtcm->buff,i,24,OMGd     ); i+=24;
    setbits(rtcm->buff,i,10,tgd1     ); i+=10;
    setbits(rtcm->buff,i,10,tgd2     ); i+=10;
    setbitu(rtcm->buff,i, 1,eph->svh ); i+= 1;
    rtcm->nbit=i;
    return 1;
}
/* encode ssr header ---------------------------------------------------------*/
static int encode_ssr_head(int type, rtcm_t *rtcm, int sys, int nsat, int sync,
                           int iod, double udint, int refd, int provid,
                           int solid)
{
    double tow;
    int i=24,msgno,epoch,week,udi,ns;
    
    trace(4,"encode_ssr_head: type=%d sys=%d nsat=%d sync=%d iod=%d udint=%.0f\n",
          type,sys,nsat,sync,iod,udint);
    
    ns=sys==SYS_QZS?4:6;
    
    switch (sys) {
        case SYS_GPS: msgno=1056+type; break;
        case SYS_GLO: msgno=1062+type; break;
        case SYS_GAL: msgno=1239+type; break;
        case SYS_QZS: msgno=1245+type; break;
        case SYS_CMP: msgno=1257+type; break;
        case SYS_SBS: msgno=1251+type; break;
        default: return 0;
    }
    setbitu(rtcm->buff,i,12,msgno    ); i+=12; /* message number */
    
    if (sys==SYS_GLO) {
        tow=time2gpst(timeadd(gpst2utc(rtcm->time),10800.0),&week);
        epoch=ROUND(fmod(tow,86400.0));
        setbitu(rtcm->buff,i,17,epoch); i+=17; /* glonass epoch time */
    }
    else {
        tow=time2gpst(rtcm->time,&week);
        epoch=ROUND(tow);
        setbitu(rtcm->buff,i,20,epoch); i+=20; /* gps epoch time */
    }
    for (udi=0;udi<15;udi++) {
        if (ssrudint[udi]>=udint) break;
    }
    setbitu(rtcm->buff,i, 4,udi    ); i+= 4; /* update interval */
    setbitu(rtcm->buff,i, 1,sync   ); i+= 1; /* multiple message indicator */
    if (type==1||type==4) {
        setbitu(rtcm->buff,i,1,refd); i+= 1; /* satellite ref datum */
    }
    setbitu(rtcm->buff,i, 4,iod    ); i+= 4; /* iod ssr */
    setbitu(rtcm->buff,i,16,provid ); i+=16; /* provider id */
    setbitu(rtcm->buff,i, 4,solid  ); i+= 4; /* solution id */
    setbitu(rtcm->buff,i,ns,nsat   ); i+=ns; /* no of satellites */
    return i;
}
/* ssr signal and tracking mode ids ------------------------------------------*/
static  const int codes_gps[]={
    CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1Y,CODE_L1M,CODE_L2C,CODE_L2D,CODE_L2S,
    CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,CODE_L2Y,CODE_L2M,CODE_L5I,CODE_L5Q,
    CODE_L5X
};
static const int codes_glo[]={
    CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P
};
static const int codes_gal[]={
    CODE_L1A,CODE_L1B,CODE_L1C,CODE_L1X,CODE_L1Z,CODE_L5I,CODE_L5Q,CODE_L5X,
    CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L8I,CODE_L8Q,CODE_L8X,CODE_L6A,CODE_L6B,
    CODE_L6C,CODE_L6X,CODE_L6Z
};
static const int codes_qzs[]={
    CODE_L1C,CODE_L1S,CODE_L1L,CODE_L2S,CODE_L2L,CODE_L2X,CODE_L5I,CODE_L5Q,
    CODE_L5X,CODE_L6S,CODE_L6L,CODE_L6X,CODE_L1X
};
static const int codes_bds[]={
    CODE_L1I,CODE_L1Q,CODE_L1X,CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L6I,CODE_L6Q,
    CODE_L6X
};
static const int codes_sbs[]={
    CODE_L1C,CODE_L5I,CODE_L5Q,CODE_L5X
};
/* encode ssr 1: orbit corrections -------------------------------------------*/
static int encode_ssr1(rtcm_t *rtcm, int sys, int sync)
{
    double udint=0.0;
    int i,j,iod=0,nsat,prn,iode,iodcrc,refd=0,np,ni,nj,offp,deph[3],ddeph[3];
    
    trace(3,"encode_ssr1: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; ni= 8; nj= 0; offp=  0; break;
        case SYS_GLO: np=5; ni= 8; nj= 0; offp=  0; break;
        case SYS_GAL: np=6; ni=10; nj= 0; offp=  0; break;
        case SYS_QZS: np=4; ni= 8; nj= 0; offp=192; break;
        case SYS_CMP: np=6; ni=10; nj=24; offp=  1; break;
        case SYS_SBS: np=6; ni= 9; nj=24; offp=120; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[0];
        iod  =rtcm->ssr[j].iod[0];
        refd =rtcm->ssr[j].refd;
    }
    /* encode ssr header */
    i=encode_ssr_head(1,rtcm,sys,nsat,sync,iod,udint,refd,0,0);
    
    for (j=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        iode=rtcm->ssr[j].iode;      /* sbas/bds: toe/t0 modulo */
        iodcrc=rtcm->ssr[j].iodcrc;  /* sbas/bds: iod crc */
        
        deph [0]=ROUND(rtcm->ssr[j].deph [0]/1E-4);
        deph [1]=ROUND(rtcm->ssr[j].deph [1]/4E-4);
        deph [2]=ROUND(rtcm->ssr[j].deph [2]/4E-4);
        ddeph[0]=ROUND(rtcm->ssr[j].ddeph[0]/1E-6);
        ddeph[1]=ROUND(rtcm->ssr[j].ddeph[1]/4E-6);
        ddeph[2]=ROUND(rtcm->ssr[j].ddeph[2]/4E-6);
        
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbitu(rtcm->buff,i,ni,iode    ); i+=ni; /* iode */
        setbitu(rtcm->buff,i,nj,iodcrc  ); i+=nj; /* iodcrc */
        setbits(rtcm->buff,i,22,deph [0]); i+=22; /* delta radial */
        setbits(rtcm->buff,i,20,deph [1]); i+=20; /* delta along-track */
        setbits(rtcm->buff,i,20,deph [2]); i+=20; /* delta cross-track */
        setbits(rtcm->buff,i,21,ddeph[0]); i+=21; /* dot delta radial */
        setbits(rtcm->buff,i,19,ddeph[1]); i+=19; /* dot delta along-track */
        setbits(rtcm->buff,i,19,ddeph[2]); i+=19; /* dot delta cross-track */
    }
    rtcm->nbit=i;
    return 1;
}
/* encode ssr 2: clock corrections -------------------------------------------*/
static int encode_ssr2(rtcm_t *rtcm, int sys, int sync)
{
    double udint=0.0;
    int i,j,iod=0,nsat,prn,np,offp,iode,dclk[3];
    
    trace(3,"encode_ssr2: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; offp=  0; break;
        case SYS_GLO: np=5; offp=  0; break;
        case SYS_GAL: np=6; offp=  0; break;
        case SYS_QZS: np=4; offp=192; break;
        case SYS_CMP: np=6; offp=  1; break;
        case SYS_SBS: np=6; offp=120; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[1];
        iod  =rtcm->ssr[j].iod[1];
    }
    /* encode ssr header */
    i=encode_ssr_head(2,rtcm,sys,nsat,sync,iod,udint,0,0,0);
    
    for (j=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        iode=rtcm->ssr[j].iode;
        
        dclk[0]=ROUND(rtcm->ssr[j].dclk[0]/1E-4);
        dclk[1]=ROUND(rtcm->ssr[j].dclk[1]/1E-6);
        dclk[2]=ROUND(rtcm->ssr[j].dclk[2]/1E-8);
        
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbits(rtcm->buff,i,22,dclk[0] ); i+=22; /* delta clock c0 */
        setbits(rtcm->buff,i,21,dclk[1] ); i+=21; /* delta clock c1 */
        setbits(rtcm->buff,i,27,dclk[2] ); i+=27; /* delta clock c2 */
    }
    rtcm->nbit=i;
    return 1;
}
/* encode ssr 3: satellite code biases ---------------------------------------*/
static int encode_ssr3(rtcm_t *rtcm, int sys, int sync)
{
    const int *codes;
    double udint=0.0;
    int i,j,k,iod=0,nsat,prn,nbias,np,offp,ncode;
    int code[MAXCODE],bias[MAXCODE];
    
    trace(3,"encode_ssr3: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; offp=  0; codes=codes_gps; ncode=17; break;
        case SYS_GLO: np=5; offp=  0; codes=codes_glo; ncode= 4; break;
        case SYS_GAL: np=6; offp=  0; codes=codes_gal; ncode=19; break;
        case SYS_QZS: np=4; offp=192; codes=codes_qzs; ncode=13; break;
        case SYS_CMP: np=6; offp=  1; codes=codes_bds; ncode= 9; break;
        case SYS_SBS: np=6; offp=120; codes=codes_sbs; ncode= 4; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[4];
        iod  =rtcm->ssr[j].iod[4];
    }
    /* encode ssr header */
    i=encode_ssr_head(3,rtcm,sys,nsat,sync,iod,udint,0,0,0);
    
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        for (k=nbias=0;k<ncode;k++) {
            if (rtcm->ssr[j].cbias[codes[k]-1]==0.0) continue;
            code[nbias]=k;
            bias[nbias++]=ROUND(rtcm->ssr[j].cbias[codes[k]-1]/0.01);
        }
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbitu(rtcm->buff,i, 5,nbias);    i+= 5; /* number of code biases */
        
        for (k=0;k<nbias;k++) {
            setbitu(rtcm->buff,i, 5,code[k]); i+= 5; /* signal indicator */
            setbits(rtcm->buff,i,14,bias[k]); i+=14; /* code bias */
        }
    }
    rtcm->nbit=i;
    return 1;
}
/* encode ssr 4: combined orbit and clock corrections ------------------------*/
static int encode_ssr4(rtcm_t *rtcm, int sys, int sync)
{
    double udint=0.0;
    int i,j,iod=0,nsat,prn,iode,iodcrc,refd=0,np,ni,nj,offp;
    int deph[3],ddeph[3],dclk[3];
    
    trace(3,"encode_ssr4: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; ni= 8; nj= 0; offp=  0; break;
        case SYS_GLO: np=5; ni= 8; nj= 0; offp=  0; break;
        case SYS_GAL: np=6; ni=10; nj= 0; offp=  0; break;
        case SYS_QZS: np=4; ni= 8; nj= 0; offp=192; break;
        case SYS_CMP: np=6; ni=10; nj=24; offp=  1; break;
        case SYS_SBS: np=6; ni= 9; nj=24; offp=120; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[0];
        iod  =rtcm->ssr[j].iod[0];
        refd =rtcm->ssr[j].refd;
    }
    /* encode ssr header */
    i=encode_ssr_head(4,rtcm,sys,nsat,sync,iod,udint,refd,0,0);
    
    for (j=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        iode=rtcm->ssr[j].iode;
        iodcrc=rtcm->ssr[j].iodcrc;
        
        deph [0]=ROUND(rtcm->ssr[j].deph [0]/1E-4);
        deph [1]=ROUND(rtcm->ssr[j].deph [1]/4E-4);
        deph [2]=ROUND(rtcm->ssr[j].deph [2]/4E-4);
        ddeph[0]=ROUND(rtcm->ssr[j].ddeph[0]/1E-6);
        ddeph[1]=ROUND(rtcm->ssr[j].ddeph[1]/4E-6);
        ddeph[2]=ROUND(rtcm->ssr[j].ddeph[2]/4E-6);
        dclk [0]=ROUND(rtcm->ssr[j].dclk [0]/1E-4);
        dclk [1]=ROUND(rtcm->ssr[j].dclk [1]/1E-6);
        dclk [2]=ROUND(rtcm->ssr[j].dclk [2]/1E-8);
        
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbitu(rtcm->buff,i,ni,iode    ); i+=ni; /* iode */
        setbitu(rtcm->buff,i,nj,iodcrc  ); i+=nj; /* iodcrc */
        setbits(rtcm->buff,i,22,deph [0]); i+=22; /* delta raidal */
        setbits(rtcm->buff,i,20,deph [1]); i+=20; /* delta along-track */
        setbits(rtcm->buff,i,20,deph [2]); i+=20; /* delta cross-track */
        setbits(rtcm->buff,i,21,ddeph[0]); i+=21; /* dot delta radial */
        setbits(rtcm->buff,i,19,ddeph[1]); i+=19; /* dot delta along-track */
        setbits(rtcm->buff,i,19,ddeph[2]); i+=19; /* dot delta cross-track */
        setbits(rtcm->buff,i,22,dclk [0]); i+=22; /* delta clock c0 */
        setbits(rtcm->buff,i,21,dclk [1]); i+=21; /* delta clock c1 */
        setbits(rtcm->buff,i,27,dclk [2]); i+=27; /* delta clock c2 */
    }
    rtcm->nbit=i;
    return 1;
}
/* encode ssr 5: ura ---------------------------------------------------------*/
static int encode_ssr5(rtcm_t *rtcm, int sys, int sync)
{
    double udint=0.0;
    int i,j,nsat,iod=0,prn,ura,np,offp;
    
    trace(3,"encode_ssr5: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; offp=  0; break;
        case SYS_GLO: np=5; offp=  0; break;
        case SYS_GAL: np=6; offp=  0; break;
        case SYS_QZS: np=4; offp=192; break;
        case SYS_CMP: np=6; offp=  1; break;
        case SYS_SBS: np=6; offp=120; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[3];
        iod  =rtcm->ssr[j].iod[3];
    }
    /* encode ssr header */
    i=encode_ssr_head(5,rtcm,sys,nsat,sync,iod,udint,0,0,0);
    
    for (j=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        ura=rtcm->ssr[j].ura;
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbitu(rtcm->buff,i, 6,ura     ); i+= 6; /* ssr ura */
    }
    rtcm->nbit=i;
    return 1;
}
/* encode ssr 6: high rate clock correction ----------------------------------*/
static int encode_ssr6(rtcm_t *rtcm, int sys, int sync)
{
    double udint=0.0;
    int i,j,nsat,iod=0,prn,hrclk,np,offp;
    
    trace(3,"encode_ssr6: sys=%d sync=%d\n",sys,sync);
    
    switch (sys) {
        case SYS_GPS: np=6; offp=  0; break;
        case SYS_GLO: np=5; offp=  0; break;
        case SYS_GAL: np=6; offp=  0; break;
        case SYS_QZS: np=4; offp=192; break;
        case SYS_CMP: np=6; offp=  1; break;
        case SYS_SBS: np=6; offp=120; break;
        default: return 0;
    }
    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        nsat++;
        udint=rtcm->ssr[j].udi[2];
        iod  =rtcm->ssr[j].iod[2];
    }
    /* encode ssr header */
    i=encode_ssr_head(6,rtcm,sys,nsat,sync,iod,udint,0,0,0);
    
    for (j=0;j<MAXSAT;j++) {
        if (satsys(j+1,&prn)!=sys||!rtcm->ssr[j].update) continue;
        
        hrclk=ROUND(rtcm->ssr[j].hrclk/1E-4);
        
        setbitu(rtcm->buff,i,np,prn-offp); i+=np; /* satellite id */
        setbits(rtcm->buff,i,22,hrclk   ); i+=22; /* high rate clock corr */
    }
    rtcm->nbit=i;
    return 1;
}
/* satellite no to msm satellite id ------------------------------------------*/
static int to_satid(int sys, int sat)
{
    int prn;
    
    if (satsys(sat,&prn)!=sys) return 0;
    
    if      (sys==SYS_QZS) prn-=MINPRNQZS-1;
    else if (sys==SYS_SBS) prn-=MINPRNSBS-1;
    
    return prn;
}
/* observation code to msm signal id -----------------------------------------*/
static int to_sigid(int sys, unsigned char code, int *freq)
{
    const char **msm_sig;
    char *sig;
    int i;
    
    /* signal conversion for undefined signal by rtcm */
    if (sys==SYS_GPS) {
        if      (code==CODE_L1Y) code=CODE_L1P;
        else if (code==CODE_L1M) code=CODE_L1P;
        else if (code==CODE_L1N) code=CODE_L1P;
        else if (code==CODE_L2D) code=CODE_L2P;
        else if (code==CODE_L2Y) code=CODE_L2P;
        else if (code==CODE_L2M) code=CODE_L2P;
        else if (code==CODE_L2N) code=CODE_L2P;
    }
    if (!*(sig=code2obs(code,freq))) return 0;
    
    switch (sys) {
        case SYS_GPS: msm_sig=msm_sig_gps; break;
        case SYS_GLO: msm_sig=msm_sig_glo; break;
        case SYS_GAL: msm_sig=msm_sig_gal; break;
        case SYS_QZS: msm_sig=msm_sig_qzs; break;
        case SYS_SBS: msm_sig=msm_sig_sbs; break;
        case SYS_CMP: msm_sig=msm_sig_cmp; break;
        default: return 0;
    }
    /* freqency index for beidou */
    if (sys==SYS_CMP) {
        if      (*freq==5) *freq=2; /* B2 */
        else if (*freq==4) *freq=3; /* B3 */
    }
    for (i=0;i<32;i++) {
        if (!strcmp(sig,msm_sig[i])) return i+1;
    }
    return 0;
}
/* generate msm satellite, signal and cell index -----------------------------*/
static void gen_msm_index(rtcm_t *rtcm, int sys, int *nsat, int *nsig,
                          int *ncell, unsigned char *sat_ind,
                          unsigned char *sig_ind, unsigned char *cell_ind)
{
    int i,j,sat,sig,cell,f;
    
    *nsat=*nsig=*ncell=0;
    
    /* generate satellite and signal index */
    for (i=0;i<rtcm->obs.n;i++) {
        if (!(sat=to_satid(sys,rtcm->obs.data[i].sat))) continue;
        
        for (j=0;j<NFREQ+NEXOBS;j++) {
            if (!(sig=to_sigid(sys,rtcm->obs.data[i].code[j],&f))) continue;
            
            sat_ind[sat-1]=sig_ind[sig-1]=1;
        }
    }
    for (i=0;i<64;i++) {
        if (sat_ind[i]) sat_ind[i]=++(*nsat);
    }
    for (i=0;i<32;i++) {
        if (sig_ind[i]) sig_ind[i]=++(*nsig);
    }
    /* generate cell index */
    for (i=0;i<rtcm->obs.n;i++) {
        if (!(sat=to_satid(sys,rtcm->obs.data[i].sat))) continue;
        
        for (j=0;j<NFREQ+NEXOBS;j++) {
            if (!(sig=to_sigid(sys,rtcm->obs.data[i].code[j],&f))) continue;
            
            cell=sig_ind[sig-1]-1+(sat_ind[sat-1]-1)*(*nsig);
            cell_ind[cell]=1;
        }
    }
    for (i=0;i<*nsat*(*nsig);i++) {
        if (cell_ind[i]&&*ncell<64) cell_ind[i]=++(*ncell);
    }
}
/* generate msm satellite data fields ----------------------------------------*/
static void gen_msm_sat(rtcm_t *rtcm, int sys, int nsat,
                        const unsigned char *sat_ind, double *rrng,
                        double *rrate, unsigned char *info)
{
    obsd_t *data;
    double lambda,rrng_s,rrate_s;
    int i,j,k,sat,sig,f,fcn;
    
    for (i=0;i<64;i++) rrng[i]=rrate[i]=0.0;
    
    for (i=0;i<rtcm->obs.n;i++) {
        data=rtcm->obs.data+i;
        if (!(sat=to_satid(sys,data->sat))) continue;
        fcn=fcn_glo(data->sat,rtcm);
        
        for (j=0;j<NFREQ+NEXOBS;j++) {
            if (!(sig=to_sigid(sys,data->code[j],&f))) continue;
            k=sat_ind[sat-1]-1;
            lambda=satwavelen(data->sat,f-1,&rtcm->nav);
            
            /* rough range (ms) and rough phase-range-rate (m/s) */
            rrng_s =ROUND( data->P[j]/RANGE_MS/P2_10)*RANGE_MS*P2_10;
            rrate_s=ROUND(-data->D[j]*lambda)*1.0;
            if (rrng [k]==0.0&&data->P[j]!=0.0) rrng [k]=rrng_s;
            if (rrate[k]==0.0&&data->D[j]!=0.0) rrate[k]=rrate_s;
            
            /* extended satellite info */
            if (info) info[k]=sys!=SYS_GLO?0:(fcn<0?15:fcn);
        }
    }
}
/* generate msm signal data fields -------------------------------------------*/
static void gen_msm_sig(rtcm_t *rtcm, int sys, int nsat, int nsig, int ncell,
                        const unsigned char *sat_ind,
                        const unsigned char *sig_ind,
                        const unsigned char *cell_ind, const double *rrng,
                        const double *rrate, double *psrng, double *phrng,
                        double *rate, double *lock, unsigned char *half,
                        float *cnr)
{
    obsd_t *data;
    double lambda,psrng_s,phrng_s,rate_s,lt;
    int i,j,k,sat,sig,cell,f,LLI;
    
    for (i=0;i<ncell;i++) {
        if (psrng) psrng[i]=0.0;
        if (phrng) phrng[i]=0.0;
        if (rate ) rate [i]=0.0;
    }
    for (i=0;i<rtcm->obs.n;i++) {
        data=rtcm->obs.data+i;
        
        if (!(sat=to_satid(sys,data->sat))) continue;
        
        for (j=0;j<NFREQ+NEXOBS;j++) {
            if (!(sig=to_sigid(sys,data->code[j],&f))) continue;
            k=sat_ind[sat-1]-1;
            if ((cell=cell_ind[sig_ind[sig-1]-1+k*nsig])>=64) continue;
            
            lambda=satwavelen(data->sat,f-1,&rtcm->nav);
            psrng_s=data->P[j]==0.0?0.0:data->P[j]-rrng[k];
            phrng_s=data->L[j]==0.0||lambda<=0.0?0.0: data->L[j]*lambda-rrng [k];
            rate_s =data->D[j]==0.0||lambda<=0.0?0.0:-data->D[j]*lambda-rrate[k];
            
            /* subtract phase - psudorange integer cycle offset */
            LLI=data->LLI[j];
            if ((LLI&1)||fabs(phrng_s-rtcm->cp[data->sat-1][j])>1171.0) {
                rtcm->cp[data->sat-1][j]=ROUND(phrng_s/lambda)*lambda;
                LLI|=1;
            }
            phrng_s-=rtcm->cp[data->sat-1][j];
            
            lt=locktime_d(data->time,rtcm->lltime[data->sat-1]+j,LLI);
            
            if (psrng&&psrng_s!=0.0) psrng[cell-1]=psrng_s;
            if (phrng&&phrng_s!=0.0) phrng[cell-1]=phrng_s;
            if (rate &&rate_s !=0.0) rate [cell-1]=rate_s;
            if (lock) lock[cell-1]=lt;
            if (half) half[cell-1]=(data->LLI[j]&2)?1:0;
            if (cnr ) cnr [cell-1]=(float)(data->SNR[j]*0.25);
        }
    }
}
/* encode msm header ---------------------------------------------------------*/
static int encode_msm_head(int type, rtcm_t *rtcm, int sys, int sync, int *nsat,
                           int *ncell, double *rrng, double *rrate,
                           unsigned char *info, double *psrng, double *phrng,
                           double *rate, double *lock, unsigned char *half,
                           float *cnr)
{
    double tow;
    unsigned char sat_ind[64]={0},sig_ind[32]={0},cell_ind[32*64]={0};
    unsigned int dow,epoch;
    int i=24,j,nsig=0;
    
    switch (sys) {
        case SYS_GPS: type+=1070; break;
        case SYS_GLO: type+=1080; break;
        case SYS_GAL: type+=1090; break;
        case SYS_QZS: type+=1110; break;
        case SYS_SBS: type+=1100; break;
        case SYS_CMP: type+=1120; break;
        default: return 0;
    }
    /* generate msm satellite, signal and cell index */
    gen_msm_index(rtcm,sys,nsat,&nsig,ncell,sat_ind,sig_ind,cell_ind);
    
    if (sys==SYS_GLO) {
        /* glonass time (dow + tod-ms) */
        tow=time2gpst(timeadd(gpst2utc(rtcm->time),10800.0),NULL);
        dow=(unsigned int)(tow/86400.0);
        epoch=(dow<<27)+ROUND_U(fmod(tow,86400.0)*1E3);
    }
    else if (sys==SYS_CMP) {
        /* beidou time (tow-ms) */
        epoch=ROUND_U(time2gpst(gpst2bdt(rtcm->time),NULL)*1E3);
    }
    else {
        /* gps, qzs and galileo time (tow-ms) */
        epoch=ROUND_U(time2gpst(rtcm->time,NULL)*1E3);
    }
    /* encode msm header (ref [15] table 3.5-78) */
    setbitu(rtcm->buff,i,12,type       ); i+=12; /* message number */
    setbitu(rtcm->buff,i,12,rtcm->staid); i+=12; /* reference station id */
    setbitu(rtcm->buff,i,30,epoch      ); i+=30; /* epoch time */
    setbitu(rtcm->buff,i, 1,sync       ); i+= 1; /* multiple message bit */
    setbitu(rtcm->buff,i, 3,rtcm->seqno); i+= 3; /* issue of data station */
    setbitu(rtcm->buff,i, 7,0          ); i+= 7; /* reserved */
    setbitu(rtcm->buff,i, 2,0          ); i+= 2; /* clock streering indicator */
    setbitu(rtcm->buff,i, 2,0          ); i+= 2; /* external clock indicator */
    setbitu(rtcm->buff,i, 1,0          ); i+= 1; /* smoothing indicator */
    setbitu(rtcm->buff,i, 3,0          ); i+= 3; /* smoothing interval */
    
    /* satellite mask */
    for (j=0;j<64;j++) {
        setbitu(rtcm->buff,i,1,sat_ind[j]?1:0); i+=1;
    }
    /* signal mask */
    for (j=0;j<32;j++) {
        setbitu(rtcm->buff,i,1,sig_ind[j]?1:0); i+=1;
    }
    /* cell mask */
    for (j=0;j<*nsat*nsig&&j<64;j++) {
        setbitu(rtcm->buff,i,1,cell_ind[j]?1:0); i+=1;
    }
    /* generate msm satellite data fields */
    gen_msm_sat(rtcm,sys,*nsat,sat_ind,rrng,rrate,info);
    
    /* generate msm signal data fields */
    gen_msm_sig(rtcm,sys,*nsat,nsig,*ncell,sat_ind,sig_ind,cell_ind,rrng,rrate,
                psrng,phrng,rate,lock,half,cnr);
    
    return i;
}
/* encode rough range integer ms ---------------------------------------------*/
static int encode_msm_int_rrng(rtcm_t *rtcm, int i, const double *rrng,
                               int nsat)
{
    unsigned int int_ms;
    int j;
    
    for (j=0;j<nsat;j++) {
        if (rrng[j]==0.0) {
            int_ms=255;
        }
        else if (rrng[j]<0.0||rrng[j]>RANGE_MS*255.0) {
            trace(2,"msm rough range overflow %s rrng=%.3f\n",
                 time_str(rtcm->time,0),rrng[j]);
            int_ms=255;
        }
        else {
            int_ms=ROUND_U(rrng[j]/RANGE_MS/P2_10)>>10;
        }
        setbitu(rtcm->buff,i,8,int_ms); i+=8;
    }
    return i;
}
/* encode rough range modulo 1 ms --------------------------------------------*/
static int encode_msm_mod_rrng(rtcm_t *rtcm, int i, const double *rrng,
                               int nsat)
{
    unsigned int mod_ms;
    int j;
    
    for (j=0;j<nsat;j++) {
        if (rrng[j]<=0.0||rrng[j]>RANGE_MS*255.0) {
            mod_ms=0;
        }
        else {
            mod_ms=ROUND_U(rrng[j]/RANGE_MS/P2_10)&0x3FFu;
        }
        setbitu(rtcm->buff,i,10,mod_ms); i+=10;
    }
    return i;
}
/* encode extended satellite info --------------------------------------------*/
static int encode_msm_info(rtcm_t *rtcm, int i, const unsigned char *info,
                           int nsat)
{
    int j;
    
    for (j=0;j<nsat;j++) {
        setbitu(rtcm->buff,i,4,info[j]); i+=4;
    }
    return i;
}
/* encode rough phase-range-rate ---------------------------------------------*/
static int encode_msm_rrate(rtcm_t *rtcm, int i, const double *rrate, int nsat)
{
    int j,rrate_val;
    
    for (j=0;j<nsat;j++) {
        if (fabs(rrate[j])>8191.0) {
            trace(2,"msm rough phase-range-rate overflow %s rrate=%.4f\n",
                 time_str(rtcm->time,0),rrate[j]);
            rrate_val=-8192;
        }
        else {
            rrate_val=ROUND(rrate[j]/1.0);
        }
        setbits(rtcm->buff,i,14,rrate_val); i+=14;
    }
    return i;
}
/* encode fine pseudorange ---------------------------------------------------*/
static int encode_msm_psrng(rtcm_t *rtcm, int i, const double *psrng, int ncell)
{
    int j,psrng_val;
    
    for (j=0;j<ncell;j++) {
        if (psrng[j]==0.0) {
            psrng_val=-16384;
        }
        else if (fabs(psrng[j])>292.7) {
            trace(2,"msm fine pseudorange overflow %s psrng=%.3f\n",
                 time_str(rtcm->time,0),psrng[j]);
            psrng_val=-16384;
        }
        else {
            psrng_val=ROUND(psrng[j]/RANGE_MS/P2_24);
        }
        setbits(rtcm->buff,i,15,psrng_val); i+=15;
    }
    return i;
}
/* encode fine pseudorange with extended resolution --------------------------*/
static int encode_msm_psrng_ex(rtcm_t *rtcm, int i, const double *psrng,
                               int ncell)
{
    int j,psrng_val;
    
    for (j=0;j<ncell;j++) {
        if (psrng[j]==0.0) {
            psrng_val=-524288;
        }
        else if (fabs(psrng[j])>292.7) {
            trace(2,"msm fine pseudorange ext overflow %s psrng=%.3f\n",
                 time_str(rtcm->time,0),psrng[j]);
            psrng_val=-524288;
        }
        else {
            psrng_val=ROUND(psrng[j]/RANGE_MS/P2_29);
        }
        setbits(rtcm->buff,i,20,psrng_val); i+=20;
    }
    return i;
}
/* encode fine phase-range ---------------------------------------------------*/
static int encode_msm_phrng(rtcm_t *rtcm, int i, const double *phrng, int ncell)
{
    int j,phrng_val;
    
    for (j=0;j<ncell;j++) {
        if (phrng[j]==0.0) {
            phrng_val=-2097152;
        }
        else if (fabs(phrng[j])>1171.0) {
            trace(2,"msm fine phase-range overflow %s phrng=%.3f\n",
                 time_str(rtcm->time,0),phrng[j]);
            phrng_val=-2097152;
        }
        else {
            phrng_val=ROUND(phrng[j]/RANGE_MS/P2_29);
        }
        setbits(rtcm->buff,i,22,phrng_val); i+=22;
    }
    return i;
}
/* encode fine phase-range with extended resolution --------------------------*/
static int encode_msm_phrng_ex(rtcm_t *rtcm, int i, const double *phrng,
                               int ncell)
{
    int j,phrng_val;
    
    for (j=0;j<ncell;j++) {
        if (phrng[j]==0.0) {
            phrng_val=-8388608;
        }
        else if (fabs(phrng[j])>1171.0) {
            trace(2,"msm fine phase-range ext overflow %s phrng=%.3f\n",
                 time_str(rtcm->time,0),phrng[j]);
            phrng_val=-8388608;
        }
        else {
            phrng_val=ROUND(phrng[j]/RANGE_MS/P2_31);
        }
        setbits(rtcm->buff,i,24,phrng_val); i+=24;
    }
    return i;
}
/* encode lock-time indicator ------------------------------------------------*/
static int encode_msm_lock(rtcm_t *rtcm, int i, const double *lock, int ncell)
{
    int j,lock_val;
    
    for (j=0;j<ncell;j++) {
        lock_val=to_msm_lock(lock[j]);
        setbitu(rtcm->buff,i,4,lock_val); i+=4;
    }
    return i;
}
/* encode lock-time indicator with extended range and resolution -------------*/
static int encode_msm_lock_ex(rtcm_t *rtcm, int i, const double *lock,
                              int ncell)
{
    int j,lock_val;
    
    for (j=0;j<ncell;j++) {
        lock_val=to_msm_lock_ex(lock[j]);
        setbitu(rtcm->buff,i,10,lock_val); i+=10;
    }
    return i;
}
/* encode half-cycle-ambiguity indicator -------------------------------------*/
static int encode_msm_half_amb(rtcm_t *rtcm, int i, const unsigned char *half,
                               int ncell)
{
    int j;
    
    for (j=0;j<ncell;j++) {
        setbitu(rtcm->buff,i,1,half[j]); i+=1;
    }
    return i;
}
/* encode signal cnr ---------------------------------------------------------*/
static int encode_msm_cnr(rtcm_t *rtcm, int i, const float *cnr, int ncell)
{
    int j,cnr_val;
    
    for (j=0;j<ncell;j++) {
        cnr_val=ROUND(cnr[j]/1.0);
        setbitu(rtcm->buff,i,6,cnr_val); i+=6;
    }
    return i;
}
/* encode signal cnr with extended resolution --------------------------------*/
static int encode_msm_cnr_ex(rtcm_t *rtcm, int i, const float *cnr, int ncell)
{
    int j,cnr_val;
    
    for (j=0;j<ncell;j++) {
        cnr_val=ROUND(cnr[j]/0.0625);
        setbitu(rtcm->buff,i,10,cnr_val); i+=10;
    }
    return i;
}
/* encode fine phase-range-rate ----------------------------------------------*/
static int encode_msm_rate(rtcm_t *rtcm, int i, const double *rate, int ncell)
{
    int j,rate_val;
    
    for (j=0;j<ncell;j++) {
        if (rate[j]==0.0) {
            rate_val=-16384;
        }
        else if (fabs(rate[j])>1.6384) {
            trace(2,"msm fine phase-range-rate overflow %s rate=%.3f\n",
                 time_str(rtcm->time,0),rate[j]);
            rate_val=-16384;
        }
        else {
            rate_val=ROUND(rate[j]/0.0001);
        }
        setbitu(rtcm->buff,i,15,rate_val); i+=15;
    }
    return i;
}
/* encode msm 1: compact pseudorange -----------------------------------------*/
static int encode_msm1(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm1: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(1,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,NULL,psrng,
                            NULL,NULL,NULL,NULL,NULL))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    
    /* encode msm signal data */
    i=encode_msm_psrng   (rtcm,i,psrng,ncell); /* fine pseudorange */
    
    rtcm->nbit=i;
    return 1;
}
/* encode msm 2: compact phaserange ------------------------------------------*/
static int encode_msm2(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],phrng[64],lock[64];
    unsigned char half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm2: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(2,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,NULL,NULL,
                            phrng,NULL,lock,half,NULL))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    
    /* encode msm signal data */
    i=encode_msm_phrng   (rtcm,i,phrng,ncell); /* fine phase-range */
    i=encode_msm_lock    (rtcm,i,lock ,ncell); /* lock-time indicator */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    
    rtcm->nbit=i;
    return 1;
}
/* encode msm 3: compact pseudorange and phaserange --------------------------*/
static int encode_msm3(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64],phrng[64],lock[64];
    unsigned char half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm3: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(3,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,NULL,psrng,
                            phrng,NULL,lock,half,NULL))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    
    /* encode msm signal data */
    i=encode_msm_psrng   (rtcm,i,psrng,ncell); /* fine pseudorange */
    i=encode_msm_phrng   (rtcm,i,phrng,ncell); /* fine phase-range */
    i=encode_msm_lock    (rtcm,i,lock ,ncell); /* lock-time indicator */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    
    rtcm->nbit=i;
    return 1;
}
/* encode msm 4: full pseudorange and phaserange plus cnr --------------------*/
static int encode_msm4(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64],phrng[64],lock[64];
    float cnr[64];
    unsigned char half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm4: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(4,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,NULL,psrng,
                            phrng,NULL,lock,half,cnr))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_int_rrng(rtcm,i,rrng ,nsat ); /* rough range integer ms */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    
    /* encode msm signal data */
    i=encode_msm_psrng   (rtcm,i,psrng,ncell); /* fine pseudorange */
    i=encode_msm_phrng   (rtcm,i,phrng,ncell); /* fine phase-range */
    i=encode_msm_lock    (rtcm,i,lock ,ncell); /* lock-time indicator */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    i=encode_msm_cnr     (rtcm,i,cnr  ,ncell); /* signal cnr */
    rtcm->nbit=i;
    return 1;
}
/* encode msm 5: full pseudorange, phaserange, phaserangerate and cnr --------*/
static int encode_msm5(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64],phrng[64],rate[64],lock[64];
    float cnr[64];
    unsigned char info[64],half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm5: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(5,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,info,psrng,
                            phrng,rate,lock,half,cnr))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_int_rrng(rtcm,i,rrng ,nsat ); /* rough range integer ms */
    i=encode_msm_info    (rtcm,i,info ,nsat ); /* extended satellite info */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    i=encode_msm_rrate   (rtcm,i,rrate,nsat ); /* rough phase-range-rate */
    
    /* encode msm signal data */
    i=encode_msm_psrng   (rtcm,i,psrng,ncell); /* fine pseudorange */
    i=encode_msm_phrng   (rtcm,i,phrng,ncell); /* fine phase-range */
    i=encode_msm_lock    (rtcm,i,lock ,ncell); /* lock-time indicator */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    i=encode_msm_cnr     (rtcm,i,cnr  ,ncell); /* signal cnr */
    i=encode_msm_rate    (rtcm,i,rate ,ncell); /* fine phase-range-rate */
    rtcm->nbit=i;
    return 1;
}
/* encode msm 6: full pseudorange and phaserange plus cnr (high-res) ---------*/
static int encode_msm6(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64],phrng[64],lock[64];
    float cnr[64];
    unsigned char half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm6: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(6,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,NULL,psrng,
                            phrng,NULL,lock,half,cnr))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_int_rrng(rtcm,i,rrng ,nsat ); /* rough range integer ms */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    
    /* encode msm signal data */
    i=encode_msm_psrng_ex(rtcm,i,psrng,ncell); /* fine pseudorange ext */
    i=encode_msm_phrng_ex(rtcm,i,phrng,ncell); /* fine phase-range ext */
    i=encode_msm_lock_ex (rtcm,i,lock ,ncell); /* lock-time indicator ext */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    i=encode_msm_cnr_ex  (rtcm,i,cnr  ,ncell); /* signal cnr ext */
    rtcm->nbit=i;
    return 1;
}
/* encode msm 7: full pseudorange, phaserange, phaserangerate and cnr (h-res) */
static int encode_msm7(rtcm_t *rtcm, int sys, int sync)
{
    double rrng[64],rrate[64],psrng[64],phrng[64],rate[64],lock[64];
    float cnr[64];
    unsigned char info[64],half[64];
    int i,nsat,ncell;
    
    trace(3,"encode_msm7: sys=%d sync=%d\n",sys,sync);
    
    /* encode msm header */
    if (!(i=encode_msm_head(7,rtcm,sys,sync,&nsat,&ncell,rrng,rrate,info,psrng,
                            phrng,rate,lock,half,cnr))) {
        return 0;
    }
    /* encode msm satellite data */
    i=encode_msm_int_rrng(rtcm,i,rrng ,nsat ); /* rough range integer ms */
    i=encode_msm_info    (rtcm,i,info ,nsat ); /* extended satellite info */
    i=encode_msm_mod_rrng(rtcm,i,rrng ,nsat ); /* rough range modulo 1 ms */
    i=encode_msm_rrate   (rtcm,i,rrate,nsat ); /* rough phase-range-rate */
    
    /* encode msm signal data */
    i=encode_msm_psrng_ex(rtcm,i,psrng,ncell); /* fine pseudorange ext */
    i=encode_msm_phrng_ex(rtcm,i,phrng,ncell); /* fine phase-range ext */
    i=encode_msm_lock_ex (rtcm,i,lock ,ncell); /* lock-time indicator ext */
    i=encode_msm_half_amb(rtcm,i,half ,ncell); /* half-cycle-amb indicator */
    i=encode_msm_cnr_ex  (rtcm,i,cnr  ,ncell); /* signal cnr ext */
    i=encode_msm_rate    (rtcm,i,rate ,ncell); /* fine phase-range-rate */
    rtcm->nbit=i;
    return 1;
}
/* encode rtcm ver.3 message -------------------------------------------------*/
extern int encode_rtcm3(rtcm_t *rtcm, int type, int sync)
{
    int ret=0;
    
    trace(3,"encode_rtcm3: type=%d sync=%d\n",type,sync);
    
    switch (type) {
        case 1001: ret=encode_type1001(rtcm,sync);     break;
        case 1002: ret=encode_type1002(rtcm,sync);     break;
        case 1003: ret=encode_type1003(rtcm,sync);     break;
        case 1004: ret=encode_type1004(rtcm,sync);     break;
        case 1005: ret=encode_type1005(rtcm,sync);     break;
        case 1006: ret=encode_type1006(rtcm,sync);     break;
        case 1007: ret=encode_type1007(rtcm,sync);     break;
        case 1008: ret=encode_type1008(rtcm,sync);     break;
        case 1009: ret=encode_type1009(rtcm,sync);     break;
        case 1010: ret=encode_type1010(rtcm,sync);     break;
        case 1011: ret=encode_type1011(rtcm,sync);     break;
        case 1012: ret=encode_type1012(rtcm,sync);     break;
        case 1019: ret=encode_type1019(rtcm,sync);     break;
        case 1020: ret=encode_type1020(rtcm,sync);     break;
        case 1033: ret=encode_type1033(rtcm,sync);     break;
        case 1042: ret=encode_type1042(rtcm,sync);     break;
        case 1044: ret=encode_type1044(rtcm,sync);     break;
        case 1045: ret=encode_type1045(rtcm,sync);     break;
        case 1046: ret=encode_type1046(rtcm,sync);     break;
        case   63: ret=encode_type63  (rtcm,sync);     break; /* draft */
        case 1057: ret=encode_ssr1(rtcm,SYS_GPS,sync); break;
        case 1058: ret=encode_ssr2(rtcm,SYS_GPS,sync); break;
        case 1059: ret=encode_ssr3(rtcm,SYS_GPS,sync); break;
        case 1060: ret=encode_ssr4(rtcm,SYS_GPS,sync); break;
        case 1061: ret=encode_ssr5(rtcm,SYS_GPS,sync); break;
        case 1062: ret=encode_ssr6(rtcm,SYS_GPS,sync); break;
        case 1063: ret=encode_ssr1(rtcm,SYS_GLO,sync); break;
        case 1064: ret=encode_ssr2(rtcm,SYS_GLO,sync); break;
        case 1065: ret=encode_ssr3(rtcm,SYS_GLO,sync); break;
        case 1066: ret=encode_ssr4(rtcm,SYS_GLO,sync); break;
        case 1067: ret=encode_ssr5(rtcm,SYS_GLO,sync); break;
        case 1068: ret=encode_ssr6(rtcm,SYS_GLO,sync); break;
        case 1071: ret=encode_msm1(rtcm,SYS_GPS,sync); break;
        case 1072: ret=encode_msm2(rtcm,SYS_GPS,sync); break;
        case 1073: ret=encode_msm3(rtcm,SYS_GPS,sync); break;
        case 1074: ret=encode_msm4(rtcm,SYS_GPS,sync); break;
        case 1075: ret=encode_msm5(rtcm,SYS_GPS,sync); break;
        case 1076: ret=encode_msm6(rtcm,SYS_GPS,sync); break;
        case 1077: ret=encode_msm7(rtcm,SYS_GPS,sync); break;
        case 1081: ret=encode_msm1(rtcm,SYS_GLO,sync); break;
        case 1082: ret=encode_msm2(rtcm,SYS_GLO,sync); break;
        case 1083: ret=encode_msm3(rtcm,SYS_GLO,sync); break;
        case 1084: ret=encode_msm4(rtcm,SYS_GLO,sync); break;
        case 1085: ret=encode_msm5(rtcm,SYS_GLO,sync); break;
        case 1086: ret=encode_msm6(rtcm,SYS_GLO,sync); break;
        case 1087: ret=encode_msm7(rtcm,SYS_GLO,sync); break;
        case 1091: ret=encode_msm1(rtcm,SYS_GAL,sync); break;
        case 1092: ret=encode_msm2(rtcm,SYS_GAL,sync); break;
        case 1093: ret=encode_msm3(rtcm,SYS_GAL,sync); break;
        case 1094: ret=encode_msm4(rtcm,SYS_GAL,sync); break;
        case 1095: ret=encode_msm5(rtcm,SYS_GAL,sync); break;
        case 1096: ret=encode_msm6(rtcm,SYS_GAL,sync); break;
        case 1097: ret=encode_msm7(rtcm,SYS_GAL,sync); break;
        case 1101: ret=encode_msm1(rtcm,SYS_SBS,sync); break;
        case 1102: ret=encode_msm2(rtcm,SYS_SBS,sync); break;
        case 1103: ret=encode_msm3(rtcm,SYS_SBS,sync); break;
        case 1104: ret=encode_msm4(rtcm,SYS_SBS,sync); break;
        case 1105: ret=encode_msm5(rtcm,SYS_SBS,sync); break;
        case 1106: ret=encode_msm6(rtcm,SYS_SBS,sync); break;
        case 1107: ret=encode_msm7(rtcm,SYS_SBS,sync); break;
        case 1111: ret=encode_msm1(rtcm,SYS_QZS,sync); break;
        case 1112: ret=encode_msm2(rtcm,SYS_QZS,sync); break;
        case 1113: ret=encode_msm3(rtcm,SYS_QZS,sync); break;
        case 1114: ret=encode_msm4(rtcm,SYS_QZS,sync); break;
        case 1115: ret=encode_msm5(rtcm,SYS_QZS,sync); break;
        case 1116: ret=encode_msm6(rtcm,SYS_QZS,sync); break;
        case 1117: ret=encode_msm7(rtcm,SYS_QZS,sync); break;
        case 1121: ret=encode_msm1(rtcm,SYS_CMP,sync); break;
        case 1122: ret=encode_msm2(rtcm,SYS_CMP,sync); break;
        case 1123: ret=encode_msm3(rtcm,SYS_CMP,sync); break;
        case 1124: ret=encode_msm4(rtcm,SYS_CMP,sync); break;
        case 1125: ret=encode_msm5(rtcm,SYS_CMP,sync); break;
        case 1126: ret=encode_msm6(rtcm,SYS_CMP,sync); break;
        case 1127: ret=encode_msm7(rtcm,SYS_CMP,sync); break;
        case 1240: ret=encode_ssr1(rtcm,SYS_GAL,sync); break;
        case 1241: ret=encode_ssr2(rtcm,SYS_GAL,sync); break;
        case 1242: ret=encode_ssr3(rtcm,SYS_GAL,sync); break;
        case 1243: ret=encode_ssr4(rtcm,SYS_GAL,sync); break;
        case 1244: ret=encode_ssr5(rtcm,SYS_GAL,sync); break;
        case 1245: ret=encode_ssr6(rtcm,SYS_GAL,sync); break;
        case 1246: ret=encode_ssr1(rtcm,SYS_QZS,sync); break;
        case 1247: ret=encode_ssr2(rtcm,SYS_QZS,sync); break;
        case 1248: ret=encode_ssr3(rtcm,SYS_QZS,sync); break;
        case 1249: ret=encode_ssr4(rtcm,SYS_QZS,sync); break;
        case 1250: ret=encode_ssr5(rtcm,SYS_QZS,sync); break;
        case 1251: ret=encode_ssr6(rtcm,SYS_QZS,sync); break;
        case 1252: ret=encode_ssr1(rtcm,SYS_SBS,sync); break;
        case 1253: ret=encode_ssr2(rtcm,SYS_SBS,sync); break;
        case 1254: ret=encode_ssr3(rtcm,SYS_SBS,sync); break;
        case 1255: ret=encode_ssr4(rtcm,SYS_SBS,sync); break;
        case 1256: ret=encode_ssr5(rtcm,SYS_SBS,sync); break;
        case 1257: ret=encode_ssr6(rtcm,SYS_SBS,sync); break;
        case 1258: ret=encode_ssr1(rtcm,SYS_CMP,sync); break;
        case 1259: ret=encode_ssr2(rtcm,SYS_CMP,sync); break;
        case 1260: ret=encode_ssr3(rtcm,SYS_CMP,sync); break;
        case 1261: ret=encode_ssr4(rtcm,SYS_CMP,sync); break;
        case 1262: ret=encode_ssr5(rtcm,SYS_CMP,sync); break;
        case 1263: ret=encode_ssr6(rtcm,SYS_CMP,sync); break;
    }
    if (ret>0) {
        type-=1000;
        if (1<=type&&type<=299) rtcm->nmsg3[type]++; else rtcm->nmsg3[0]++;
    }
    return ret;
}
