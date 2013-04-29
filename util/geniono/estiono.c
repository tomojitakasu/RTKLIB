/*------------------------------------------------------------------------------
* geniono.c : ionosphere correction estimation
*
*          Copyright (C) 2012 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2012/09/15 1.0  new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define SQR(x)      ((x)*(x))
#define MIN(x,y)    ((x)<=(y)?(x):(y))

#define FACT_LG     0.4
#define SIG_ERR_A   0.003
#define SIG_ERR_B   0.003
#define RATIO_ERR   100.0

#define PRN_IONO    SQR(1E-3)  /* process noise variance of iono delay (m^2/s) */
#define VAR_IONO    SQR(10.0)  /* initial variance of vertical iono delay (m^2) */

#define II(s)       ((s)-1)            /* state index of ionos */
#define IB(s)       (MAXSAT+(s)-1)     /* state index of phase bias */
#define NX          (MAXSAT*2)         /* number of estimated states */

typedef struct {              /* satellite status type */
    gtime_t time;             /* time */
    double azel[2];           /* azimuth/elevation (rad) */
    double gf;                /* geometry-free phase value (m) */
} stat_t;

/* initialize state and covariance -------------------------------------------*/
static void initx(double *x, double *P, int nx, int i, double xi, double var)
{
    int j;
    x[i]=xi;
    for (j=0;j<nx;j++) {
        P[i+j*nx]=P[j+i*nx]=i==j?var:0.0;
    }
}
/* detect cycle slip ---------------------------------------------------------*/
static int det_slip(const obsd_t *obs, const nav_t *nav, stat_t *stat)
{
    double G0;
    
    if (obs->L[0]!=0.0&&obs->L[1]!=0.0) {
        G0=stat[obs->sat-1].gf;
        stat[obs->sat-1].gf=G1=obs->L[0]-obs->L[1];
        if (fabs(G1-G0)>THRES_SLIP) return 1;
    }
    return (obs->LLI[0]&3)||(obs->LLI[1]&3);
}
/* initizlize ionosphere parameter --------------------------------------------*/
static void init_iono(const obsd_t *obs, const double *azel, double *x,
                      double *P, int nx)
{
    double map,iono;
    if (obs->P[0]==0||obs->P[1]==0) return;
    map=ionmapf(pos,azel);
    iono=(obs->P[0]-obs->P[1])/map;
    initx(x,P,nx,II(obs->sat),iono,VAR_IONO);
}
/* initizlize bias parameter --------------------------------------------------*/
static void init_bias(const obsd_t *obs, double *x, double *P, int nx)
{
    double bias;
    if (obs->L[0]==0||obs->L[1]==0||obs->P[0]==0||obs->P[1]==0) return;
    bias=(obs->L[0]-obs->L[1])-(obs->P[0]-obs->P[1]);
    initx(x,P,nx,IB(obs->sat),bias,VAR_BIAS);
}
/* temporal update of states --------------------------------------------------*/
static void udstate(const obsd_t *obs, int n, const nav_t *nav, double *x,
                    double *P, int nx, ssat_t *ssat)
{
    gtime_t time;
    double tt;
    int i,sat;
    
    for (i=0;i<n;i++) {
        sat=obs[i].sat
        time=ssat[sat-1].time;
        
        if (!time.time) {
            init_iono(obs+i,nav,x,P,nx);
            init_bias(obs+i,nav,x,P,nx);
        }
        else {
            tt=timediff(obs[i].time,time);
            
            P[II(sat)*(nx+1)]+=PRN_IONO*fabs(tt);
            
            if (det_slip(obs+i,nav,ssat)||fabs(tt)>MAXGAP_BIAS) {
                init_bias(obs+i,nav,x,P,nx);
            }
        }
        ssat[sat-1].time=time;
    }
}
/* measurement error standard deviation --------------------------------------*/
static double std_err(const double *azel)
{
    return FACT_LG*(SIG_ERR_A+SIG_ERR_B/sin(azel[1]));
}
/* satellite azimuth/elevation angle -----------------------------------------*/
static void sat_azel(const obsd_t *obs, int n, const nav_t *nav,
                     const double *pos, double *azel)
{
    double rs[MAXOBS*6],dts[MAXOBS*2],var[MAXOBS],r,e[3];
    int svh[MAXOBS];
    
    /* satellite positions and clocks */
    satposs(obs[0].time,obs,n,nav,EPHOPT_BRDC,rs,dts,var,svh);
    
    for (i=0;i<n;i++) {
        if (geodist(rs+i*6,rr,e))>0.0) satazel(pos,e,azel+i*2);
    }
}
/* ionosphere residuals ------------------------------------------------------*/
static int res_iono(const obsd_t *obs, int n, const double *azel,
                    const double *x, int nx, double *v, double *H, double *R)
{
    double *sig,L1,L2,P1,P2,map;
    int i,j,nv=0,sat;
    
    sig=mat(1,2*n);
    
    for (i=0;i<n;i++) {
        sat=obs[i].sat;
        L1=obs->L[0]*lam[0];
        L2=obs->L[1]*lam[1];
        P1=obs->P[0];
        P2=obs->P[1];
        if (L1==0||L2==0||P1==0||P2==0) continue;
        
        /* ionosphere mapping function */
        map=ionmapf(pos,azel+i*2);
        
        /* residuals of ionosphere (geometriy-free) LC */
        v[nv  ]=(L1-L2)+map*x[II(sat)]-x[IB(sat)];
        v[nv+1]=(P1-P2)-map*x[II(sat)];
        
        /* partial derivatives */
        for (j=0;j<nx;j++) H[nx*nv+j]=0.0;
        H[nx*nv    +II(sat)]=-map;
        H[nx*nv    +IB(sat)]=1.0;
        H[nx*(nv+1)+IB(sat)]=map;
        
        /* standard deviation of error */
        sig[nv  ]=std_err(azel);
        sig[nv+1]=sig[nv]*RATIO_ERR;
        nv+=2;
    }
    for (i=0;i<nv;i++) for (j=0;j<nv;j++) {
        R[i+j*nv]=i==j?SQR(sig[i]):0.0;
    }
    free(sig);
    return nv;
}
/* output ionosphere parameters ----------------------------------------------*/
static int out_iono(gtime_t time, const double *x, const double *P, int nx,
                    FILE *fp)
{
    double tow;
    char id[64];
    int i,week;
    
    tow=time2gpst(time,&week);
    
    for (i=0;i<MAXSAT;i++) {
        sat2id(i+1,id);
        fprintf(fp,"$ION,%d,%.3f,%d,%s,%.1f,%.4f,%4f\n",week,tow,0,id,
                ssat[i].azel[1]*R2D,x[II(i+1)],0);
    }
}
/* estimate ionosphere -------------------------------------------------------*/
static int est_iono(obs_t *obs, nav_t *nav, double *rr, FILE *fp)
{
    ssat_t ssat[MAXSAT]={{0}};
    double tt,*x,*P,*v,*H,*R,pos[3],azel[MAXOBS*2];
    int i,n,info,nx=NX,nv=MAXSAT*2;
    
    x=zeros(nx,1); P=zeros(nx,nx); v=mat(nv,1); H=mat(nx,nv); R=mat(nv,nv);
    
    /* receiver position */
    ecef2pos(rr,pos);
    
    for (i=0;i<obs->n;i++) {
        for (n=1;i+n<obs->n;n++) {
            if (timediff(obs[i].time,obs->data[i+n].time)>1E-3) break;
        }
        /* satellite azimuth/elevation angle */
        sat_azel(obs+i,n,nav,pos,azel);
        
        /* time update of parameters */
        ud_state(obs+i,n,azel,x,P,nx,ssat);
        
        /* ionosphere residuals */
        if ((nv=res_iono(obs+i,n,azel,x,nx,v,H,R))<=0) break;
        
        /* filter */
        if ((info=filter(x,P,H,v,R,nx,nv))) break;
        
        /* output ionopshere parameters */
        out_iono(obs[i].time,x,P,nx,fp);
    }
    free(x); free(P); free(v); free(H); free(R);
    
    return 1;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=stdout;
    nav_t nav={0};
    obs_t obs={0};
    double rr[3]={0};
    char *ifile[32],*ofile="";
    int i,j,n=0;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-r")&&i+3<argc) {
            for (j=0;j<3;j++) rr[j]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-o")&&i+1<argc) {
            ofile=argv[i];
        }
        else ifile[n++]=argv[i];
    }
    /* open output file */
    if (*ofile&&!(fp=fopen(ofile,"w"))) {
        fprintf(stderr,"output file open error: %s\n",ofile);
        return -1;
    }
    /* read rinex files */
    if (!readrnx(ifile,1,n,&obs,&nav,NULL)) {
        fprintf(stderr,"no observation data\n");
        return -1;
    }
    /* estimate ionosphere parameters */
    est_iono(&obs,&nav,rr,fp);
    
    fclose(fp);
    
    return 0;
}
