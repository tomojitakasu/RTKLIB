/*------------------------------------------------------------------------------
* rtcm2.c : rtcm ver.2 message functions
*
*          Copyright (C) 2009-2014 by T.TAKASU, All rights reserved.
*
* references :
*     see rtcm.c
*
* version : $Revision:$ $Date:$
* history : 2011/11/28 1.0  separated from rtcm.c
*           2014/10/21 1.1  fix problem on week rollover in rtcm 2 type 14
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* adjust hourly rollover of rtcm 2 time -------------------------------------*/
static void adjhour(rtcm_t *rtcm, double zcnt)
{
    double tow,hour,sec;
    int week;
    
    /* if no time, get cpu time */
    if (rtcm->time.time==0) rtcm->time=utc2gpst(timeget());
    tow=time2gpst(rtcm->time,&week);
    hour=floor(tow/3600.0);
    sec=tow-hour*3600.0;
    if      (zcnt<sec-1800.0) zcnt+=3600.0;
    else if (zcnt>sec+1800.0) zcnt-=3600.0;
    rtcm->time=gpst2time(week,hour*3600+zcnt);
}
/* get observation data index ------------------------------------------------*/
static int obsindex(obs_t *obs, gtime_t time, int sat)
{
    int i,j;
    
    for (i=0;i<obs->n;i++) {
        if (obs->data[i].sat==sat) return i; /* field already exists */
    }
    if (i>=MAXOBS) return -1; /* overflow */
    
    /* add new field */
    obs->data[i].time=time;
    obs->data[i].sat=sat;
    for (j=0;j<NFREQ;j++) {
        obs->data[i].L[j]=obs->data[i].P[j]=0.0;
        obs->data[i].D[j]=0.0;
        obs->data[i].SNR[j]=obs->data[i].LLI[j]=obs->data[i].code[j]=0;
    }
    obs->n++;
    return i;
}
/* decode type 1/9: differential gps correction/partial correction set -------*/
static int decode_type1(rtcm_t *rtcm)
{
    int i=48,fact,udre,prn,sat,iod;
    double prc,rrc;
    
    trace(4,"decode_type1: len=%d\n",rtcm->len);
    
    while (i+40<=rtcm->len*8) {
        fact=getbitu(rtcm->buff,i, 1); i+= 1;
        udre=getbitu(rtcm->buff,i, 2); i+= 2;
        prn =getbitu(rtcm->buff,i, 5); i+= 5;
        prc =getbits(rtcm->buff,i,16); i+=16;
        rrc =getbits(rtcm->buff,i, 8); i+= 8;
        iod =getbits(rtcm->buff,i, 8); i+= 8;
        if (prn==0) prn=32;
        if (prc==0x80000000||rrc==0xFFFF8000) {
            trace(2,"rtcm2 1 prc/rrc indicates satellite problem: prn=%d\n",prn);
            continue;
        }
        if (rtcm->dgps) {
            sat=satno(SYS_GPS,prn);
            rtcm->dgps[sat-1].t0=rtcm->time;
            rtcm->dgps[sat-1].prc=prc*(fact?0.32:0.02);
            rtcm->dgps[sat-1].rrc=rrc*(fact?0.032:0.002);
            rtcm->dgps[sat-1].iod=iod;
            rtcm->dgps[sat-1].udre=udre;
        }
    }
    return 7;
}
/* decode type 3: reference station parameter --------------------------------*/
static int decode_type3(rtcm_t *rtcm)
{
    int i=48;
    
    trace(4,"decode_type3: len=%d\n",rtcm->len);
    
    if (i+96<=rtcm->len*8) {
        rtcm->sta.pos[0]=getbits(rtcm->buff,i,32)*0.01; i+=32;
        rtcm->sta.pos[1]=getbits(rtcm->buff,i,32)*0.01; i+=32;
        rtcm->sta.pos[2]=getbits(rtcm->buff,i,32)*0.01;
    }
    else {
        trace(2,"rtcm2 3 length error: len=%d\n",rtcm->len);
        return -1;
    }
    return 5;
}
/* decode type 14: gps time of week ------------------------------------------*/
static int decode_type14(rtcm_t *rtcm)
{
    double zcnt;
    int i=48,week,hour,leaps;
    
    trace(4,"decode_type14: len=%d\n",rtcm->len);
    
    zcnt=getbitu(rtcm->buff,24,13);
    if (i+24<=rtcm->len*8) {
        week =getbitu(rtcm->buff,i,10); i+=10;
        hour =getbitu(rtcm->buff,i, 8); i+= 8;
        leaps=getbitu(rtcm->buff,i, 6);
    }
    else {
        trace(2,"rtcm2 14 length error: len=%d\n",rtcm->len);
        return -1;
    }
    week=adjgpsweek(week);
    rtcm->time=gpst2time(week,hour*3600.0+zcnt*0.6);
    rtcm->nav.leaps=leaps;
    return 6;
}
/* decode type 16: gps special message ---------------------------------------*/
static int decode_type16(rtcm_t *rtcm)
{
    int i=48,n=0;
    
    trace(4,"decode_type16: len=%d\n",rtcm->len);
    
    while (i+8<=rtcm->len*8&&n<90) {
        rtcm->msg[n++]=getbitu(rtcm->buff,i,8); i+=8;
    }
    rtcm->msg[n]='\0';
    
    trace(3,"rtcm2 16 message: %s\n",rtcm->msg);
    return 9;
}
/* decode type 17: gps ephemerides -------------------------------------------*/
static int decode_type17(rtcm_t *rtcm)
{
    eph_t eph={0};
    double toc,sqrtA;
    int i=48,week,prn,sat;
    
    trace(4,"decode_type17: len=%d\n",rtcm->len);
    
    if (i+480<=rtcm->len*8) {
        week      =getbitu(rtcm->buff,i,10);              i+=10;
        eph.idot  =getbits(rtcm->buff,i,14)*P2_43*SC2RAD; i+=14;
        eph.iode  =getbitu(rtcm->buff,i, 8);              i+= 8;
        toc       =getbitu(rtcm->buff,i,16)*16.0;         i+=16;
        eph.f1    =getbits(rtcm->buff,i,16)*P2_43;        i+=16;
        eph.f2    =getbits(rtcm->buff,i, 8)*P2_55;        i+= 8;
        eph.crs   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.deln  =getbits(rtcm->buff,i,16)*P2_43*SC2RAD; i+=16;
        eph.cuc   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.e     =getbitu(rtcm->buff,i,32)*P2_33;        i+=32;
        eph.cus   =getbits(rtcm->buff,i,16);              i+=16;
        sqrtA     =getbitu(rtcm->buff,i,32)*P2_19;        i+=32;
        eph.toes  =getbitu(rtcm->buff,i,16);              i+=16;
        eph.OMG0  =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cic   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.i0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cis   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.omg   =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.crc   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.OMGd  =getbits(rtcm->buff,i,24)*P2_43*SC2RAD; i+=24;
        eph.M0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.iodc  =getbitu(rtcm->buff,i,10);              i+=10;
        eph.f0    =getbits(rtcm->buff,i,22)*P2_31;        i+=22;
        prn       =getbitu(rtcm->buff,i, 5);              i+= 5+3;
        eph.tgd[0]=getbits(rtcm->buff,i, 8)*P2_31;        i+= 8;
        eph.code  =getbitu(rtcm->buff,i, 2);              i+= 2;
        eph.sva   =getbitu(rtcm->buff,i, 4);              i+= 4;
        eph.svh   =getbitu(rtcm->buff,i, 6);              i+= 6;
        eph.flag  =getbitu(rtcm->buff,i, 1);
    }
    else {
        trace(2,"rtcm2 17 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (prn==0) prn=32;
    sat=satno(SYS_GPS,prn);
    eph.sat=sat;
    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=rtcm->time;
    eph.A=sqrtA*sqrtA;
    rtcm->nav.eph[sat-1]=eph;
    rtcm->ephsat=sat;
    return 2;
}
/* decode type 18: rtk uncorrected carrier-phase -----------------------------*/
static int decode_type18(rtcm_t *rtcm)
{
    gtime_t time;
    double usec,cp,tt;
    int i=48,index,freq,sync=1,code,sys,prn,sat,loss;
    
    trace(4,"decode_type18: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        freq=getbitu(rtcm->buff,i, 2); i+= 2+2;
        usec=getbitu(rtcm->buff,i,20); i+=20;
    }
    else {
        trace(2,"rtcm2 18 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (freq&0x1) {
        trace(2,"rtcm2 18 not supported frequency: freq=%d\n",freq);
        return -1;
    }
    freq>>=1;
    
    while (i+48<=rtcm->len*8&&rtcm->obs.n<MAXOBS) {
        sync=getbitu(rtcm->buff,i, 1); i+= 1;
        code=getbitu(rtcm->buff,i, 1); i+= 1;
        sys =getbitu(rtcm->buff,i, 1); i+= 1;
        prn =getbitu(rtcm->buff,i, 5); i+= 5+3;
        loss=getbitu(rtcm->buff,i, 5); i+= 5;
        cp  =getbits(rtcm->buff,i,32); i+=32;
        if (prn==0) prn=32;
        if (!(sat=satno(sys?SYS_GLO:SYS_GPS,prn))) {
            trace(2,"rtcm2 18 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        time=timeadd(rtcm->time,usec*1E-6);
        if (sys) time=utc2gpst(time); /* convert glonass time to gpst */
        
        tt=timediff(rtcm->obs.data[0].time,time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,time,sat))>=0) {
            rtcm->obs.data[index].L[freq]=-cp/256.0;
            rtcm->obs.data[index].LLI[freq]=rtcm->loss[sat-1][freq]!=loss;
            rtcm->obs.data[index].code[freq]=
                !freq?(code?CODE_L1P:CODE_L1C):(code?CODE_L2P:CODE_L2C);
            rtcm->loss[sat-1][freq]=loss;
        }
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* decode type 19: rtk uncorrected pseudorange -------------------------------*/
static int decode_type19(rtcm_t *rtcm)
{
    gtime_t time;
    double usec,pr,tt;
    int i=48,index,freq,sync=1,code,sys,prn,sat;
    
    trace(4,"decode_type19: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        freq=getbitu(rtcm->buff,i, 2); i+= 2+2;
        usec=getbitu(rtcm->buff,i,20); i+=20;
    }
    else {
        trace(2,"rtcm2 19 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (freq&0x1) {
        trace(2,"rtcm2 19 not supported frequency: freq=%d\n",freq);
        return -1;
    }
    freq>>=1;
    
    while (i+48<=rtcm->len*8&&rtcm->obs.n<MAXOBS) {
        sync=getbitu(rtcm->buff,i, 1); i+= 1;
        code=getbitu(rtcm->buff,i, 1); i+= 1;
        sys =getbitu(rtcm->buff,i, 1); i+= 1;
        prn =getbitu(rtcm->buff,i, 5); i+= 5+8;
        pr  =getbitu(rtcm->buff,i,32); i+=32;
        if (prn==0) prn=32;
        if (!(sat=satno(sys?SYS_GLO:SYS_GPS,prn))) {
            trace(2,"rtcm2 19 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        time=timeadd(rtcm->time,usec*1E-6);
        if (sys) time=utc2gpst(time); /* convert glonass time to gpst */
        
        tt=timediff(rtcm->obs.data[0].time,time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,time,sat))>=0) {
            rtcm->obs.data[index].P[freq]=pr*0.02;
            rtcm->obs.data[index].code[freq]=
                !freq?(code?CODE_L1P:CODE_L1C):(code?CODE_L2P:CODE_L2C);
        }
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* decode type 22: extended reference station parameter ----------------------*/
static int decode_type22(rtcm_t *rtcm)
{
    double del[2][3]={{0}},hgt=0.0;
    int i=48,j,noh;
    
    trace(4,"decode_type22: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        del[0][0]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
        del[0][1]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
        del[0][2]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
    }
    else {
        trace(2,"rtcm2 22 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (i+24<=rtcm->len*8) {
        i+=5; noh=getbits(rtcm->buff,i,1); i+=1;
        hgt=noh?0.0:getbitu(rtcm->buff,i,18)/25600.0;
        i+=18;
    }
    if (i+24<=rtcm->len*8) {
        del[1][0]=getbits(rtcm->buff,i,8)/1600.0; i+=8;
        del[1][1]=getbits(rtcm->buff,i,8)/1600.0; i+=8;
        del[1][2]=getbits(rtcm->buff,i,8)/1600.0;
    }
    rtcm->sta.deltype=1; /* xyz */
    for (j=0;j<3;j++) rtcm->sta.del[j]=del[0][j];
    rtcm->sta.hgt=hgt;
    return 5;
}
/* decode type 23: antenna type definition record ----------------------------*/
static int decode_type23(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 24: antenna reference point (arp) -----------------------------*/
static int decode_type24(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 31: differential glonass correction ---------------------------*/
static int decode_type31(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 32: differential glonass reference station parameters ---------*/
static int decode_type32(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 34: glonass partial differential correction set ---------------*/
static int decode_type34(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 36: glonass special message -----------------------------------*/
static int decode_type36(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 37: gnss system time offset -----------------------------------*/
static int decode_type37(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 59: proprietary message ---------------------------------------*/
static int decode_type59(rtcm_t *rtcm)
{
    return 0;
}
/* decode rtcm ver.2 message -------------------------------------------------*/
extern int decode_rtcm2(rtcm_t *rtcm)
{
    double zcnt;
    int staid,seqno,stah,ret=0,type=getbitu(rtcm->buff,8,6);
    
    trace(3,"decode_rtcm2: type=%2d len=%3d\n",type,rtcm->len);
    
    if ((zcnt=getbitu(rtcm->buff,24,13)*0.6)>=3600.0) {
        trace(2,"rtcm2 modified z-count error: zcnt=%.1f\n",zcnt);
        return -1;
    }
    adjhour(rtcm,zcnt);
    staid=getbitu(rtcm->buff,14,10);
    seqno=getbitu(rtcm->buff,37, 3);
    stah =getbitu(rtcm->buff,45, 3);
    if (seqno-rtcm->seqno!=1&&seqno-rtcm->seqno!=-7) {
        trace(2,"rtcm2 message outage: seqno=%d->%d\n",rtcm->seqno,seqno);
    }
    rtcm->seqno=seqno;
    rtcm->stah =stah;
    
    if (rtcm->outtype) {
        sprintf(rtcm->msgtype,"RTCM %2d (%4d) zcnt=%7.1f staid=%3d seqno=%d",
                type,rtcm->len,zcnt,staid,seqno);
    }
    if (type==3||type==22||type==23||type==24) {
        if (rtcm->staid!=0&&staid!=rtcm->staid) {
           trace(2,"rtcm2 station id changed: %d->%d\n",rtcm->staid,staid);
        }
        rtcm->staid=staid;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm2 station id invalid: %d %d\n",staid,rtcm->staid);
        return -1;
    }
    switch (type) {
        case  1: ret=decode_type1 (rtcm); break;
        case  3: ret=decode_type3 (rtcm); break;
        case  9: ret=decode_type1 (rtcm); break;
        case 14: ret=decode_type14(rtcm); break;
        case 16: ret=decode_type16(rtcm); break;
        case 17: ret=decode_type17(rtcm); break;
        case 18: ret=decode_type18(rtcm); break;
        case 19: ret=decode_type19(rtcm); break;
        case 22: ret=decode_type22(rtcm); break;
        case 23: ret=decode_type23(rtcm); break; /* not supported */
        case 24: ret=decode_type24(rtcm); break; /* not supported */
        case 31: ret=decode_type31(rtcm); break; /* not supported */
        case 32: ret=decode_type32(rtcm); break; /* not supported */
        case 34: ret=decode_type34(rtcm); break; /* not supported */
        case 36: ret=decode_type36(rtcm); break; /* not supported */
        case 37: ret=decode_type37(rtcm); break; /* not supported */
        case 59: ret=decode_type59(rtcm); break; /* not supported */
    }
    if (ret>=0) {
        if (1<=type&&type<=99) rtcm->nmsg2[type]++; else rtcm->nmsg2[0]++;
    }
    return ret;
}
