/*------------------------------------------------------------------------------
* convgpx.c : gpx converter
*
*          Copyright (C) 2016 by T.TAKASU, All rights reserved.
*
* references :
*     [1] GPX The GPS Exchange Format http://www.topografix.com/gpx.asp
*
* version : $Revision:$ $Date:$
* history : 2016/06/11  1.0  new
*           2016/09/18  1.1  modify <fix> labels according GPX specs
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

/* constants -----------------------------------------------------------------*/

#define HEADXML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define HEADGPX "<gpx version=\"1.1\" creator=\"%s\" xmlns=\"%s\">\n"
#define TAILGPX "</gpx>"

static const char *XMLNS="http://www.topografix.com/GPX/1/1";

/* output waypoint -----------------------------------------------------------*/
static void outpoint(FILE *fp, gtime_t time, const double *pos,
                     const char *label, int stat, int outalt, int outtime)
{
    /* fix, float, sbas and ppp are rtklib extentions to GPX */
    const char *fix_label[]={"fix","float","sbas","dgps","3d","ppp"};
    double ep[6];
    
    fprintf(fp,"<wpt lat=\"%.9f\" lon=\"%.9f\">\n",pos[0]*R2D,pos[1]*R2D);
    if (outalt) {
        fprintf(fp," <ele>%.4f</ele>\n",pos[2]-(outalt==2?geoidh(pos):0.0));
    }
    if (outtime) {
        if      (outtime==2) time=gpst2utc(time);
        else if (outtime==3) time=timeadd(gpst2utc(time),9*3600.0);
        time2epoch(time,ep);
        fprintf(fp," <time>%04.0f-%02.0f-%02.0fT%02.0f:%02.0f:%05.2fZ</time>\n",
                ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
    }
    if (outalt==2) {
        fprintf(fp," <geoidheight>%.4f</geoidheight>\n",geoidh(pos));
    }
    if (stat>=1&&stat<=6) {
        fprintf(fp," <fix>%s</fix>\n",fix_label[stat-1]);
    }
    if (*label) {
        fprintf(fp," <name>%s</name>\n",label);
    }
    fprintf(fp,"</wpt>\n");
}
/* output track --------------------------------------------------------------*/
static void outtrack(FILE *fp, const solbuf_t *solbuf, int outalt, int outtime)
{
    gtime_t time;
    double pos[3],ep[6];
    int i;
    
    fprintf(fp,"<trk>\n");
    fprintf(fp," <trkseg>\n");
    for (i=0;i<solbuf->n;i++) {
        ecef2pos(solbuf->data[i].rr,pos);
        fprintf(fp,"  <trkpt lat=\"%.9f\" lon=\"%.9f\">\n",pos[0]*R2D,
                pos[1]*R2D);
        if (outalt) {
            fprintf(fp,"   <ele>%.4f</ele>\n",pos[2]-(outalt==2?geoidh(pos):0.0));
        }
        if (outtime) {
            time=solbuf->data[i].time;
            if      (outtime==2) time=gpst2utc(time);
            else if (outtime==3) time=timeadd(gpst2utc(time),9*3600.0);
            time2epoch(time,ep);
            fprintf(fp,"   <time>%04.0f-%02.0f-%02.0fT%02.0f:%02.0f:%05.2fZ</time>\n",
                    ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
        }
        if (outalt==2) {
            fprintf(fp,"   <geoidheight>%.4f</geoidheight>\n",geoidh(pos));
        }
        fprintf(fp,"  </trkpt>\n");
    }
    fprintf(fp," </trkseg>\n");
    fprintf(fp,"</trk>\n");
}
/* save gpx file -------------------------------------------------------------*/
static int savegpx(const char *file, const solbuf_t *solbuf, int outtrk,
                   int outpnt, int outalt, int outtime)
{
    FILE *fp;
    double pos[3];
    int i;
    
    if (!(fp=fopen(file,"w"))) {
        fprintf(stderr,"file open error : %s\n",file);
        return 0;
    }
    fprintf(fp,HEADXML);
    fprintf(fp,HEADGPX,"RTKLIB " VER_RTKLIB,XMLNS);
    
    /* output waypoint */
    if (outpnt) {
        for (i=0;i<solbuf->n;i++) {
            ecef2pos(solbuf->data[i].rr,pos);
            outpoint(fp,solbuf->data[i].time,pos,"",solbuf->data[i].stat,outalt,
                     outtime);
        }
    }
    /* output waypoint of ref position */
    if (norm(solbuf->rb,3)>0.0) {
        ecef2pos(solbuf->rb,pos);
        outpoint(fp,solbuf->data[0].time,pos,"Reference Position",0,outalt,0);
    }
    /* output track */
    if (outtrk) {
        outtrack(fp,solbuf,outalt,outtime);
    }
    fprintf(fp,"%s\n",TAILGPX);
    fclose(fp);
    return 1;
}
/* convert to GPX file ---------------------------------------------------------
* convert solutions to GPX file [1]
* args   : char   *infile   I   input solutions file
*          char   *outfile  I   output google earth kml file ("":<infile>.kml)
*          gtime_t ts,te    I   start/end time (gpst)
*          int    tint      I   time interval (s) (0.0:all)
*          int    qflg      I   quality flag (0:all)
*          double *offset   I   add offset {east,north,up} (m)
*          int    outtrk    I   output track    (0:off,1:on)
*          int    outpnt    I   output waypoint (0:off,1:on)
*          int    outalt    I   output altitude (0:off,1:elipsoidal,2:geodetic)
*          int    outtime   I   output time (0:off,1:gpst,2:utc,3:jst)
* return : status (0:ok,-1:file read,-2:file format,-3:no data,-4:file write)
*-----------------------------------------------------------------------------*/
extern int convgpx(const char *infile, const char *outfile, gtime_t ts,
                   gtime_t te, double tint, int qflg, double *offset,
                   int outtrk, int outpnt, int outalt, int outtime)
{
    solbuf_t solbuf={0};
    double rr[3]={0},pos[3],dr[3];
    int i,j;
    char *p,file[1024];
    
    trace(3,"convgpx : infile=%s outfile=%s\n",infile,outfile);
    
    if (!*outfile) {
        if ((p=strrchr(infile,'.'))) {
            strncpy(file,infile,p-infile);
            strcpy(file+(p-infile),".gpx");
        }
        else sprintf(file,"%s.gpx",infile);
    }
    else strcpy(file,outfile);
    
    /* read solution file */
    if (!readsolt((char **)&infile,1,ts,te,tint,qflg,&solbuf)) return -1;
    
	/* mean position */
	for (i=0;i<3;i++) {
        for (j=0;j<solbuf.n;j++) rr[i]+=solbuf.data[j].rr[i];
        rr[i]/=solbuf.n;
    }
    /* add offset */
    ecef2pos(rr,pos);
    enu2ecef(pos,offset,dr);
    for (i=0;i<solbuf.n;i++) {
        for (j=0;j<3;j++) solbuf.data[i].rr[j]+=dr[j];
    }
    if (norm(solbuf.rb,3)>0.0) {
        for (i=0;i<3;i++) solbuf.rb[i]+=dr[i];
    }
    /* save gpx file */
    return savegpx(file,&solbuf,outtrk,outpnt,outalt,outtime)?0:-4;
}

