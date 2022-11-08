/*------------------------------------------------------------------------------
 * simobs.c : observation data simulator
 *
 *          Copyright (C) 2009 by T.TAKASU, All rights reserved.
 *
 * version : $Revision: 1.1 $ $Date: 2008/07/17 21:55:16 $
 * history : 2009/03/23  1.0 new
 *-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

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
/* generate snr --------------------------------------------------------------*/
static void snrmodel(const double *azel, double *snr)
{
  /* snr and snr deviation pattern (dbHz) by elevation (5 deg interval) */
  double snrs[]={40,42,44,45,46,47,48,49,49,50,50,51,51,51,51,51,51,51,51};
  double sdvs[]={ 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  double loss[]={ 0, 3, 0, 3};
  int i,j;
  for (i=0;i<NFREQ;i++) {
    j=(int)(azel[1]*R2D/5.0);
    if (snr[i]==0.0) {
      snr[i]=snrs[j]+randn(0.0,sdvs[j])-loss[i];
    }
    else {
      snr[i]=0.5*snr[i]+0.5*(snrs[j]+randn(0.0,sdvs[j])-loss[i]);
    }
  }
}
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
                  nav_t *nav, obs_t *obs) {

  gtime_t   time;
  obsd_t    data[MAXSAT]={0};
  double    pos[3],rs[6],dts[2],r,e[3],azel[2];
  double    ecp[NFREQ]={0};
  double    epr[NFREQ]={0};
  double    snr[NFREQ]={0};
  double    var;
  int       svh;
  double    iono,trop,fact,cp,pr,dtr=1.0e-9;
  int       i,j,k,n,ns,sys,prn;
  char      s[64];
  double    f0,fk;

  trace(3,"simobs:nnav=%d ngnav=%d\n",nav->n,nav->ng);

  for (j=0; j<MAXSAT; j++) {
      data[j].sat  = j+1;
      data[j].P[0] = 2.0e7;
  };

  /* Station position */
  ecef2pos(rr,pos);

  /* Loop over measurement epochs */
  n=(int)(timediff(te,ts)/tint+1.0);
  for (i=0;i<n;i++) {

    time = timeadd(ts,tint*i);
    time2str(time,s,0);

    for (j=0;j<MAXSAT;j++) data[j].time = time;

    /* reset number of satellites */
    ns = 0;

    /* loop over satellites */
    for (j=0;j<MAXSAT;j++) {

      time = timeadd(data[j].time,dtr);

      satpos(time,time,data[j].sat,EPHOPT_PREC,nav,rs,dts,&var,&svh);
      if ((r=geodist(rs,rr,e))<=0.0) continue;

      time = timeadd(time,-r/CLIGHT);

      satpos(time,time,data[j].sat,EPHOPT_PREC,nav,rs,dts,&var,&svh);
      if ((r=geodist(rs,rr,e))<=0.0) continue;

      satazel(pos,e,azel);
      if (azel[1]<minel*D2R) continue;

      /* Compute L1 ionospheric delay */
      iono = ionmodel(data[j].time,nav->ion_gps,pos,azel);

      /* Compute tropospheric delay */
      trop = tropmodel(data[j].time,pos,azel,0.3);

      snrmodel(azel,snr);
      errmodel(azel,snr,ecp,epr);

      sys = satsys(data[j].sat,&prn);

      for (k=0;k<NFREQ;k++) {

        data[j].L[k]=0.0;
        data[j].P[k]=0.0;
        data[j].SNR[k]=0;
        data[j].LLI[k]=0;
        data[j].code[k]=CODE_NONE;

        if (sys==SYS_GPS) {
          switch(k) {
            case(0) : data[j].code[k]=CODE_L1C; break;
            case(1) : data[j].code[k]=CODE_L1W; break;
            case(2) : data[j].code[k]=CODE_L2W; break;
            case(3) : data[j].code[k]=CODE_L5Q; break;
            default : continue;
          };
        }
        else if (sys==SYS_GLO) {
          switch(k) {
            case(0) : data[j].code[k]=CODE_L1C; break;
            case(1) : data[j].code[k]=CODE_L1P; break;
            case(2) : data[j].code[k]=CODE_L1C; break;
            case(3) : data[j].code[k]=CODE_L2P; break;
            default : continue;
          };
        }
        else if (sys==SYS_GAL) {
          switch(k) {
            case(0) : data[j].code[k]=CODE_L1C; break;
            case(1) : data[j].code[k]=CODE_L5Q; break;
            case(2) : data[j].code[k]=CODE_L7Q; break;
            case(3) : data[j].code[k]=CODE_L6C; break;
            default : continue;
          };
        }
        else {
          continue;
        };

        /* ionosphere scaling factor */
        f0 = sat2freq(data[j].sat, CODE_L1C, nav);
        fk = sat2freq(data[j].sat, data[j].code[k], nav);
        fact = pow(f0/fk,2);

        /* generate observation data */
        cp = r + CLIGHT*(dtr - dts[0])/*-fact*iono+trop+ecp[k]*/;
        pr = r + CLIGHT*(dtr - dts[0])/*+fact*iono+trop+epr[k]*/;

        data[j].L[k] = cp/CLIGHT*fk;
        data[j].P[k] = pr;
        data[j].SNR[k] = (uint16_t)(snr[k]/SNR_UNIT + 0.5);
        data[j].LLI[k] = 0; /* (data[j].SNR[k]<slipthres? 1 : 0); */

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
  int       i,j,n=0;
  int       nNav=0,nSp3=0,nClk=0;

  traceopen("simobs.log");
  tracelevel(5);

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
    else if (!strcmp(argv[i],"-sys")&&i+1<argc) {
      char *p;
      for (p=argv[++i];*p;p++) {
        switch (*p) {
          case 'G': rnxopt.navsys|=SYS_GPS; break;
          case 'R': rnxopt.navsys|=SYS_GLO; break;
          case 'E': rnxopt.navsys|=SYS_GAL; break;
          case 'J': rnxopt.navsys|=SYS_QZS; break;
          case 'C': rnxopt.navsys|=SYS_CMP; break;
          case 'I': rnxopt.navsys|=SYS_IRN; break;
          case 'S': rnxopt.navsys|=SYS_SBS; break;
          default : {
            fprintf(stderr,"invalid system %c\n",*p);
            return -1;
          };
        };
        if (!(p=strchr(p,','))) break;
      };
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


  pos[0]*=D2R; pos[1]*=D2R; pos2ecef(pos,rr);

  /* Default navigation systems */
  if (!rnxopt.navsys) {
    rnxopt.navsys=SYS_GPS|SYS_GLO;
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

  /* generate simulated observation data */
  if (!simobs(ts,te,tint,rr,&nav,&obs)) return -1;

  /* output rinex obs file */
  if (!(fp=fopen(outfile,"w"))) {
    fprintf(stderr,"error : outfile open %s\n",outfile);
    return -1;
  };

  fprintf(stderr,"saving...: %s\n",outfile);
  strcpy(rnxopt.prog,PROGNAME);
  strcpy(rnxopt.comment[0],"SIMULATED OBS DATA");
  rnxopt.rnxver=305;
  rnxopt.tstart=ts;
  rnxopt.tend=te;
  rnxopt.tint=tint;
  rnxopt.obstype=OBSTYPE_PR|OBSTYPE_CP|OBSTYPE_SNR;
  rnxopt.freqtype=FREQTYPE_L1|FREQTYPE_L2|FREQTYPE_L3|FREQTYPE_L4;
  for (i=0;i<3;i++) rnxopt.apppos[i]=rr[i];

  uint8_t codes[7][33]={{0}};

  const char type_str[]="CLS";
  char type[16];

  int iSys,iObs,iTyp;
  int nSys,nObs,nTyp;

  nObs = 4;
  nTyp = 3;
  nSys = 7;

  /* GPS */

  codes[0][0]=CODE_L1C;
  codes[0][1]=CODE_L1W;
  codes[0][2]=CODE_L2W;
  codes[0][3]=CODE_L5Q;

  /* GLONASS */

  codes[1][0]=CODE_L1C;
  codes[1][1]=CODE_L1P;
  codes[1][2]=CODE_L2C;
  codes[1][3]=CODE_L2P;

  /* Galileo */

  codes[2][0]=CODE_L1C;
  codes[2][1]=CODE_L5Q;
  codes[2][2]=CODE_L7Q;
  codes[2][3]=CODE_L6C;

  for (iSys=0;iSys<nSys;iSys++) {

    rnxopt.nobs[iSys] = (iSys<3? nObs*nTyp : 0);
    if (iSys>2) continue;

    n = 0;

    for (iObs=0;iObs<nObs;iObs++) {
      for (iTyp=0;iTyp<nTyp;iTyp++) {
        sprintf(type,"%c%s",type_str[iTyp],code2obs(codes[iSys][iObs]));
        setstr(rnxopt.tobs[iSys][n++],type,3);
      };
    };

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
