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
static double errcp1    = 0.002;        /* carrier-phase error (m) */
static double errcp2    = 0.002;        /* carrier-phase error/sin(el) (m) */
static double errpr1    = 0.200;        /* pseudorange   error (m) */
static double errpr2    = 0.200;        /* pseudorange   error/sin(el) (m) */

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" Synopsis",
"",
" simobs [option ...] file",
"",
" Description",
"",
" Read RINEX OBS/NAV/CLK and SP3 files and simulate GNSS pseudorange, ",
" carrier-phase and C/N0 measurements for a given receiver position.",
" The output will be written to a RINEX OBS file.",
"",
" Options (units) [default]",
"",
" -?                   print help",
" -n      file         RINEX NAV input file",
" -s      file         SP3 input file",
" -c      file         Clock-RINEX input file",
" -e      file         ERP input file",
" -o      file         RINEX OBS output file",
" -v      ver          RINEX OBS version [3.05]",
" -ts     ds ts        start date/time (ds=y/m/d ts=h:m:s)",
" -te     de te        end   date/time (de=y/m/d te=h:m:s)",
" -ti     tint         time interval (sec) [30]",
" -hc     comment      rinex header: comment line",
" -hm     marker       rinex header: marker name",
" -hn     markno       rinex header: marker number",
" -ht     marktype     rinex header: marker type",
" -ho     observ       rinex header: observer name and agency separated by /",
" -hr     rec          rinex header: receiver number, type and version separated by /",
" -ha     ant          rinex header: antenna number and type separated by /",
" -hd     delta        rinex header: antenna delta h/e/n separated by / (m)",
" -r      x y z        receiver ecef position (m)",
" -l      lat lon hgt  receiver latitude/longitude/height (deg/m)",
" --mask  [sig[,...]]  signal mask(s) (sig={G|R|E|J|S|C|I}L{1C|1P|1W|...})",
" --noise              use observation noise",
" --tropo              use tropospheric delay",
" --iono               use ionospheric  delay (requires -n)",
" --tides              use solid tides and pole tides (requires -e)",
" -x      level        debug trace level (0:off,>0:on) [0]",
""
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<(int)(sizeof(help)/sizeof(*help));i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
};
/* processing options --------------------------------------------------------*/
typedef struct {        /* processing options type */
    gtime_t ts;
    gtime_t te;
    double  tint;
    double  rr[3];
    int     navsys;           /* navigation system */
    int     noise;            /* Measurement noise  (0:off,1:on) */
    int     ionoopt;          /* ionosphere  option (0:off,1:on) */
    int     tropopt;          /* troposphere option (0:off,1:on) */
    int     nf;               /* number of frequencies (1:L1,2:L1+L2,3:L1+L2+L5) */
    double  elmin;            /* elevation mask angle (rad) */
    int     sateph;           /* satellite ephemeris/clock (EPHOPT_???) */
    int     tidecorr;         /* earth tide correction (0:off,1:solid,5:solid+pole,7:solid+pole+otl) */
    double  antdel[3];        /* antenna delta {d_e,d_n,d_u} */
    double  odisp[6*11];      /* ocean tide loading parameters */
    int     trace;            /* trace output for debugging */
} simopt_t;

/* generate random number with normal distribution ---------------------------*/
static double randn(double myu, double sig)
{
  double a,b;
  a=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<a<=1 */
  b=((double)rand()+1.0)/((double)RAND_MAX+1.0);  /* 0<b<=1 */
  return myu+sqrt(-2.0*log(a))*sin(2.0*PI*b)*sig;
};

/* set string without tail space ---------------------------------------------*/
static void setstr(char *dst, const char *src, int n)
{
  char *p=dst;
  const char *q=src;
  while (*q&&q<src+n) *p++=*q++;
  *p--='\0';
  while (p>=dst&&*p==' ') *p--='\0';
};

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
};

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

/* generate errors------------------------------------------------------------*/
static void errmodel(const double *azel, double *ecp, double *epr)
{
  ecp[0]=randn(0.0,errcp1)+randn(0.0,errcp2)/sin(azel[1]);
  epr[0]=randn(0.0,errpr1)+randn(0.0,errpr2)/sin(azel[1]);
};

/* generate simulated observation data ---------------------------------------*/
static int simobs(simopt_t simopt, rnxopt_t rnxopt, nav_t *nav, obs_t *obs) {

  gtime_t   time;
  obsd_t    data[MAXSAT]={0};
  double    pos[3],rr[3],dr[3],rs[6],dts[2],r,e[3],azel[2];
  double    var;
  int       svh;
  double    snr;
  double    iono,trop,fact,cp,pr,dtr=0.0;
  double    epr,ecp;
  int       i,j,k,n,m,ns,sys,prn;
  int       iSys;
  char      s[64];
  double    f0,fk;

  trace(3,"simobs:nnav=%d ngnav=%d\n",nav->n,nav->ng);

  for (j=0; j<MAXSAT; j++) {
    data[j].sat  = j+1;
    data[j].P[0] = 0.0;
    data[j].code[0] = CODE_NONE;
  };

  /* Loop over measurement epochs */
  n = (int)(timediff(simopt.te,simopt.ts)/simopt.tint+1.0);
  for (i=0;i<n;i++) {

    /* set the satellite and observation epoch */
    time = timeadd(simopt.ts,simopt.tint*i);
    time2str(time,s,0);

    for (j=0;j<MAXSAT;j++) data[j].time = time;

    for (k=0;k<3;k++) rr[k] = simopt.rr[k];

    /* earth tides correction */
    if (simopt.tidecorr>0) {
      dr[0]=dr[1]=dr[2]=0.0;
      tidedisp(gpst2utc(time),simopt.rr,simopt.tidecorr,&nav->erp,
               simopt.odisp,dr);
      for (k=0;k<3;k++) rr[k] += dr[k];
    };

    /* Station position */
    ecef2pos(rr,pos);

    /* Apply antenna delta */
    dr[0]=dr[1]=dr[2]=0.0;
    enu2ecef(pos, simopt.antdel, dr);
    for (k=0;k<3;k++) rr[k] += dr[k];

    /* reset number of satellites */
    ns = 0;

    /* loop over satellites */
    for (j=0;j<MAXSAT;j++) {

      time = timeadd(data[j].time,-70e-6+dtr);

      if (satpos(time,time,data[j].sat,EPHOPT_PREC,nav,rs,dts,&var,&svh)==0)
        continue;
      if ((r=geodist(rs,rr,e))<=0.0) continue;

      time = timeadd(data[j].time,-r/CLIGHT);

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
        f0 = sat2freq(data[j].sat, CODE_L1C,        nav);
        fk = sat2freq(data[j].sat, data[j].code[m], nav);
        fact = pow(f0/fk,2);

        /* Measurement noise */
        errmodel(azel, &epr, &ecp);

        /* generate observation data */
        cp  = r + CLIGHT*(dtr - dts[0]) \
                 -(simopt.ionoopt>0? fact*iono : 0.0) \
                 +(simopt.tropopt>0? trop : 0.0) \
                 +(simopt.noise>0?   epr  : 0.0);
        pr  = r + CLIGHT*(dtr - dts[0]) \
                 +(simopt.ionoopt>0? fact*iono : 0.0) \
                 +(simopt.tropopt>0? trop : 0.0) \
                 +(simopt.noise>0?   ecp  : 0.0);
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

};

/* simobs main ---------------------------------------------------------------*/
int main(int argc, char **argv) {

  FILE      *fp;
  rnxopt_t  rnxopt={{0}};
  simopt_t  simopt={{0}};
  obs_t     obs={0};
  nav_t     nav={0};
  sta_t     sta={0};
  double    es[]={2000,1,1,0,0,0};
  double    ee[]={2000,1,1,0,0,0};
  double    pos[3]={0};
  char      *navFileName[16]={0};
  char      *sp3FileName[16]={0};
  char      *clkFileName[16]={0};
  char      *erpFileName={0};
  char      *outfile="";
  char      *p,buff[256];
  int       i,j,k,nc=0;
  int       nNav=0,nSp3=0,nClk=0,nErp=0;

  /* Default simulation options */
  simopt.noise    = 0;
  simopt.tropopt  = 0;
  simopt.ionoopt  = 0;
  simopt.tidecorr = 0;
  simopt.trace    = 0;

  simopt.tint = 30.0;
  simopt.antdel[0] = 0.0;
  simopt.antdel[1] = 0.0;
  simopt.antdel[2] = 0.0;

  /* Default RINEX options */
  rnxopt.rnxver = 305;
  strcpy(rnxopt.comment[0],"SIMULATED OBS DATA");
  strcpy(rnxopt.rec[1],"UNKNOWN");
  strcpy(rnxopt.ant[1],"UNKNOWN         NONE");
  rnxopt.antdel[0] = 0.0;
  rnxopt.antdel[1] = 0.0;
  rnxopt.antdel[2] = 0.0;

  /* seed random number generator for reproducible noise */
  srand(0);

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
      simopt.ts=epoch2time(es);

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
      simopt.te=epoch2time(ee);

    }
    else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
      simopt.tint=atof(argv[++i]);
    }

    else if (!strcmp(argv[i],"-hc")&&i+1<argc) {
      if (nc<MAXCOMMENT)
        strcpy(rnxopt.comment[nc++],argv[++i]);
    }
    else if (!strcmp(argv[i],"-hm")&&i+1<argc) {
      strcpy(rnxopt.marker,argv[++i]);
    }
    else if (!strcmp(argv[i],"-hn")&&i+1<argc) {
      strcpy(rnxopt.markerno,argv[++i]);
    }
    else if (!strcmp(argv[i],"-ht")&&i+1<argc) {
      strcpy(rnxopt.markertype,argv[++i]);
    }
    else if (!strcmp(argv[i],"-ho")&&i+1<argc) {
      strcpy(buff,argv[++i]);
      for (j=0,p=strtok(buff,"/");j<2&&p;j++,p=strtok(NULL,"/")) {
        strcpy(rnxopt.name[j],p);
      };
    }
    else if (!strcmp(argv[i],"-hr")&&i+1<argc) {
      strcpy(buff,argv[++i]);
      for (j=0,p=strtok(buff,"/");j<3&&p;j++,p=strtok(NULL,"/")) {
        strcpy(rnxopt.rec[j],p);
      };
    }
    else if (!strcmp(argv[i],"-ha")&&i+1<argc) {
      strcpy(buff,argv[++i]);
      for (j=0,p=strtok(buff,"/");j<3&&p;j++,p=strtok(NULL,"/")) {
        strcpy(rnxopt.ant[j],p);
      };
    }
    else if (!strcmp(argv[i],"-hd")&&i+1<argc) {
      strcpy(buff,argv[++i]);
      for (j=0,p=strtok(buff,"/");j<3&&p;j++,p=strtok(NULL,"/")) { /* h,e,n */
        rnxopt.antdel[j]=atof(p);
      };
      simopt.antdel[0] = rnxopt.antdel[1]; /* E <- H */
      simopt.antdel[1] = rnxopt.antdel[2]; /* N <- E */
      simopt.antdel[2] = rnxopt.antdel[0]; /* U <- N */
    }
    else if (!strcmp(argv[i],"-v" )&&i+1<argc) {
        rnxopt.rnxver=(int)(atof(argv[++i])*100.0);
    }
    else if (!strcmp(argv[i],"-l")&&i+3<argc) {
      for (j=0;j<3;j++) pos[j]=atof(argv[++i]); /* lat,lon,hgt */
      pos[0]*=D2R; pos[1]*=D2R;
      pos2ecef(pos,simopt.rr);
    }
    else if (!strcmp(argv[i],"-r")&&i+3<argc) {
      for (j=0;j<3;j++) simopt.rr[j]=atof(argv[++i]); /* x,y,z */
      ecef2pos(simopt.rr, pos);
    }
    /* Signal mask */
    else if (!strcmp(argv[i],"-mask")&&i+1<argc) {
      for (j=0;j<7;j++) for (k=0;k<64;k++) rnxopt.mask[j][k]='0';
      setmask(argv[++i],&rnxopt,1);
    }
    /* RINEX-NAV files */
    else if (!strcmp(argv[i],"-n")&&i+1<argc) {
      navFileName[nNav++]=argv[++i];
    }
    /* SP3 files */
    else if (!strcmp(argv[i],"-s")&&i+1<argc) {
      sp3FileName[nSp3++]=argv[++i];
    }
    /* Clock-RINEX files */
    else if (!strcmp(argv[i],"-c")&&i+1<argc) {
      clkFileName[nClk++]=argv[++i];
    }
    else if (!strcmp(argv[i],"-e")&&i+1<argc) {
      erpFileName=argv[++i];
      nErp++;
    }
    else if (!strcmp(argv[i],"--noise")) {
      simopt.noise = 1;
    }
    else if (!strcmp(argv[i],"--tropo")) {
      simopt.tropopt = 1;
    }
    else if (!strcmp(argv[i],"--iono")) {
      simopt.ionoopt = 1;
    }
    else if (!strcmp(argv[i],"--tides")) {
      simopt.tidecorr = 5;
    }
    else if (!strcmp(argv[i],"-x")&&i+1<argc) {
      simopt.trace = argv[++i];
    }
    else if (*argv[i]=='-') {
      printhelp();
    };

  };

   /* Debugging output level */
  if (simopt.trace>0) {
    traceopen("simobs_tracelog.txt");
    tracelevel(simopt.trace);
  };

  /* Check required inputs */
  if (!*outfile) {
    fprintf(stderr,"no output file\n");
    return -1;
  };
  if (norm(pos,3)<=0.0) {
    fprintf(stderr,"no receiver pos\n");
    return -1;
  };
  if (simopt.ionoopt>0 && nNav==0) {
    fprintf(stderr,"ERROR: require RINEX-NAV file for iono delay modeling!\n");
    return -1;
  };

  /* Default navigation systems */

  if (!rnxopt.navsys) {
    rnxopt.navsys = SYS_GPS|SYS_GAL;
  };

  /* read RINEX-NAV files */

  if (nNav>0) {
    for (i=0;i<nNav;i++) {
      fprintf(stdout,"Reading %s\n",navFileName[i]);
      readrnx(navFileName[i],0,"",&obs,&nav,&sta);
    };
    if (nav.n<=0) {
      fprintf(stderr,"no BRDC data\n");
      return -1;
    };
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

  /* read precise Clock-RINEX files */

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

  /* read ERP data */

  if (nErp>0) {
    fprintf(stdout,"Reading %s\n",erpFileName);
    if (!readerp(erpFileName,&nav.erp)) {
      fprintf(stdout,"ERROR: cannot read ERP file %s\n",erpFileName);
      return -1;
    };
  };

  /* RINEX OBS file options */

  strcpy(rnxopt.prog,PROGNAME);
  rnxopt.tstart  = simopt.ts;
  rnxopt.tend    = simopt.te;
  rnxopt.tint    = simopt.tint;
  rnxopt.obstype = OBSTYPE_PR|OBSTYPE_CP|OBSTYPE_SNR;
  for (i=0;i<3;i++) {
    rnxopt.apppos[i] = simopt.rr[i];
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
  if (!simobs(simopt,rnxopt,&nav,&obs)) return -1;

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

};
