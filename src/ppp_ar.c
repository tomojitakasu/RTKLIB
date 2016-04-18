/*------------------------------------------------------------------------------
* ppp_ar.c : ppp ambiguity resolution
*
* reference :
*    [1] H.Okumura, C-gengo niyoru saishin algorithm jiten (in Japanese),
*        Software Technology, 1991
*
*          Copyright (C) 2012-2015 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/03/11  1.0  new
*           2015/05/15  1.1  refine complete algorithms
*           2015/05/31  1.2  delete WL-ambiguity resolution by ILS
*                            add PAR (partial ambiguity resolution)
*           2015/11/26  1.3  support option opt->pppopt=-TRACE_AR
*           2015/12/25  1.4  disable GPS-QZS ambiguity resolution
*                            fix problem on zero-divide for ratio-factor
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define MIN_AMB_RES 4         /* min number of ambiguities for ILS-AR */
#define MIN_ARC_GAP 300.0     /* min arc gap (s) */
#define CONST_AMB   0.003     /* constraint to fixed ambiguity */

#define LOG_PI      1.14472988584940017 /* log(pi) */
#define SQRT2       1.41421356237309510 /* sqrt(2) */

#define MIN(x,y)    ((x)<(y)?(x):(y))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define SQR(x)      ((x)*(x))
#define ROUND(x)    (int)floor((x)+0.5)
#define SWAP_I(x,y) do {int    _tmp=x; x=y; y=_tmp;} while (0)
#define SWAP_D(x,y) do {double _tmp=x; x=y; y=_tmp;} while (0)

/* number and index of ekf states */
#define NF(opt)     ((opt)->ionoopt==IONOOPT_IFLC?1:(opt)->nf)
#define NP(opt)     ((opt)->dynamics?9:3)
#define NC(opt)     (NSYS)
#define NT(opt)     ((opt)->tropopt<TROPOPT_EST?0:((opt)->tropopt==TROPOPT_EST?1:3))
#define NI(opt)     ((opt)->ionoopt==IONOOPT_EST?MAXSAT:0)
#define ND(opt)     ((opt)->nf>=3?1:0)
#define NR(opt)     (NP(opt)+NC(opt)+NT(opt)+NI(opt)+ND(opt))
#define IB(s,f,opt) (NR(opt)+MAXSAT*(f)+(s)-1)

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
static double conf_func(int N, double B, double var)
{
    double x,p=1.0,sig=sqrt(var);
    int i;
    
    x=fabs(B-N);
    for (i=1;i<6;i++) {
        p-=f_erfc((i-x)/(SQRT2*sig))-f_erfc((i+x)/(SQRT2*sig));
    }
    return p;
}
/* generate satellite SD (single-difference) ---------------------------------*/
static int gen_sat_sd(rtk_t *rtk, const obsd_t *obs, int n, const int *exc,
                      const double *azel, int f, int *sat1, int *sat2, int *frq)
{
#if 0
    const int sys[]={SYS_GPS|SYS_QZS,SYS_GAL,SYS_CMP,0};
#else
    const int sys[]={SYS_GPS,SYS_GAL,SYS_CMP,0};
#endif
    double elmask,el[MAXOBS];
    int i,j,k,m,ns=0,sat[MAXOBS];
    
    elmask=MAX(rtk->opt.elmaskar,rtk->opt.elmin);
    
    for (i=0;sys[i];i++) { /* for each system */
        
        /* sort by elevation angle */
        for (j=m=0;j<n;j++) {
            if (exc[j]||!(satsys(obs[j].sat,NULL)&sys[i])||
                !rtk->ssat[obs[j].sat-1].vsat[f]||azel[1+j*2]<elmask) continue;
            if (obs[j].L[f]==0.0) continue;
            sat[m]=obs[j].sat;
            el[m++]=azel[1+j*2];
        }
        for (j=0;j<m-1;j++) for (k=j+1;k<m;k++) {
            if (el[j]>=el[k]) continue;
            SWAP_I(sat[j],sat[k]); SWAP_D(el[j],el[k]);
        }
        /* generate SD referenced to max elevation angle */
        for (j=1;j<m;j++) {
            sat1[ns]=sat[j];
            sat2[ns]=sat[0];
            frq[ns++]=f;
        }
    }
    return ns; /* # of SD */
}
/* filter EWL-ambiguity ------------------------------------------------------*/
static void filter_EWL(rtk_t *rtk, const obsd_t *obs, int n, const int *exc,
                       const nav_t *nav, const double *azel)
{
    const double *lam;
    ambc_t *amb;
    double lamE,lamN,EW,var,K;
    int i,sat,sys;
    
    for (i=0;i<n;i++) {
        sat=obs[i].sat;
        sys=satsys(sat,NULL);
        if (sys!=SYS_GPS&&sys!=SYS_QZS) continue;
        amb=rtk->ambc+sat-1;
        lam=nav->lam[sat-1];
        
        if (rtk->ssat[sat-1].slip[1]||rtk->ssat[sat-1].slip[2]||
            fabs(timediff(amb->epoch[0],obs[i].time))>MIN_ARC_GAP) {
            amb->n[1]=0;
        }
        if (exc[i]||azel[1+2*i]<rtk->opt.elmin||lam[1]==0.0||lam[2]==0.0||
            obs[i].L[1]==0.0||obs[i].P[1]==0.0||
            obs[i].L[2]==0.0||obs[i].P[2]==0.0) continue;
        
        /* EMW-LC and variance */
        lamE=lam[1]*lam[2]/(lam[2]-lam[1]);
        lamN=lam[1]*lam[2]/(lam[2]+lam[1]);
        EW=lamE*(obs[i].L[1]-obs[i].L[2])-
           lamN*(obs[i].P[1]/lam[1]+obs[i].P[2]/lam[2]);
        var=SQR(0.7*rtk->opt.eratio[0])*
            (SQR(rtk->opt.err[1])+SQR(rtk->opt.err[2]/sin(azel[1+2*i])));
        
        /* filter EWL-ambiguity */
        if (amb->n[1]<=0) {
            amb->LC[1]=EW;
            amb->LCv[1]=SQR(10.0);
            amb->n[1]=1;
            amb->epoch[1]=obs[i].time;
        }
        else if (SQR(EW-amb->LC[1])<=amb->LCv[1]*25.0) {
            K=amb->LCv[1]/(amb->LCv[1]+var);
            amb->LC[1]+=K*(EW-amb->LC[1]);
            amb->LCv[1]-=K*amb->LCv[1];
            amb->n[1]++;
            amb->epoch[1]=obs[i].time;
        }
    }
}
/* filter WL-ambiguity -------------------------------------------------------*/
static void filter_WL(rtk_t *rtk, const obsd_t *obs, int n, const int *exc,
                      const nav_t *nav, const double *azel)
{
    const double *lam;
    ambc_t *amb;
    double lamW,lamN,MW,var,K;
    int i,l,sat;
    
    for (i=0;i<n;i++) {
        sat=obs[i].sat;
        amb=rtk->ambc+sat-1;
        lam=nav->lam[sat-1];
        l=satsys(sat,NULL)==SYS_GAL?2:1; /* L1/L2 or L1/L5 */
        
        if (rtk->ssat[sat-1].slip[0]||rtk->ssat[sat-1].slip[l]||
            fabs(timediff(amb->epoch[0],obs[i].time))>MIN_ARC_GAP) {
            amb->n[0]=0;
        }
        if (exc[i]||azel[1+2*i]<rtk->opt.elmin||lam[0]==0.0||lam[l]==0.0||
            obs[i].L[0]==0.0||obs[i].P[0]==0.0||
            obs[i].L[l]==0.0||obs[i].P[l]==0.0) continue;
        
        /* MW-LC and variance */
        lamW=lam[0]*lam[l]/(lam[l]-lam[0]);
        lamN=lam[0]*lam[l]/(lam[l]+lam[0]);
        MW=lamW*(obs[i].L[0]-obs[i].L[l])-
           lamN*(obs[i].P[0]/lam[0]+obs[i].P[l]/lam[l]);
        var=SQR(0.7*rtk->opt.eratio[0])*
            (SQR(rtk->opt.err[1])+SQR(rtk->opt.err[2]/sin(azel[1+2*i])));
        
        /* filter WL-ambiguity */
        if (amb->n[0]<=0) {
            amb->LC[0]=MW;
            amb->LCv[0]=SQR(10.0);
            amb->n[0]=1;
            amb->epoch[0]=obs[i].time;
        }
        else if (SQR(MW-amb->LC[0])<=amb->LCv[0]*25.0) {
            K=amb->LCv[0]/(amb->LCv[0]+var);
            amb->LC[0]+=K*(MW-amb->LC[0]);
            amb->LCv[0]-=K*amb->LCv[0];
            amb->n[0]++;
            amb->epoch[0]=obs[i].time;
        }
    }
}
/* ambiguity resolution by WL/NL for iono-free LC ----------------------------*/
static int ppp_amb_IFLC(rtk_t *rtk, const obsd_t *obs, int n, int *exc,
                        const nav_t *nav, const double *azel, double *x,
                        double *P)
{
    const double *lam;
    ambc_t *amb1,*amb2;
    double lamN,lamW,lamE,C1,C2,Be,Bw,B1,varw,var1;
    double v[MAXOBS],R[MAXOBS*MAXOBS]={0},*H,*thres=rtk->opt.thresar;
    int i,j,k,l,info,ns,nw=0,na=0,Ne,Nw,N1,sat1[MAXOBS],sat2[MAXOBS],frq[MAXOBS];
    
    /* filter EWL-ambiguity */
    filter_EWL(rtk,obs,n,exc,nav,azel);
    
    /* filter WL-ambiguity */
    filter_WL(rtk,obs,n,exc,nav,azel);
    
    /* generate satellite SD */
    if (!(ns=gen_sat_sd(rtk,obs,n,exc,azel,0,sat1,sat2,frq))) return 0;
    
    H=zeros(rtk->nx,ns);
    
    for (i=0;i<ns;i++) {
        lam=nav->lam[sat1[i]-1];
        l=satsys(sat1[i],NULL)==SYS_GAL?2:1; /* L1/L2 or L1/L5 */
        lamE=lam[1]*lam[2]/(lam[2]-lam[1]);
        lamW=lam[0]*lam[l]/(lam[l]-lam[0]);
        lamN=lam[0]*lam[l]/(lam[l]+lam[0]);
        C1= SQR(lam[l])/(SQR(lam[l])-SQR(lam[0]));
        C2=-SQR(lam[0])/(SQR(lam[l])-SQR(lam[0]));
        
        j=IB(sat1[i],0,&rtk->opt);
        k=IB(sat2[i],0,&rtk->opt);
        amb1=rtk->ambc+sat1[i]-1;
        amb2=rtk->ambc+sat2[i]-1;
        if (!amb1->n[0]||!amb2->n[0]) continue;
        
        if (amb1->n[1]&&amb2->n[1]) {
            Be=(amb1->LC[1]-amb2->LC[1])/lamE;
            Ne=ROUND(Be);
#if 0
            trace(2,"%s sat=%2d-%2d:Be=%13.4f Fe=%7.4f\n",time_str(obs[0].time,0),
                  sat1[i],sat2[i],Be,Be-Ne);
#endif
        }
        /* round WL- and L1-ambiguity */
        Bw=(amb1->LC[0]-amb2->LC[0])/lamW;
        Bw+=nav->wlbias[sat1[i]-1]-nav->wlbias[sat2[i]-1]; /* correct WL-bias */
        Nw=ROUND(Bw);
        B1=(x[j]-x[k]+C2*lam[l]*Nw)/lamN;
        N1=ROUND(B1);
        varw=(amb1->LCv[0]+amb2->LCv[0])/SQR(lamW);
        var1=(P[j+j*rtk->nx]+P[k+k*rtk->nx]-2.0*P[j+k*rtk->nx])/SQR(lamN);
        
        if (conf_func(Nw,Bw,varw)<thres[1]||fabs(Nw-Bw)>thres[2]) continue;
        nw++;
        if (conf_func(N1,B1,var1)<thres[1]||fabs(N1-B1)>thres[3]) continue;
        
        /* constraint to fixed LC-ambiguty */
        H[j+na*rtk->nx]= 1.0;
        H[k+na*rtk->nx]=-1.0;
        v[na++]=C1*lam[0]*N1+C2*lam[l]*(N1-Nw)-(rtk->x[j]-rtk->x[k]);
        
        /* update fix flags */
        rtk->ssat[sat1[i]-1].fix[0]=rtk->ssat[sat2[i]-1].fix[0]=2;
    }
    rtk->sol.age  =(float)nw; /* # of WL-fixed */
    rtk->sol.ratio=(float)na; /* # of NL-fixed */
    
    if (na<=0) {
        free(H);
        return 0;
    }
    for (i=0;i<na;i++) R[i+i*na]=SQR(CONST_AMB);
    
    /* update states with fixed LC-ambiguity constraints */
    if ((info=filter(x,P,H,v,R,rtk->nx,na))) {
        trace(1,"filter error (info=%d)\n",info);
        free(H);
        return 0;
    }
    free(H);
    return 1;
}
/* write debug trace for PAR -------------------------------------------------*/
static void write_trace1(rtk_t *rtk, const double *Z, const double *a,
                         const double *Q, int na, const int *sat1,
                         const int *sat2, const int *frq)
{
    const char freq[]="125678";
    char buff[1024],s[32],*p=buff;
    int i,j;
    
    trace(2,"EPOCH=%s NFIX=%d\n",time_str(rtk->sol.time,0),rtk->nfix);
    
    for (i=0,p=buff;i<na;i++) {
        satno2id(sat1[i],s); p+=sprintf(p,"%s ",s);
    }
    trace(2,"     %s          Z*a     STD\n",buff);
    for (i=0,p=buff;i<na;i++) {
        satno2id(sat2[i],s); p+=sprintf(p,"%s ",s);
    }
    trace(2,"     %s         (cyc)   (cyc)\n",buff);
    for (i=0,p=buff;i<na;i++) {
        p+=sprintf(p,"L%c  ",freq[frq[i]]);
    }
    trace(2,"      %s\n",buff);
    for (i=na-1;i>=0;i--) {
        p=buff;
        p+=sprintf(p,"%3d: ",na-i);
        for (j=0;j<na;j++) p+=sprintf(p,"%3.0f ",Z[j+i*na]);
        p+=sprintf(p,"%14.3f %7.3f",a[i],sqrt(Q[i+i*na]));
        trace(2,"%s\n",buff);
    }
    trace(2,"%3s: %7s %9s (%9s/%9s) [ N1 N2 ... NN ]\n","FIX","STD-POS","RATIO",
          "S1","S2");
}
static void write_trace2(rtk_t *rtk, const double *x, const double *P,
                         const double *a, const double *N, const double *D,
                         int na, const double *s)
{
    double *xp,*Pp,b[256],R[256*256]={0},std[3];
    char buff[1024],*p=buff;
    int i;
    
    xp=mat(rtk->nx,1); Pp=mat(rtk->nx,rtk->nx);
    matcpy(xp,x,rtk->nx,1);
    matcpy(Pp,P,rtk->nx,rtk->nx);
    for (i=0;i<na;i++) {
        b[i]=N[i]-a[i];
        R[i+i*na]=SQR(CONST_AMB);
    }
    for (i=na-1;i>=0;i--) {
        p+=sprintf(p,"%s%d",i==na-1?"":" ",(int)N[i]);
    }
    if (!filter(xp,Pp,D,b,R,rtk->nx,na)) {
        for (i=0;i<3;i++) std[i]=sqrt(Pp[i+i*rtk->nx]);
        trace(2,"%3d: %7.3f %9.3f (%9.3f/%9.3f) [%s]\n",na,norm(std,3),
              MIN(99999.999,s[1]/s[0]),s[0],s[1],buff);
    }
    free(xp); free(Pp);
}
/* decorrelate search space --------------------------------------------------*/
static int decorr_space(rtk_t *rtk, double *a, double *Q, double *D, int na,
                        const int *sat1, const int *sat2, const int *frq)
{
    double *W=mat(na,rtk->nx),*Z=eye(na);
    
    /* lambda reduction */
    if (lambda_reduction(na,Q,Z)) {
        free(W); free(Z);
        return 0;
    }
    /* a=Z'*a, Q=Z'*Q*Z, D'=Z'*D' */
    matmul("TN",na,1,na,1.0,Z,a,0.0,W);
    matcpy(a,W,na,1);
    matmul("TN",na,na,na,1.0,Z,Q,0.0,W);
    matmul("NN",na,na,na,1.0,W,Z,0.0,Q);
    matmul("NN",rtk->nx,na,na,1.0,D,Z,0.0,W);
    matcpy(D,W,rtk->nx,na);
    
    if (strstr(rtk->opt.pppopt,"-TRACE_AR")) {
        write_trace1(rtk,Z,a,Q,na,sat1,sat2,frq);
    }
    free(W); free(Z);
    return 1;
}
/* shrink search space -------------------------------------------------------*/
static int shrink_space(double *a, double *Q, double *H, int is, int na, int nx)
{
    int i,j,n=0,*index=imat(na,1);
    
    for (i=is;i<na;i++) {
        index[n++]=i;
    }
    for (i=0;i<n;i++) {
        a[i]=a[index[i]];
        for (j=0;j<n;j++) Q[j+i*n]=Q[index[j]+index[i]*na];
        matcpy(H+i*nx,H+index[i]*nx,nx,1);
    }
    free(index);
    return n;
}
/* freq-ambiguity resolution by ILS ------------------------------------------*/
static int ppp_amb_ILS_FRQ(rtk_t *rtk, const obsd_t *obs, int n, const int *exc,
                           const nav_t *nav, const double *azel,
                           const double *x, const double *P, double *D,
                           double *a, double *N)
{
    const double *lam,thres_fact[]={3.0,2.5,2.0,1.5,1.2};
    double *W,*Q,s[2]={0},thres=0.0;
    int i,j,k,na=0,sat1[MAXOBS*NFREQ],sat2[MAXOBS*NFREQ],frq[MAXOBS*NFREQ];
    int trace_AR=0;
    
    if (strstr(rtk->opt.pppopt,"-TRACE_AR")) trace_AR=1;
    
    /* generate satellite SD */
    for (i=0;i<rtk->opt.nf;i++) {
        if (rtk->opt.freqopt==1&&i==1) continue;
        na+=gen_sat_sd(rtk,obs,n,exc,azel,i,sat1+na,sat2+na,frq+na);
    }
    if (na<=0) return 0;
    
    /* freq-ambiguity SD-matrix */
    for (i=0;i<na;i++) {
        lam=nav->lam[sat1[i]-1];
        j=IB(sat1[i],frq[i],&rtk->opt);
        k=IB(sat2[i],frq[i],&rtk->opt);
        D[j+i*rtk->nx]= 1.0/lam[frq[i]];
        D[k+i*rtk->nx]=-1.0/lam[frq[i]];
    }
    /* a=D'*x, Q=D'*P*D */
    W=mat(rtk->nx,na); Q=mat(na,na);
    matmul("TN",na,1,rtk->nx,1.0,D,x,0.0,a);
    matmul("TN",na,rtk->nx,rtk->nx,1.0,D,P,0.0,W);
    matmul("NN",na,na,rtk->nx,1.0,W,D,0.0,Q);
    
    /* decorrelate search space */
    if (!decorr_space(rtk,a,Q,D,na,sat1,sat2,frq)) {
        free(W); free(Q);
        return 0;
    }
    for (i=0;i<rtk->opt.armaxiter&&na>=MIN_AMB_RES;i++) {
        
        s[0]=s[1]=0.0;
        
        /* integer least-square (a->N) */
        if (lambda_search(na,2,a,Q,N,s)||s[0]<=0.0) {
            free(W); free(Q);
            return 0;
        }
        if (trace_AR) {
            write_trace2(rtk,x,P,a,N,D,na,s);
        }
        thres=rtk->opt.thresar[0];
        if (na-MIN_AMB_RES<5) thres*=thres_fact[na-MIN_AMB_RES];
        
        /* validation by ratio-test */
        if (s[1]/s[0]>=thres) break;
        
        /* shrink search space */
        na=shrink_space(a,Q,D,1,na,rtk->nx);
    }
    free(W); free(Q);
    
    rtk->sol.ratio=s[0]==0.0?0.0:(float)MIN(s[1]/s[0],999.9);
    rtk->sol.thres=(float)thres;
    
    if (i>=rtk->opt.armaxiter||na<MIN_AMB_RES) return 0;
    
    /* update fix flags */
    for (i=0;i<na;i++) {
        rtk->ssat[sat1[i]-1].fix[frq[i]]=rtk->ssat[sat2[i]-1].fix[frq[i]]=2;
    }
    return na;
}
/* ambiguity resolution by ILS (integer-least-square) ------------------------*/
static int ppp_amb_ILS(rtk_t *rtk, const obsd_t *obs, int n, int *exc,
                       const nav_t *nav, const double *azel, double *x,
                       double *P)
{
    double *D,*R,a[MAXOBS*NFREQ],N[MAXOBS*NFREQ*2];
    int i,na,info;
    
    D=zeros(rtk->nx,n*NF(&rtk->opt));
    
    /* freq-ambiguity resolution */
    if (!(na=ppp_amb_ILS_FRQ(rtk,obs,n,exc,nav,azel,x,P,D,a,N))) {
        free(D);
        return 0;
    }
    R=zeros(na,na);
    
    /* update states with integer ambiguity constraints */
    for (i=0;i<na;i++) {
        a[i]=N[i]-a[i];
        R[i+i*na]=SQR(CONST_AMB);
    }
    if ((info=filter(x,P,D,a,R,rtk->nx,na))) {
        trace(1,"filter error (info=%d)\n",info);
    }
    free(D); free(R);
    return info?0:1;
}
/* ambiguity resolution in ppp -----------------------------------------------*/
extern int ppp_ar(rtk_t *rtk, const obsd_t *obs, int n, int *exc,
                  const nav_t *nav, const double *azel, double *x, double *P)
{
    if (n<=0||rtk->opt.modear<ARMODE_CONT) return 0;
    
    /* ambiguity resolution by WL/NL for iono-free LC */
    if (rtk->opt.ionoopt==IONOOPT_IFLC) {
        return ppp_amb_IFLC(rtk,obs,n,exc,nav,azel,x,P);
    }
    /* ambiguity resolution by ILS */
    else {
        return ppp_amb_ILS(rtk,obs,n,exc,nav,azel,x,P);
    }
}
