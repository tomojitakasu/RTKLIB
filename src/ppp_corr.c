/*------------------------------------------------------------------------------
* ppp_corr.c : ppp corrections functions
*
*          Copyright (C) 2015 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2015/05/20 1.0 new
*           2015/06/11 1.1 station weighting only by distance
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define MAX_DIST_STEC  50.0      /* max distance for stec correction (km) */
#define MAX_DIST_TROP  100.0     /* max distance for trop correction (km) */
#define PRN_ZTD        1E-4      /* process noise of ztd (m/sqrt(s)) */
#define PRN_GRAD       1E-5      /* process noise of gradient (m/sqrt(s)) */
#define STD_ZTD_DIST   1E-4      /* distance dependent factor ztd (m/km) */
#define STD_GRAD_DIST  1E-5      /* distance dependent factor gradient (m/km) */
#define STD_VTEC_DIST  1E-3      /* distance dependent factor vtec (m/km) */

#define SQR(x)      ((x)*(x))
#define MAX(x,y)    ((x)>(y)?(x):(y))

/* add station position to ppp corrections -----------------------------------*/
static int add_pos(pppcorr_t *corr, const char *sta, const double *rr)
{
    int i;
    
    for (i=0;i<corr->nsta;i++) if (!strcmp(corr->stas[i],sta)) break;
    if (i>=MAXSTA) return 0;
    if (i>=corr->nsta) {
        strcpy(corr->stas[corr->nsta++],sta);
    }
    matcpy(corr->rr[i],rr,3,1);
    return 1;
}
/* add stec data to ppp corrections ------------------------------------------*/
static int add_stec(pppcorr_t *corr, gtime_t time, const char *sat,
                    const char *sta, double ion, double std, const double *azel,
                    int flag)
{
    stec_t *corr_stec,stec={{0}};
    int i,j;
    
    for (i=0;i<corr->nsta;i++) if (!strcmp(corr->stas[i],sta)) break;
    if (i>=MAXSTA) return 0;
    if (i>=corr->nsta) {
        for (j=0;j<3;j++) corr->rr[corr->nsta][j]=0.0;
        strcpy(corr->stas[corr->nsta++],sta);
    }
    if (corr->ns[i]>=corr->nsmax[i]) {
        corr->nsmax[i]=corr->nsmax[i]<=0?256:corr->nsmax[i]*2;
        corr_stec=(stec_t *)realloc(corr->stec[i],sizeof(stec_t)*corr->nsmax[i]);
        if (!corr_stec) {
            free(corr->stec[i]); corr->stec[i]=NULL;
            corr->ns[i]=corr->nsmax[i]=0;
            return 0;
        }
        corr->stec[i]=corr_stec;
    }
    stec.time=time;
    stec.sat=(unsigned char)satid2no(sat);
    stec.ion=ion;
    stec.std=(float)std;
    stec.azel[0]=(float)(azel[0]*D2R);
    stec.azel[1]=(float)(azel[1]*D2R);
    stec.flag=(unsigned char)flag;
    corr->stec[i][corr->ns[i]++]=stec;
    return 1;
}
/* add tropos ztd to ppp corrections -----------------------------------------*/
static int add_trop_ztd(pppcorr_t *corr, gtime_t time, const char *sta,
                        double ztd, double std)
{
    trop_t *corr_trop,trop={{0}};
    int i,j;
    
    for (i=0;i<corr->nsta;i++) if (!strcmp(corr->stas[i],sta)) break;
    
    if (i>=MAXSTA) return 0;
    if (i>=corr->nsta) {
        for (j=0;j<3;j++) corr->rr[corr->nsta][j]=0.0;
        strcpy(corr->stas[corr->nsta++],sta);
    }
    if (corr->nt[i]>=corr->ntmax[i]) {
        corr->ntmax[i]=corr->ntmax[i]<=0?256:corr->ntmax[i]*2;
        corr_trop=(trop_t *)realloc(corr->trop[i],sizeof(trop_t)*corr->ntmax[i]);
        if (!corr_trop) {
            free(corr->trop[i]); corr->trop[i]=NULL;
            corr->nt[i]=corr->ntmax[i]=0;
            return 0;
        }
        corr->trop[i]=corr_trop;
    }
    trop.time=time;
    trop.trp[0]=ztd;
    trop.std[0]=(float)std;
    corr->trop[i][corr->nt[i]++]=trop;
    return 1;
}
/* add tropos gradient to ppp corrections ------------------------------------*/
static int add_trop_grad(pppcorr_t *corr, gtime_t time, const char *sta,
                         const double *grad, const double *std)
{
    trop_t *trop;
    int i;
    
    for (i=0;i<corr->nsta;i++) if (!strcmp(corr->stas[i],sta)) break;
    
    if (i>=corr->nsta||corr->nt[i]<=0) return 1;
    trop=corr->trop[i]+corr->nt[i]-1;
    if (timediff(time,trop->time)!=0.0) return 1;
    trop->trp[1]=grad[0];
    trop->trp[2]=grad[1];
    trop->std[1]=(float)std[0];
    trop->std[2]=(float)std[1];
    return 1;
}
/* compare stec data ---------------------------------------------------------*/
static int cmp_stec(const void *p1, const void *p2)
{
    stec_t *q1=(stec_t *)p1,*q2=(stec_t *)p2;
    double tt=timediff(q1->time,q2->time);
    if (fabs(tt)>1E-3) return tt<0.0?-1:1;
    return q1->sat-q2->sat;
}
/* compare trop data ---------------------------------------------------------*/
static int cmp_trop(const void *p1, const void *p2)
{
    trop_t *q1=(trop_t *)p1,*q2=(trop_t *)p2;
    double tt=timediff(q1->time,q2->time);
    if (fabs(tt)>1E-3) return tt<0.0?-1:1;
    return 0;
}
/* read solution status file -------------------------------------------------*/
extern int read_solstat(pppcorr_t *corr, const char *file)
{
    FILE *fp;
    gtime_t time;
    double tow,rr[3],azel[2],ion,ztd,grad[2],std[2];
    char buff[1024],sat[64],sta[64]="",sys,*p;
    int i,week,stat,prn,rcv;
    
    trace(2,"read_solstat: file=%s\n",file);
    fprintf(stderr,"read_solstat: file=%s\n",file);
    
    /* station name as first 4 character of file */
    if ((p=strrchr(file,FILEPATHSEP))) strncpy(sta,p+1,4);
    else strncpy(sta,file,4);
    for (i=0;i<4&&sta[i];i++) sta[i]=toupper(sta[i]);
    
    if (!(fp=fopen(file,"r"))) {
        trace(2,"solution status file read error: %s\n",file);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        
        /* station position */
        if (sscanf(buff,"$POS,%d,%lf,%d,%lf,%lf,%lf",&week,&tow,&stat,rr,rr+1,
                   rr+2)>=6) {
            if (!add_pos(corr,sta,rr)) break;
        }
        /* ionosphere */
        else if (sscanf(buff,"$ION,%d,%lf,%d,%c%d,%lf,%lf,%lf,%lf",&week,&tow,
                        &stat,&sys,&prn,azel,azel+1,&ion,std)>=9) {
            time=gpst2time(week,tow);
            sprintf(sat,"%c%02d",sys,prn);
            if (!add_stec(corr,time,sat,sta,ion,std[0],azel,stat==1)) break;
        }
        /* troposphere ztd */
        else if (sscanf(buff,"$TROP,%d,%lf,%d,%d,%lf,%lf",&week,&tow,&stat,&rcv,
                        &ztd,std)>=6) {
            time=gpst2time(week,tow);
            if (!add_trop_ztd(corr,time,sta,ztd,std[0])) break;
        }
        /* troposphere gradient */
        else if (sscanf(buff,"$TRPG,%d,%lf,%d,%d,%lf,%lf,%lf,%lf",&week,&tow,
                        &stat,&rcv,grad,grad+1,std,std+1)>=6) {
            time=gpst2time(week,tow);
            if (!add_trop_grad(corr,time,sta,grad,std)) break;
        }
    }
    fclose(fp);
    return 1;
}
/* read stec file ------------------------------------------------------------*/
static int read_stec(pppcorr_t *corr, const char *file)
{
    FILE *fp;
    gtime_t time;
    double ep[6],rr[3],ion[2],std[2],azel[2];
    char buff[1024],sat[64],sta[64],*p;
    int sec=0,flag;
    
    trace(2,"read_stec: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) {
        trace(2,"stec parameter file read error: %s\n",file);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        if (strstr(buff,"STATION POSITION")) sec=1;
        if ((p=strchr(buff,'#'))) *p='\0';
        if (sec) {
            if (sscanf(buff,"%63s %lf %lf %lf",sta,rr,rr+1,rr+2)<4) continue;
            if (!add_pos(corr,sta,rr)) break;
        }
        else {
            if (sscanf(buff,"%lf/%lf/%lf %lf:%lf:%lf %63s %63s %lf %lf %lf %lf %lf %lf %d",
                       ep,ep+1,ep+2,ep+3,ep+4,ep+5,sat,sta,ion,ion+1,std,std+1,
                       azel,azel+1,&flag)<15) continue;
            time=epoch2time(ep);
            if (!add_stec(corr,time,sat,sta,ion[0],std[0],azel,flag)) break;
        }
    }
    fclose(fp);
    return 1;
}
/* read sinex tropos file ----------------------------------------------------*/
static int read_snxtrop(pppcorr_t *corr, const char *file)
{
    FILE *fp;
    gtime_t time;
    double ep[6]={0,1,1},rr[3],tod,trp[3]={0},std[3]={0};
    char buff[1024],sta[64];
    int i,sec=0,year,doy;
    
    trace(2,"read_snxtrop: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) {
        trace(2,"sinex trop file read error: %s\n",file);
        return 0;
    }
    if (!fgets(buff,sizeof(buff),fp)||strstr(buff,"%=TRD")!=buff) {
        trace(2,"sinex trop file read error: %s\n",file);
        fclose(fp);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        if (buff[0]=='-') sec=0;
        else if (strstr(buff,"+TROP/STA_COORDINATES")==buff) sec=1;
        else if (strstr(buff,"+TROP/SOLUTION"       )==buff) sec=2;
        if (sec==1) {
            if (sscanf(buff,"%63s %*s %*s %*s %lf %lf %lf",sta,rr,rr+1,
                       rr+2)<4) continue;
            if (!add_pos(corr,sta,rr)) break;
        }
        else if (sec==2) {
            if (sscanf(buff,"%63s %d:%d:%lf %lf %lf %lf %lf %lf %lf",sta,&year,
                       &doy,&tod,trp,std,trp+1,std+1,trp+2,std+2)<6) continue;
            ep[0]=2000.0+year;
            time=timeadd(epoch2time(ep),(doy-1)*86400.0+tod);
            for (i=0;i<3;i++) {
                trp[i]/=1E3;
                std[i]/=1E3;
            }
            if (!add_trop_ztd(corr,time,sta,trp[0],std[0])) break;
            if (!add_trop_grad(corr,time,sta,trp+1,std+1)) break;
        }
    }
    fclose(fp);
    return 1;
}
/* read ppp corrections --------------------------------------------------------
* read ppp correction data from external file
* args   : pppcorr_t *corr  IO  ppp correction data
*          char   *file     I   file
* return : status (1:ok,0:error)
* notes  : file types are recognized by file extenstions as follows.
*            .stat,.STAT : solution status file by rtklib
*            .stec,.STEC : stec parameters file by mgest
*            others      : sinex troposphere file
*          read data are added to ppp correction data.
*          To clear data, call pppcorr_free()
*-----------------------------------------------------------------------------*/
extern int pppcorr_read(pppcorr_t *corr, const char *file)
{
    char *efiles[MAXEXFILE]={0},*ext;
    int i,n;
    
    trace(2,"pppcorr_read: file=%s\n",file);
    
    for (i=0;i<MAXEXFILE;i++) {
        if (!(efiles[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(efiles[i]);
            return 0;
        }
    }
    n=expath(file,efiles,MAXEXFILE);
    
    for (i=0;i<n;i++) {
        if (!(ext=strrchr(efiles[i],'.'))) continue;
        
        if (!strcmp(ext,".stat")||!strcmp(ext,".STAT")) {
            read_solstat(corr,efiles[i]);
        }
        else if (!strcmp(ext,".stec")||!strcmp(ext,".STEC")) {
            read_stec(corr,efiles[i]);
        }
        else {
            read_snxtrop(corr,efiles[i]);
        }
    }
    for (i=0;i<MAXEXFILE;i++) free(efiles[i]);
    
    for (i=0;i<corr->nsta;i++) {
        if (corr->ns[i]>1) {
            qsort(corr->stec[i],corr->ns[i],sizeof(stec_t),cmp_stec);
        }
        if (corr->nt[i]>1) {
            qsort(corr->trop[i],corr->nt[i],sizeof(trop_t),cmp_trop);
        }
    }
    for (i=0;i<corr->nsta;i++) {
        trace(2,"%-8s %6d %6d\n",corr->stas[i],corr->ns[i],corr->nt[i]);
    }
    return 1;
}
/* free ppp corrections --------------------------------------------------------
* free and clear ppp correction data
* args   : pppcorr_t *corr  IO  ppp correction data
* return : none
*-----------------------------------------------------------------------------*/
extern void pppcorr_free(pppcorr_t *corr)
{
    int i;
    
    for (i=0;i<corr->nsta;i++) {
        free(corr->stec[i]); corr->stec[i]=NULL;
        free(corr->trop[i]); corr->trop[i]=NULL;
        corr->ns[i]=corr->nsmax[i]=corr->nt[i]=corr->ntmax[i]=0;
    }
    corr->nsta=0;
}
/* get neighbour stations ----------------------------------------------------*/
static int get_sta(const pppcorr_t *corr, const double *pos, double max_dist,
                   int *sta, double *dist, double gpos[][3])
{
     double r[3],rr[3];
     int i,j,n=0;
     
     pos2ecef(pos,rr);
     
     for (i=0;i<corr->nsta;i++) {
         for (j=0;j<3;j++) r[j]=corr->rr[i][j]-rr[j];
         if ((dist[n]=MAX(norm(r,3)/1E3,0.1))>max_dist) continue;
         ecef2pos(corr->rr[i],gpos[n]);
         sta[n++]=i;
     }
     return n;
}
/* get trop correction for a station -----------------------------------------*/
static void get_trop_sta(gtime_t time, const trop_t *trop, int n, double *trp,
                         double *std)
{
    double a,tt;
    int i,j;
    
    if (n<=0) return;
    
    for (i=0;i<n;i++) if (timediff(trop[i].time,time)>0.0) break;
    if (i==0) {
        tt=timediff(time,trop[0].time);
        for (j=0;j<3;j++) {
            trp[j]=trop[0].trp[j];
            std[j]=sqrt(SQR(trop[0].std[j])+SQR(tt*(j==0?PRN_ZTD:PRN_GRAD)));
        }
    }
    else if (i>=n) {
        tt=timediff(time,trop[n-1].time);
        for (j=0;j<3;j++) {
            trp[j]=trop[n-1].trp[j];
            std[j]=sqrt(SQR(trop[n-1].std[j])+SQR(tt*(j==0?PRN_ZTD:PRN_GRAD)));
        }
    }
    else {
        a=timediff(time,trop[i-1].time)/timediff(trop[i].time,trop[i-1].time);
        for (j=0;j<3;j++) {
            trp[j]=(1.0-a)*trop[i-1].trp[j]+a*trop[i].trp[j];
            std[j]=sqrt((1.0-a)*SQR(trop[i-1].std[j])+a*SQR(trop[i].std[j]));
        }
    }
}
/* get stec correction for a station -----------------------------------------*/
static void get_stec_sta(gtime_t time, const stec_t *stec, int n, double *ion,
                         double *std, double *el, int *flag)
{
    double tt;
    int i;
    
    for (i=0;i<n;i++) {
        tt=timediff(stec[i].time,time);
        if (tt<-15.0) continue;
        if (tt>=15.0) break;
        ion[stec[i].sat-1]=stec[i].ion;
        std[stec[i].sat-1]=stec[i].std;
        el[stec[i].sat-1]=stec[i].azel[1];
        flag[stec[i].sat-1]=stec[i].flag;
    }
}
/* get tropospheric correction -------------------------------------------------
* get tropospheric correction from ppp correcion data
* args   : pppcorr_t *corr  I   ppp correction data
*          gtime_t time     I   time (GPST)
*          double *pos      I   receiver position {lat,lon,heght} (rad,m)
*          double *trp      O   tropos parameters {ztd,grade,gradn} (m)
*          double *std      O   standard deviation (m)
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int pppcorr_trop(const pppcorr_t *corr, gtime_t time, const double *pos,
                        double *trp, double *std)
{
     const double azel[]={0.0,PI/2.0};
     double trp_s[3],std_s[3];
     double var,wgt[3]={0},cof[3]={0},dist[MAXSTA],pos_s[MAXSTA][3];
     int i,j,n,sta[MAXSTA];
     
     for (i=0;i<3;i++) trp[i]=std[i]=0.0;
     
     if (!(n=get_sta(corr,pos,MAX_DIST_TROP,sta,dist,pos_s))) return 0;
     
     for (i=0;i<n;i++) {
         if (corr->nt[sta[i]]<=0) continue;
         
         /* get trop correction for a station */
         get_trop_sta(time,corr->trop[sta[i]],corr->nt[sta[i]],trp_s,std_s);
         
         /* convert ztd to zwd */
         trp_s[0]-=tropmodel(time,pos_s[i],azel,0.0);
         
         for (j=0;j<3;j++) {
             var=SQR(std_s[j])+SQR(dist[i]*(j==0?STD_ZTD_DIST:STD_GRAD_DIST));
             trp[j]+=trp_s[j]/dist[i];
             wgt[j]+=1.0/dist[i];
             cof[j]+=1.0/var;
         }
     }
     if (wgt[0]==0.0) return 0;
     
     for (i=0;i<3;i++) {
         trp[i]/=wgt[i];
         std[i]=sqrt(1.0/cof[i]);
     }
     /* convert zwd to ztd */
     trp[0]+=tropmodel(time,pos,azel,0.0);
     return 1;
}
/* get ionospherec correction --------------------------------------------------
* get ionospheric correction from ppp correction data
* args   : pppcorr_t *corr  I   ppp correction data
*          gtime_t time     I   time (GPST)
*          double *pos      I   receiver ecef position {x,y,z} (m)
*          double *ion      O   L1 slant ionos delay for each sat (MAXSAT x 1)
*                               (ion[i]==0: no correction data)
*          double *std      O   standard deviation (m)
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int pppcorr_stec(const pppcorr_t *corr, gtime_t time, const double *pos,
                        double *ion, double *std)
{
     double ion_s[MAXSAT],std_s[MAXSAT],el_s[MAXSAT];
     double var,wgt[MAXSAT]={0},cof[MAXSAT]={0},dist[MAXSTA],pos_s[MAXSTA][3];
     int i,j,n,sta[MAXSTA],flag_s[MAXSTA];
     
     for (i=0;i<MAXSAT;i++) ion[i]=0.0;
     
     if (!(n=get_sta(corr,pos,MAX_DIST_STEC,sta,dist,pos_s))) return 0;
     
     for (i=0;i<n;i++) {
         if (corr->ns[sta[i]]<=0) continue;
         
         for (j=0;j<MAXSAT;j++) ion_s[j]=std_s[j]=el_s[j]=0.0;
         
         /* get stec correction for a station */
         get_stec_sta(time,corr->stec[sta[i]],corr->ns[sta[i]],ion_s,std_s,
                      el_s,flag_s);
         
         for (j=0;j<MAXSAT;j++) {
             if (ion_s[j]==0.0||el_s[j]<=0.0) continue;
             var=SQR(std_s[j])+SQR(dist[i]*STD_VTEC_DIST/sin(el_s[j]));
             ion[j]+=ion_s[j]/dist[i];
             wgt[j]+=1.0/dist[i];
             cof[j]+=1.0/var;
         }
     }
     for (i=0;i<MAXSAT;i++) {
         if (wgt[i]==0.0) continue;
         ion[i]/=wgt[i];
         std[i]=sqrt(1.0/cof[i]);
     }
     return 1;
}
