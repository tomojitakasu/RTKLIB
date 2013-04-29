/*-----------------------------------------------------------------------------
* prec2nav.c : generate navigation data by precise ephemeris
*
*          Copyright (C) 2012 by T.TAKASU, All rights reserved.
*
* references :
*     [1] IS-GPS-200E, Navstar GPS Space Segment/Navigation User Interfaces,
*         June 8, 2010
*
* version : $Revision:$ $Date:$
* history : 2012/11/26  1.0  new
*----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define PRG_NAME    "PREC2NAV"       /* program name */
#define MAX_PATH    256              /* max number of input file paths */
#define MAX_ITER    20               /* max number of iteration */
#define MAX_RMS     10.0             /* max rms of residuals (m) */
#define EPH_TINT    300.0            /* interval of precise ephemeris (s) */
#define NX          17               /* number of ephemeris parameters */
#define INIT_DAMP   1E-6             /* initial damping factor */

#define MU_GPS      3.9860050E14     /* earth gravitational constant for GPS */
#define MU_GAL      3.986004418E14   /* earth gravitational constant for GAL */

#define SQR(x)      ((x)*(x))

/* print usage ---------------------------------------------------------------*/
static const char *help[]={
    "",
    "usage: prec2nav [options] [infile]",
    "",
    "options:",
    "  -td  y/m/d        start date (y=year,m=month,d=date in GPST)",
    "  -tt  h:m:s        start time (h=hour,m=minute,s=sec in GPST)",
    "  -ts  tspan        time span (hrs)",
    "  -fit tfit         fit interval (hrs)",
    "  -ns  nspan        number of time spans",
    "  -out outfile      output rinex nav file",
    "  -ant antfile      antex satellite antenna parameters",
    "  -ver ver          output rinex version",
    "  -sat sat[,sat...] select satlelites (G:GPS,E:GAL,J:QZS,C:CMP)",
    "  -exc sat[,sat...] exclude satlelites",
    "  -iod iod          initial iode and iodc",
    "  -rms rms          threashold of residuals rms (m)",
    "  -csv              output csv file instead of rinex nav",
    "  -opt opt          options (1:diff-approx+2:output-rms+4:output-error)",
    ""
};
static void print_help(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* read precise ephemeris ----------------------------------------------------*/
static int read_peph(gtime_t epoch, double tspan, double tfit,
                     const char *infile, const char *antfile, nav_t *nav)
{
    gtime_t ts,te;
    char *paths[MAX_PATH];
    int i,n;
    
    for (i=0;i<MAX_PATH;i++) {
        if (!(paths[i]=(char *)malloc(1024))) return 0;
    }
    ts=timeadd(epoch,-tfit*1800.0);
    te=timeadd(epoch,tspan*3600.0+tfit*1800.0);
    
    n=reppaths(infile,paths,MAX_PATH,ts,te,"","");
    
    for (i=0;i<n;i++) {
        fprintf(stderr,"reading...  %s\n",paths[i]);
        readsp3(paths[i],nav,0);
    }
    if (*antfile) {
        fprintf(stderr,"reading...  %s\n",antfile);
        if (!readsap(antfile,epoch,nav)) {
            fprintf(stderr,"antenna file read error\n");
        }
    }
    for (i=0;i<MAX_PATH;i++) free(paths[i]);
    
    return nav->ne>0;
}
/* solve kepler equaiton -----------------------------------------------------*/
static double kepler(double e, double M)
{
    double E=M,Ek;
    int i;
    
    for (i=0;i<10;i++) {
        Ek=E; E=E-(E-e*sin(E)-M)/(1.0-e*cos(E));
        if (fabs(E-Ek)<1E-14) break;
    }
    return E;
}
/* satellite position and clock by navigation data (ref [1] table 20-IV) -----*/
static void nav_pos(gtime_t time, const eph_t *eph, double *rs, double *dts)
{
    double mu,a,e,i0,OMG0,omg,M0,dn,OMGd,idot,Cus,Cuc,Crs,Crc,Cis,Cic;
    double tk,a3,e2,M,E,sinE,cosE,p,sin2p,cos2p,du,dr,di,u,r,i,OMG,sinu,cosu;
    double sini,cosi,sinO,cosO;
    
    mu=satsys(eph->sat,NULL)==SYS_GAL?MU_GAL:MU_GPS;
    a=eph->A; e=eph->e; i0=eph->i0; OMG0=eph->OMG0; omg=eph->omg; M0=eph->M0;
    dn=eph->deln; OMGd=eph->OMGd; idot=eph->idot; Cus=eph->cus; Cuc=eph->cuc;
    Crs=eph->crs; Crc=eph->crc; Cis=eph->cis; Cic=eph->cic;
    a3=a*a*a; e2=e*e;
    
    tk=timediff(time,eph->toe);
    M=M0+(sqrt(mu/a3)+dn)*tk;
    E=kepler(e,M);
    sinE=sin(E); cosE=cos(E);
    p=atan2(sqrt(1.0-e2)*sinE,cosE-e)+omg;
    sin2p=sin(2.0*p);
    cos2p=cos(2.0*p);
    du=Cus*sin2p+Cuc*cos2p;
    dr=Crs*sin2p+Crc*cos2p;
    di=Cis*sin2p+Cic*cos2p;
    u=p+du;
    r=a*(1.0-e*cosE)+dr;
    i=i0+idot*tk+di;
    OMG=OMG0+(OMGd-OMGE)*tk-OMGE*eph->toes;
    sinu=sin(u); sini=sin(i); sinO=sin(OMG);
    cosu=cos(u); cosi=cos(i); cosO=cos(OMG);
    rs[0]=r*(cosu*cosO-sinu*cosi*sinO);
    rs[1]=r*(cosu*sinO+sinu*cosi*cosO);
    rs[2]=r*sinu*sini;
    tk=timediff(time,eph->toc);
    dts[0]=eph->f0+eph->f1*tk+eph->f2*tk*tk;
    dts[1]=eph->f1+eph->f2*tk;
}
/* satellite reference position and clock by precise ephemeris ---------------*/
static int ref_pos(gtime_t time, int sat, const nav_t *nav, double *rs,
                   double *dts)
{
    double var;
    
    /* antenna phase center position by precise ephemeris */
    if (!peph2pos(time,sat,nav,1,rs,dts,&var)) return 0;
    
    /* exclude relativity correction */
    dts[0]+=2.0*dot(rs,rs+3,3)/CLIGHT/CLIGHT;
    
    return 1;
}
/* osculating orbital elements -----------------------------------------------*/
static int obtele(const double *rs, double *a, double *e, double *i,
                  double *OMG, double *omg, double *M)
{
    double rr,v2,h[3],w[3],n,E,u;
    
    if ((rr=norm(rs,3))<1E-6||(v2=dot(rs+3,rs+3,3))<1E-16) return 0;
    cross3(rs,rs+3,h);
    normv3(h,w);
    *i=atan2(norm(w,2),w[2]);
    *OMG=atan2(w[0],-w[1]);
    *a=1.0/(2.0/rr-v2/MU_GPS);
    *e=sqrt(1.0-dot(h,h,3)/MU_GPS/(*a));
    n=sqrt(MU_GPS/SQR(*a)/(*a));
    E=atan2(dot(rs,rs+3,3)/SQR(*a)/n,1.0-rr/(*a));
    *M=E-(*e)*sin(E);
    u=atan2(rs[2],-rs[0]*w[1]+rs[1]*w[0]);
    *omg=u-atan2(sqrt(1-SQR(*e))*sin(E),cos(E)-(*e));
    return 1;
}
/* set initial navigation data -----------------------------------------------*/
static int init_nav(gtime_t toe, int sat, int iod, double fit, eph_t *eph,
                    const nav_t *nav)
{
    double a,e,i,OMG,omg,M,rs[6],dts[2];
    char str[32];
    int j;
    
    /* satellite eci position/velocity at toe */
    if (!ref_pos(toe,sat,nav,rs,dts)) {
        satno2id(sat,str);
        fprintf(stderr,"%s TOE=%s  NO EPHEMERIS\n",str,time_str(toe,0));
        return 0;
    }
    rs[3]-=OMGE*rs[1];
    rs[4]+=OMGE*rs[0];
    
    /* osculating orbital elements */
    if (!obtele(rs,&a,&e,&i,&OMG,&omg,&M)) {
        fprintf(stderr,"orbital elements error: sat=%2d\n",sat);
        return 0;
    }
    /* set initial navigation data */
    eph->sat=sat;
    eph->iode=eph->iodc=iod%1024;
    eph->sva=eph->svh=eph->code=eph->flag=0;
    eph->toe=eph->toc=toe;
    eph->ttr=timeadd(toe,fit*1800.0);
    eph->toes=time2gpst(toe,&eph->week);
    eph->fit=fit;
    eph->A=a;
    eph->e=e;
    eph->i0=i;
    eph->OMG0=OMG+OMGE*eph->toes;
    eph->omg=omg;
    eph->M0=M;
    eph->deln=eph->OMGd=eph->idot=0.0;
    eph->cus=eph->cuc=eph->crs=eph->crc=eph->cis=eph->cic=0.0;
    eph->f0=dts[0];
    eph->f1=dts[1];
    eph->f2=0.0;
    for (j=0;j<4;j++) eph->tgd[j]=0.0;
    return 1;
}
/* partial derivatives of position and clock wrt navigation data -------------*/
static void partial_p(gtime_t epoch, const eph_t *eph, int opt, double *H)
{
    double mu,a,e,i0,OMG0,omg,M0,dn,OMGd,idot,Cus,Cuc,Crs,Crc,Cis,Cic;
    double tk,a3,a5,e2,M,E,sinE,cosE,p,sin2p,cos2p,du,dr,di,u,r,i,OMG,sinu,cosu;
    double sini,cosi,sinO,cosO,dMda,dMddn,dEde,dEdM0,dEddn,dAdE,dAde,dpdA,dpdE;
    double dpda,dpde,dpdM0,dpddn,duda,dude,dudomg,dudM0,duddn,drda,drde,drdomg;
    double drdM0,drddn,dida,dide,didomg,didM0,diddn,dxda,dxde,dxdi0,dxdOMG0;
    double dxdomg,dxdM0,dxddn,dxdidot,dxdOMGd,dxdCus,dxdCuc,dxdCrs,dxdCrc;
    double dxdCis,dxdCic,dyda,dyde,dydi0,dydOMG0,dydomg,dydM0,dyddn,dydidot;
    double dydOMGd,dydCus,dydCuc,dydCrs,dydCrc,dydCis,dydCic,dzda,dzde,dzdi0;
    double dzdOMG0,dzdomg,dzdM0,dzddn,dzdidot,dzdOMGd,dzdCus,dzdCuc,dzdCrs;
    double dzdCrc,dzdCis,dzdCic,dtdaf0,dtdaf1,X,Y,V,W;
    int j;
    
    mu=satsys(eph->sat,NULL)==SYS_GAL?MU_GAL:MU_GPS;
    a=eph->A; e=eph->e; i0=eph->i0; OMG0=eph->OMG0; omg=eph->omg; M0=eph->M0;
    dn=eph->deln; OMGd=eph->OMGd; idot=eph->idot; Cus=eph->cus; Cuc=eph->cuc;
    Crs=eph->crs; Crc=eph->crc; Cis=eph->cis; Cic=eph->cic;
    a3=a*a*a; a5=a3*a*a; e2=e*e;
    
    tk=timediff(epoch,eph->toe);
    M=M0+(sqrt(mu/a3)+dn)*tk;
    E=kepler(e,M);
    sinE=sin(E); cosE=cos(E);
    p=atan2(sqrt(1.0-e2)*sinE,cosE-e)+omg;
    sin2p=sin(2.0*p);
    cos2p=cos(2.0*p);
    du=Cus*sin2p+Cuc*cos2p;
    dr=Crs*sin2p+Crc*cos2p;
    di=Cis*sin2p+Cic*cos2p;
    u=p+du;
    r=a*(1.0-e*cosE)+dr;
    i=i0+idot*tk+di;
    OMG=OMG0+(OMGd-OMGE)*tk-OMGE*eph->toes;
    sinu=sin(u); sini=sin(i); sinO=sin(OMG);
    cosu=cos(u); cosi=cos(i); cosO=cos(OMG);
    
    dMda=-1.5*sqrt(mu/a5)*tk;
    dMddn=tk;
    dEde=sinE/(1.0-e*cosE);
    dEdM0=1.0/(1.0-e*cosE);
    dEddn=tk/(1.0-e*cosE);
    dAdE=sqrt(1.0-e2)*(1-e*cosE)/SQR(cosE-e);
    dAde=sqrt(1.0-e2)/(cosE-e)*
         (-e*sinE/(1-e2)+sinE/(cosE-e)*(1.0+sinE*dEde)+cosE*dEde);
    dpdA=SQR((cosE-e)/(1.0-e*cosE));
    dpdE=sqrt(1.0-e2)/(1.0-e*cosE);
    dpda=dpdE*dEdM0*dMda;
    dpde=dpdA*dAde;
    dpdM0=dpdE*dEdM0;
    dpddn=dpdE*dEddn;
    
    /* d{u,r,i}/d{a,e,omg,M0,dn} */
    duda=(1.0+2.0*(Cus*cos2p-Cuc*sin2p))*dpda;
    dude=(1.0+2.0*(Cus*cos2p-Cuc*sin2p))*dpde;
    dudomg=1.0+2.0*(Cus*cos2p-Cuc*sin2p);
    dudM0=(1.0+2.0*(Cus*cos2p-Cuc*sin2p))*dpdM0;
    duddn=(1.0+2.0*(Cus*cos2p-Cuc*sin2p))*dpddn;
    drda=1.0-e*cosE+a*e*sinE*dEdM0*dMda+2.0*(Crs*cos2p-Crc*sin2p)*dpda;
    drde=a*(-cosE+e*sinE*dEde)+2.0*(Crs*cos2p-Crc*sin2p)*dpde;
    drdomg=1.0+2.0*(Crs*cos2p-Crc*sin2p);
    drdM0=a*e*sinE*dEdM0+(1.0+2.0*(Crs*cos2p-Crc*sin2p))*dpdM0;
    drddn=a*e*sinE*dEddn+(1.0+2.0*(Crs*cos2p-Crc*sin2p))*dpddn;
    dida=2.0*(Cis*cos2p-Cic*sin2p)*dpda;
    dide=2.0*(Cis*cos2p-Cic*sin2p)*dpde;
    didomg=2.0*(Cis*cos2p-Cic*sin2p);
    didM0=2.0*(Cis*cos2p-Cic*sin2p)*dpdM0;
    diddn=2.0*(Cis*cos2p-Cic*sin2p)*dpddn;
    X=cosu*cosO-sinu*cosi*sinO;
    Y=cosu*sinO+sinu*cosi*cosO;
    V=-sinu*cosO-cosu*cosi*sinO;
    W=-sinu*sinO+cosu*cosi*cosO;
    
    /* d{x,y,z}/d{a,e,i0,OMG0,omg,M0,dn,idot,OMGd,Cus,Cuc,Crs,Crc,Cis,Cic} */
    dxda=X*drda+r*(V*duda+sinu*sini*sinO*dida);
    dxde=X*drde+r*(V*dude+sinu*sini*sinO*dide);
    dxdi0=r*sinu*sini*sinO;
    dxdOMG0=-r*Y;
    dxdomg=X*drdomg+r*(V*dudomg+sinu*sini*sinO*didomg);
    dxdM0=X*drdM0+r*(V*dudM0+sinu*sini*sinO*didM0);
    dxddn=X*drddn+r*(V*duddn+sinu*sini*sinO*diddn);
    dxdidot=r*sinu*sini*sinO*tk;
    dxdOMGd=-r*Y*tk;
    dxdCus=r*V*sin2p;
    dxdCuc=r*V*cos2p;
    dxdCrs=X*sin2p;
    dxdCrc=X*cos2p;
    dxdCis=r*sinu*sini*sinO*sin2p;
    dxdCic=r*sinu*sini*sinO*cos2p;
    dyda=Y*drda+r*(W*duda-sinu*sini*cosO*dida);
    dyde=Y*drde+r*(W*dude-sinu*sini*cosO*dide);
    dydi0=-r*sinu*sini*cosO;
    dydOMG0=r*X;
    dydomg=Y*drdomg+r*(W*dudomg-sinu*sini*cosO*didomg);
    dydM0=Y*drdM0+r*(W*dudM0-sinu*sini*cosO*didM0);
    dyddn=Y*drddn+r*(W*duddn-sinu*sini*cosO*diddn);
    dydidot=-r*sinu*sini*cosO*tk;
    dydOMGd=r*X*tk;
    dydCus=r*W*sin2p;
    dydCuc=r*W*cos2p;
    dydCrs=Y*sin2p;
    dydCrc=Y*cos2p;
    dydCis=-r*sinu*sini*cosO*sin2p;
    dydCic=-r*sinu*sini*cosO*cos2p;
    dzda=sinu*sini*drda+r*(cosu*sini*duda+sinu*cosi*dida);
    dzde=sinu*sini*drde+r*(cosu*sini*dude+sinu*cosi*dide);
    dzdi0=r*sinu*cosi;
    dzdOMG0=0.0;
    dzdomg=sinu*sini*drdomg+r*(cosu*sini*dudomg+sinu*cosi*didomg);
    dzdM0=sinu*sini*drdM0+r*(cosu*sini*dudM0+sinu*cosi*didM0);
    dzddn=sinu*sini*drddn+r*(cosu*sini*duddn+sinu*cosi*diddn);
    dzdidot=r*sinu*cosi*tk;
    dzdOMGd=0.0;
    dzdCus=r*cosu*sini*sin2p;
    dzdCuc=r*cosu*sini*cos2p;
    dzdCrs=sinu*sini*sin2p;
    dzdCrc=sinu*sini*cos2p;
    dzdCis=r*sinu*cosi*sin2p;
    dzdCic=r*sinu*cosi*cos2p;
    
    /* dcdts/d{af0,af1} */
    dtdaf0=CLIGHT;
    dtdaf1=CLIGHT*tk;
    
    for (j=0;j<4*NX;j++) H[j]=0.0;
    H[ 0]=dxda  ; H[ 1]=dxde  ; H[ 2]=dxdi0  ; H[ 3]=dxdOMG0; H[ 4]=dxdomg;
    H[ 5]=dxdM0 ; H[ 6]=dxddn ; H[ 7]=dxdOMGd; H[ 8]=dxdidot; H[ 9]=dxdCus;
    H[10]=dxdCuc; H[11]=dxdCrs; H[12]=dxdCrc ; H[13]=dxdCis ; H[14]=dxdCic;
    H[17]=dyda  ; H[18]=dyde  ; H[19]=dydi0  ; H[20]=dydOMG0; H[21]=dydomg;
    H[22]=dydM0 ; H[23]=dyddn ; H[24]=dydOMGd; H[25]=dydidot; H[26]=dydCus;
    H[27]=dydCuc; H[28]=dydCrs; H[29]=dydCrc ; H[30]=dydCis ; H[31]=dydCic;
    H[34]=dzda  ; H[35]=dzde  ; H[36]=dzdi0  ; H[37]=dzdOMG0; H[38]=dzdomg;
    H[39]=dzdM0 ; H[40]=dzddn ; H[41]=dzdOMGd; H[42]=dzdidot; H[43]=dzdCus;
    H[44]=dzdCuc; H[45]=dzdCrs; H[46]=dzdCrc ; H[47]=dzdCis ; H[48]=dzdCic;
    H[66]=dtdaf0; H[67]=dtdaf1;
}
/* partial derivatives of position and clock wrt navigation data ------------*/
static void partial_a(gtime_t epoch, const eph_t *eph, int opt, double *H)
{
    eph_t eph_m,eph_p;
    double rs_m[3],rs_p[3],dts_m[2],dts_p[2],dx,A[4*NX],E[4];
    int i,j;
    
    for (i=0;i<NX;i++) {
        eph_m=*eph; eph_p=*eph;
        switch (i) {
            case  0: dx=1E+1 ; eph_m.A   -=dx; eph_p.A   +=dx; break;
            case  1: dx=1E-7 ; eph_m.e   -=dx; eph_p.e   +=dx; break;
            case  2: dx=1E-6 ; eph_m.i0  -=dx; eph_p.i0  +=dx; break;
            case  3: dx=1E-6 ; eph_m.OMG0-=dx; eph_p.OMG0+=dx; break;
            case  4: dx=1E-6 ; eph_m.omg -=dx; eph_p.omg +=dx; break;
            case  5: dx=1E-6 ; eph_m.M0  -=dx; eph_p.M0  +=dx; break;
            case  6: dx=1E-8 ; eph_m.deln-=dx; eph_p.deln+=dx; break;
            case  7: dx=1E-8 ; eph_m.OMGd-=dx; eph_p.OMGd+=dx; break;
            case  8: dx=1E-8 ; eph_m.idot-=dx; eph_p.idot+=dx; break;
            case  9: dx=1E-6 ; eph_m.cus -=dx; eph_p.cus +=dx; break;
            case 10: dx=1E-6 ; eph_m.cuc -=dx; eph_p.cuc +=dx; break;
            case 11: dx=1E+1 ; eph_m.crs -=dx; eph_p.crs +=dx; break;
            case 12: dx=1E+1 ; eph_m.crc -=dx; eph_p.crc +=dx; break;
            case 13: dx=1E-6 ; eph_m.cis -=dx; eph_p.cis +=dx; break;
            case 14: dx=1E-6 ; eph_m.cic -=dx; eph_p.cic +=dx; break;
            case 15: dx=1E-8 ; eph_m.f0  -=dx; eph_p.f0  +=dx; break;
            case 16: dx=1E-11; eph_m.f1  -=dx; eph_p.f1  +=dx; break;
        }
        /* differential approximation */
        nav_pos(epoch,&eph_m,rs_m,dts_m);
        nav_pos(epoch,&eph_p,rs_p,dts_p);
        
        for (j=0;j<3;j++) H[i+j*NX]=(rs_p[j]-rs_m[j])/dx/2.0;
        H[i+3*NX]=CLIGHT*(dts_p[0]-dts_m[0])/dx/2.0;
    }
    /* output errors of differential approximation */
    if (opt&8) {
        partial_p(epoch,eph,0,A);
        for (i=0;i<NX;i++) {
            for (j=0;j<4;j++) E[j]=A[i+j*NX]?(H[i+j*NX]-A[i+j*NX])/A[i+j*NX]:0.0;
            fprintf(stderr,"%10.3e %10.3e %10.3e %10.3e\n",E[0],E[1],E[2],E[3]);
        }
    }
}
/* residuals of satellite positions ------------------------------------------*/
static int res_nav(eph_t *eph, const nav_t *nav, int opt, double *v, double *H,
                   double *rms)
{
    gtime_t epoch;
    double rs_n[6],rs_p[6],dts_n[2],dts_p[2];
    char sat[32];
    int i,j,n=eph->fit*3600.0/EPH_TINT+1,m=0;
    
    satno2id(eph->sat,sat);
    
    for (i=0;i<n;i++) {
        
        epoch=timeadd(eph->toe,EPH_TINT*(i-(n-1)/2));
        
        /* satellite position and clock bias */
        nav_pos(epoch,eph,rs_n,dts_n);
        
        /* reference position and clock bias */
        if (!ref_pos(epoch,eph->sat,nav,rs_p,dts_p)) continue;
        
        /* residuals */
        for (j=0;j<3;j++) v[j+m]=rs_p[j]-rs_n[j];
        v[3+m]=CLIGHT*(dts_p[0]-dts_n[0]);
        
        /* parital derivatives */
        if (opt&1) partial_a(epoch,eph,opt,H+m*NX);
        else       partial_p(epoch,eph,opt,H+m*NX);
        
        if (opt&4) {
            fprintf(stderr,"(%2d) %s T  =%s RES=%9.3f %9.3f %9.3f %9.3f\n",
                    i+1,sat,time_str(epoch,0),v[m],v[1+m],v[2+m],v[3+m]);
        }
        m+=4;
    }
    *rms=m>0?sqrt(dot(v,v,m)/m):0.0;
    return m;
}
/* correct navigation data ---------------------------------------------------*/
static void corr_nav(eph_t *eph, const double *x)
{
    eph->A   +=x[ 0]; eph->e  +=x[ 1]; eph->i0  +=x[ 2]; eph->OMG0+=x[ 3];
    eph->omg +=x[ 4]; eph->M0 +=x[ 5]; eph->deln+=x[ 6]; eph->OMGd+=x[ 7];
    eph->idot+=x[ 8]; eph->cus+=x[ 9]; eph->cuc +=x[10]; eph->crs +=x[11];
    eph->crc +=x[12]; eph->cis+=x[13]; eph->cic +=x[14];
    eph->f0  +=x[15]; eph->f1 +=x[16];
}
/* fit navigation data by gauss-newton ---------------------------------------*/
static int fit_nav_gn(eph_t *eph, const nav_t *nav, int opt, double max_rms)
{
    double *v,*H,*Q,x[NX],rms,rmsp=0.0;
    char sat[32];
    int i,n=eph->fit*3600.0/EPH_TINT+1,m,stat=1;
    
    v=mat(n*4,1); H=mat(NX,n*4); Q=mat(NX,NX);
    
    satno2id(eph->sat,sat);
    
    for (i=0;i<MAX_ITER;i++) {
        
        /* residuals */
        if ((m=res_nav(eph,nav,opt,v,H,&rms))<NX) {
            stat=0;
            break;
        }
        if (opt&2) {
            fprintf(stderr,"(%2d) %s TOE=%s RMS=%9.3f\n",i,sat,
                    time_str(eph->toe,0),rms);
        }
        if (fabs(rms-rmsp)<1E-3) break;
        
        rmsp=rms;
        
        /* least square estimation */
        if (lsq(H,v,NX,m,x,Q)) {
            stat=0;
            break;
        }
        /* correct navigation data */
        corr_nav(eph,x);
    }
    free(v); free(H); free(Q);
    
    if (!stat||i>=MAX_ITER||rms>max_rms) return 0;
    
    fprintf(stderr,"%s TOE=%s  RMS=%9.3f (GN)\n",sat,time_str(eph->toe,0),rms);
    
    return 1;
}
/* fit navigation data by levenberg-marquardt --------------------------------*/
static int fit_nav_lm(eph_t *eph, const nav_t *nav, int opt, double max_rms)
{
    eph_t ephp;
    double x[NX]={0},*v,*vp,*H,*Hp,b[NX],N[NX*NX],rms,rmsp=0.0,lam=INIT_DAMP;
    char sat[32];
    int i,j,n=eph->fit*3600.0/EPH_TINT+1,m,stat=1;
    
    v=mat(n*4,1); vp=mat(n*4,1); H=mat(NX,n*4); Hp=mat(NX,n*4);
    
    satno2id(eph->sat,sat);
    
    for (i=0;i<MAX_ITER;i++) {
        
        /* residuals */
        m=res_nav(eph,nav,opt,v,H,&rms);
        
        if (opt&2) {
            fprintf(stderr,"(%2d) %s TOE=%s RMS=%9.3f LAMBDA=%.1e\n",i,sat,
                    time_str(eph->toe,0),rms,lam);
        }
        if (fabs(rms-rmsp)<1E-3) break;
        
        if (rmsp==0.0||rms<rmsp) {
            lam/=10.0;
            ephp=*eph; matcpy(vp,v,n*4,1); matcpy(Hp,H,NX,n*4); rmsp=rms;
        }
        else {
            lam*=10.0;
            *eph=ephp; matcpy(v,vp,n*4,1); matcpy(H,Hp,NX,n*4);
        }
        /* damping least-square estimation */
        matmul("NN",NX,1 ,m,1.0,H,v,0.0,b);
        matmul("NT",NX,NX,m,1.0,H,H,0.0,N);
        for (j=0;j<NX;j++) N[j+j*NX]+=lam;
        
        if (solve("N",N,b,NX,1,x)) {
            stat=0;
            break;
        }
        /* correct navigation data */
        corr_nav(eph,x);
    }
    free(v); free(H); free(vp); free(Hp);
    
    if (!stat||i>=MAX_ITER||rms>max_rms) return 0;
    
    fprintf(stderr,"%s TOE=%s  RMS=%9.3f (LM)\n",sat,time_str(eph->toe,0),rms);
    return 1;
}
/* write navigation data header ----------------------------------------------*/
static int write_head(FILE *fp, int csv, const rnxopt_t *rnxopt, nav_t *nav)
{
    if (csv) {
        fprintf(fp,"%3s,%4s,%6s,%4s,%12s,%12s,%10s,%11s,%11s,%11s,%11s,%11s,"
                "%11s,%11s,%11s,%11s,%11s,%11s,%11s,%11s,%11s,%11s\n","sat",
                "week","Toe","IOD","A","e","i0","OMG0","omg","M0","deln","idot",
                "OMGd","Cus","Cuc","Crs","Crc","Cis","Cic","af0","af1","af2");
    }
    else if (!outrnxnavh(fp,rnxopt,nav)) {
        return 0;
    }
    return 1;
}
/* write navigation data -----------------------------------------------------*/
static int write_data(FILE *fp, int csv, const rnxopt_t *rnxopt, eph_t *eph)
{
    double toe;
    char str[32];
    int week;
    
    /* -pi <= OMG0,omg,M0 < pi */
    eph->OMG0=fmod(eph->OMG0+PI,2.0*PI)-PI;
    eph->omg =fmod(eph->omg +PI,2.0*PI)-PI;
    eph->M0  =fmod(eph->M0  +PI,2.0*PI)-PI;
    
    if (csv) {
        satno2id(eph->sat,str);
        toe=time2gpst(eph->toe,&week);
        fprintf(fp,"%-3s,%4d,%6.0f,%4d,%12.3f,%12.10f,%10.6f,%11.6f,%11.6f,"
                "%11.6f,%11.4e,%11.4e,%11.4e,%11.4e,%11.4e,%11.4e,%11.4e,"
                "%11.4e,%11.4e,%11.3f,%11.8f,%11.8f\n",str,week,toe,eph->iode,
                eph->A,eph->e,eph->i0*R2D,eph->OMG0*R2D,eph->omg*R2D,
                eph->M0*R2D,eph->deln*R2D,eph->idot*R2D,eph->OMGd*R2D,eph->cus,
                eph->cuc,eph->crs,eph->crc,eph->cis,eph->cic,eph->f0*1E9,
                eph->f1*1E9,eph->f2*1E9);
    }
    else if (!outrnxnavb(fp,rnxopt,eph)) {
        return 0;
    }
    return 1;
}
/* generate navigation data --------------------------------------------------*/
static int gen_nav(gtime_t epoch, double tspan, double tfit, double ver, 
                   int sys, const int *mask, int iod, double max_rms, int csv,
                   int opt, const char *infile, const char *antfile,
                   const char *outfile)
{
    FILE *fp=stdout;
    rnxopt_t rnxopt={{0}};
    nav_t nav={0};
    eph_t eph;
    gtime_t toe;
    char outpath[1024];
    int i,j;
    
    strcpy(rnxopt.prog,PRG_NAME);
    sprintf(rnxopt.comment[0],"SOURCE: %s",infile);
    rnxopt.rnxver=ver;
    rnxopt.navsys=sys;
    
    /* read precise ephemeris */
    if (!read_peph(epoch,tspan,tfit,infile,antfile,&nav)) {
        fprintf(stderr,"precise ephemeris read error: %s\n",infile);
        return 0;
    }
    reppath(outfile,outpath,epoch,"","");
    
    if (*outpath) {
        if (!(fp=fopen(outpath,"w"))) {
            fprintf(stderr,"output file open error: %s\n",outpath);
            return 0;
        }
        fprintf(stderr,"writing...  %s\n",outpath);
    }
    /* write navigation data header */
    if (!write_head(fp,csv,&rnxopt,&nav)) {
        fprintf(stderr,"output header error: %s\n",outpath);
        fclose(fp);
        return 0;
    }
    for (i=0;i<=(int)(tspan/tfit+1E-3);i++) {
        
        toe=timeadd(epoch,i*tfit*3600.0);
        
        for (j=0;j<MAXSAT;j++) {
            if (!mask[j]) continue;
            
            /* set initial navigation data */
            if (!init_nav(toe,j+1,iod+i,tfit,&eph,&nav)) continue;
            
            /* fit navigation data */
            if (!fit_nav_gn(&eph,&nav,opt,max_rms)&&
                !fit_nav_lm(&eph,&nav,opt,max_rms)) {
                continue;
            }
            /* write navigation data */
            if (!write_data(fp,csv,&rnxopt,&eph)) {
                fprintf(stderr,"output data error: %s\n",outpath);
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
    return 1;
}
/*------------------------------------------------------------------------------
*  name:
*    prec2nav - generate navigation data by precise ephemeris
*
*  synopsis:
*    prec2nav [options] [infile]
*
*  description:
*    Generate navigation data by curve fitting to satellite positions provided
*    by precise ephemerides. Specify a sp3 precise ephemeris file as infile.
*    If infile ommitted, igs%W%D.sp3 is assumed as infile. If outfile option
*    omitted, the results are output to stdout.
*
*  options:
*    -td  y/m/d        start date (y=year,m=month,d=date in GPST)
*    -tt  h:m:s        start time (h=hour,m=minute,s=sec in GPST)
*    -ts  tspan        time span (hrs)
*    -fit tfit         fit interval (hrs)
*    -ns  nspan        number of time spans
*    -out outfile      output rinex nav file
*    -ant antfile      antex satellite antenna parameters
*    -ver ver          output rinex version
*    -sat sat[,sat...] select satellites (G:GPS,E:GAL,J:QZS,C:CMP)
*    -exc sat[,sat...] exclude satlelites
*    -iod iod          initial iode and iodc
*    -rms rms          threashold of residuals rms (m)
*    -res res          convergence threashold of residuals rms (m)
*    -csv              output csv file instead of rinex nav
*    -opt opt          options (1:diff-approx+2:output-rms+4:output-error)
*
*  keywords replaced in infile or outfile:
*    %Y -> yyyy        year (4 digits) (2000-2099)
*    %y -> yy          year (2 digits) (00-99)
*    %m -> mm          month           (01-12)
*    %d -> dd          day of month    (01-31)
*    %h -> hh          hours           (00-23)
*    %H -> a           hour code       (a-x)
*    %M -> mm          minutes         (00-59)
*    %n -> ddd         day of year     (001-366)
*    %W -> wwww        gps week        (0001-9999)
*    %D -> d           day of gps week (0-6)
*-----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    gtime_t epoch;
    double ep[6]={0},tspan=24.0,tfit=2.0,ver=3.0,max_rms=MAX_RMS;
    char *infile="igs%W%D.sp3",*outfile="",*antfile="",buff[256],*p;
    int i,sys=0,sat=0,nspan=1,iod=1,csv=0,opt=0;
    int mask[MAXSAT]={0},exc[MAXSAT]={0};
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-td")&&i+1<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",ep,ep+1,ep+2);
        }
        else if (!strcmp(argv[i],"-tt")&&i+1<argc) {
            sscanf(argv[++i],"%lf:%lf:%lf",ep+3,ep+4,ep+5);
        }
        else if (!strcmp(argv[i],"-ts" )&&i+1<argc) tspan=atof(argv[++i]);
        else if (!strcmp(argv[i],"-ns" )&&i+1<argc) nspan=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-fit")&&i+1<argc) tfit =atof(argv[++i]);
        else if (!strcmp(argv[i],"-ver")&&i+1<argc) ver  =atof(argv[++i]);
        else if (!strcmp(argv[i],"-out")&&i+1<argc) outfile=argv[++i];
        else if (!strcmp(argv[i],"-ant")&&i+1<argc) antfile=argv[++i];
        else if (!strcmp(argv[i],"-iod")&&i+1<argc) iod=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-opt")&&i+1<argc) opt=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-rms")&&i+1<argc) max_rms=atof(argv[++i]);
        else if (!strcmp(argv[i],"-csv")) csv=1;
        else if (!strcmp(argv[i],"-sat")&&i+1<argc) {
            strcpy(buff,argv[++i]);
            for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
                if      (!strcmp(p,"G")) sys|=SYS_GPS;
                else if (!strcmp(p,"E")) sys|=SYS_GAL;
                else if (!strcmp(p,"J")) sys|=SYS_QZS;
                else if (!strcmp(p,"C")) sys|=SYS_CMP;
                else if ((sat=satid2no(p))) mask[sat-1]=1;
            }
        }
        else if (!strcmp(argv[i],"-exc")&&i+1<argc) {
            strcpy(buff,argv[++i]);
            for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
                if ((sat=satid2no(p))) exc[sat-1]=1;
            }
        }
        else if (!strncmp(argv[i],"-",1)) print_help();
        else infile=argv[i];
    }
    if (!ep[0]) {
        fprintf(stderr,"start data/time error\n");
        return -1;
    }
    epoch=epoch2time(ep);
    
    if (!sys&&!sat) sys=SYS_GPS;
    if (sys) for (i=0;i<MAXSAT;i++) if (satsys(i+1,NULL)&sys) mask[i]=1;
    if (sat) for (i=0;i<MAXSAT;i++) if (mask[i]) sys|=satsys(i+1,NULL);
    for (i=0;i<MAXSAT;i++) if (exc[i]) mask[i]=0;
    
    for (i=0;i<nspan;i++) {
        
        /* generate navigation data */
        if (!gen_nav(epoch,tspan,tfit,ver,sys,mask,iod,max_rms,csv,opt,infile,
                     antfile,outfile)) {
            return -1;
        }
        epoch=timeadd(epoch,tspan*3600.0);
    }
    return 0;
}
