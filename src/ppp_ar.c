/*------------------------------------------------------------------------------
* ppp_ar.c : ppp ambiguity resolution
*
* options : -DREV_WL_FCB reversed polarity of WL FCB
*
* reference :
*    [1] H.Okumura, C-gengo niyoru saishin algorithm jiten (in Japanese),
*        Software Technology, 1991
*
*          Copyright (C) 2012-2013 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/03/11 1.0  new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* constants/macros ----------------------------------------------------------*/

#define MIN_ARC_GAP     300.0       /* min arc gap (s) */
#define CONST_AMB       0.001       /* constraint to fixed ambiguity */
#define THRES_RES       0.3         /* threashold of residuals test (m) */
#define LOG_PI          1.14472988584940017 /* log(pi) */
#define SQRT2           1.41421356237309510 /* sqrt(2) */

#define MIN(x,y)        ((x)<(y)?(x):(y))
#define SQR(x)          ((x)*(x))
#define ROUND(x)        (int)floor((x)+0.5)

#define SWAP_I(x,y)     do {int _z=x; x=y; y=_z;} while (0)
#define SWAP_D(x,y)     do {double _z=x; x=y; y=_z;} while (0)

#define NP(opt)     ((opt)->dynamics?9:3) /* number of pos solution */
#define IC(s,opt)   (NP(opt)+(s))      /* state index of clocks (s=0:gps,1:glo) */
#define IT(opt)     (IC(0,opt)+NSYS)   /* state index of tropos */
#define NR(opt)     (IT(opt)+((opt)->tropopt<TROPOPT_EST?0:((opt)->tropopt==TROPOPT_EST?1:3)))
                                       /* number of solutions */
#define IB(s,opt)   (NR(opt)+(s)-1)    /* state index of phase bias */

/* wave length of LC (m) -----------------------------------------------------*/
static double lam_LC(int i, int j, int k)
{
    const double f1=FREQ1,f2=FREQ2,f5=FREQ5;
    
    return CLIGHT/(i*f1+j*f2+k*f5);
}
/* carrier-phase LC (m) ------------------------------------------------------*/
static double L_LC(int i, int j, int k, const double *L)
{
    const double f1=FREQ1,f2=FREQ2,f5=FREQ5;
    double L1,L2,L5;
    
    if ((i&&!L[0])||(j&&!L[1])||(k&&!L[2])) {
        return 0.0;
    }
    L1=CLIGHT/f1*L[0];
    L2=CLIGHT/f2*L[1];
    L5=CLIGHT/f5*L[2];
    return (i*f1*L1+j*f2*L2+k*f5*L5)/(i*f1+j*f2+k*f5);
}
/* pseudorange LC (m) --------------------------------------------------------*/
static double P_LC(int i, int j, int k, const double *P)
{
    const double f1=FREQ1,f2=FREQ2,f5=FREQ5;
    double P1,P2,P5;
    
    if ((i&&!P[0])||(j&&!P[1])||(k&&!P[2])) {
        return 0.0;
    }
    P1=P[0];
    P2=P[1];
    P5=P[2];
    return (i*f1*P1+j*f2*P2+k*f5*P5)/(i*f1+j*f2+k*f5);
}
/* noise variance of LC (m) --------------------------------------------------*/
static double var_LC(int i, int j, int k, double sig)
{
    const double f1=FREQ1,f2=FREQ2,f5=FREQ5;
    
    return (SQR(i*f1)+SQR(j*f2)+SQR(k*f5))/SQR(i*f1+j*f2+k*f5)*SQR(sig);
}
/* complementaty error function (ref [1] p.227-229) --------------------------*/
static double q_gamma(double a, double x, double log_gamma_a);
static double p_gamma(double a, double x, double log_gamma_a)
{
    double y,w;
    int i;
    
    if (x==0.0) return 0.0;
    if (x>=a+1.0) return 1.0-q_gamma(a,x,log_gamma_a);
    
    y=w=exp(a*log(x)-x-log_gamma_a)/a;
    
    for (i=1;i<100;i++) {
        w*=x/(a+i);
        y+=w;
        if (fabs(w)<1E-15) break;
    }
    return y;
}
static double q_gamma(double a, double x, double log_gamma_a)
{
    double y,w,la=1.0,lb=x+1.0-a,lc;
    int i;
    
    if (x<a+1.0) return 1.0-p_gamma(a,x,log_gamma_a);
    w=exp(-x+a*log(x)-log_gamma_a);
    y=w/lb;
    for (i=2;i<100;i++) {
        lc=((i-1-a)*(lb-la)+(i+x)*lb)/i;
        la=lb; lb=lc;
        w*=(i-1-a)/i;
        y+=w/la/lb;
        if (fabs(w/la/lb)<1E-15) break;
    }
    return y;
}
static double f_erfc(double x)
{
    return x>=0.0?q_gamma(0.5,x*x,LOG_PI/2.0):1.0+p_gamma(0.5,x*x,LOG_PI/2.0);
}
/* confidence function of integer ambiguity ----------------------------------*/
static double conffunc(int N, double B, double sig)
{
    double x,p=1.0;
    int i;
    
    x=fabs(B-N);
    for (i=1;i<8;i++) {
        p-=f_erfc((i-x)/(SQRT2*sig))-f_erfc((i+x)/(SQRT2*sig));
    }
    return p;
}
/* average LC ----------------------------------------------------------------*/
static void average_LC(rtk_t *rtk, const obsd_t *obs, int n, const nav_t *nav,
                       const double *azel)
{
    ambc_t *amb;
    double LC1,LC2,LC3,var1,var2,var3,sig;
    int i,j,sat;
    
    for (i=0;i<n;i++) {
        sat=obs[i].sat;
        
        if (azel[1+2*i]<rtk->opt.elmin) continue;
        
        if (satsys(sat,NULL)!=SYS_GPS) continue;
        
        /* triple-freq carrier and code LC (m) */
        LC1=L_LC(1,-1, 0,obs[i].L)-P_LC(1,1,0,obs[i].P);
        LC2=L_LC(0, 1,-1,obs[i].L)-P_LC(0,1,1,obs[i].P);
        LC3=L_LC(1,-6, 5,obs[i].L)-P_LC(1,1,0,obs[i].P);
        
        sig=sqrt(SQR(rtk->opt.err[1])+SQR(rtk->opt.err[2]/sin(azel[1+2*i])));
        
        /* measurement noise variance (m) */
        var1=var_LC(1,1,0,sig*rtk->opt.eratio[0]);
        var2=var_LC(0,1,1,sig*rtk->opt.eratio[0]);
        var3=var_LC(1,1,0,sig*rtk->opt.eratio[0]);
        
        amb=rtk->ambc+sat-1;
        
        if (rtk->ssat[sat-1].slip[0]||rtk->ssat[sat-1].slip[1]||
            rtk->ssat[sat-1].slip[2]||amb->n[0]==0.0||
            fabs(timediff(amb->epoch[0],obs[0].time))>MIN_ARC_GAP) {
            
            amb->n[0]=amb->n[1]=amb->n[2]=0.0;
            amb->LC[0]=amb->LC[1]=amb->LC[2]=0.0;
            amb->LCv[0]=amb->LCv[1]=amb->LCv[2]=0.0;
            amb->fixcnt=0;
            for (j=0;j<MAXSAT;j++) amb->flags[j]=0;
        }
        /* averaging */
        if (LC1) {
            amb->n[0]+=1.0;
            amb->LC [0]+=(LC1 -amb->LC [0])/amb->n[0];
            amb->LCv[0]+=(var1-amb->LCv[0])/amb->n[0];
        }
        if (LC2) {
            amb->n[1]+=1.0;
            amb->LC [1]+=(LC2 -amb->LC [1])/amb->n[1];
            amb->LCv[1]+=(var2-amb->LCv[1])/amb->n[1];
        }
        if (LC3) {
            amb->n[2]+=1.0;
            amb->LC [2]+=(LC3 -amb->LC [2])/amb->n[2];
            amb->LCv[2]+=(var3-amb->LCv[2])/amb->n[2];
        }
        amb->epoch[0]=obs[0].time;
    }
}
/* fix wide-lane ambiguity ---------------------------------------------------*/
static int fix_amb_WL(rtk_t *rtk, const nav_t *nav, int sat1, int sat2, int *NW)
{
    ambc_t *amb1,*amb2;
    double BW,vW,lam_WL=lam_LC(1,-1,0);
    
    amb1=rtk->ambc+sat1-1;
    amb2=rtk->ambc+sat2-1;
    if (!amb1->n[0]||!amb2->n[0]) return 0;
    
    /* wide-lane ambiguity */
#ifndef REV_WL_FCB
    BW=(amb1->LC[0]-amb2->LC[0])/lam_WL+nav->wlbias[sat1-1]-nav->wlbias[sat2-1];
#else
    BW=(amb1->LC[0]-amb2->LC[0])/lam_WL-nav->wlbias[sat1-1]+nav->wlbias[sat2-1];
#endif
    *NW=ROUND(BW);
    
    /* variance of wide-lane ambiguity */
    vW=(amb1->LCv[0]/amb1->n[0]+amb2->LCv[0]/amb2->n[0])/SQR(lam_WL);
    
    /* validation of integer wide-lane ambigyity */
    return fabs(*NW-BW)<=rtk->opt.thresar[2]&&
           conffunc(*NW,BW,sqrt(vW))>=rtk->opt.thresar[1];
}
/* linear dependency check ---------------------------------------------------*/
static int is_depend(int sat1, int sat2, int *flgs, int *max_flg)
{
    int i;
    
    if (flgs[sat1-1]==0&&flgs[sat2-1]==0) {
        flgs[sat1-1]=flgs[sat2-1]=++(*max_flg);
    }
    else if (flgs[sat1-1]==0&&flgs[sat2-1]!=0) {
        flgs[sat1-1]=flgs[sat2-1];
    }
    else if (flgs[sat1-1]!=0&&flgs[sat2-1]==0) {
        flgs[sat2-1]=flgs[sat1-1];
    }
    else if (flgs[sat1-1]>flgs[sat2-1]) {
        for (i=0;i<MAXSAT;i++) if (flgs[i]==flgs[sat2-1]) flgs[i]=flgs[sat1-1];
    }
    else if (flgs[sat1-1]<flgs[sat2-1]) {
        for (i=0;i<MAXSAT;i++) if (flgs[i]==flgs[sat1-1]) flgs[i]=flgs[sat2-1];
    }
    else return 0; /* linear depenent */
    return 1;
}
/* select fixed ambiguities --------------------------------------------------*/
static int sel_amb(int *sat1, int *sat2, double *N, double *var, int n)
{
    int i,j,flgs[MAXSAT]={0},max_flg=0;
    
    /* sort by variance */
    for (i=0;i<n;i++) for (j=1;j<n-i;j++) {
        if (var[j]>=var[j-1]) continue;
        SWAP_I(sat1[j],sat1[j-1]);
        SWAP_I(sat2[j],sat2[j-1]);
        SWAP_D(N[j],N[j-1]);
        SWAP_D(var[j],var[j-1]);
    }
    /* select linearly independent satellite pair */
    for (i=j=0;i<n;i++) {
        if (!is_depend(sat1[i],sat2[i],flgs,&max_flg)) continue;
        sat1[j]=sat1[i];
        sat2[j]=sat2[i];
        N[j]=N[i];
        var[j++]=var[i];
    }
    return j;
}
/* fixed solution ------------------------------------------------------------*/
static int fix_sol(rtk_t *rtk, const int *sat1, const int *sat2,
                   const double *NC, int n)
{
    double *v,*H,*R;
    int i,j,k,info;
    
    if (n<=0) return 0;
    
    v=zeros(n,1); H=zeros(rtk->nx,n); R=zeros(n,n);
    
    /* constraints to fixed ambiguities */
    for (i=0;i<n;i++) {
        j=IB(sat1[i],&rtk->opt);
        k=IB(sat2[i],&rtk->opt);
        v[i]=NC[i]-(rtk->x[j]-rtk->x[k]);
        H[j+i*rtk->nx]= 1.0;
        H[k+i*rtk->nx]=-1.0;
        R[i+i*n]=SQR(CONST_AMB);
    }
    /* update states with constraints */
    if ((info=filter(rtk->x,rtk->P,H,v,R,rtk->nx,n))) {
        trace(1,"filter error (info=%d)\n",info);
        free(v); free(H); free(R);
        return 0;
    }
    /* set solution */
    for (i=0;i<rtk->na;i++) {
        rtk->xa[i]=rtk->x[i];
        for (j=0;j<rtk->na;j++) {
            rtk->Pa[i+j*rtk->na]=rtk->Pa[j+i*rtk->na]=rtk->P[i+j*rtk->nx];
        }
    }
    /* set flags */
    for (i=0;i<n;i++) {
        rtk->ambc[sat1[i]-1].flags[sat2[i]-1]=1;
        rtk->ambc[sat2[i]-1].flags[sat1[i]-1]=1;
    }
    free(v); free(H); free(R);
    return 1;
}
/* fix narrow-lane ambiguity by rounding -------------------------------------*/
static int fix_amb_ROUND(rtk_t *rtk, int *sat1, int *sat2, const int *NW, int n)
{
    double C1,C2,B1,v1,BC,v,vc,*NC,*var,lam_NL=lam_LC(1,1,0),lam1,lam2;
    int i,j,k,m=0,N1,stat;
    
    lam1=lam_carr[0]; lam2=lam_carr[1];
    
    C1= SQR(lam2)/(SQR(lam2)-SQR(lam1));
    C2=-SQR(lam1)/(SQR(lam2)-SQR(lam1));
    
    NC=zeros(n,1); var=zeros(n,1);
    
    for (i=0;i<n;i++) {
        j=IB(sat1[i],&rtk->opt);
        k=IB(sat2[i],&rtk->opt);
        
        /* narrow-lane ambiguity */
        B1=(rtk->x[j]-rtk->x[k]+C2*lam2*NW[i])/lam_NL;
        N1=ROUND(B1);
        
        /* variance of narrow-lane ambiguity */
        var[m]=rtk->P[j+j*rtk->nx]+rtk->P[k+k*rtk->nx]-2.0*rtk->P[j+k*rtk->nx];
        v1=var[m]/SQR(lam_NL);
        
        /* validation of narrow-lane ambiguity */
        if (fabs(N1-B1)>rtk->opt.thresar[2]||
            conffunc(N1,B1,sqrt(v1))<rtk->opt.thresar[1]) {
            continue;
        }
        /* iono-free ambiguity (m) */
        BC=C1*lam1*N1+C2*lam2*(N1-NW[i]);
        
        /* check residuals */
        v=rtk->ssat[sat1[i]-1].resc[0]-rtk->ssat[sat2[i]-1].resc[0];
        vc=v+(BC-(rtk->x[j]-rtk->x[k]));
        if (fabs(vc)>THRES_RES) continue;
        
        sat1[m]=sat1[i];
        sat2[m]=sat2[i];
        NC[m++]=BC;
    }
    /* select fixed ambiguities by dependancy check */
    m=sel_amb(sat1,sat2,NC,var,m);
    
    /* fixed solution */
    stat=fix_sol(rtk,sat1,sat2,NC,m);
    
    free(NC); free(var);
    
    return stat&&m>=3;
}
/* fix narrow-lane ambiguity by ILS ------------------------------------------*/
static int fix_amb_ILS(rtk_t *rtk, int *sat1, int *sat2, int *NW, int n)
{
    double C1,C2,*B1,*N1,*NC,*D,*E,*Q,s[2],lam_NL=lam_LC(1,1,0),lam1,lam2;
    int i,j,k,m=0,info,stat,flgs[MAXSAT]={0},max_flg=0;
    
    lam1=lam_carr[0]; lam2=lam_carr[1];
    
    C1= SQR(lam2)/(SQR(lam2)-SQR(lam1));
    C2=-SQR(lam1)/(SQR(lam2)-SQR(lam1));
    
    B1=zeros(n,1); N1=zeros(n,2); D=zeros(rtk->nx,n); E=mat(n,rtk->nx);
    Q=mat(n,n); NC=mat(n,1);
    
    for (i=0;i<n;i++) {
        
        /* check linear independency */
        if (!is_depend(sat1[i],sat2[i],flgs,&max_flg)) continue;
        
        j=IB(sat1[i],&rtk->opt);
        k=IB(sat2[i],&rtk->opt);
        
        /* float narrow-lane ambiguity (cycle) */
        B1[m]=(rtk->x[j]-rtk->x[k]+C2*lam2*NW[i])/lam_NL;
        N1[m]=ROUND(B1[m]);
        
        /* validation of narrow-lane ambiguity */
        if (fabs(N1[m]-B1[m])>rtk->opt.thresar[2]) continue;
        
        /* narrow-lane ambiguity transformation matrix */
        D[j+m*rtk->nx]= 1.0/lam_NL;
        D[k+m*rtk->nx]=-1.0/lam_NL;
        
        sat1[m]=sat1[i];
        sat2[m]=sat2[i];
        NW[m++]=NW[i];
    }
    if (m<3) return 0;
    
    /* covariance of narrow-lane ambiguities */
    matmul("TN",m,rtk->nx,rtk->nx,1.0,D,rtk->P,0.0,E);
    matmul("NN",m,m,rtk->nx,1.0,E,D,0.0,Q);
    
    /* integer least square */
    if ((info=lambda(m,2,B1,Q,N1,s))) {
        trace(2,"lambda error: info=%d\n",info);
        return 0;
    }
    if (s[0]<=0.0) return 0;
    
    rtk->sol.ratio=(float)(MIN(s[1]/s[0],999.9));
    
    /* varidation by ratio-test */
    if (rtk->opt.thresar[0]>0.0&&rtk->sol.ratio<rtk->opt.thresar[0]) {
        trace(2,"varidation error: n=%2d ratio=%8.3f\n",m,rtk->sol.ratio);
        return 0;
    }
    trace(2,"varidation ok: %s n=%2d ratio=%8.3f\n",time_str(rtk->sol.time,0),m,
          rtk->sol.ratio);
    
    /* narrow-lane to iono-free ambiguity */
    for (i=0;i<m;i++) {
        NC[i]=C1*lam1*N1[i]+C2*lam2*(N1[i]-NW[i]);
    }
    /* fixed solution */
    stat=fix_sol(rtk,sat1,sat2,NC,m);
    
    free(B1); free(N1); free(D); free(E); free(Q); free(NC);
    
    return stat;
}
/* resolve integer ambiguity for ppp -----------------------------------------*/
extern int pppamb(rtk_t *rtk, const obsd_t *obs, int n, const nav_t *nav,
                  const double *azel)
{
    double elmask;
    int i,j,m=0,stat=0,*NW,*sat1,*sat2;
    
    if (n<=0||rtk->opt.ionoopt!=IONOOPT_IFLC||rtk->opt.nf<2) return 0;
    
    trace(3,"pppamb: time=%s n=%d\n",time_str(obs[0].time,0),n);
    
    elmask=rtk->opt.elmaskar>0.0?rtk->opt.elmaskar:rtk->opt.elmin;
    
    sat1=imat(n*n,1); sat2=imat(n*n,1); NW=imat(n*n,1);
    
    /* average LC */
    average_LC(rtk,obs,n,nav,azel);
    
    /* fix wide-lane ambiguity */
    for (i=0;i<n-1;i++) for (j=i+1;j<n;j++) {
        
        if (!rtk->ssat[obs[i].sat-1].vsat[0]||
            !rtk->ssat[obs[j].sat-1].vsat[0]||
            azel[1+i*2]<elmask||azel[1+j*2]<elmask) continue;
#if 0
        /* test already fixed */
        if (rtk->ambc[obs[i].sat-1].flags[obs[j].sat-1]&&
            rtk->ambc[obs[j].sat-1].flags[obs[i].sat-1]) continue;
#endif
        sat1[m]=obs[i].sat;
        sat2[m]=obs[j].sat;
        if (fix_amb_WL(rtk,nav,sat1[m],sat2[m],NW+m)) m++;
    }
    /* fix narrow-lane ambiguity */
    if (rtk->opt.modear==ARMODE_PPPAR) {
        stat=fix_amb_ROUND(rtk,sat1,sat2,NW,m);
    }
    else if (rtk->opt.modear==ARMODE_PPPAR_ILS) {
        stat=fix_amb_ILS(rtk,sat1,sat2,NW,m);
    }
    free(sat1); free(sat2); free(NW);
    
    return stat;
}
