/*------------------------------------------------------------------------------
* genstec.c : generate stec corrections
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

#define THRES_LG    0.1
#define MAXGAP_IONO 300.0
#define MAXGAP_BIAS 300.0

#define SPOS_RID    "$SPOS"         /* stec position id */
#define STEC_RID    "$STEC"         /* stec record id */

#define VAR_IONO    SQR(10)         /* initial var of iono (m^2) */
#define VAR_IONR    SQR(0.1)        /* initial var of iono rate (m^2) */
#define VAR_BIAS    SQR(10)         /* initial var of bias (m^2) */
#define PRN_IONO    SQR(0)          /* process noise var of iono (m^2/s) */
#define PRN_IONR    SQR(1E-3)       /* process noise var of iono rate (m^2/s) */

#define II(s)       (((s)-1)*2)     /* state index of ionos */
#define IB(s)       (MAXSAT*2+(s)-1) /* state index of bias */
#define NX          (MAXSAT*3)      /* number of estimated states */

#define SQR(x)      ((x)*(x))

/* type definition -----------------------------------------------------------*/

typedef struct {                    /* satellite status type */
    gtime_t time;                   /* time */
    double azel[2];                 /* azimuth/elevation (rad) */
    double LG,PG;                   /* geometry-free phase/code (m) */
    int slip;                       /* slip flag */
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
/* measurement error standard deviation --------------------------------------*/
static double sig_err(const double *azel)
{
    return FACT_LG*(SIG_ERR_A+SIG_ERR_B/sin(azel[1]));
}
/* initialize states of ekf --------------------------------------------------*/
static void ekf_init(ekf_t *ekf, const double *xi, double *Pi, int ix, int nx)
{
    int i,j;
    
    /* x(ix)=xi */
    matcpy(ekf->x+ix,xi,nx,1);
    
    /* P(ix,:)=0; P(ix,:)=0; P(ix,ix)=diag(Pi) */
    for (i=0;i<nx;i++) for (j=0;j<ekf->nx;j++) {
        ekf->P[ix+i+ekf->nx*j]=ekf->P[j+ekf->nx*(ix+i)]=j==ix+i?Pi[i]:0.0;
    }
}
/* predict states of ekf -----------------------------------------------------*/
static void ekf_pred(ekf_t *ekf, double *F, double *Q, int ix, int nx)
{
    double *A;
    int i;
    
    A=mat(ekf->nx,nx);
    
    /* x(ix)=F*x(ix) */
    matmul("NN",nx,1,nx,1.0,F,ekf->x+ix,0.0,A);
    matcpy(ekf->x+ix,A,nx,1);
    
    /* P(ix,:)=F*P(ix,:); P(:,ix)=P(:,ix)*F' */
    for (i=0;i<ekf->nx;i++) {
        matmul("NN",nx,1,nx,1.0,F,ekf->P+ekf->nx*i+ix,0.0,A);
        matcpy(ekf->P+ekf->nx*i+ix,A,nx,1);
    }
    matmul("NT",ekf->nx,nx,nx,1.0,ekf->P+ekf->nx*ix,F,0.0,A);
    matcpy(ekf->P+ekf->nx*ix,A,ekf->nx,nx);
    
    /* P(ix,ix)+=diag(Q) */
    for (i=0;i<nx;i++) {
        ekf->P[(ix+i)*(ekf->nx+1)]+=Q[i];
    }
    free(A);
}
/* raw pseudorange and phase range  with dcb correction ----------------------*/
static int raw_obs(const obsd_t *obs, const nav_t *nav, double *P1, double *P2,
                   double *L1, double *L2)
{
    double gamma=SQR(lam[0]/lam[1]);
    
    *L1=obs->L[0]*lam[0];
    *L2=obs->L[1]*lam[1];
    *P1=obs->P[0];
    *P2=obs->P[1];
    if (*L1==0.0||*L2==0.0||*P1==0.0||*P2==0.0) return 0;
    
    *P1+=nav->cbias[obs->sat-1][0];
    *P2+=nav->cbias[obs->sat-1][0]*gamma;
    if (obs->code[0]==CODE_L1C) *P1+=nav->cbias[obs->sat-1][1];
    return 1;
}
/* temporal update of states --------------------------------------------------*/
static void ud_state(const obsd_t *obs, int n, const nav_t *nav,
                     const double *pos, const double *azel, ekf_t *ekf,
                     sstat_t *sstat)
{
    double P1,P2,L1,L2,PG,LG,tt,F[4]={0},Q[2]={0};
    double x[2]={0},P[2],c_iono=1.0-SQR(lam[1]/lam[0]);
    int i,sat,slip;
    
    for (i=0;i<n;i++) {
        
        /* raw pseudorange and phase range */
        if (!raw_obs(obs+i,nav,&P1,&P2,&L1,&L2)||azel[i*2+1]<MIN_EL) continue;
        
        sat=obs[i].sat;
        tt=timediff(obs[i].time,sstat[sat-1].time);
        LG=L1-L2;
        PG=P1-P2;
        slip=(obs[i].LLI[0]&3)||(obs[i].LLI[1]&3);
        slip|=fabs(LG-sstat[sat-1].LG)>THRES_LG;
        
        if (fabs(tt)>MAXGAP_IONO) {
#if 1
            x[0]=PG/c_iono;
#else
            x[0]=ionmodel(obs[i].time,nav->ion_gps,pos,azel+i*2);
#endif
            x[1]=1E-6;
            P[0]=VAR_IONO;
            P[1]=VAR_IONR;
            ekf_init(ekf,x,P,II(sat),2);
        }
        else {
            F[0]=F[3]=1.0;
            F[2]=tt;
            Q[0]=PRN_IONO*fabs(tt);
            Q[1]=PRN_IONR*fabs(tt);
            ekf_pred(ekf,F,Q,II(sat),2);
        }
        if (tt>MAXGAP_BIAS||slip) {
            x[0]=LG+PG;
            P[0]=VAR_BIAS;
            ekf_init(ekf,x,P,IB(sat),1);
        }
        sstat[sat-1].time=obs[i].time;
        sstat[sat-1].azel[0]=azel[i*2];
        sstat[sat-1].azel[1]=azel[i*2+1];
        sstat[sat-1].slip=slip;
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
    double *sig,P1,P2,L1,L2,c_iono=1.0-SQR(lam[1]/lam[0]);
    double LG,PG,antdel[3]={0},dant[NFREQ]={0};
    int i,j,nv=0,sat;
    
    sig=mat(1,2*n);
    
    for (i=0;i<n;i++) {
        if (!raw_obs(obs+i,nav,&P1,&P2,&L1,&L2)||azel[i*2+1]<MIN_EL) continue;
        
        sat=obs[i].sat;
        
        /* ionosphere-LC model */
        LG=-c_iono*ekf->x[II(sat)]+ekf->x[IB(sat)];
        PG= c_iono*ekf->x[II(sat)]+nav->cbias[sat-1][0];
        
        /* receiver antenna phase center offset and variation */
        if (pcv) {
            antmodel(pcv,antdel,azel+i*2,dant);
            LG+=dant[0]-dant[1];
            PG+=dant[0]-dant[1];
        }
        /* phase windup correction */
        windupcorr(obs[i].time,rs+i*6,rr,phw+obs[i].sat-1);
        LG+=(lam[0]-lam[1])*phw[obs[i].sat-1];
        
        /* residuals of ionosphere (geometriy-free) LC */
        v[nv  ]=(L1-L2)-LG;
#if 0
        v[nv+1]=(P1-P2)-PG;
#else
        v[nv+1]=0.0;
#endif
        for (j=0;j<ekf->nx*2;j++) H[ekf->nx*nv+j]=0.0;
        H[ekf->nx*nv    +II(sat)]=-c_iono;
        H[ekf->nx*nv    +IB(sat)]=1.0;
        H[ekf->nx*(nv+1)+II(sat)]=c_iono;
        
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
/* output ionosphere header --------------------------------------------------*/
static void out_head(gtime_t time, const double *pos, FILE *fp)
{
    double tow;
    int week;
    
    tow=time2gpst(time,&week);
    
    fprintf(fp,"%s %4d %5.0f %7.3f %8.3f\n",SPOS_RID,week,tow,pos[0]*R2D,
            pos[1]*R2D);
}
/* output ionosphere parameters ----------------------------------------------*/
static void out_iono(gtime_t time, const ekf_t *ekf, const sstat_t *sstat,
                     FILE *fp)
{
    double tow;
    char id[64];
    int sat,week;
    
    tow=time2gpst(time,&week);
    
    for (sat=1;sat<=MAXSAT;sat++) {
        if (sstat[sat-1].time.time==0||
            timediff(time,sstat[sat-1].time)>MAXGAP_IONO) continue;
        satno2id(sat,id);
        fprintf(fp,"%s %4d %6.0f %-3s %d %8.4f %9.6f %7.4f %6.1f %5.1f %7.3f %11.3f\n",
                STEC_RID,week,tow,id,sstat[sat-1].slip,
                ekf->x[II(sat)],ekf->x[II(sat)+1],
                sqrt(ekf->P[II(sat)*(ekf->nx+1)]),sstat[sat-1].azel[0]*R2D,
                sstat[sat-1].azel[1]*R2D, sstat[sat-1].PG,sstat[sat-1].LG);
    }
}
/* estimate ionosphere -------------------------------------------------------*/
static int est_iono(obs_t *obs, nav_t *nav, const pcv_t *pcv, double *rr,
                    double tint, FILE *fp)
{
    sstat_t sstat[MAXSAT]={{{0}}};
    ekf_t *ekf;
    gtime_t time;
    double pos[3],rs[MAXOBS*6],dts[MAXOBS*2],var[MAXOBS],e[3],azel[MAXOBS*2];
    double *v,*H,*R,phw[MAXSAT]={0};
    int i,j,n=0,info,nx=NX,nv=MAXSAT*2,svh[MAXOBS];
    
    ekf=ekf_new(NX); v=mat(nv,1); H=mat(nx,nv); R=mat(nv,nv);
    
    /* receiver position */
    ecef2pos(rr,pos);
    
    out_head(obs->data[0].time,pos,fp);
    
    for (i=0;i<obs->n;i+=n) {
        for (n=1;i+n<obs->n;n++) {
            if (timediff(obs->data[i+n].time,obs->data[i].time)>1E-3) break;
        }
        time=obs->data[i].time;
        
        /* satellite positions and clocks */
        satposs(time,obs->data+i,n,nav,EPHOPT_BRDC,rs,dts,var,svh);
        
        /* satellite azimuth/elevation angle */
        for (j=0;j<n;j++) {
            if (geodist(rs+j*6,rr,e)>0.0) satazel(pos,e,azel+j*2);
            else azel[j*2]=azel[1+j*2]=0.0;
        }
        /* time update of parameters */
        ud_state(obs->data+i,n,nav,pos,azel,ekf,sstat);
        
        /* ionosphere residuals */
        if ((nv=res_iono(obs->data+i,n,nav,rs,rr,pos,azel,pcv,ekf,phw,v,H,R))<=0) {
            continue;
        }
        /* filter */
        if ((info=filter(ekf->x,ekf->P,H,v,R,ekf->nx,nv))) {
            fprintf(stderr,"filter error: info=%d\n",info);
            break;
        }
        /* output ionopshere parameters */
        if (tint<=0.0||fmod(time2gpst(time,NULL)+0.005,tint)<0.01) {
            out_iono(obs->data[i].time,ekf,sstat,fp);
        }
    }
    ekf_free(ekf); free(v); free(H); free(R);
    
    return 1;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=stdout;
    pcvs_t pcvs={0};
    nav_t nav={0};
    obs_t obs={0};
    sta_t sta={{0}};
    pcv_t *pcv=NULL;
    gtime_t ts={0},te={0};
    double eps[6]={0},epe[6]={0},rr[3]={0},tint=30.0;
    char *ifile[32],*ofile="",*afile="",*dfile="",ant[64]="";
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
        else if (!strcmp(argv[i],"-o")&&i+1<argc) ofile=argv[++i];
        else if (!strcmp(argv[i],"-a")&&i+1<argc) afile=argv[++i];
        else if (!strcmp(argv[i],"-d")&&i+1<argc) dfile=argv[++i];
        else ifile[n++]=argv[i];
    }
    /* open output file */
    if (*ofile&&!(fp=fopen(ofile,"w"))) {
        fprintf(stderr,"output file open error: %s\n",ofile);
        return -1;
    }
    if (eps[2]>=1.0) ts=epoch2time(eps);
    if (epe[2]>=1.0) te=epoch2time(epe);
    
    /* read rinex obs/nav */
    for (i=0;i<n;i++) {
        fprintf(stderr,"reading: %s\n",ifile[i]);
        
        readrnxt(ifile[i],1,ts,te,0.0,"",&obs,&nav,&sta);
        
        if (*sta.antdes) strcpy(ant,sta.antdes);
        if (norm(sta.pos,3)>0.0) matcpy(rr,sta.pos,3,1);
    }
    if (!sortobs(&obs)) {
        fprintf(stderr,"no observation data\n");
        return -1;
    }
    uniqnav(&nav);
    
    /* read antenna file */
    if (*afile&&*ant) {
        if (!readpcv(afile,&pcvs)) {
            fprintf(stderr,"antenna file open error: %s\n",afile);
            return -1;
        }
        /* search pcv */
        if (!(pcv=searchpcv(0,ant,obs.data[0].time,&pcvs))) {
            fprintf(stderr,"no antenna parmeter: %s\n",ant);
        }
    }
    /* read p1-c1 dcb parameters */
    if (*dfile) readdcb(dfile,&nav);
    
    /* set p1-p2 dcb parameters */
    for (i=0;i<MAXSAT;i++) {
        for (j=0;j<nav.n;j++) {
            if (nav.eph[j].sat!=i+1) continue;
            nav.cbias[i][0]=nav.eph[j].tgd[0]*CLIGHT;
            break;
        }
    }
    /* estimate ionosphere parameters */
    est_iono(&obs,&nav,pcv,rr,tint,fp);
    
    fclose(fp);
    
    if (*ofile) fprintf(stderr,"output: %s\n",ofile);
    
    return 0;
}
