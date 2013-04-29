/*------------------------------------------------------------------------------
* diffeph.c : output difference between ephemerides
*
* history : 2010/06/17  0.1 new
*           2011/09/17  0.2 add comparison of 2 precise ephemeris
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

static const char *usage[]={
"Synopsys",
" diffeph [-t0 y/m/d h:m:s][-ts ts][-ti ti][-s s][-1 r][-2 b][-t]",
"         [-o][-u lat lon][-x level] file ...",
"",
"Description",
"compute ephemeris and clock error and output errors.",
"",
"Options [default]",
"-t0 y/m/d h:m:s  start time expressed in gpst",
"-ts ts      time span (hr) [24]",
"-ti ti      time interval (s) [30]",
"-s s        satellite number (all)",
"-1 r        compared ephemeris type (0:broadcast,1:precise,2:sbas,3:ssrapc,5:lex)",
"-2 b        reference ephemeris type (same as above)",
"-t          output time as yyyy/mm/dd hh:mm:ss",
"-e          output difference as ecef-x/y/z [radial/along-trk/cross-trk]",
"-o          output satellite position/clock-bias",
"-u lat lon  refrence position latitude and longitude for ure",
"-x level    trace level",
"file ...    compared ephemeris files and antenna parameters file",
NULL};

/* print usage ---------------------------------------------------------------*/
static void prusage(void)
{
	int i;
	for (i=0;usage[i];i++) fprintf(stderr,"%s\n",usage[i]);
}
/* update rtcm struct --------------------------------------------------------*/
static void updatertcm(gtime_t time, rtcm_t *rtcm, nav_t *nav, FILE *fp)
{
    char s1[32],s2[32];
    int i;
    
    while (input_rtcm3f(rtcm,fp)>=0) {
        time2str(time      ,s1,0);
        time2str(rtcm->time,s2,0);
        trace(2,"rtcm.time=%s time=%s\n",s1,s2);
        
        if (timediff(rtcm->time,time)>=5.0) break;
    }
    for (i=0;i<MAXSAT;i++) nav->ssr[i]=rtcm->ssr[i];
}
/* update lex ephemeris ------------------------------------------------------*/
static int updatelex(int index, gtime_t time, lex_t *lex, nav_t *nav)
{
    gtime_t tof;
    
    for (;index<lex->n;index++) {
        if (!lexupdatecorr(lex->msgs+index,nav,&tof)) continue;
        if (timediff(tof,time)>=0.0) break;
    }
    return index;
}
/* print difference ----------------------------------------------------------*/
static void printephdiff(gtime_t time, int sat, int eph1, int eph2,
                         const nav_t *nav1, const nav_t *nav2, int topt,
                         int eopt, int mopt, const double *pos)
{
    double tow,rs1[6],rs2[6],dts1[2],dts2[2],var1,var2;
    double drs[3],drss[3],rc[3],er[3],ea[3],ec[3];
    double rr[3],e[3],rr1[3],rr2[3],r1,r2,azel[2];
    int i,week,svh1,svh2;
    char tstr[32];
    
    if (!satpos(time,time,sat,eph1,nav1,rs1,dts1,&var1,&svh1)) return;
    if (!satpos(time,time,sat,eph2,nav2,rs2,dts2,&var2,&svh2)) return;
    if (svh1||svh2) return;
    
    for (i=0;i<3;i++) drs[i]=rs1[i]-rs2[i];
    if (!normv3(rs2+3,ea)) return;
    cross3(rs2,rs2+3,rc);
    if (!normv3(rc,ec)) return;
    cross3(ea,ec,er);
    drss[0]=dot(drs,er,3); /* radial/along-trk/cross-trk */
    drss[1]=dot(drs,ea,3);
    drss[2]=dot(drs,ec,3);
    
    if (topt) {
        time2str(time,tstr,0);
        printf("%s ",tstr);
    }
    else {
        tow=time2gpst(time,&week);
        printf("%4d %6.0f",week,tow);
    }
    printf("%3d ",sat);
    
    if (mopt) {
        printf("%8.3f %8.3f %8.3f ",drss[0],drss[1],drss[2]);
    }
    else {
        printf("%8.3f %8.3f %8.3f ",drs[0],drs[1],drs[2]);
    }
    printf("%8.3f ",dts1[0]==0.0||dts2[0]==0.0?0.0:(dts1[0]-dts2[0])*CLIGHT);
    if (mopt) {
        printf(" %13.3f %13.3f %13.3f %12.3f",rs1[0],rs1[1],rs1[2],dts1[0]*CLIGHT);
    }
    if (norm(pos,3)>0.0) {
        pos2ecef(pos,rr);
        for (i=0;i<3;i++) {
            rr1[i]=rs1[i]-rr[i];
            rr2[i]=rs2[i]-rr[i];
        }
        r1=norm(rr1,3);
        r2=norm(rr2,3);
        for (i=0;i<3;i++) e[i]=rr2[i]/r2;
        satazel(pos,e,azel);
        printf(" %8.3f %5.1f",(r1-r2)-(dts1[0]-dts2[0])*CLIGHT,azel[1]*R2D);
    }
    printf("\n");
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=NULL;
    nav_t nav={0},nav2={0};
    rtcm_t rtcm;
    lex_t lex={0};
    gtime_t t0,time;
    double ep0[]={2000,1,1,0,0,0},tspan=24.0,tint=300,pos[]={0};
    int i,s,n=0,nx=0,sat=0,topt=0,eopt=0,mopt=0,trl=0,index=0;
    int eph1=EPHOPT_BRDC,eph2=EPHOPT_PREC;
    char *files[32],*ext;
    
    t0=epoch2time(ep0);
    
    init_rtcm(&rtcm);
    rtcm.time=t0;
    
    for (i=1;i<argc;i++) {
       if      (!strcmp(argv[i],"-s" )&&i+1<argc) sat  =atoi(argv[++i]);
       else if (!strcmp(argv[i],"-1" )&&i+1<argc) eph1 =atoi(argv[++i]);
       else if (!strcmp(argv[i],"-2" )&&i+1<argc) eph2 =atoi(argv[++i]);
       else if (!strcmp(argv[i],"-x" )&&i+1<argc) trl  =atoi(argv[++i]);
       else if (!strcmp(argv[i],"-t0")&&i+2<argc) {
           if (sscanf(argv[++i],"%lf/%lf/%lf",ep0  ,ep0+1,ep0+2)<3||
               sscanf(argv[++i],"%lf:%lf:%lf",ep0+3,ep0+4,ep0+5)<1) {
               fprintf(stderr,"invalid time\n");
               return -1;
           }
       }
       else if (!strcmp(argv[i],"-ts")&&i+1<argc) tspan=atof(argv[++i]);
       else if (!strcmp(argv[i],"-ti")&&i+1<argc) tint =atof(argv[++i]);
       else if (!strcmp(argv[i],"-t" )) topt=1;
       else if (!strcmp(argv[i],"-u" )&&i+2<argc) {
           pos[0]=atof(argv[++i])*D2R;
           pos[1]=atof(argv[++i])*D2R;
       }
       else if (!strcmp(argv[i],"-e" )) eopt=1;
       else if (!strcmp(argv[i],"-o" )) mopt=1;
       else if (!strncmp(argv[i],"-",1)) {
           prusage();
           return 0;
       }
       else files[n++]=argv[i];
    }
    if (trl>0) {
       traceopen("diffeph.trace");
       tracelevel(trl);
    }
    t0=epoch2time(ep0);
    
    init_rtcm(&rtcm);
    rtcm.time=t0;
    
    for (i=0;i<n;i++) {
        if (!(ext=strrchr(files[i],'.'))) ext="";
        if (!strcmp(ext,".sp3")||!strcmp(ext,".SP3")||
           !strcmp(ext,".eph")||!strcmp(ext,".EPH")) {
           if (nav.ne>0) {
               readsp3(files[i],&nav2); /* second precise ephemeris */
           }
           else {
               readsp3(files[i],&nav);
           }
        }
        else if (!strcmp(ext,".atx")) {
           readsap(files[i],t0,&nav);
        }
        else if (!strcmp(ext,".rtcm3")||!strcmp(ext,".log")) {
           if (!(fp=fopen(files[i],"rb"))) {
               fprintf(stderr,"file open error: %s\n",files[i]);
               return -1;
           }
        }
        else if (!strcmp(ext,".lex")) {
           if (!lexreadmsg(files[i],0,&lex)) {
               fprintf(stderr,"file read error: %s\n",files[i]);
               return -1;
           }
        }
        else files[nx++]=files[i]; /* rinex clock */
    }
    for (i=0;i<nx;i++) {
        readrnx(files[i],1,"",NULL,&nav,NULL);
        if (nav.nc>0) {
           readrnxc(files[i],&nav2); /* second precise clock */
        }
        else {
           readrnxc(files[i],&nav);
        }
    } 
    for (i=0;i<(int)(tspan*3600.0/tint);i++) {
       time=timeadd(t0,tint*i);
       
       fprintf(stderr,"time=%s\r",time_str(time,0));
       
       /* update ephemeris in navigation data */
       if (fp) {
           updatertcm(time,&rtcm,&nav,fp);
       }
       else if (lex.n>0) {
           index=updatelex(index,time,&lex,&nav);
       }  
       for (s=1;s<=MAXSAT;s++) {
           if (sat&&s!=sat) continue;
           
           printephdiff(time,s,eph1,eph2,&nav,eph1==1&&eph2==1?&nav2:&nav,topt,
                        eopt,mopt,pos);
       }
    }
    if (fp) fclose(fp);
    if (trl>0) traceclose();
    return 0;
}
