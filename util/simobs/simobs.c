/*------------------------------------------------------------------------------
 * simobs.c : observation data simulator
 *
 *          Copyright (C) 2009 by T.TAKASU, All rights reserved.
 *
 * version : $Revision: 1.1 $ $Date: 2008/07/17 21:55:16 $
 * history : 2009/03/23  1.0 new
 *-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define PROGNAME    "SIMOBS"            /* program name */

/* simulation parameters -----------------------------------------------------*/

static double minel     = 5.0;          /* minimum elevation angle (deg) */
static double errcp1    = 0.002;        /* carrier-phase meas error (m) */
static double errcp2    = 0.002;        /* carrier-phase meas error/sin(el) (m) */
static double errpr1    = 0.2;          /* pseudorange error (m) */
static double errpr2    = 0.2;          /* pseudorange error/sin(el) (m) */

/* generate random number with normal distribution ---------------------------*/
static double randn(double myu, double sig)
{
  double a,b;
  a=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<a<=1 */
  b=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<b<=1 */
  return myu+sqrt(-2.0*log(a))*sin(2.0*PI*b)*sig;
}
/* set string without tail space ---------------------------------------------*/
static void setstr(char *dst, const char *src, int n)
{
  char *p=dst;
  const char *q=src;
  while (*q&&q<src+n) *p++=*q++;
  *p--='\0';
  while (p>=dst&&*p==' ') *p--='\0';
}
/* set signal mask -----------------------------------------------------------*/
static void setmask(const char *argv, rnxopt_t *opt, int mask)
{
  char buff[1024],*p;
  int i,code;

  strcpy(buff,argv);
  for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
    if (strlen(p)<4||p[1]!='L') continue;
    if      (p[0]=='G') { i=0; opt->navsys|=SYS_GPS; }
    else if (p[0]=='R') { i=1; opt->navsys|=SYS_GLO; }
    else if (p[0]=='E') { i=2; opt->navsys|=SYS_GAL; }
    else if (p[0]=='J') { i=3; opt->navsys|=SYS_QZS; }
    else if (p[0]=='S') { i=4; opt->navsys|=SYS_CMP; }
    else if (p[0]=='C') { i=5; opt->navsys|=SYS_IRN; }
    else if (p[0]=='I') { i=6; opt->navsys|=SYS_SBS; }
    else continue;
    if ((code=obs2code(p+2))) {
      opt->mask[i][code-1]=mask?'1':'0';
    };
  };
}
/*
 * GPS,GLO,GAL,QZS,SBS,CMP,IRN
 */
static int sys2idx(int sys) {
  switch(sys) {
    case(SYS_GPS): return 0;
    case(SYS_GLO): return 1;
    case(SYS_GAL): return 2;
    case(SYS_QZS): return 3;
    case(SYS_SBS): return 4;
    case(SYS_CMP): return 5;
    case(SYS_IRN): return 6;
    default      : return -1;
  };
};
/* generate snr --------------------------------------------------------------*/
static double snrmodel(const double *azel, uint8_t code)
{
  /* snr and snr deviation pattern (db-Hz) by elevation (5 deg interval) */
  double snrs[]={40,42,44,45,46,47,48,49,49,50,50,51,51,51,51,51,51,51,51};
  double sdvs[]={ 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int j;
  j=(int)(azel[1]*R2D/5.0);
  return snrs[j] + randn(0.0,sdvs[j]) - (code==CODE_L1W||code==CODE_L2W? 10: 0);
};
/* generate error ------------------------------------------------------------*/
static void errmodel(const double *azel, double *snr, double *ecp, double *epr)
{
  int i;
  double ec,ep;

  for (i=0;i<NFREQ;i++) {
    ec=randn(0.0,errcp1)+randn(0.0,errcp2)/sin(azel[1]);
    ep=randn(0.0,errpr1)+randn(0.0,errpr2)/sin(azel[1]);

    ecp[i]=0.5*ecp[i]+0.5*ec; /* filter */
    epr[i]=0.5*epr[i]+0.5*ep; /* filter */
  }
}
/* generate simulated observation data ---------------------------------------*/
static int simobs(gtime_t ts, gtime_t te, double tint, const double *rr,
                  rnxopt_t rnxopt, nav_t *nav, obs_t *obs) {

  gtime_t   time;
  obsd_t    data[MAXSAT]={0};
  double    pos[3],rs[6],dts[2],r,e[3],azel[2];
  double    var;
  int       svh;
  double    snr;
  double    iono,trop,fact,cp,pr,dtr=0.0;
  int       i,j,k,n,m,ns,sys,prn;
  int       iSys;
  char      s[64];
  double    f0,fk;

  trace(3,"simobs:nnav=%d ngnav=%d\n",nav->n,nav->ng);

  for (j=0; j<MAXSAT; j++) {
      data[j].sat  = j+1;
      data[j].P[0] = 2.0e7;
      data[j].code[0] = CODE_NONE;
  };

  /* Station position */
  ecef2pos(rr,pos);

  /* Loop over measurement epochs */
  n = (int)(timediff(te,ts)/tint+1.0);
  for (i=0;i<n;i++) {

    /* set the satellite and observation epoch */
    time = timeadd(ts,tint*i);
    time2str(time,s,0);

    for (j=0;j<MAXSAT;j++) data[j].time = time;

    /* reset number of satellites */
    ns = 0;

    /* loop over satellites */
    for (j=0;j<MAXSAT;j++) {

      time = timeadd(data[j].time,dtr);

      if (satpos(time,time,data[j].sat,EPHOPT_PREC,nav,rs,dts,&var,&svh)==0)
        continue;
      if ((r=geodist(rs,rr,e))<=0.0) continue;

      time = timeadd(time,-r/CLIGHT);

      if (satpos(time,time,data[j].sat,EPHOPT_PREC,nav,rs,dts,&var,&svh)==0)
        continue;
      if ((r=geodist(rs,rr,e))<=0.0) continue;

      satazel(pos,e,azel);
      if (azel[1]<minel*D2R) continue;

      /* Compute L1 ionospheric delay */
      iono = ionmodel(data[j].time,nav->ion_gps,pos,azel);

      /* Compute tropospheric delay */
      trop = tropmodel(data[j].time,pos,azel,0.3);

      /* GNSS and system index */
      sys  = satsys(data[j].sat,&prn);
      iSys = sys2idx(sys);

      if (iSys<0 || rnxopt.nobs[iSys]==0) continue;

      m = 0;
      for (k=0;k<rnxopt.nobs[iSys];k++) {

        if (rnxopt.tobs[iSys][k][0]!='C') continue;

        data[j].L[m]   = 0.0;
        data[j].P[m]   = 0.0;
        data[j].SNR[m] = 0;
        data[j].LLI[m] = 0;

        data[j].code[m] = obs2code(rnxopt.tobs[iSys][k]+1);

        /* ionosphere scaling factor */
        f0 = sat2freq(data[j].sat, CODE_L1C, nav);
        fk = sat2freq(data[j].sat, data[j].code[m], nav);
        fact = pow(f0/fk,2);

        /* generate observation data */
        cp  = r + CLIGHT*(dtr - dts[0])/*-fact*iono*//*+trop*/;
        pr  = r + CLIGHT*(dtr - dts[0])/*+fact*iono*//*+trop*/;
        snr = snrmodel(azel,data[j].code[m]);

        data[j].L[m]   = cp/CLIGHT*fk;
        data[j].P[m]   = pr;
        data[j].SNR[m] = (uint16_t)(snr/SNR_UNIT + 0.5);
        data[j].LLI[m] = 0; /* (data[j].SNR[m]<slipthres? 1 : 0); */

        m++;

      };
      if (obs->nmax<=obs->n) {

        if (obs->nmax==0) obs->nmax=65532; else obs->nmax+=65532;
        if (!(obs->data=(obsd_t *)realloc(obs->data,sizeof(obsd_t)*obs->nmax))) {
          fprintf(stderr,"malloc error\n");
          return 0;
        };

      };

      obs->data[obs->n++]=data[j];
      ns++;

    };
    fprintf(stdout,"time=%s nsat=%2d\r",s,ns);
  };

  fprintf(stdout,"\n");

  return 1;

}
/* simobs main ---------------------------------------------------------------*/
int main(int argc, char **argv) {

  FILE      *fp;
  rnxopt_t  rnxopt={{0}};
  obs_t     obs={0};
  nav_t     nav={0};
  sta_t     sta={0};
  gtime_t   ts={0},te={0};
  double    es[]={2000,1,1,0,0,0};
  double    ee[]={2000,1,1,0,0,0};
  double    tint=30.0;
  double    pos[3]={0},rr[3];
  char      *navFileName[16]={0};
  char      *sp3FileName[16]={0};
  char      *clkFileName[16]={0};
  char      *outfile="";
  int       i,j,k;
  int       nNav=0,nSp3=0,nClk=0;

  /*
  traceopen("simobs_tracelog.txt");
  tracelevel(5);
  */

  /* Process command line arguments */

  for (i=1;i<argc;i++) {

    if      (!strcmp(argv[i],"-o")&&i+1<argc) {
      outfile=argv[++i];
    }
    else if (!strcmp(argv[i],"-ts")) {

      if (i+2<argc && argv[i+2][0]!='-') {
        sscanf(argv[++i],"%lf/%lf/%lf",es,es+1,es+2);
        sscanf(argv[++i],"%lf:%lf:%lf",es+3,es+4,es+5);
      }
      else if (i+1<argc) {
        sscanf(argv[++i],"%lf/%lf/%lf",es,es+1,es+2);
      }
      else {
        fprintf(stderr,"invalid date/time format for -ts\n");
        return -1;
      };
      ts=epoch2time(es);

    }
    else if (!strcmp(argv[i],"-te")&&i+1<argc) {

      if (i+2<argc && argv[i+2][0]!='-') {
        sscanf(argv[++i],"%lf/%lf/%lf",ee,ee+1,ee+2);
        sscanf(argv[++i],"%lf:%lf:%lf",ee+3,ee+4,ee+5);
      }
      else if (i+1<argc) {
        sscanf(argv[++i],"%lf/%lf/%lf",ee,ee+1,ee+2);
      }
      else {
        fprintf(stderr,"invalid date/time format for -te\n");
        return -1;
      };
      te=epoch2time(ee);

    }
    else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
      tint=atof(argv[++i]);
    }
    else if (!strcmp(argv[i],"-r")&&i+3<argc) {
      for (j=0;j<3;j++) pos[j]=atof(argv[++i]); /* lat,lon,hgt */
    }
    /* Signal mask */
    else if (!strcmp(argv[i],"-mask")&&i+1<argc) {
      for (j=0;j<7;j++) for (k=0;k<64;k++) rnxopt.mask[j][k]='0';
      setmask(argv[++i],&rnxopt,1);
    }
    /* SP3 files */
    else if (!strcmp(argv[i],"-s")&&i+1<argc) {
      sp3FileName[nSp3++]=argv[++i];
    }
    /* Clock-RINEX files */
    else if (!strcmp(argv[i],"-c")&&i+1<argc) {
      clkFileName[nClk++]=argv[++i];
    }
    else {
      navFileName[nNav++]=argv[i];
    };
  };

  if (nNav<=0) {
    fprintf(stderr,"no RINEX-NAV input file\n");
    return -1;
  };
  if (!*outfile) {
    fprintf(stderr,"no output file\n");
    return -1;
  };
  if (norm(pos,3)<=0.0) {
    fprintf(stderr,"no receiver pos\n");
    return -1;
  };

  /* Default navigation systems */

  if (!rnxopt.navsys) {
    rnxopt.navsys = SYS_GPS|SYS_GAL;
  };

  /* read RINEX-NAV files */

  for (i=0;i<nNav;i++) {
    fprintf(stdout,"Reading %s\n",navFileName[i]);
    readrnx(navFileName[i],0,"",&obs,&nav,&sta);
  };
  if (nav.n<=0) {
    fprintf(stderr,"no BRDC data\n");
    return -1;
  };

  /* read precise ephemeris files */

  if (nSp3>0) {
    for (i=0;i<nSp3;i++) {
      fprintf(stdout,"Reading %s\n",sp3FileName[i]);
      readsp3(sp3FileName[i],&nav,0);
    };
    if (nav.ne<=0) {
      fprintf(stderr,"no SP3 data\n");
      return -1;
    };
  };

  /* read precise clock-RINEX files */

  if (nClk>0) {
    for (i=0;i<nClk;i++) {
      fprintf(stdout,"Reading %s\n",clkFileName[i]);
      readrnxc(clkFileName[i],&nav);
    };
    if (nav.nc<=0) {
      fprintf(stderr,"no Clk-RINEX data\n");
      return -1;
    };
  };

  /* Reference position for RINEX header */

  pos[0]*=D2R; pos[1]*=D2R; pos2ecef(pos,rr);

  /* RINEX OBS file options */

  strcpy(rnxopt.prog,PROGNAME);
  strcpy(rnxopt.comment[0],"SIMULATED OBS DATA");
  rnxopt.rnxver=305;
  rnxopt.tstart=ts;
  rnxopt.tend=te;
  rnxopt.tint=tint;
  rnxopt.obstype=OBSTYPE_PR|OBSTYPE_CP|OBSTYPE_SNR;
  for (i=0;i<3;i++) {
    rnxopt.apppos[i] = rr[i];
  };

  char  obst[4];
  int   iSys,iObs;
  int   nSys,nObs;

  nObs = 64;
  nSys = 7;

  for (iSys=0;iSys<nSys;iSys++) {

    rnxopt.nobs[iSys] = 0;
    k=0;

    for (iObs=0;iObs<nObs;iObs++) {

      if (rnxopt.mask[iSys][iObs]=='0') continue;

      if (rnxopt.obstype & OBSTYPE_PR) {
        sprintf(obst,"C%s",code2obs(iObs+1));
        setstr(rnxopt.tobs[iSys][k++],obst,3);
        rnxopt.nobs[iSys]++;
      };
      if (rnxopt.obstype & OBSTYPE_CP) {
        sprintf(obst,"L%s",code2obs(iObs+1));
        setstr(rnxopt.tobs[iSys][k++],obst,3);
        rnxopt.nobs[iSys]++;
      };
      if (rnxopt.obstype & OBSTYPE_DOP) {
        sprintf(obst,"D%s",code2obs(iObs+1));
        setstr(rnxopt.tobs[iSys][k++],obst,3);
        rnxopt.nobs[iSys]++;
      };
      if (rnxopt.obstype & OBSTYPE_SNR) {
        sprintf(obst,"S%s",code2obs(iObs+1));
        setstr(rnxopt.tobs[iSys][k++],obst,3);
        rnxopt.nobs[iSys]++;
      };

    };

  };

  fprintf(stdout,"Generate observations \n");

  /* generate simulated observation data */
  if (!simobs(ts,te,tint,rr,rnxopt,&nav,&obs)) return -1;

  fprintf(stdout,"saving...: %s\n",outfile);

  /* output RINEX OBS file */
  if (!(fp=fopen(outfile,"w"))) {
    fprintf(stderr,"ERROR : cannot open output file %s\n",outfile);
    return -1;
  };

  outrnxobsh(fp,&rnxopt,&nav);

  for (i=0;i<obs.n;i=j) {
    for (j=i;j<obs.n;j++) {
      if (timediff(obs.data[j].time,obs.data[i].time)>0.001) break;
    }
    outrnxobsb(fp,&rnxopt,obs.data+i,j-i,0);
  };
  fclose(fp);

  return 0;

}
