/*------------------------------------------------------------------------------
* pos2kml.c : convert positions to google earth KML or GPX file
*
*          Copyright (C) 2007-2016 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:54:53 $
* history : 2007/01/20  1.0 new
*           2007/03/15  1.1 modify color sequence
*           2007/04/03  1.2 add geodetic height option
*                           support input of NMEA GGA sentence
*                           delete altitude info for track
*                           add time stamp option
*                           separate readsol.c file
*           2008/07/18  1.3 support change of convkml() arguments
*           2016/06/11  1.4 add option -gpx for gpx conversion
*-----------------------------------------------------------------------------*/
#include <stdarg.h>
#include "rtklib.h"

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" usage: pos2kml [option]... file [...]",
"",
" Read solution file(s) and convert it to Google Earth KML file or GPX file.",
" Each line in the input file shall contain fields of time, position fields ",
" (latitude/longitude/height or x/y/z-ecef), and quality flag(option). The line",
" started with '%', '#', ';' is treated as comment. Command options are as ",
" follows. ([]:default)",
"",
" -h        print help",
" -o file   output file [infile + .kml]",
" -c color  track color (0:off,1:white,2:green,3:orange,4:red,5:yellow) [5]",
" -p color  point color (0:off,1:white,2:green,3:orange,4:red,5:by qflag) [5]",
" -a        output altitude information [off]",
" -ag       output geodetic altitude [off]",
" -tg       output time stamp of gpst [off]",
" -tu       output time stamp of utc [gpst]",
" -i tint   output time interval (s) (0:all) [0]",
" -q qflg   output q-flags (0:all) [0]",
" -f n e h  add north/east/height offset to position (m) [0 0 0]",
" -gpx      output GPX file"
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<(int)(sizeof(help)/sizeof(*help));i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* pos2kml main --------------------------------------------------------------*/
int main(int argc, char **argv)
{
    int i,j,n,outalt=0,outtime=0,qflg=0,tcolor=5,pcolor=5,gpx=0,stat;
    char *infile[32],*outfile="";
    double offset[3]={0.0},tint=0.0,es[6]={2000,1,1},ee[6]={2000,1,1};
    gtime_t ts={0},te={0};
    
    for (i=1,n=0;i<argc;i++) {
        if      (!strcmp(argv[i],"-o")&&i+1<argc) outfile=argv[++i];
        else if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",es  ,es+1,es+2);
            sscanf(argv[++i],"%lf:%lf:%lf",es+3,es+4,es+5);
            ts=epoch2time(es);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",ee  ,ee+1,ee+2);
            sscanf(argv[++i],"%lf:%lf:%lf",ee+3,ee+4,ee+5);
            te=epoch2time(ee);
        }
        else if (!strcmp(argv[i],"-c")&&i+1<argc) tcolor=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-p")&&i+1<argc) pcolor=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-f")&&i+3<argc) {
            for (j=0;j<3;j++) offset[j]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-a")) outalt=1;
        else if (!strcmp(argv[i],"-ag")) outalt=2;
        else if (!strcmp(argv[i],"-tg")) outtime=1;
        else if (!strcmp(argv[i],"-tu")) outtime=2;
        else if (!strcmp(argv[i],"-i")&&i+i<argc) tint=atof(argv[++i]);
        else if (!strcmp(argv[i],"-q")&&i+i<argc) qflg=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-gpx")) gpx=1;
        else if (*argv[i]=='-') printhelp();
        else if (n<32) infile[n++]=argv[i];
    }
    if (tcolor<0||5<tcolor||pcolor<0||5<pcolor) {
        fprintf(stderr,"pos2kml : command option error\n");
        return -1;
    }
    if (n<=0) {
        fprintf(stderr,"pos2kml : no input file\n");
        return -1;
    }
    for (i=0;i<n;i++) {
        if (gpx) {
            stat=convgpx(infile[i],outfile,ts,te,tint,qflg,offset,tcolor,pcolor,
                         outalt,outtime);
        }
        else {
            stat=convkml(infile[i],outfile,ts,te,tint,qflg,offset,tcolor,pcolor,
                         outalt,outtime);
        }
        switch (stat) {
        case -1: fprintf(stderr,"pos2kml : file read error (%d)\n",i+1);   break;
        case -2: fprintf(stderr,"pos2kml : file format error (%d)\n",i+1); break;
        case -3: fprintf(stderr,"pos2kml : no input data (%d)\n",i+1);     break;
        case -4: fprintf(stderr,"pos2kml : file write error (%d)\n",i+1);  break;
        }
    }
    return 0;
}
