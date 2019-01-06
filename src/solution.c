/*------------------------------------------------------------------------------
* solution.c : solution functions
*
*          Copyright (C) 2007-2018 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] National Marine Electronic Association and International Marine
*         Electronics Association, NMEA 0183 version 4.10, August 1, 2012
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2007/11/03  1.0 new
*           2009/01/05  1.1  add function outsols(), outsolheads(),
*                            setsolformat(), outsolexs, outsolex
*           2009/04/02  1.2  add dummy fields in NMEA mesassage
*                            fix bug to format lat/lon as deg-min-sec
*           2009/04/14  1.3  add age and ratio field to solution
*           2009/11/25  1.4  add function readsolstat()
*           2010/02/14  1.5  fix bug on output of gpstime at week boundary
*           2010/07/05  1.6  added api:
*                                initsolbuf(),freesolbuf(),addsol(),getsol(),
*                                inputsol(),outprcopts(),outprcopt()
*                            modified api:
*                                readsol(),readsolt(),readsolstat(),
*                                readsolstatt(),outsolheads(),outsols(),
*                                outsolexs(),outsolhead(),outsol(),outsolex(),
*                                outnmea_rmc(),outnmea_gga(),outnmea_gsa(),
*                                outnmea_gsv()
*                            deleted api:
*                                setsolopt(),setsolformat()
*           2010/08/14  1.7  fix bug on initialize solution buffer (2.4.0_p2)
*                            suppress enu-solution if base pos not available
*                            (2.4.0_p3)
*           2010/08/16  1.8  suppress null record if solution is not avalilable
*                            (2.4.0_p4)
*           2011/01/23  1.9  fix bug on reading nmea solution data
*                            add api freesolstatbuf()
*           2012/02/05  1.10 fix bug on output nmea gpgsv
*           2013/02/18  1.11 support nmea GLGSA,GAGSA,GLCSV,GACSV sentence
*           2013/09/01  1.12 fix bug on presentation of nmea time tag
*           2015/02/11  1.13 fix bug on checksum of $GLGSA and $GAGSA
*                            fix bug on satellite id of $GAGSA
*           2016/01/17  1.14 support reading NMEA GxZDA
*                            ignore NMEA talker ID
*           2016/07/30  1.15 suppress output if std is over opt->maxsolstd
*           2017/06/13  1.16 support output/input of velocity solution
*           2018/10/10  1.17 support reading solution status file
*-----------------------------------------------------------------------------*/
#include <ctype.h>
#include "rtklib.h"

/* constants and macros ------------------------------------------------------*/

#define SQR(x)     ((x)<0.0?-(x)*(x):(x)*(x))
#define SQRT(x)    ((x)<0.0||(x)!=(x)?0.0:sqrt(x))

#define MAXFIELD   64           /* max number of fields in a record */
#define MAXNMEA    256          /* max length of nmea sentence */

#define KNOT2M     0.514444444  /* m/knot */

static const int solq_nmea[]={  /* nmea quality flags to rtklib sol quality */
    /* nmea 0183 v.2.3 quality flags: */
    /*  0=invalid, 1=gps fix (sps), 2=dgps fix, 3=pps fix, 4=rtk, 5=float rtk */
    /*  6=estimated (dead reckoning), 7=manual input, 8=simulation */
    
    SOLQ_NONE ,SOLQ_SINGLE, SOLQ_DGPS, SOLQ_PPP , SOLQ_FIX,
    SOLQ_FLOAT,SOLQ_DR    , SOLQ_NONE, SOLQ_NONE, SOLQ_NONE
};
/* solution option to field separator ----------------------------------------*/
static const char *opt2sep(const solopt_t *opt)
{
    if (!*opt->sep) return " ";
    else if (!strcmp(opt->sep,"\\t")) return "\t";
    return opt->sep;
}
/* separate fields -----------------------------------------------------------*/
static int tonum(char *buff, const char *sep, double *v)
{
    int n,len=(int)strlen(sep);
    char *p,*q;
    
    for (p=buff,n=0;n<MAXFIELD;p=q+len) {
        if ((q=strstr(p,sep))) *q='\0'; 
        if (*p) v[n++]=atof(p);
        if (!q) break;
    }
    return n;
}
/* sqrt of covariance --------------------------------------------------------*/
static double sqvar(double covar)
{
    return covar<0.0?-sqrt(-covar):sqrt(covar);
}
/* convert ddmm.mm in nmea format to deg -------------------------------------*/
static double dmm2deg(double dmm)
{
    return floor(dmm/100.0)+fmod(dmm,100.0)/60.0;
}
/* convert time in nmea format to time ---------------------------------------*/
static void septime(double t, double *t1, double *t2, double *t3)
{
    *t1=floor(t/10000.0);
    t-=*t1*10000.0;
    *t2=floor(t/100.0);
    *t3=t-*t2*100.0;
}
/* solution to covariance ----------------------------------------------------*/
static void soltocov(const sol_t *sol, double *P)
{
    P[0]     =sol->qr[0]; /* xx or ee */
    P[4]     =sol->qr[1]; /* yy or nn */
    P[8]     =sol->qr[2]; /* zz or uu */
    P[1]=P[3]=sol->qr[3]; /* xy or en */
    P[5]=P[7]=sol->qr[4]; /* yz or nu */
    P[2]=P[6]=sol->qr[5]; /* zx or ue */
}
/* covariance to solution ----------------------------------------------------*/
static void covtosol(const double *P, sol_t *sol)
{
    sol->qr[0]=(float)P[0]; /* xx or ee */
    sol->qr[1]=(float)P[4]; /* yy or nn */
    sol->qr[2]=(float)P[8]; /* zz or uu */
    sol->qr[3]=(float)P[1]; /* xy or en */
    sol->qr[4]=(float)P[5]; /* yz or nu */
    sol->qr[5]=(float)P[2]; /* zx or ue */
}
/* solution to velocity covariance -------------------------------------------*/
static void soltocov_vel(const sol_t *sol, double *P)
{
    P[0]     =sol->qv[0]; /* xx */
    P[4]     =sol->qv[1]; /* yy */
    P[8]     =sol->qv[2]; /* zz */
    P[1]=P[3]=sol->qv[3]; /* xy */
    P[5]=P[7]=sol->qv[4]; /* yz */
    P[2]=P[6]=sol->qv[5]; /* zx */
}
/* velocity covariance to solution -------------------------------------------*/
static void covtosol_vel(const double *P, sol_t *sol)
{
    sol->qv[0]=(float)P[0]; /* xx */
    sol->qv[1]=(float)P[4]; /* yy */
    sol->qv[2]=(float)P[8]; /* zz */
    sol->qv[3]=(float)P[1]; /* xy */
    sol->qv[4]=(float)P[5]; /* yz */
    sol->qv[5]=(float)P[2]; /* zx */
}
/* decode nmea gxrmc: recommended minumum data for gps -----------------------*/
static int decode_nmearmc(char **val, int n, sol_t *sol)
{
    double tod=0.0,lat=0.0,lon=0.0,vel=0.0,dir=0.0,date=0.0,ang=0.0,ep[6];
    double pos[3]={0};
    char act=' ',ns='N',ew='E',mew='E',mode='A';
    int i;
    
    trace(4,"decode_nmearmc: n=%d\n",n);
    
    for (i=0;i<n;i++) {
        switch (i) {
            case  0: tod =atof(val[i]); break; /* time in utc (hhmmss) */
            case  1: act =*val[i];      break; /* A=active,V=void */
            case  2: lat =atof(val[i]); break; /* latitude (ddmm.mmm) */
            case  3: ns  =*val[i];      break; /* N=north,S=south */
            case  4: lon =atof(val[i]); break; /* longitude (dddmm.mmm) */
            case  5: ew  =*val[i];      break; /* E=east,W=west */
            case  6: vel =atof(val[i]); break; /* speed (knots) */
            case  7: dir =atof(val[i]); break; /* track angle (deg) */
            case  8: date=atof(val[i]); break; /* date (ddmmyy) */
            case  9: ang =atof(val[i]); break; /* magnetic variation */
            case 10: mew =*val[i];      break; /* E=east,W=west */
            case 11: mode=*val[i];      break; /* mode indicator (>nmea 2) */
                                      /* A=autonomous,D=differential */
                                      /* E=estimated,N=not valid,S=simulator */
        }
    }
    if ((act!='A'&&act!='V')||(ns!='N'&&ns!='S')||(ew!='E'&&ew!='W')) {
        trace(2,"invalid nmea gprmc format\n");
        return 0;
    }
    pos[0]=(ns=='S'?-1.0:1.0)*dmm2deg(lat)*D2R;
    pos[1]=(ew=='W'?-1.0:1.0)*dmm2deg(lon)*D2R;
    septime(date,ep+2,ep+1,ep);
    septime(tod,ep+3,ep+4,ep+5);
    ep[0]+=ep[0]<80.0?2000.0:1900.0;
    sol->time=utc2gpst(epoch2time(ep));
    pos2ecef(pos,sol->rr);
    sol->stat=mode=='D'?SOLQ_DGPS:SOLQ_SINGLE;
    sol->ns=0;
    
    sol->type=0; /* postion type = xyz */
    
    trace(5,"decode_nmearmc: %s rr=%.3f %.3f %.3f stat=%d ns=%d vel=%.2f dir=%.0f ang=%.0f mew=%c mode=%c\n",
          time_str(sol->time,0),sol->rr[0],sol->rr[1],sol->rr[2],sol->stat,sol->ns,
          vel,dir,ang,mew,mode);
    
    return 2; /* update time */
}
/* decode nmea gxzda: utc day,month,year and local time zone offset ----------*/
static int decode_nmeazda(char **val, int n, sol_t *sol)
{
    double tod=0.0,ep[6]={0};
    int i;
    
    trace(4,"decode_nmeazda: n=%d\n",n);
    
    for (i=0;i<n;i++) {
        switch (i) {
            case  0: tod  =atof(val[i]); break; /* time in utc (hhmmss) */
            case  1: ep[2]=atof(val[i]); break; /* day (0-31) */
            case  2: ep[1]=atof(val[i]); break; /* mon (1-12) */
            case  3: ep[0]=atof(val[i]); break; /* year */
        }
    }
    septime(tod,ep+3,ep+4,ep+5);
    sol->time=utc2gpst(epoch2time(ep));
    sol->ns=0;
    
    trace(5,"decode_nmeazda: %s\n",time_str(sol->time,0));
    
    return 2; /* update time */
}
/* decode nmea gxgga: fix information ----------------------------------------*/
static int decode_nmeagga(char **val, int n, sol_t *sol)
{
    gtime_t time;
    double tod=0.0,lat=0.0,lon=0.0,hdop=0.0,alt=0.0,msl=0.0,ep[6],tt;
    double pos[3]={0};
    char ns='N',ew='E',ua=' ',um=' ';
    int i,solq=0,nrcv=0;
    
    trace(4,"decode_nmeagga: n=%d\n",n);
    
    for (i=0;i<n;i++) {
        switch (i) {
            case  0: tod =atof(val[i]); break; /* time in utc (hhmmss) */
            case  1: lat =atof(val[i]); break; /* latitude (ddmm.mmm) */
            case  2: ns  =*val[i];      break; /* N=north,S=south */
            case  3: lon =atof(val[i]); break; /* longitude (dddmm.mmm) */
            case  4: ew  =*val[i];      break; /* E=east,W=west */
            case  5: solq=atoi(val[i]); break; /* fix quality */
            case  6: nrcv=atoi(val[i]); break; /* # of satellite tracked */
            case  7: hdop=atof(val[i]); break; /* hdop */
            case  8: alt =atof(val[i]); break; /* altitude in msl */
            case  9: ua  =*val[i];      break; /* unit (M) */
            case 10: msl =atof(val[i]); break; /* height of geoid */
            case 11: um  =*val[i];      break; /* unit (M) */
        }
    }
    if ((ns!='N'&&ns!='S')||(ew!='E'&&ew!='W')) {
        trace(2,"invalid nmea gpgga format\n");
        return 0;
    }
    if (sol->time.time==0.0) {
        trace(2,"no date info for nmea gpgga\n");
        return 0;
    }
    pos[0]=(ns=='N'?1.0:-1.0)*dmm2deg(lat)*D2R;
    pos[1]=(ew=='E'?1.0:-1.0)*dmm2deg(lon)*D2R;
    pos[2]=alt+msl;
    
    time2epoch(sol->time,ep);
    septime(tod,ep+3,ep+4,ep+5);
    time=utc2gpst(epoch2time(ep));
    tt=timediff(time,sol->time);
    if      (tt<-43200.0) sol->time=timeadd(time, 86400.0);
    else if (tt> 43200.0) sol->time=timeadd(time,-86400.0);
    else sol->time=time;
    pos2ecef(pos,sol->rr);
    sol->stat=0<=solq&&solq<=8?solq_nmea[solq]:SOLQ_NONE;
    sol->ns=nrcv;
    
    sol->type=0; /* postion type = xyz */
    
    trace(5,"decode_nmeagga: %s rr=%.3f %.3f %.3f stat=%d ns=%d hdop=%.1f ua=%c um=%c\n",
          time_str(sol->time,0),sol->rr[0],sol->rr[1],sol->rr[2],sol->stat,sol->ns,
          hdop,ua,um);
    
    return 1;
}
/* test nmea -----------------------------------------------------------------*/
static int test_nmea(const char *buff)
{
    if (strlen(buff)<6||buff[0]!='$') return 0;
    return !strncmp(buff+1,"GP",2)||!strncmp(buff+1,"GA",2)||
           !strncmp(buff+1,"GL",2)||!strncmp(buff+1,"GN",2)||
           !strncmp(buff+1,"GB",2)||!strncmp(buff+1,"BD",2)||
           !strncmp(buff+1,"QZ",2);
}
/* test solution status ------------------------------------------------------*/
static int test_solstat(const char *buff)
{
    if (strlen(buff)<7||buff[0]!='$') return 0;
    return !strncmp(buff+1,"POS" ,3)||!strncmp(buff+1,"VELACC",6)||
           !strncmp(buff+1,"CLK" ,3)||!strncmp(buff+1,"ION"   ,3)||
           !strncmp(buff+1,"TROP",4)||!strncmp(buff+1,"HWBIAS",6)||
           !strncmp(buff+1,"TRPG",4)||!strncmp(buff+1,"AMB"   ,3)||
           !strncmp(buff+1,"SAT" ,3);
}
/* decode nmea ---------------------------------------------------------------*/
static int decode_nmea(char *buff, sol_t *sol)
{
    char *p,*q,*val[MAXFIELD];
    int n=0;
    
    trace(4,"decode_nmea: buff=%s\n",buff);
    
    /* parse fields */
    for (p=buff;*p&&n<MAXFIELD;p=q+1) {
        if ((q=strchr(p,','))||(q=strchr(p,'*'))) {
            val[n++]=p; *q='\0';
        }
        else break;
    }
    if (!strcmp(val[0]+3,"RMC")) { /* $xxRMC */
        return decode_nmearmc(val+1,n-1,sol);
    }
    else if (!strcmp(val[0]+3,"ZDA")) { /* $xxZDA */
        return decode_nmeazda(val+1,n-1,sol);
    }
    else if (!strcmp(val[0]+3,"GGA")) { /* $xxGGA */
        return decode_nmeagga(val+1,n-1,sol);
    }
    return 0;
}
/* decode solution time ------------------------------------------------------*/
static char *decode_soltime(char *buff, const solopt_t *opt, gtime_t *time)
{
    double v[MAXFIELD];
    char *p,*q,s[64]=" ";
    int n,len;
    
    trace(4,"decode_soltime:\n");
    
    if (!strcmp(opt->sep,"\\t")) strcpy(s,"\t");
    else if (*opt->sep) strcpy(s,opt->sep);
    len=(int)strlen(s);
    
    if (opt->posf==SOLF_STAT) {
        return buff;
    }
    /* yyyy/mm/dd hh:mm:ss or yyyy mm dd hh:mm:ss */
    if (sscanf(buff,"%lf/%lf/%lf %lf:%lf:%lf",v,v+1,v+2,v+3,v+4,v+5)>=6) {
        if (v[0]<100.0) {
            v[0]+=v[0]<80.0?2000.0:1900.0;
        }
        *time=epoch2time(v);
        if (opt->times==TIMES_UTC) {
            *time=utc2gpst(*time);
        }
        else if (opt->times==TIMES_JST) {
            *time=utc2gpst(timeadd(*time,-9*3600.0));
        }
        if (!(p=strchr(buff,':'))||!(p=strchr(p+1,':'))) return NULL;
        for (p++;isdigit((int)*p)||*p=='.';) p++;
        return p+len;
    }
    if (opt->posf==SOLF_GSIF) {
        if (sscanf(buff,"%lf %lf %lf %lf:%lf:%lf",v,v+1,v+2,v+3,v+4,v+5)<6) {
            return NULL;
        }
        *time=timeadd(epoch2time(v),-12.0*3600.0);
        if (!(p=strchr(buff,':'))||!(p=strchr(p+1,':'))) return NULL;
        for (p++;isdigit((int)*p)||*p=='.';) p++;
        return p+len;
    }
    /* wwww ssss */
    for (p=buff,n=0;n<2;p=q+len) {
        if ((q=strstr(p,s))) *q='\0'; 
        if (*p) v[n++]=atof(p);
        if (!q) break;
    }
    if (n>=2&&0.0<=v[0]&&v[0]<=3000.0&&0.0<=v[1]&&v[1]<604800.0) {
        *time=gpst2time((int)v[0],v[1]);
        return p;
    }
    return NULL;
}
/* decode x/y/z-ecef ---------------------------------------------------------*/
static int decode_solxyz(char *buff, const solopt_t *opt, sol_t *sol)
{
    double val[MAXFIELD],P[9]={0};
    int i=0,j,n;
    const char *sep=opt2sep(opt);
    
    trace(4,"decode_solxyz:\n");
    
    if ((n=tonum(buff,sep,val))<3) return 0;
    
    for (j=0;j<3;j++) {
        sol->rr[j]=val[i++]; /* xyz */
    }
    if (i<n) sol->stat=(unsigned char)val[i++];
    if (i<n) sol->ns  =(unsigned char)val[i++];
    if (i+3<=n) {
        P[0]=val[i]*val[i]; i++; /* sdx */
        P[4]=val[i]*val[i]; i++; /* sdy */
        P[8]=val[i]*val[i]; i++; /* sdz */
        if (i+3<=n) {
            P[1]=P[3]=SQR(val[i]); i++; /* sdxy */
            P[5]=P[7]=SQR(val[i]); i++; /* sdyz */
            P[2]=P[6]=SQR(val[i]); i++; /* sdzx */
        }
        covtosol(P,sol);
    }
    if (i<n) sol->age  =(float)val[i++];
    if (i<n) sol->ratio=(float)val[i++];
    
    if (i+3<=n) { /* velocity */
        for (j=0;j<3;j++) {
            sol->rr[j+3]=val[i++]; /* xyz */
        }
    }
    if (i+3<=n) {
        for (j=0;j<9;j++) P[j]=0.0;
        P[0]=val[i]*val[i]; i++; /* sdx */
        P[4]=val[i]*val[i]; i++; /* sdy */
        P[8]=val[i]*val[i]; i++; /* sdz */
        if (i+3<n) {
            P[1]=P[3]=SQR(val[i]); i++; /* sdxy */
            P[5]=P[7]=SQR(val[i]); i++; /* sdyz */
            P[2]=P[6]=SQR(val[i]); i++; /* sdzx */
        }
        covtosol_vel(P,sol);
    }
    sol->type=0; /* postion type = xyz */
    
    if (MAXSOLQ<sol->stat) sol->stat=SOLQ_NONE;
    return 1;
}
/* decode lat/lon/height -----------------------------------------------------*/
static int decode_solllh(char *buff, const solopt_t *opt, sol_t *sol)
{
    double val[MAXFIELD],pos[3],vel[3],Q[9]={0},P[9];
    int i=0,j,n;
    const char *sep=opt2sep(opt);
    
    trace(4,"decode_solllh:\n");
    
    n=tonum(buff,sep,val);
    
    if (!opt->degf) {
        if (n<3) return 0;
        pos[0]=val[i++]*D2R; /* lat/lon/hgt (ddd.ddd) */
        pos[1]=val[i++]*D2R;
        pos[2]=val[i++];
    }
    else {
        if (n<7) return 0;
        pos[0]=dms2deg(val  )*D2R; /* lat/lon/hgt (ddd mm ss) */
        pos[1]=dms2deg(val+3)*D2R;
        pos[2]=val[6];
        i+=7;
    }
    pos2ecef(pos,sol->rr);
    if (i<n) sol->stat=(unsigned char)val[i++];
    if (i<n) sol->ns  =(unsigned char)val[i++];
    if (i+3<=n) {
        Q[4]=val[i]*val[i]; i++; /* sdn */
        Q[0]=val[i]*val[i]; i++; /* sde */
        Q[8]=val[i]*val[i]; i++; /* sdu */
        if (i+3<n) {
            Q[1]=Q[3]=SQR(val[i]); i++; /* sdne */
            Q[2]=Q[6]=SQR(val[i]); i++; /* sdeu */
            Q[5]=Q[7]=SQR(val[i]); i++; /* sdun */
        }
        covecef(pos,Q,P);
        covtosol(P,sol);
    }
    if (i<n) sol->age  =(float)val[i++];
    if (i<n) sol->ratio=(float)val[i++];
    
    if (i+3<=n) { /* velocity */
        vel[1]=val[i++]; /* vel-n */
        vel[0]=val[i++]; /* vel-e */
        vel[2]=val[i++]; /* vel-u */
        enu2ecef(pos,vel,sol->rr+3);
    }
    if (i+3<=n) {
        for (j=0;j<9;j++) Q[j]=0.0;
        Q[4]=val[i]*val[i]; i++; /* sdn */
        Q[0]=val[i]*val[i]; i++; /* sde */
        Q[8]=val[i]*val[i]; i++; /* sdu */
        if (i+3<=n) {
            Q[1]=Q[3]=SQR(val[i]); i++; /* sdne */
            Q[2]=Q[6]=SQR(val[i]); i++; /* sdeu */
            Q[5]=Q[7]=SQR(val[i]); i++; /* sdun */
        }
        covecef(pos,Q,P);
        covtosol_vel(P,sol);
    }
    sol->type=0; /* postion type = xyz */
    
    if (MAXSOLQ<sol->stat) sol->stat=SOLQ_NONE;
    return 1;
}
/* decode e/n/u-baseline -----------------------------------------------------*/
static int decode_solenu(char *buff, const solopt_t *opt, sol_t *sol)
{
    double val[MAXFIELD],Q[9]={0};
    int i=0,j,n;
    const char *sep=opt2sep(opt);
    
    trace(4,"decode_solenu:\n");
    
    if ((n=tonum(buff,sep,val))<3) return 0;
    
    for (j=0;j<3;j++) {
        sol->rr[j]=val[i++]; /* enu */
    }
    if (i<n) sol->stat=(unsigned char)val[i++];
    if (i<n) sol->ns  =(unsigned char)val[i++];
    if (i+3<=n) {
        Q[0]=val[i]*val[i]; i++; /* sde */
        Q[4]=val[i]*val[i]; i++; /* sdn */
        Q[8]=val[i]*val[i]; i++; /* sdu */
        if (i+3<=n) {
            Q[1]=Q[3]=SQR(val[i]); i++; /* sden */
            Q[5]=Q[7]=SQR(val[i]); i++; /* sdnu */
            Q[2]=Q[6]=SQR(val[i]); i++; /* sdue */
        }
        covtosol(Q,sol);
    }
    if (i<n) sol->age  =(float)val[i++];
    if (i<n) sol->ratio=(float)val[i++];
    
    sol->type=1; /* postion type = enu */
    
    if (MAXSOLQ<sol->stat) sol->stat=SOLQ_NONE;
    return 1;
}
/* decode solution status ----------------------------------------------------*/
static int decode_solsss(char *buff, sol_t *sol)
{
    double tow,pos[3],std[3]={0};
    int i,week,solq;
    
    trace(4,"decode_solssss:\n");
    
    if (sscanf(buff,"$POS,%d,%lf,%d,%lf,%lf,%lf,%lf,%lf,%lf",&week,&tow,&solq,
               pos,pos+1,pos+2,std,std+1,std+2)<6) {
        return 0;
    }
    if (week<=0||norm(pos,3)<=0.0||solq==SOLQ_NONE) {
        return 0;
    }
    sol->time=gpst2time(week,tow);
    for (i=0;i<6;i++) {
        sol->rr[i]=i<3?pos[i]:0.0;
        sol->qr[i]=i<3?(float)SQR(std[i]):0.0f;
        sol->dtr[i]=0.0;
    }
    sol->ns=0;
    sol->age=sol->ratio=sol->thres=0.0f;
    sol->type=0; /* position type = xyz */
    sol->stat=solq;
    return 1;
}
/* decode gsi f solution -----------------------------------------------------*/
static int decode_solgsi(char *buff, const solopt_t *opt, sol_t *sol)
{
    double val[MAXFIELD];
    int i=0,j;
    
    trace(4,"decode_solgsi:\n");
    
    if (tonum(buff," ",val)<3) return 0;
    
    for (j=0;j<3;j++) {
        sol->rr[j]=val[i++]; /* xyz */
    }
    sol->stat=SOLQ_FIX;
    return 1;
}
/* decode solution position --------------------------------------------------*/
static int decode_solpos(char *buff, const solopt_t *opt, sol_t *sol)
{
    sol_t sol0={{0}};
    char *p=buff;
    
    trace(4,"decode_solpos: buff=%s\n",buff);
    
    *sol=sol0;
    
    /* decode solution time */
    if (!(p=decode_soltime(p,opt,&sol->time))) {
        return 0;
    }
    /* decode solution position */
    switch (opt->posf) {
        case SOLF_XYZ : return decode_solxyz(p,opt,sol);
        case SOLF_LLH : return decode_solllh(p,opt,sol);
        case SOLF_ENU : return decode_solenu(p,opt,sol);
        case SOLF_GSIF: return decode_solgsi(p,opt,sol);
    }
    return 0;
}
/* decode reference position -------------------------------------------------*/
static void decode_refpos(char *buff, const solopt_t *opt, double *rb)
{
    double val[MAXFIELD],pos[3];
    int i,n;
    const char *sep=opt2sep(opt);
    
    trace(3,"decode_refpos: buff=%s\n",buff);
    
    if ((n=tonum(buff,sep,val))<3) return;
    
    if (opt->posf==SOLF_XYZ) { /* xyz */
        for (i=0;i<3;i++) rb[i]=val[i];
    }
    else if (opt->degf==0) { /* lat/lon/hgt (ddd.ddd) */
        pos[0]=val[0]*D2R;
        pos[1]=val[1]*D2R;
        pos[2]=val[2];
        pos2ecef(pos,rb);
    }
    else if (opt->degf==1&&n>=7) { /* lat/lon/hgt (ddd mm ss) */
        pos[0]=dms2deg(val  )*D2R;
        pos[1]=dms2deg(val+3)*D2R;
        pos[2]=val[6];
        pos2ecef(pos,rb);
    }
}
/* decode solution -----------------------------------------------------------*/
static int decode_sol(char *buff, const solopt_t *opt, sol_t *sol, double *rb)
{
    char *p;
    
    trace(4,"decode_sol: buff=%s\n",buff);
    
    if (test_nmea(buff)) { /* decode nmea */
        return decode_nmea(buff,sol);
    }
    else if (test_solstat(buff)) { /* decode solution status */
        return decode_solsss(buff,sol);
    }
    if (!strncmp(buff,COMMENTH,1)) { /* reference position */
        if (!strstr(buff,"ref pos")&&!strstr(buff,"slave pos")) return 0;
        if (!(p=strchr(buff,':'))) return 0;
        decode_refpos(p+1,opt,rb);
        return 0;
    }
    /* decode position record */
    return decode_solpos(buff,opt,sol);
}
/* decode solution options ---------------------------------------------------*/
static void decode_solopt(char *buff, solopt_t *opt)
{
    char *p;
    
    trace(4,"decode_solhead: buff=%s\n",buff);
    
    if (strncmp(buff,COMMENTH,1)&&strncmp(buff,"+",1)) return;
    
    if      (strstr(buff,"GPST")) opt->times=TIMES_GPST;
    else if (strstr(buff,"UTC" )) opt->times=TIMES_UTC;
    else if (strstr(buff,"JST" )) opt->times=TIMES_JST;
    
    if ((p=strstr(buff,"x-ecef(m)"))) {
        opt->posf=SOLF_XYZ;
        opt->degf=0;
        strncpy(opt->sep,p+9,1);
        opt->sep[1]='\0';
    }
    else if ((p=strstr(buff,"latitude(d'\")"))) {
        opt->posf=SOLF_LLH;
        opt->degf=1;
        strncpy(opt->sep,p+14,1);
        opt->sep[1]='\0';
    }
    else if ((p=strstr(buff,"latitude(deg)"))) {
        opt->posf=SOLF_LLH;
        opt->degf=0;
        strncpy(opt->sep,p+13,1);
        opt->sep[1]='\0';
    }
    else if ((p=strstr(buff,"e-baseline(m)"))) {
        opt->posf=SOLF_ENU;
        opt->degf=0;
        strncpy(opt->sep,p+13,1);
        opt->sep[1]='\0';
    }
    else if ((p=strstr(buff,"+SITE/INF"))) { /* gsi f2/f3 solution */
        opt->times=TIMES_GPST;
        opt->posf=SOLF_GSIF;
        opt->degf=0;
        strcpy(opt->sep," ");
    }
}
/* read solution option ------------------------------------------------------*/
static void readsolopt(FILE *fp, solopt_t *opt)
{
    char buff[MAXSOLMSG+1];
    int i;
    
    trace(3,"readsolopt:\n");
    
    for (i=0;fgets(buff,sizeof(buff),fp)&&i<100;i++) { /* only 100 lines */
        
        /* decode solution options */
        decode_solopt(buff,opt);
    }
}
/* input solution data from stream ---------------------------------------------
* input solution data from stream
* args   : unsigned char data I stream data
*          gtime_t ts       I  start time (ts.time==0: from start)
*          gtime_t te       I  end time   (te.time==0: to end)
*          double tint      I  time interval (0: all)
*          int    qflag     I  quality flag  (0: all)
*          solbuf_t *solbuf IO solution buffer
* return : status (1:solution received,0:no solution,-1:disconnect received)
*-----------------------------------------------------------------------------*/
extern int inputsol(unsigned char data, gtime_t ts, gtime_t te, double tint,
                    int qflag, const solopt_t *opt, solbuf_t *solbuf)
{
    sol_t sol={{0}};
    int stat;
    
    trace(4,"inputsol: data=0x%02x\n",data);
    
    sol.time=solbuf->time;
    
    if (data=='$'||(!isprint(data)&&data!='\r'&&data!='\n')) { /* sync header */
        solbuf->nb=0;
    }
    solbuf->buff[solbuf->nb++]=data;
    if (data!='\n'&&solbuf->nb<MAXSOLMSG) return 0; /* sync trailer */
    
    solbuf->buff[solbuf->nb]='\0';
    solbuf->nb=0;
    
    /* check disconnect message */
    if (!strcmp((char *)solbuf->buff,MSG_DISCONN)) {
        trace(3,"disconnect received\n");
        return -1;
    }
    /* decode solution */
    sol.time=solbuf->time;
    if ((stat=decode_sol((char *)solbuf->buff,opt,&sol,solbuf->rb))>0) {
        if (stat) solbuf->time=sol.time; /* update current time */
        if (stat!=1) return 0;
    }
    if (stat!=1||!screent(sol.time,ts,te,tint)||(qflag&&sol.stat!=qflag)) {
        return 0;
    }
    /* add solution to solution buffer */
    return addsol(solbuf,&sol);
}
/* read solution data --------------------------------------------------------*/
static int readsoldata(FILE *fp, gtime_t ts, gtime_t te, double tint, int qflag,
                      const solopt_t *opt, solbuf_t *solbuf)
{
    int c;
    
    trace(3,"readsoldata:\n");
    
    while ((c=fgetc(fp))!=EOF) {
        
        /* input solution */
        inputsol((unsigned char)c,ts,te,tint,qflag,opt,solbuf);
    }
    return solbuf->n>0;
}
/* compare solution data -----------------------------------------------------*/
static int cmpsol(const void *p1, const void *p2)
{
    sol_t *q1=(sol_t *)p1,*q2=(sol_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-0.0?-1:(tt>0.0?1:0);
}
/* sort solution data --------------------------------------------------------*/
static int sort_solbuf(solbuf_t *solbuf)
{
    sol_t *solbuf_data;
    
    trace(4,"sort_solbuf: n=%d\n",solbuf->n);
    
    if (solbuf->n<=0) return 0;
    
    if (!(solbuf_data=(sol_t *)realloc(solbuf->data,sizeof(sol_t)*solbuf->n))) {
        trace(1,"sort_solbuf: memory allocation error\n");
        free(solbuf->data); solbuf->data=NULL; solbuf->n=solbuf->nmax=0;
        return 0;
    }
    solbuf->data=solbuf_data;
    qsort(solbuf->data,solbuf->n,sizeof(sol_t),cmpsol);
    solbuf->nmax=solbuf->n;
    solbuf->start=0;
    solbuf->end=solbuf->n-1;
    return 1;
}
/* read solutions data from solution files -------------------------------------
* read solution data from soluiton files
* args   : char   *files[]  I  solution files
*          int    nfile     I  number of files
*         (gtime_t ts)      I  start time (ts.time==0: from start)
*         (gtime_t te)      I  end time   (te.time==0: to end)
*         (double tint)     I  time interval (0: all)
*         (int    qflag)    I  quality flag  (0: all)
*          solbuf_t *solbuf O  solution buffer
* return : status (1:ok,0:no data or error)
*-----------------------------------------------------------------------------*/
extern int readsolt(char *files[], int nfile, gtime_t ts, gtime_t te,
                    double tint, int qflag, solbuf_t *solbuf)
{
    FILE *fp;
    solopt_t opt=solopt_default;
    int i;
    
    trace(3,"readsolt: nfile=%d\n",nfile);
    
    initsolbuf(solbuf,0,0);
    
    for (i=0;i<nfile;i++) {
        if (!(fp=fopen(files[i],"rb"))) {
            trace(2,"readsolt: file open error %s\n",files[i]);
            continue;
        }
        /* read solution options in header */
        readsolopt(fp,&opt);
        rewind(fp);
        
        /* read solution data */
        if (!readsoldata(fp,ts,te,tint,qflag,&opt,solbuf)) {
            trace(2,"readsolt: no solution in %s\n",files[i]);
        }
        fclose(fp);
    }
    return sort_solbuf(solbuf);
}
extern int readsol(char *files[], int nfile, solbuf_t *sol)
{
    gtime_t time={0};
    
    trace(3,"readsol: nfile=%d\n",nfile);
    
    return readsolt(files,nfile,time,time,0.0,0,sol);
}
/* add solution data to solution buffer ----------------------------------------
* add solution data to solution buffer
* args   : solbuf_t *solbuf IO solution buffer
*          sol_t  *sol      I  solution data
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int addsol(solbuf_t *solbuf, const sol_t *sol)
{
    sol_t *solbuf_data;
    
    trace(4,"addsol:\n");
    
    if (solbuf->cyclic) { /* ring buffer */
        if (solbuf->nmax<=1) return 0;
        solbuf->data[solbuf->end]=*sol;
        if (++solbuf->end>=solbuf->nmax) solbuf->end=0;
        if (solbuf->start==solbuf->end) {
            if (++solbuf->start>=solbuf->nmax) solbuf->start=0;
        }
        else solbuf->n++;
        
        return 1;
    }
    if (solbuf->n>=solbuf->nmax) {
        solbuf->nmax=solbuf->nmax==0?8192:solbuf->nmax*2;
        if (!(solbuf_data=(sol_t *)realloc(solbuf->data,sizeof(sol_t)*solbuf->nmax))) {
            trace(1,"addsol: memory allocation error\n");
            free(solbuf->data); solbuf->data=NULL; solbuf->n=solbuf->nmax=0;
            return 0;
        }
        solbuf->data=solbuf_data;
    }
    solbuf->data[solbuf->n++]=*sol;
    return 1;
}
/* get solution data from solution buffer --------------------------------------
* get solution data by index from solution buffer
* args   : solbuf_t *solbuf I  solution buffer
*          int    index     I  index of solution (0...)
* return : solution data pointer (NULL: no solution, out of range)
*-----------------------------------------------------------------------------*/
extern sol_t *getsol(solbuf_t *solbuf, int index)
{
    trace(4,"getsol: index=%d\n",index);
    
    if (index<0||solbuf->n<=index) return NULL;
    if ((index=solbuf->start+index)>=solbuf->nmax) {
        index-=solbuf->nmax;
    }
    return solbuf->data+index;
}
/* initialize solution buffer --------------------------------------------------
* initialize position solutions
* args   : solbuf_t *solbuf I  solution buffer
*          int    cyclic    I  solution data buffer type (0:linear,1:cyclic)
*          int    nmax      I  initial number of solution data
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern void initsolbuf(solbuf_t *solbuf, int cyclic, int nmax)
{
#if 0
    gtime_t time0={0};
#endif
    int i;
    
    trace(3,"initsolbuf: cyclic=%d nmax=%d\n",cyclic,nmax);
    
    solbuf->n=solbuf->nmax=solbuf->start=solbuf->end=solbuf->nb=0;
    solbuf->cyclic=cyclic;
#if 0
    solbuf->time=time0;
#endif
    solbuf->data=NULL;
    for (i=0;i<3;i++) {
        solbuf->rb[i]=0.0;
    }
    if (cyclic) {
        if (nmax<=2) nmax=2;
        if (!(solbuf->data=malloc(sizeof(sol_t)*nmax))) {
            trace(1,"initsolbuf: memory allocation error\n");
            return;
        }
        solbuf->nmax=nmax;
    }
}
/* free solution ---------------------------------------------------------------
* free memory for solution buffer
* args   : solbuf_t *solbuf I  solution buffer
* return : none
*-----------------------------------------------------------------------------*/
extern void freesolbuf(solbuf_t *solbuf)
{
    int i;
    
    trace(3,"freesolbuf: n=%d\n",solbuf->n);
    
    free(solbuf->data);
    solbuf->n=solbuf->nmax=solbuf->start=solbuf->end=solbuf->nb=0;
    solbuf->data=NULL;
    for (i=0;i<3;i++) {
        solbuf->rb[i]=0.0;
    }
}
extern void freesolstatbuf(solstatbuf_t *solstatbuf)
{
    trace(3,"freesolstatbuf: n=%d\n",solstatbuf->n);
    
    solstatbuf->n=solstatbuf->nmax=0;
    free(solstatbuf->data);
    solstatbuf->data=NULL;
}
/* compare solution status ---------------------------------------------------*/
static int cmpsolstat(const void *p1, const void *p2)
{
    solstat_t *q1=(solstat_t *)p1,*q2=(solstat_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-0.0?-1:(tt>0.0?1:0);
}
/* sort solution data --------------------------------------------------------*/
static int sort_solstat(solstatbuf_t *statbuf)
{
    solstat_t *statbuf_data;
    
    trace(4,"sort_solstat: n=%d\n",statbuf->n);
    
    if (statbuf->n<=0) return 0;
    
    if (!(statbuf_data=realloc(statbuf->data,sizeof(solstat_t)*statbuf->n))) {
        trace(1,"sort_solstat: memory allocation error\n");
        free(statbuf->data); statbuf->data=NULL; statbuf->n=statbuf->nmax=0;
        return 0;
    }
    statbuf->data=statbuf_data;
    qsort(statbuf->data,statbuf->n,sizeof(solstat_t),cmpsolstat);
    statbuf->nmax=statbuf->n;
    return 1;
}
/* decode solution status ----------------------------------------------------*/
static int decode_solstat(char *buff, solstat_t *stat)
{
    static const solstat_t stat0={{0}};
    double tow,az,el,resp,resc;
    int n,week,sat,frq,vsat,snr,fix,slip,lock,outc,slipc,rejc;
    char id[32]="",*p;
    
    trace(4,"decode_solstat: buff=%s\n",buff);
    
    if (strstr(buff,"$SAT")!=buff) return 0;
    
    for (p=buff;*p;p++) if (*p==',') *p=' ';
    
    n=sscanf(buff,"$SAT%d%lf%s%d%lf%lf%lf%lf%d%d%d%d%d%d%d%d",
             &week,&tow,id,&frq,&az,&el,&resp,&resc,&vsat,&snr,&fix,&slip,
             &lock,&outc,&slipc,&rejc);
    
    if (n<15) {
        trace(2,"invalid format of solution status: %s\n",buff);
        return 0;
    }
    if ((sat=satid2no(id))<=0) {
        trace(2,"invalid satellite in solution status: %s\n",id);
        return 0;
    }
    *stat=stat0;
    stat->time=gpst2time(week,tow);
    stat->sat  =(unsigned char)sat;
    stat->frq  =(unsigned char)frq;
    stat->az   =(float)(az*D2R);
    stat->el   =(float)(el*D2R);
    stat->resp =(float)resp;
    stat->resc =(float)resc;
    stat->flag =(unsigned char)((vsat<<5)+(slip<<3)+fix);
    stat->snr  =(unsigned char)(snr*4.0+0.5);
    stat->lock =(unsigned short)lock;
    stat->outc =(unsigned short)outc;
    stat->slipc=(unsigned short)slipc;
    stat->rejc =(unsigned short)rejc;
    return 1;
}
/* add solution status data --------------------------------------------------*/
static void addsolstat(solstatbuf_t *statbuf, const solstat_t *stat)
{
    solstat_t *statbuf_data;
    
    trace(4,"addsolstat:\n");
    
    if (statbuf->n>=statbuf->nmax) {
        statbuf->nmax=statbuf->nmax==0?8192:statbuf->nmax*2;
        if (!(statbuf_data=(solstat_t *)realloc(statbuf->data,sizeof(solstat_t)*
                                                statbuf->nmax))) {
            trace(1,"addsolstat: memory allocation error\n");
            free(statbuf->data); statbuf->data=NULL; statbuf->n=statbuf->nmax=0;
            return;
        }
        statbuf->data=statbuf_data;
    }
    statbuf->data[statbuf->n++]=*stat;
}
/* read solution status data -------------------------------------------------*/
static int readsolstatdata(FILE *fp, gtime_t ts, gtime_t te, double tint,
                           solstatbuf_t *statbuf)
{
    solstat_t stat={{0}};
    char buff[MAXSOLMSG+1];
    
    trace(3,"readsolstatdata:\n");
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        /* decode solution status */
        if (!decode_solstat(buff,&stat)) continue;
        
        /* add solution to solution buffer */
        if (screent(stat.time,ts,te,tint)) {
            addsolstat(statbuf,&stat);
        }
    }
    return statbuf->n>0;
}
/* read solution status --------------------------------------------------------
* read solution status from solution status files
* args   : char   *files[]  I  solution status files
*          int    nfile     I  number of files
*         (gtime_t ts)      I  start time (ts.time==0: from start)
*         (gtime_t te)      I  end time   (te.time==0: to end)
*         (double tint)     I  time interval (0: all)
*          solstatbuf_t *statbuf O  solution status buffer
* return : status (1:ok,0:no data or error)
*-----------------------------------------------------------------------------*/
extern int readsolstatt(char *files[], int nfile, gtime_t ts, gtime_t te,
                        double tint, solstatbuf_t *statbuf)
{
    FILE *fp;
    char path[1024],*p;
    int i;
    
    trace(3,"readsolstatt: nfile=%d\n",nfile);
    
    statbuf->n=statbuf->nmax=0;
    statbuf->data=NULL;
    
    for (i=0;i<nfile;i++) {
        if ((p=strrchr(files[i],'.'))&&!strcmp(p,".stat")) {
            sprintf(path,"%s",files[i]);
        }
        else {
            sprintf(path,"%s.stat",files[i]);
        }
        if (!(fp=fopen(path,"r"))) {
            trace(2,"readsolstatt: file open error %s\n",path);
            continue;
        }
        /* read solution status data */
        if (!readsolstatdata(fp,ts,te,tint,statbuf)) {
            trace(2,"readsolstatt: no solution in %s\n",path);
        }
        fclose(fp);
    }
    return sort_solstat(statbuf);
}
extern int readsolstat(char *files[], int nfile, solstatbuf_t *statbuf)
{
    gtime_t time={0};
    
    trace(3,"readsolstat: nfile=%d\n",nfile);
    
    return readsolstatt(files,nfile,time,time,0.0,statbuf);
}
/* output solution as the form of x/y/z-ecef ---------------------------------*/
static int outecef(unsigned char *buff, const char *s, const sol_t *sol,
                   const solopt_t *opt)
{
    const char *sep=opt2sep(opt);
    char *p=(char *)buff;
    
    trace(3,"outecef:\n");
    
    p+=sprintf(p,"%s%s%14.4f%s%14.4f%s%14.4f%s%3d%s%3d%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%6.2f%s%6.1f",
               s,sep,sol->rr[0],sep,sol->rr[1],sep,sol->rr[2],sep,sol->stat,sep,
               sol->ns,sep,SQRT(sol->qr[0]),sep,SQRT(sol->qr[1]),sep,SQRT(sol->qr[2]),
               sep,sqvar(sol->qr[3]),sep,sqvar(sol->qr[4]),sep,sqvar(sol->qr[5]),
               sep,sol->age,sep,sol->ratio);
    
    if (opt->outvel) { /* output velocity */
        p+=sprintf(p,"%s%10.5f%s%10.5f%s%10.5f%s%9.5f%s%8.5f%s%8.5f%s%8.5f%s%8.5f%s%8.5f",
                   sep,sol->rr[3],sep,sol->rr[4],sep,sol->rr[5],sep,
                   SQRT(sol->qv[0]),sep,SQRT(sol->qv[1]),sep,SQRT(sol->qv[2]),
                   sep,sqvar(sol->qv[3]),sep,sqvar(sol->qv[4]),sep,
                   sqvar(sol->qv[5]));
    }
    p+=sprintf(p,"\n");
    return p-(char *)buff;
}
/* output solution as the form of lat/lon/height -----------------------------*/
static int outpos(unsigned char *buff, const char *s, const sol_t *sol,
                  const solopt_t *opt)
{
    double pos[3],vel[3],dms1[3],dms2[3],P[9],Q[9];
    const char *sep=opt2sep(opt);
    char *p=(char *)buff;
    
    trace(3,"outpos  :\n");
    
    ecef2pos(sol->rr,pos);
    soltocov(sol,P);
    covenu(pos,P,Q);
    if (opt->height==1) { /* geodetic height */
        pos[2]-=geoidh(pos);
    }
    if (opt->degf) {
        deg2dms(pos[0]*R2D,dms1,5);
        deg2dms(pos[1]*R2D,dms2,5);
        p+=sprintf(p,"%s%s%4.0f%s%02.0f%s%08.5f%s%4.0f%s%02.0f%s%08.5f",s,sep,
                   dms1[0],sep,dms1[1],sep,dms1[2],sep,dms2[0],sep,dms2[1],sep,
                   dms2[2]);
    }
    else {
        p+=sprintf(p,"%s%s%14.9f%s%14.9f",s,sep,pos[0]*R2D,sep,pos[1]*R2D);
    }
    p+=sprintf(p,"%s%10.4f%s%3d%s%3d%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%6.2f%s%6.1f",
               sep,pos[2],sep,sol->stat,sep,sol->ns,sep,SQRT(Q[4]),sep,
               SQRT(Q[0]),sep,SQRT(Q[8]),sep,sqvar(Q[1]),sep,sqvar(Q[2]),
               sep,sqvar(Q[5]),sep,sol->age,sep,sol->ratio);
    
    if (opt->outvel) { /* output velocity */
        soltocov_vel(sol,P);
        ecef2enu(pos,sol->rr+3,vel);
        covenu(pos,P,Q);
        p+=sprintf(p,"%s%10.5f%s%10.5f%s%10.5f%s%9.5f%s%8.5f%s%8.5f%s%8.5f%s%8.5f%s%8.5f",
                   sep,vel[1],sep,vel[0],sep,vel[2],sep,SQRT(Q[4]),sep,
                   SQRT(Q[0]),sep,SQRT(Q[8]),sep,sqvar(Q[1]),sep,sqvar(Q[2]),
                   sep,sqvar(Q[5]));
    }
    p+=sprintf(p,"\n");
    return p-(char *)buff;
}
/* output solution as the form of e/n/u-baseline -----------------------------*/
static int outenu(unsigned char *buff, const char *s, const sol_t *sol,
                  const double *rb, const solopt_t *opt)
{
    double pos[3],rr[3],enu[3],P[9],Q[9];
    int i;
    const char *sep=opt2sep(opt);
    char *p=(char *)buff;
    
    trace(3,"outenu  :\n");
    
    for (i=0;i<3;i++) rr[i]=sol->rr[i]-rb[i];
    ecef2pos(rb,pos);
    soltocov(sol,P);
    covenu(pos,P,Q);
    ecef2enu(pos,rr,enu);
    p+=sprintf(p,"%s%s%14.4f%s%14.4f%s%14.4f%s%3d%s%3d%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%8.4f%s%6.2f%s%6.1f\n",
               s,sep,enu[0],sep,enu[1],sep,enu[2],sep,sol->stat,sep,sol->ns,sep,
               SQRT(Q[0]),sep,SQRT(Q[4]),sep,SQRT(Q[8]),sep,sqvar(Q[1]),
               sep,sqvar(Q[5]),sep,sqvar(Q[2]),sep,sol->age,sep,sol->ratio);
    return p-(char *)buff;
}
/* output solution in the form of nmea RMC sentence --------------------------*/
extern int outnmea_rmc(unsigned char *buff, const sol_t *sol)
{
    static double dirp=0.0;
    gtime_t time;
    double ep[6],pos[3],enuv[3],dms1[3],dms2[3],vel,dir,amag=0.0;
    char *p=(char *)buff,*q,sum,*emag="E";
    
    trace(3,"outnmea_rmc:\n");
    
    if (sol->stat<=SOLQ_NONE) {
        p+=sprintf(p,"$GPRMC,,,,,,,,,,,,");
        for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q;
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
        return p-(char *)buff;
    }
    time=gpst2utc(sol->time);
    if (time.sec>=0.995) {time.time++; time.sec=0.0;}
    time2epoch(time,ep);
    ecef2pos(sol->rr,pos);
    ecef2enu(pos,sol->rr+3,enuv);
    vel=norm(enuv,3);
    if (vel>=1.0) {
        dir=atan2(enuv[0],enuv[1])*R2D;
        if (dir<0.0) dir+=360.0;
        dirp=dir;
    }
    else {
        dir=dirp;
    }
    deg2dms(fabs(pos[0])*R2D,dms1,7);
    deg2dms(fabs(pos[1])*R2D,dms2,7);
    p+=sprintf(p,"$GPRMC,%02.0f%02.0f%05.2f,A,%02.0f%010.7f,%s,%03.0f%010.7f,%s,%4.2f,%4.2f,%02.0f%02.0f%02d,%.1f,%s,%s",
               ep[3],ep[4],ep[5],dms1[0],dms1[1]+dms1[2]/60.0,pos[0]>=0?"N":"S",
               dms2[0],dms2[1]+dms2[2]/60.0,pos[1]>=0?"E":"W",vel/KNOT2M,dir,
               ep[2],ep[1],(int)ep[0]%100,amag,emag,
               sol->stat==SOLQ_DGPS||sol->stat==SOLQ_FLOAT||sol->stat==SOLQ_FIX?"D":"A");
    for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q; /* check-sum */
    p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    return p-(char *)buff;
}
/* output solution in the form of nmea GGA sentence --------------------------*/
extern int outnmea_gga(unsigned char *buff, const sol_t *sol)
{
    gtime_t time;
    double h,ep[6],pos[3],dms1[3],dms2[3],dop=1.0;
    int solq;
    char *p=(char *)buff,*q,sum;
    
    trace(3,"outnmea_gga:\n");
    
    if (sol->stat<=SOLQ_NONE) {
        p+=sprintf(p,"$GPGGA,,,,,,,,,,,,,,");
        for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q;
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
        return p-(char *)buff;
    }
    for (solq=0;solq<8;solq++) if (solq_nmea[solq]==sol->stat) break;
    if (solq>=8) solq=0;
    time=gpst2utc(sol->time);
    if (time.sec>=0.995) {time.time++; time.sec=0.0;}
    time2epoch(time,ep);
    ecef2pos(sol->rr,pos);
    h=geoidh(pos);
    deg2dms(fabs(pos[0])*R2D,dms1,7);
    deg2dms(fabs(pos[1])*R2D,dms2,7);
    p+=sprintf(p,"$GPGGA,%02.0f%02.0f%05.2f,%02.0f%010.7f,%s,%03.0f%010.7f,%s,%d,%02d,%.1f,%.3f,M,%.3f,M,%.1f,",
               ep[3],ep[4],ep[5],dms1[0],dms1[1]+dms1[2]/60.0,pos[0]>=0?"N":"S",
               dms2[0],dms2[1]+dms2[2]/60.0,pos[1]>=0?"E":"W",solq,
               sol->ns,dop,pos[2]-h,h,sol->age);
    for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q; /* check-sum */
    p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    return p-(char *)buff;
}
/* output solution in the form of nmea GSA sentences -------------------------*/
extern int outnmea_gsa(unsigned char *buff, const sol_t *sol,
                       const ssat_t *ssat)
{
    double azel[MAXSAT*2],dop[4];
    int i,sat,sys,nsat,prn[MAXSAT];
    char *p=(char *)buff,*q,*s,sum;
    
    trace(3,"outnmea_gsa:\n");
    
    if (sol->stat<=SOLQ_NONE) {
        p+=sprintf(p,"$GPGSA,A,1,,,,,,,,,,,,,,,");
        for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q;
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
        return p-(char *)buff;
    }
    /* GPGSA: gps/sbas */
    for (sat=1,nsat=0;sat<=MAXSAT&&nsat<12;sat++) {
        if (!ssat[sat-1].vs||ssat[sat-1].azel[1]<=0.0) continue;
        sys=satsys(sat,prn+nsat);
        if (sys!=SYS_GPS&&sys!=SYS_SBS) continue;
        if (sys==SYS_SBS) prn[nsat]+=33-MINPRNSBS;
        for (i=0;i<2;i++) azel[i+nsat*2]=ssat[sat-1].azel[i];
        nsat++;
    }
    if (nsat>0) {
        s=p;
        p+=sprintf(p,"$GPGSA,A,%d",sol->stat<=0?1:3);
        for (i=0;i<12;i++) {
            if (i<nsat) p+=sprintf(p,",%02d",prn[i]);
            else        p+=sprintf(p,",");
        }
        dops(nsat,azel,0.0,dop);
        p+=sprintf(p,",%3.1f,%3.1f,%3.1f,1",dop[1],dop[2],dop[3]);
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    /* GLGSA: glonass */
    for (sat=1,nsat=0;sat<=MAXSAT&&nsat<12;sat++) {
        if (!ssat[sat-1].vs||ssat[sat-1].azel[1]<=0.0) continue;
        if (satsys(sat,prn+nsat)!=SYS_GLO) continue;
        for (i=0;i<2;i++) azel[i+nsat*2]=ssat[sat-1].azel[i];
        nsat++;
    }
    if (nsat>0) {
        s=p;
        p+=sprintf(p,"$GLGSA,A,%d",sol->stat<=0?1:3);
        for (i=0;i<12;i++) {
            if (i<nsat) p+=sprintf(p,",%02d",prn[i]+64);
            else        p+=sprintf(p,",");
        }
        dops(nsat,azel,0.0,dop);
        p+=sprintf(p,",%3.1f,%3.1f,%3.1f,2",dop[1],dop[2],dop[3]);
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    /* GAGSA: galileo */
    for (sat=1,nsat=0;sat<=MAXSAT&&nsat<12;sat++) {
        if (!ssat[sat-1].vs||ssat[sat-1].azel[1]<=0.0) continue;
        if (satsys(sat,prn+nsat)!=SYS_GAL) continue;
        for (i=0;i<2;i++) azel[i+nsat*2]=ssat[sat-1].azel[i];
        nsat++;
    }
    if (nsat>0) {
        s=p;
        p+=sprintf(p,"$GAGSA,A,%d",sol->stat<=0?1:3);
        for (i=0;i<12;i++) {
            if (i<nsat) p+=sprintf(p,",%02d",prn[i]);
            else        p+=sprintf(p,",");
        }
        dops(nsat,azel,0.0,dop);
        p+=sprintf(p,",%3.1f,%3.1f,%3.1f,3",dop[1],dop[2],dop[3]);
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    return p-(char *)buff;
}
/* output solution in the form of nmea GSV sentence --------------------------*/
extern int outnmea_gsv(unsigned char *buff, const sol_t *sol,
                       const ssat_t *ssat)
{
    double az,el,snr;
    int i,j,k,n,sat,prn,sys,nmsg,sats[MAXSAT];
    char *p=(char *)buff,*q,*s,sum;
    
    trace(3,"outnmea_gsv:\n");
    
    if (sol->stat<=SOLQ_NONE) {
        p+=sprintf(p,"$GPGSV,1,1,0,,,,,,,,,,,,,,,,");
        for (q=(char *)buff+1,sum=0;*q;q++) sum^=*q;
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
        return p-(char *)buff;
    }
    /* GPGSV: gps/sbas */
    for (sat=1,n=0;sat<MAXSAT&&n<12;sat++) {
        sys=satsys(sat,&prn);
        if (sys!=SYS_GPS&&sys!=SYS_SBS) continue;
        if (ssat[sat-1].vs&&ssat[sat-1].azel[1]>0.0) sats[n++]=sat;
    }
    nmsg=n<=0?0:(n-1)/4+1;
    
    for (i=k=0;i<nmsg;i++) {
        s=p;
        p+=sprintf(p,"$GPGSV,%d,%d,%02d",nmsg,i+1,n);
        
        for (j=0;j<4;j++,k++) {
            if (k<n) {
                if (satsys(sats[k],&prn)==SYS_SBS) prn+=33-MINPRNSBS;
                az =ssat[sats[k]-1].azel[0]*R2D; if (az<0.0) az+=360.0;
                el =ssat[sats[k]-1].azel[1]*R2D;
                snr=ssat[sats[k]-1].snr[0]*0.25;
                p+=sprintf(p,",%02d,%02.0f,%03.0f,%02.0f",prn,el,az,snr);
            }
            else p+=sprintf(p,",,,,");
        }
        p+=sprintf(p,",1"); /* L1C/A */
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    /* GLGSV: glonass */
    for (sat=1,n=0;sat<MAXSAT&&n<12;sat++) {
        if (satsys(sat,&prn)!=SYS_GLO) continue;
        if (ssat[sat-1].vs&&ssat[sat-1].azel[1]>0.0) sats[n++]=sat;
    }
    nmsg=n<=0?0:(n-1)/4+1;
    
    for (i=k=0;i<nmsg;i++) {
        s=p;
        p+=sprintf(p,"$GLGSV,%d,%d,%02d",nmsg,i+1,n);
        
        for (j=0;j<4;j++,k++) {
            if (k<n) {
                satsys(sats[k],&prn); prn+=64; /* 65-99 */
                az =ssat[sats[k]-1].azel[0]*R2D; if (az<0.0) az+=360.0;
                el =ssat[sats[k]-1].azel[1]*R2D;
                snr=ssat[sats[k]-1].snr[0]*0.25;
                p+=sprintf(p,",%02d,%02.0f,%03.0f,%02.0f",prn,el,az,snr);
            }
            else p+=sprintf(p,",,,,");
        }
        p+=sprintf(p,",1"); /* L1C/A */
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    /* GAGSV: galileo */
    for (sat=1,n=0;sat<MAXSAT&&n<12;sat++) {
        if (satsys(sat,&prn)!=SYS_GAL) continue;
        if (ssat[sat-1].vs&&ssat[sat-1].azel[1]>0.0) sats[n++]=sat;
    }
    nmsg=n<=0?0:(n-1)/4+1;
    
    for (i=k=0;i<nmsg;i++) {
        s=p;
        p+=sprintf(p,"$GAGSV,%d,%d,%02d",nmsg,i+1,n);
        
        for (j=0;j<4;j++,k++) {
            if (k<n) {
                satsys(sats[k],&prn); /* 1-36 */
                az =ssat[sats[k]-1].azel[0]*R2D; if (az<0.0) az+=360.0;
                el =ssat[sats[k]-1].azel[1]*R2D;
                snr=ssat[sats[k]-1].snr[0]*0.25;
                p+=sprintf(p,",%02d,%02.0f,%03.0f,%02.0f",prn,el,az,snr);
            }
            else p+=sprintf(p,",,,,");
        }
        p+=sprintf(p,",7"); /* L1BC */
        for (q=s+1,sum=0;*q;q++) sum^=*q; /* check-sum */
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    }
    return p-(char *)buff;
}
/* output processing options ---------------------------------------------------
* output processing options to buffer
* args   : unsigned char *buff IO output buffer
*          prcopt_t *opt    I   processign options
* return : number of output bytes
*-----------------------------------------------------------------------------*/
extern int outprcopts(unsigned char *buff, const prcopt_t *opt)
{
    const int sys[]={SYS_GPS,SYS_GLO,SYS_GAL,SYS_QZS,SYS_CMP,SYS_IRN,SYS_SBS,0};
    const char *s1[]={"single","dgps","kinematic","static","moving-base","fixed",
                 "ppp-kinematic","ppp-static","ppp-fixed",""};
    const char *s2[]={"L1","L1+L2","L1+L2+L5","L1+L2+L5+L6","L1+L2+L5+L6+L7",
                      "L1+L2+L5+L6+L7+L8",""};
    const char *s3[]={"forward","backward","combined"};
    const char *s4[]={"off","broadcast","sbas","iono-free","estimation",
                      "ionex tec","qzs","lex","vtec_sf","vtec_ef","gtec",""};
    const char *s5[]={"off","saastamoinen","sbas","est ztd","est ztd+grad",""};
    const char *s6[]={"broadcast","precise","broadcast+sbas","broadcast+ssr apc",
                      "broadcast+ssr com","qzss lex",""};
    const char *s7[]={"gps","glonass","galileo","qzss","beidou","irnss","sbas",""};
    const char *s8[]={"off","continuous","instantaneous","fix and hold",""};
    const char *s9[]={"off","on","auto calib","external calib",""};
    int i;
    char *p=(char *)buff;
    
    trace(3,"outprcopts:\n");
    
    p+=sprintf(p,"%s pos mode  : %s\n",COMMENTH,s1[opt->mode]);
    
    if (PMODE_DGPS<=opt->mode&&opt->mode<=PMODE_FIXED) {
        p+=sprintf(p,"%s freqs     : %s\n",COMMENTH,s2[opt->nf-1]);
    }
    if (opt->mode>PMODE_SINGLE) {
        p+=sprintf(p,"%s solution  : %s\n",COMMENTH,s3[opt->soltype]);
    }
    p+=sprintf(p,"%s elev mask : %.1f deg\n",COMMENTH,opt->elmin*R2D);
    if (opt->mode>PMODE_SINGLE) {
        p+=sprintf(p,"%s dynamics  : %s\n",COMMENTH,opt->dynamics?"on":"off");
        p+=sprintf(p,"%s tidecorr  : %s\n",COMMENTH,opt->tidecorr?"on":"off");
    }
    if (opt->mode<=PMODE_FIXED) {
        p+=sprintf(p,"%s ionos opt : %s\n",COMMENTH,s4[opt->ionoopt]);
    }
    p+=sprintf(p,"%s tropo opt : %s\n",COMMENTH,s5[opt->tropopt]);
    p+=sprintf(p,"%s ephemeris : %s\n",COMMENTH,s6[opt->sateph]);
    if (opt->navsys!=SYS_GPS) {
        p+=sprintf(p,"%s navi sys  :",COMMENTH);
        for (i=0;sys[i];i++) {
            if (opt->navsys&sys[i]) p+=sprintf(p," %s",s7[i]);
        }
        p+=sprintf(p,"\n");
    }
    if (PMODE_KINEMA<=opt->mode&&opt->mode<=PMODE_FIXED) {
        p+=sprintf(p,"%s amb res   : %s\n",COMMENTH,s8[opt->modear]);
        if (opt->navsys&SYS_GLO) {
            p+=sprintf(p,"%s amb glo   : %s\n",COMMENTH,s9[opt->glomodear]);
        }
        if (opt->thresar[0]>0.0) {
            p+=sprintf(p,"%s val thres : %.1f\n",COMMENTH,opt->thresar[0]);
        }
    }
    if (opt->mode==PMODE_MOVEB&&opt->baseline[0]>0.0) {
        p+=sprintf(p,"%s baseline  : %.4f %.4f m\n",COMMENTH,
                   opt->baseline[0],opt->baseline[1]);
    }
    for (i=0;i<2;i++) {
        if (opt->mode==PMODE_SINGLE||(i>=1&&opt->mode>PMODE_FIXED)) continue;
        p+=sprintf(p,"%s antenna%d  : %-21s (%7.4f %7.4f %7.4f)\n",COMMENTH,
                   i+1,opt->anttype[i],opt->antdel[i][0],opt->antdel[i][1],
                   opt->antdel[i][2]);
    }
    return p-(char *)buff;
}
/* output solution header ------------------------------------------------------
* output solution header to buffer
* args   : unsigned char *buff IO output buffer
*          solopt_t *opt    I   solution options
* return : number of output bytes
*-----------------------------------------------------------------------------*/
extern int outsolheads(unsigned char *buff, const solopt_t *opt)
{
    const char *s1[]={"WGS84","Tokyo"},*s2[]={"ellipsoidal","geodetic"};
    const char *s3[]={"GPST","UTC ","JST "},*sep=opt2sep(opt);
    char *p=(char *)buff;
    int timeu=opt->timeu<0?0:(opt->timeu>20?20:opt->timeu);
    
    trace(3,"outsolheads:\n");
    
    if (opt->posf==SOLF_NMEA||opt->posf==SOLF_STAT||opt->posf==SOLF_GSIF) {
        return 0;
    }
    if (opt->outhead) {
        p+=sprintf(p,"%s (",COMMENTH);
        if      (opt->posf==SOLF_XYZ) p+=sprintf(p,"x/y/z-ecef=WGS84");
        else if (opt->posf==SOLF_ENU) p+=sprintf(p,"e/n/u-baseline=WGS84");
        else p+=sprintf(p,"lat/lon/height=%s/%s",s1[opt->datum],s2[opt->height]);
        p+=sprintf(p,",Q=1:fix,2:float,3:sbas,4:dgps,5:single,6:ppp,ns=# of satellites)\n");
    }
    p+=sprintf(p,"%s  %-*s%s",COMMENTH,(opt->timef?16:8)+timeu+1,s3[opt->times],sep);
    
    if (opt->posf==SOLF_LLH) { /* lat/lon/hgt */
        if (opt->degf) {
            p+=sprintf(p,"%16s%s%16s%s%10s%s%3s%s%3s%s%8s%s%8s%s%8s%s%8s%s%8s%s%8s%s%6s%s%6s",
                       "latitude(d'\")",sep,"longitude(d'\")",sep,"height(m)",sep,
                       "Q",sep,"ns",sep,"sdn(m)",sep,"sde(m)",sep,"sdu(m)",sep,
                       "sdne(m)",sep,"sdeu(m)",sep,"sdue(m)",sep,"age(s)",sep,"ratio");
        }
        else {
            p+=sprintf(p,"%14s%s%14s%s%10s%s%3s%s%3s%s%8s%s%8s%s%8s%s%8s%s%8s%s%8s%s%6s%s%6s",
                       "latitude(deg)",sep,"longitude(deg)",sep,"height(m)",sep,
                       "Q",sep,"ns",sep,"sdn(m)",sep,"sde(m)",sep,"sdu(m)",sep,
                       "sdne(m)",sep,"sdeu(m)",sep,"sdun(m)",sep,"age(s)",sep,"ratio");
        }
        if (opt->outvel) {
            p+=sprintf(p,"%s%10s%s%10s%s%10s%s%9s%s%8s%s%8s%s%8s%s%8s%s%8s",
                       sep,"vn(m/s)",sep,"ve(m/s)",sep,"vu(m/s)",sep,"sdvn",sep,
                       "sdve",sep,"sdvu",sep,"sdvne",sep,"sdveu",sep,"sdvun");
        }
    }
    else if (opt->posf==SOLF_XYZ) { /* x/y/z-ecef */
        p+=sprintf(p,"%14s%s%14s%s%14s%s%3s%s%3s%s%8s%s%8s%s%8s%s%8s%s%8s%s%8s%s%6s%s%6s",
                   "x-ecef(m)",sep,"y-ecef(m)",sep,"z-ecef(m)",sep,"Q",sep,"ns",sep,
                   "sdx(m)",sep,"sdy(m)",sep,"sdz(m)",sep,"sdxy(m)",sep,
                   "sdyz(m)",sep,"sdzx(m)",sep,"age(s)",sep,"ratio");
        
        if (opt->outvel) {
            p+=sprintf(p,"%s%10s%s%10s%s%10s%s%9s%s%8s%s%8s%s%8s%s%8s%s%8s",
                       sep,"vx(m/s)",sep,"vy(m/s)",sep,"vz(m/s)",sep,"sdvx",sep,
                       "sdvy",sep,"sdvz",sep,"sdvxy",sep,"sdvyz",sep,"sdvzx");
        }
    }
    else if (opt->posf==SOLF_ENU) { /* e/n/u-baseline */
        p+=sprintf(p,"%14s%s%14s%s%14s%s%3s%s%3s%s%8s%s%8s%s%8s%s%8s%s%8s%s%8s%s%6s%s%6s",
                   "e-baseline(m)",sep,"n-baseline(m)",sep,"u-baseline(m)",sep,
                   "Q",sep,"ns",sep,"sde(m)",sep,"sdn(m)",sep,"sdu(m)",sep,
                   "sden(m)",sep,"sdnu(m)",sep,"sdue(m)",sep,"age(s)",sep,"ratio");
    }
    p+=sprintf(p,"\n");
    return p-(char *)buff;
}
/* std-dev of soltuion -------------------------------------------------------*/
static double sol_std(const sol_t *sol)
{
    /* approximate as max std-dev of 3-axis std-devs */
    if (sol->qr[0]>sol->qr[1]&&sol->qr[0]>sol->qr[2]) return SQRT(sol->qr[0]);
    if (sol->qr[1]>sol->qr[2]) return SQRT(sol->qr[1]);
    return SQRT(sol->qr[2]);
}
/* output solution body --------------------------------------------------------
* output solution body to buffer
* args   : unsigned char *buff IO output buffer
*          sol_t  *sol      I   solution
*          double *rb       I   base station position {x,y,z} (ecef) (m)
*          solopt_t *opt    I   solution options
* return : number of output bytes
*-----------------------------------------------------------------------------*/
extern int outsols(unsigned char *buff, const sol_t *sol, const double *rb,
                   const solopt_t *opt)
{
    gtime_t time,ts={0};
    double gpst;
    int week,timeu;
    const char *sep=opt2sep(opt);
    char s[64];
    unsigned char *p=buff;
    
    trace(3,"outsols :\n");
    
    /* suppress output if std is over opt->maxsolstd */
    if (opt->maxsolstd>0.0&&sol_std(sol)>opt->maxsolstd) {
        return 0;
    }
    if (opt->posf==SOLF_NMEA) {
        if (opt->nmeaintv[0]<0.0) return 0;
        if (!screent(sol->time,ts,ts,opt->nmeaintv[0])) return 0;
    }
    if (sol->stat<=SOLQ_NONE||(opt->posf==SOLF_ENU&&norm(rb,3)<=0.0)) {
        return 0;
    }
    timeu=opt->timeu<0?0:(opt->timeu>20?20:opt->timeu);
    
    time=sol->time;
    if (opt->times>=TIMES_UTC) time=gpst2utc(time);
    if (opt->times==TIMES_JST) time=timeadd(time,9*3600.0);
    
    if (opt->timef) time2str(time,s,timeu);
    else {
        gpst=time2gpst(time,&week);
        if (86400*7-gpst<0.5/pow(10.0,timeu)) {
            week++;
            gpst=0.0;
        }
        sprintf(s,"%4d%s%*.*f",week,sep,6+(timeu<=0?0:timeu+1),timeu,gpst);
    }
    switch (opt->posf) {
        case SOLF_LLH:  p+=outpos (p,s,sol,opt);   break;
        case SOLF_XYZ:  p+=outecef(p,s,sol,opt);   break;
        case SOLF_ENU:  p+=outenu(p,s,sol,rb,opt); break;
        case SOLF_NMEA: p+=outnmea_rmc(p,sol);
                        p+=outnmea_gga(p,sol); break;
    }
    return p-buff;
}
/* output solution extended ----------------------------------------------------
* output solution exteneded infomation
* args   : unsigned char *buff IO output buffer
*          sol_t  *sol      I   solution
*          ssat_t *ssat     I   satellite status
*          solopt_t *opt    I   solution options
* return : number of output bytes
* notes  : only support nmea
*-----------------------------------------------------------------------------*/
extern int outsolexs(unsigned char *buff, const sol_t *sol, const ssat_t *ssat,
                     const solopt_t *opt)
{
    gtime_t ts={0};
    unsigned char *p=buff;
    
    trace(3,"outsolexs:\n");
    
    /* suppress output if std is over opt->maxsolstd */
    if (opt->maxsolstd>0.0&&sol_std(sol)>opt->maxsolstd) {
        return 0;
    }
    if (opt->posf==SOLF_NMEA) {
        if (opt->nmeaintv[1]<0.0) return 0;
        if (!screent(sol->time,ts,ts,opt->nmeaintv[1])) return 0;
    }
    if (opt->posf==SOLF_NMEA) {
        p+=outnmea_gsa(p,sol,ssat);
        p+=outnmea_gsv(p,sol,ssat);
    }
    return p-buff;
}
/* output processing option ----------------------------------------------------
* output processing option to file
* args   : FILE   *fp       I   output file pointer
*          prcopt_t *opt    I   processing options
* return : none
*-----------------------------------------------------------------------------*/
extern void outprcopt(FILE *fp, const prcopt_t *opt)
{
    unsigned char buff[MAXSOLMSG+1];
    int n;
    
    trace(3,"outprcopt:\n");
    
    if ((n=outprcopts(buff,opt))>0) {
        fwrite(buff,n,1,fp);
    }
}
/* output solution header ------------------------------------------------------
* output solution heade to file
* args   : FILE   *fp       I   output file pointer
*          solopt_t *opt    I   solution options
* return : none
*-----------------------------------------------------------------------------*/
extern void outsolhead(FILE *fp, const solopt_t *opt)
{
    unsigned char buff[MAXSOLMSG+1];
    int n;
    
    trace(3,"outsolhead:\n");
    
    if ((n=outsolheads(buff,opt))>0) {
        fwrite(buff,n,1,fp);
    }
}
/* output solution body --------------------------------------------------------
* output solution body to file
* args   : FILE   *fp       I   output file pointer
*          sol_t  *sol      I   solution
*          double *rb       I   base station position {x,y,z} (ecef) (m)
*          solopt_t *opt    I   solution options
* return : none
*-----------------------------------------------------------------------------*/
extern void outsol(FILE *fp, const sol_t *sol, const double *rb,
                   const solopt_t *opt)
{
    unsigned char buff[MAXSOLMSG+1];
    int n;
    
    trace(3,"outsol  :\n");
    
    if ((n=outsols(buff,sol,rb,opt))>0) {
        fwrite(buff,n,1,fp);
    }
}
/* output solution extended ----------------------------------------------------
* output solution exteneded infomation to file
* args   : FILE   *fp       I   output file pointer
*          sol_t  *sol      I   solution
*          ssat_t *ssat     I   satellite status
*          solopt_t *opt    I   solution options
* return : output size (bytes)
* notes  : only support nmea
*-----------------------------------------------------------------------------*/
extern void outsolex(FILE *fp, const sol_t *sol, const ssat_t *ssat,
                     const solopt_t *opt)
{
    unsigned char buff[MAXSOLMSG+1];
    int n;
    
    trace(3,"outsolex:\n");
    
    if ((n=outsolexs(buff,sol,ssat,opt))>0) {
        fwrite(buff,n,1,fp);
    }
}
