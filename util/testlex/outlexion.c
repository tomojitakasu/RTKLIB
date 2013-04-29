/*------------------------------------------------------------------------------
* outlexion.c : output lex ionosphere correction as matrix
*
* 2010/12/09 0.1 new
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "rtklib.h"

/* update lex ephemeris ------------------------------------------------------*/
static int updatelex(int index, gtime_t time, lex_t *lex, nav_t *nav)
{
    gtime_t tof;
    
    for (;index<lex->n;index++) {
        if (!lexupdatecorr(lex->msgs+index,nav,&tof)) continue;
        fprintf(stderr,"%6d: tof=%s\r",index,time_str(tof,0));
        if (timediff(tof,time)>=0.0) break;
    }
    return index;
}
/* print tec grid data -------------------------------------------------------*/
static void printtec(int index, gtime_t time, double sec, const nav_t *nav,
                     const double *rpos, int nlat, int nlon, double dpos,
                     FILE *fp)
{
    int i,j;
    double lat0,lon0,pos[3]={0},azel[]={0,PI/2.0},ion,var;
    
    lat0=rpos[0]+dpos*((nlat-1)/2);
    lon0=rpos[1]-dpos*((nlon-1)/2);
    
    if (index==1) {
        fprintf(fp,"lons =[");
        for (i=0;i<nlon;i++) fprintf(fp,"%.1f%s",lon0+dpos*i,i<nlon-1?" ":"");
        fprintf(fp,"];\n");
        fprintf(fp,"lats =[");
        for (i=0;i<nlat;i++) fprintf(fp,"%.1f%s",lat0-dpos*i,i<nlat-1?" ":"");
        fprintf(fp,"];\n\n");
    }
    fprintf(fp,"%% %s\n",time_str(time,0));
    fprintf(fp,"time(%d)=%.0f;\n",index,sec);
    fprintf(fp,"tec(:,:,%d)=[\n",index);
    for (i=0;i<nlat;i++) {
        for (j=0;j<nlon;j++) {
            pos[0]=(lat0-dpos*i)*D2R;
            pos[1]=(lon0+dpos*j)*D2R;
            if (lexioncorr(time,nav,pos,azel,&ion,&var)) {
                /*tec=ion*FREQ1*FREQ1/40.3E16;*/ /* L1 iono delay -> tecu */
                fprintf(fp,"%7.3f ",ion);
            }
            else {
                fprintf(fp,"%7s ","nan");
            }
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"];\n");
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    FILE *fp=NULL;
    nav_t nav={0};
    lex_t lex={0};
    gtime_t t0,time;
    double ep0[]={2000,1,1,0,0,0},tspan=24.0,tint=7200,dpos=1;
    double rpos[]={35,137};
    char *ifile="",*ofile="";
    int i,trl=0,index=0,nlat=45,nlon=45;
    
    t0=epoch2time(ep0);
    
    for (i=1;i<argc;i++) {
       if (!strcmp(argv[i],"-t0")&&i+2<argc) {
           if (sscanf(argv[++i],"%lf/%lf/%lf",ep0  ,ep0+1,ep0+2)<3||
               sscanf(argv[++i],"%lf:%lf:%lf",ep0+3,ep0+4,ep0+5)<1) {
               fprintf(stderr,"invalid time\n");
               return -1;
           }
       }
       else if (!strcmp(argv[i],"-ts")&&i+1<argc) tspan=atof(argv[++i]);
       else if (!strcmp(argv[i],"-ti")&&i+1<argc) tint =atof(argv[++i]);
       else if (!strcmp(argv[i],"-u" )&&i+2<argc) {
           rpos[0]=atof(argv[++i]);
           rpos[1]=atof(argv[++i]);
       }
       else if (!strcmp(argv[i],"-n" )&&i+2<argc) {
           nlat=atoi(argv[++i]);
           nlon=atoi(argv[++i]);
       }
       else if (!strcmp(argv[i],"-d" )&&i+1<argc) dpos =atof(argv[++i]);
       else if (!strcmp(argv[i],"-o" )&&i+1<argc) ofile=argv[++i];
       else if (!strcmp(argv[i],"-x" )&&i+1<argc) trl  =atof(argv[++i]);
       else ifile=argv[i];
    }
    if (trl>0) {
       traceopen("diffeph.trace");
       tracelevel(trl);
    }
    t0=epoch2time(ep0);
    
    if (!lexreadmsg(ifile,0,&lex)) {
        fprintf(stderr,"file read error: %s\n",ifile);
        return -1;
    }
    if (!(fp=fopen(ofile,"w"))) {
        fprintf(stderr,"file open error: %s\n",ofile);
        return -1;
    }
    fprintf(fp,"epoch=[%.0f %.0f %.0f %.0f %.0f %.0f];\n",
            ep0[0],ep0[1],ep0[2],ep0[3],ep0[4],ep0[5]);
    
    for (i=0;i<(int)(tspan*3600.0/tint);i++) {
       time=timeadd(t0,tint*i);
       
       fprintf(stderr,"time=%s\r",time_str(time,0));
       
       index=updatelex(index,time,&lex,&nav);
       
       printtec(i+1,time,tint*i,&nav,rpos,nlat,nlon,dpos,fp);
    }
    fclose(fp);
    if (trl>0) traceclose();
    return 0;
}
