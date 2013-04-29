/*------------------------------------------------------------------------------
* rcvdcb.c : estimate receiver dcb
*
*          Copyright (C) 2012 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2012/09/15 1.0  new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define MIN_EL      (5.0*D2R)

#define FACT_LG     0.4
#define SIG_ERR_A   0.003
#define SIG_ERR_B   0.003
#define RATIO_ERR   100.0

#define THRES_LG    0.08
#define MAXGAP_IONO 300.0
#define MAXGAP_BIAS 300.0

#define STEC_RID    "$STEC"         /* stec record id */

#define PRN_IONO    SQR(1E-3)       /* process noise var of iono (m^2/s) */
#define PRN_IONR    SQR(1E-9)       /* process noise var of iono rate (m^2/s) */
#define VAR_IONO    SQR(10.0)       /* initial var of iono (m^2) */
#define VAR_IONR    SQR(0.1)        /* initial var of iono rate (m^2) */
#define VAR_BIAS    SQR(10.0)       /* initial var of bias (m^2) */

#define II(s)       (((s)-1)*2)     /* state index of ionos */
#define IB(s)       (MAXSAT*2+(s)-1) /* state index of bias */
#define NX          (MAXSAT*3)      /* number of estimated states */

#define SQR(x)      ((x)*(x))

/* type definition -----------------------------------------------------------*/

typedef struct {                    /* satellite status type */
    gtime_t time;                   /* time */
    double azel[2];                 /* azimuth/elevation (rad) */
    double LG,PG;                   /* geometry-free phase/code (m) */
} sstat_t;

typedef struct {                    /* ekf type */
    int nx;                         /* number of states */
    double *x,*P;                   /* state variable and covariance */
} ekf_t;

/* new ekf -------------------------------------------------------------------*/
static ekf_t *ekf_new(int nx)
{
    ekf_t *ekf;
    if (!(ekf=(ekf_t *)malloc(sizeof(ekf_t)))) return NULL;
    ekf->nx=nx;
    ekf->x=zeros(nx,1);
    ekf->P=zeros(nx,nx);
    return ekf;
}
/* free ekf ------------------------------------------------------------------*/
static void ekf_free(ekf_t *ekf)
{
    if (!ekf) return;
    free(ekf->x);
    free(ekf->P);
    free(ekf);
}
/* mapping function of ionosphere ---------------------------------------------*/
static double map_iono(const double *pos, const double *azel)
{
#if 0
    return ionmapf(pos,azel);
#else
    return 1.0;
#endif
}
/* measurement error standard deviation --------------------------------------*/
static double sig_err(const double *azel)
{
    return FACT_LG*(SIG_ERR_A+SIG_ERR_B/sin(azel[1]));
}
/* initialize state and covariance -------------------------------------------*/
static void initx(double *x, double *P, int nx, int i, double xi, double var)
{
    int j;
    x[i]=xi;
    for (j=0;j<nx;j++) {
        P[i+j*nx]=P[j+i*nx]=i==j?var:0.0;
    }
}
/* predict ekf ---------------------------------------------------------------*/
static void ekf_pred(ekf_t *ekf, double *F, int ix, int nx)
{
    double *Q;
    int i;
    
    Q=mat(nx,ekf->nx);
    
    /* x(i)=F*x(i) */
    matmul("NN",nx,1,nx,1.0,F,ekf->x+ix,0.0,Q);
    matcpy(ekf->x+ix,Q,nx,1);
    
    /* P(i,:)=F*P(i,:); P(:,i)=P(:,i)*F' */
    for (i=0;i<ekf->nx;i++) {
        matmul("NN",nx,1,nx,1.0,F,ekf->P+ekf->nx*i+ix,0.0,Q);
        matcpy(ekf->P+ekf->nx*i+ix,Q,nx,1);
    }
    matmul("NT",ekf->nx,nx,nx,1.0,ekf->P+ekf->nx*ix,F,0.0,Q);
    matcpy(ekf->P+ekf->nx*ix,Q,ekf->nx,nx);
    
    free(Q);
}
/* temporal update of states --------------------------------------------------*/
static void ud_state(const obsd_t *obs, int n, const nav_t *nav,
                     const double *pos, const double *azel, ekf_t *ekf,
                     sstat_t *sstat)
{
    double P1,P2,L1,L2,PG,LG,tt,F[4]={0};
    double iono,m_iono,c_iono=1.0-SQR(lam[1]/lam[0]);
    int i,j,k,sat,slip;
    
    for (i=0;i<n;i++) {
        P1=obs[i].P[0]; L1=obs[i].L[0]*lam[0];
        P2=obs[i].P[1]; L2=obs[i].L[1]*lam[1];
        if (L1==0.0||L2==0.0||P1==0.0||P2==0.0||azel[i*2+1]<MIN_EL) continue;
        
        sat=obs[i].sat;
        tt=timediff(obs[i].time,sstat[sat-1].time);
        LG=L1-L2;
        PG=P1-P2;
        slip=(obs[i].LLI[0]&3)||(obs[i].LLI[1]&3);
        slip|=fabs(LG-sstat[sat-1].LG)>THRES_LG;
        
        j=II(sat); k=IB(sat);
        
        if (fabs(tt)>MAXGAP_IONO) {
            m_iono=map_iono(pos,azel+i*2);
#if 1
            iono=PG/c_iono/m_iono;
#else
            iono=ionmodel(obs[i].time,nav->ion_gps,pos,azel+i*2);
#endif
            initx(ekf->x,ekf->P,ekf->nx,j,iono,VAR_IONO);
            initx(ekf->x,ekf->P,ekf->nx,j+1,0.0,VAR_IONR);
        }
        else {
#if 1
            F[0]=F[3]=1.0; F[2]=tt;
            ekf_pred(ekf,F,j,2);
#else
            ekf->P[ j   *(ekf->nx+1)]+=PRN_IONO*fabs(tt);
#endif
            ekf->P[(j+1)*(ekf->nx+1)]+=PRN_IONR*fabs(tt);
        }
        if (tt>MAXGAP_BIAS||slip) {
            initx(ekf->x,ekf->P,ekf->nx,k,LG+PG,VAR_BIAS);
        }
        sstat[sat-1].time=obs[i].time;
        sstat[sat-1].azel[0]=azel[i*2];
        sstat[sat-1].azel[1]=azel[i*2+1];
        sstat[sat-1].LG=LG;
        sstat[sat-1].PG=PG;
    }
}
/* ionosphere residuals ------------------------------------------------------*/
static int res_iono(const obsd_t *obs, int n, const nav_t *nav,
                    const double *rs, const double *rr, const double *pos,
                    const double *azel, const pcv_t *pcv, const ekf_t *ekf,
                    double *phw, double *v, double *H, double *R)
{
    double *sig,P1,P2,L1,L2,m_iono,c_iono=1.0-SQR(lam[1]/lam[0]);
    double LG,PG,antdel[3]={0},dant[NFREQ]={0};
    int i,j,nv=0,sat;
    
    sig=mat(1,2*n);
    
    for (i=0;i<n;i++) {
        P1=obs[i].P[0]; L1=obs[i].L[0]*lam[0];
        P2=obs[i].P[1]; L2=obs[i].L[1]*lam[1];
        if (P1==0.0||P2==0.0||L1==0.0||L2==0.0||azel[1+i*2]<MIN_EL) continue;
        sat=obs[i].sat;
        
        /* ionosphere mapping function */
        m_iono=map_iono(pos,azel+i*2);
        
        /* ionosphere-LC model */
        LG=-c_iono*m_iono*ekf->x[II(sat)]+ekf->x[IB(sat)];
        PG= c_iono*m_iono*ekf->x[II(sat)];
        
        /* receiver antenna phase center offset and variation */
        if (pcv) {
            antmodel(pcv,antdel,azel+i*2,dant);
            LG+=dant[0]-dant[1];
            PG+=dant[0]-dant[1];
        }
        /* phase windup correction */
        windupcorr(obs[i].time,rs+i*6,rr,phw+obs[i].sat-1);
        LG+=(lam[0]-lam[1])*phw[obs[i].sat-1];
        
        /* C1->P1 DCB correction */
        if (obs[i].code[0]==CODE_L1C) P1+=nav->cbias[obs[i].sat-1][1];
        
        /* residuals of ionosphere (geometriy-free) LC */
        v[nv  ]=(L1-L2)-LG;
#if 0
        v[nv+1]=(P1-P2)-PG;
#else
        v[nv+1]=0.0;
#endif
        for (j=0;j<ekf->nx*2;j++) H[ekf->nx*nv+j]=0.0;
        H[ekf->nx*nv    +II(sat)]=-c_iono*m_iono;
        H[ekf->nx*nv    +IB(sat)]=1.0;
        H[ekf->nx*(nv+1)+II(sat)]=c_iono*m_iono;
        
        sig[nv  ]=sig_err(azel+i*2);
        sig[nv+1]=RATIO_ERR*sig[nv];
        nv+=2;
    }
    for (i=0;i<nv;i++) for (j=0;j<nv;j++) {
        R[i+j*nv]=i==j?SQR(sig[i]):0.0;
    }
    free(sig);
    return nv;
}
/* output ionosphere parameters ----------------------------------------------*/
static void out_iono(gtime_t time, const ekf_t *ekf, const sstat_t *sstat,
                     const double *pos, FILE *fp)
{
    double tow;
    char id[64];
    int sat,week;
    
    tow=time2gpst(time,&week);
    
    for (sat=1;sat<=MAXSAT;sat++) {
        if (sstat[sat-1].time.time==0||
            timediff(time,sstat[sat-1].time)>MAXGAP_IONO) continue;
        satno2id(sat,id);
        fprintf(fp,"%s %4d %5.0f %-3s %7.3f %8.3f %8.4f %7.4f %6.1f %5.1f %7.3f %11.3f\n",
                STEC_RID,week,tow,id,pos[0]*R2D,pos[1]*R2D,ekf->x[II(sat)],
                sqrt(ekf->P[II(sat)*(ekf->nx+1)]),sstat[sat-1].azel[0]*R2D,
                sstat[sat-1].azel[1]*R2D, sstat[sat-1].PG,sstat[sat-1].LG);
    }
}
/* estimate receiver dcb -----------------------------------------------------*/
static int est_iono(obs_t *obs, nav_t *nav, double *rr, FILE *fp)
{
    sstat_t sstat[MAXSAT]={{{0}}};
    ekf_t *ekf;
    gtime_t time;
    double r,pos[3],rs[MAXOBS*6],dts[MAXOBS*2],var[MAXOBS],e[3],azel[2];
    int i,j,n=0,info,nx=NX,nv=MAXSAT*2,svh[MAXOBS];
    
    ekf=ekf_new(NX); v=mat(nv,1); H=mat(nx,nv); R=mat(nv,nv);
    
    /* receiver position */
    ecef2pos(rr,pos);
    
    for (i=0;i<obs->n;i+=n) {
        for (n=1;i+n<obs->n;n++) {
            if (timediff(obs->data[i+n].time,obs->data[i].time)>1E-3) break;
        }
        time=obs->data[i].time;
        
        /* satellite positions and clocks */
        satposs(time,obs->data+i,n,nav,EPHOPT_BRDC,rs,dts,var,svh);
        
        /* satellite azimuth/elevation angle */
        for (j=0;j<n;j++) {
            if ((r=geodist(rs+j*6,rr,e))<=0.0) continue;
            satazel(pos,e,azel);
            
            azel[j*2]=azel[1+j*2]=0.0;
        }
    }
    return 1;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=stdout;
    nav_t nav={0};
    obs_t obs={0};
    sta_t sta={{0}};
    gtime_t ts={0},te={0};
    double eps[6]={0},epe[6]={0},rr[3]={0},tint=30.0;
    char *rfile[32],*ifile="";
    int i,j,n=0;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",eps,eps+1,eps+2);
            sscanf(argv[++i],"%lf:%lf:%lf",eps+3,eps+4,eps+5);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epe,epe+1,epe+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epe+3,epe+4,epe+5);
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
            tint=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-r")&&i+3<argc) {
            for (j=0;j<3;j++) rr[j]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-i")&&i+1<argc) ifile=argv[++i];
        else rfile[n++]=argv[i];
    }
    if (eps[2]>=1.0) ts=epoch2time(eps);
    if (epe[2]>=1.0) te=epoch2time(epe);
    
    /* read rinex obs/nav */
    for (i=0;i<n;i++) {
        fprintf(stderr,"reading ... %s\n",rfile[i]);
        
        readrnxt(rfile[i],1,ts,te,0.0,&obs,&nav,&sta);
        
        if (norm(sta.pos,3)>0.0) matcpy(rr,sta.pos,3,1);
    }
    if (!sortobs(&obs)) {
        fprintf(stderr,"no observation data\n");
        return -1;
    }
    uniqnav(&nav);
    
    /* read ionex file */
    if (*ifile) readionex(ifile,&nav);
    
    /* estimate receiver dcb */
    est_rcvdcb(&obs,&nav,rr,fp);
    
    fclose(fp);
    
    return 0;
}
