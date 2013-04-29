/*------------------------------------------------------------------------------
* strmon.c : ntrip stream monitor
*
*          Copyright (C) 2013 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2013/02/01  1.0  new
*-----------------------------------------------------------------------------*/
#include <signal.h>
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define SVR_CYCLE       10              /* server cycle (ms) */
#define NBIN_LATE       14              /* number of latency bin */
#define STEP_LATE       0.2             /* step of latency bin */
#define MAXSTR          128             /* max number of monitor streams */
#define PRG_VER         "1.0"           /* version */

#define MIN(x,y)        ((x)<(y)?(x):(y))

/* type definitions ----------------------------------------------------------*/

typedef struct {                        /* stream monitor type */
    char sta[32];                       /* station */
    int  fmt;                           /* format */
    char path_str[1024];                /* input path */
    char path_log[1024];                /* log path */
    stream_t str;                       /* input stream */
    stream_t log;                       /* log stream */
    int stat;                           /* stream status */
    char msg[MAXSTRMSG];                /* stream message */
    int inb,inr;                        /* stream bytes and bps */
    int cnt[5];                         /* count {obs,nav,err,conn,dis} */
    double lat_last;                    /* obs data latency last (s) */
    double lat_ave;                     /* obs data latency average (s) */
    int latency[NBIN_LATE];             /* obs data latency (ms) */
    rtcm_t rtcm;                        /* rtcm control struct */
    raw_t raw;                          /* raw control struct */
} strmon_t;

typedef struct {                        /* stream monitor server */
    gtime_t ts;                         /* statistics start */
    int state;                          /* state (0:stop,1:run) */
    int cyc;                            /* report cycle (h) */
    int status;                         /* show status */
    char report[1024];                  /* report */
    char script[1024];                  /* script */
    int nmon;                           /* number of streams */
    strmon_t mon[MAXSTR];               /* stream monitor */
    thread_t thread;                    /* thread */
} monsvr_t;

/* global variables ----------------------------------------------------------*/
static int intr=0;

/* new stream monitor server -------------------------------------------------*/
static monsvr_t *monsvr_new(int cyc, const char *report, const char *script,
                            const char *optfile)
{
    FILE *fp;
    monsvr_t *svr;
    gtime_t time0={0};
    char buff[2048],sta[32],fmt[32],path_str[1024],path_log[1024],*p;
    
    if (!(svr=(monsvr_t *)malloc(sizeof(monsvr_t)))) return NULL;
    
    svr->ts=time0;
    svr->state=0;
    svr->cyc=cyc;
    svr->status=0;
    strcpy(svr->report,report);
    strcpy(svr->script,script);
    svr->nmon=0;
    
    if (!(fp=fopen(optfile,"r"))) {
        fprintf(stderr,"stream option file open error %s\n",optfile);
        return NULL;
    }
    while (fgets(buff,sizeof(buff),fp)&&svr->nmon<MAXSTR) {
        if ((p=strchr(buff,'#'))) *p='\0';
        
        path_log[0]='\0';
        
        if (sscanf(buff,"%31s %31s %1023s %1023s",sta,fmt,path_str,path_log)<3) {
            continue;
        }
        if      (!strcmp(fmt,"RTCM3")) svr->mon[svr->nmon].fmt=STRFMT_RTCM3;
        else if (!strcmp(fmt,"JAVAD")) svr->mon[svr->nmon].fmt=STRFMT_JAVAD;
        else if (!strcmp(fmt,"BINEX")) svr->mon[svr->nmon].fmt=STRFMT_BINEX;
        else continue;
        strcpy(svr->mon[svr->nmon].sta,sta);
        strcpy(svr->mon[svr->nmon].path_str,path_str);
        strcpy(svr->mon[svr->nmon].path_log,path_log);
        svr->nmon++;
    }
    fclose(fp);
    
    return svr;
}
/* free stream monitor server ------------------------------------------------*/
static void monsvr_free(monsvr_t *svr)
{
    if (!svr) return;
    free(svr);
}
/* replace %S in path --------------------------------------------------------*/
static void repsta(const char *path, char *rpath, const char *sta)
{
    const char *p;
    char *q=rpath;
    
    for (p=path;*p;p++) {
        if (*p!='%') *q++=*p;
        else if (*++p=='S') q+=sprintf(q,"%s",sta);
        else q+=sprintf(q,"%%%c",*p);
    }
    *q='\0';
}
/* reset stream monintor -----------------------------------------------------*/
static void strmon_reset(strmon_t *mon)
{
    int i;
    
    mon->stat=0;
    mon->msg[0]='\0';
    mon->inb=mon->inr=0;
    for (i=0;i<5;i++) mon->cnt[i]=0;
    mon->lat_last=0.0;
    mon->lat_ave=0.0;
    for (i=0;i<NBIN_LATE;i++) mon->latency[i]=0;
}
/* open stream monitor -------------------------------------------------------*/
static void strmon_open(strmon_t *mon)
{
    gtime_t time=utc2gpst(timeget());
    char path1[1024],path2[1024];
    
    strmon_reset(mon);
    
    repsta(mon->path_str,path1,mon->sta);
    repsta(mon->path_log,path2,mon->sta);
    stropen(&mon->str,STR_NTRIPCLI,STR_MODE_R,path1);
    stropen(&mon->log,STR_FILE    ,STR_MODE_W,path2);
    
    init_rtcm(&mon->rtcm);
    init_raw(&mon->raw);
    mon->rtcm.time=time;
    mon->raw.time=time;
}
/* close stream monitor ------------------------------------------------------*/
static void strmon_close(strmon_t *mon)
{
    strclose(&mon->str);
    strclose(&mon->log);
    free_rtcm(&mon->rtcm);
    free_raw(&mon->raw);
}
/* read rtcm 3 message -------------------------------------------------------*/
static void read_rtcm3(strmon_t *mon, gtime_t time, unsigned char *buff, int n)
{
    double tt;
    int i,ret;
    
    for (i=0;i<n;i++) {
        
        ret=input_rtcm3(&mon->rtcm,buff[i]);
        
        if (ret==1) {
            tt=timediff(time,mon->rtcm.obs.data[0].time);
            mon->lat_last=tt;
            mon->lat_ave+=(tt-mon->lat_ave)/(mon->cnt[0]+1);
            mon->latency[MIN((int)(tt/STEP_LATE),NBIN_LATE-1)]++;
            mon->cnt[0]++;
        }
        else if (ret==2) {
            mon->cnt[1]++;
        }
        else if (ret==-1) {
            mon->cnt[2]++;
        }
    }
}
/* read raw message ----------------------------------------------------------*/
static void read_raw(strmon_t *mon, gtime_t time, unsigned char *buff, int n)
{
    double tt;
    int i,ret;
    
    for (i=0;i<n;i++) {
        
        ret=input_raw(&mon->raw,mon->fmt,buff[i]);
        
        if (ret==1) {
            tt=timediff(time,mon->raw.obs.data[0].time);
            mon->lat_last=tt;
            mon->lat_ave+=(tt-mon->lat_ave)/(mon->cnt[0]+1);
            mon->latency[MIN((int)(tt/STEP_LATE),NBIN_LATE-1)]++;
            mon->cnt[0]++;
        }
        else if (ret==2) {
            mon->cnt[1]++;
        }
        else if (ret==-1) {
            mon->cnt[2]++;
        }
    }
}
/* read stream monitor -------------------------------------------------------*/
static void strmon_read(strmon_t *mon, gtime_t time)
{
    unsigned char buff[4096];
    int n;
    
    while ((n=strread(&mon->str,buff,sizeof(buff)))>0) {
        
        switch (mon->fmt) {
            case STRFMT_RTCM3: read_rtcm3(mon,time,buff,n); break;
            case STRFMT_JAVAD: read_raw  (mon,time,buff,n); break;
            case STRFMT_BINEX: read_raw  (mon,time,buff,n); break;
        }
        strwrite(&mon->log,buff,n);
    }
}
/* status of stream monitor --------------------------------------------------*/
static void strmon_stat(strmon_t *mon)
{
    int stat=mon->stat;
    
    mon->stat=strstat(&mon->str,mon->msg);
    
    if      ((stat==0||stat==1)&&(mon->stat==2||mon->stat==3)) mon->cnt[3]++;
    else if ((stat==2||stat==3)&&(mon->stat==0||mon->stat==1)) mon->cnt[4]++;
    
    strsum(&mon->str,&mon->inb,&mon->inr,NULL,NULL);
}
/* write stream status -------------------------------------------------------*/
static void write_stat(monsvr_t *svr, gtime_t time, FILE *fp)
{
    double avail,span;
    char *sstr[]={"E","-","W","C","C"},*fmt;
    int i;
    
    span=floor(timediff(time,svr->ts));
    
    fprintf(fp,"%-6s %6s %7s %5s %5s %5s %7s %10s %8s %1s %s\n","#STA","FMT",
            "#OBS","#NAV","#ERR","#DIS","AVAIL","IN_BYTES","IN_BPS","S",
            "NTRIP_CASTER/MOUNT_POINT");
    
    for (i=0;i<svr->nmon;i++) {
        switch (svr->mon[i].fmt) {
            case STRFMT_RTCM3: fmt="RTCM3"; break;
            case STRFMT_JAVAD: fmt="JAVAD"; break;
            case STRFMT_BINEX: fmt="BINEX"; break;
            default: fmt=""; break;
        }
        avail=span<=0?0.0:svr->mon[i].cnt[0]/span;
        fprintf(fp,"%-6s %6s %7d %5d %5d %5d %6.1f%% %10d %8d %s %s\n",
                svr->mon[i].sta,fmt,svr->mon[i].cnt[0],svr->mon[i].cnt[1],
                svr->mon[i].cnt[2],svr->mon[i].cnt[4],avail*100.0,
                svr->mon[i].inb,svr->mon[i].inr,sstr[svr->mon[i].stat+1],
                svr->mon[i].msg);
    }
    fprintf(fp,"\n");
}
/* write stream latency ------------------------------------------------------*/
static void write_late(monsvr_t *svr, gtime_t time, FILE *fp)
{
    int i,j;
    
    fprintf(fp,"%-6s %6s %6s","#STA","LATE","AVER");
    for (i=0;i<NBIN_LATE-1;i++) fprintf(fp,"  <%3.1f",(i+1)*STEP_LATE);
    fprintf(fp,"  >%3.1f\n",i*STEP_LATE);
    
    for (i=0;i<svr->nmon;i++) {
        fprintf(fp,"%-6s %6.2f %6.2f",svr->mon[i].sta,svr->mon[i].lat_last,
                svr->mon[i].lat_ave);
        for (j=0;j<NBIN_LATE;j++) fprintf(fp," %5d",svr->mon[i].latency[j]);
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
}
/* write report ----------------------------------------------------------------*/
static void write_report(monsvr_t *svr, gtime_t time, FILE *fp)
{
    char str1[32],str2[32];
    
    time2str(svr->ts,str1,0);
    time2str(time,str2,0);
    fprintf(fp,"*** NTRIP STREAM MONITOR v.%s: %s - %s ***\n\n",
            PRG_VER,str1,str2+11);
    write_stat(svr,time,fp);
    write_late(svr,time,fp);
}
/* generate report -------------------------------------------------------------*/
static int gen_report(monsvr_t *svr, gtime_t time)
{
    FILE *fp;
    char path[1024];
    int i;
    
    reppath(svr->report,path,timeadd(time,-svr->cyc*3600.0),"","");
    
    if (!(fp=fopen(path,"w"))) {
        fprintf(stderr,"report saved error %s\n",path);
        return 0;
    }
    write_report(svr,time,fp);
    
    fclose(fp);
    
    for (i=0;i<svr->nmon;i++) {
        strmon_reset(svr->mon+i);
    }
    svr->ts=time;
    
    return 1;
}
/* montor server thread ------------------------------------------------------*/
static void *monsvr_thread(void *arg)
{
    monsvr_t *svr=(monsvr_t *)arg;
    gtime_t time;
    unsigned int tick;
    int i,j,n,m=-1;
    
    for (i=0;svr->state;i++) {
        tick=tickget();
        
        time=utc2gpst(timeget());
        
        for (j=0;j<svr->nmon;j++) {
            strmon_read(svr->mon+j,time);
        }
        if (i%25==0) {
            for (j=0;j<svr->nmon;j++) {
                strmon_stat(svr->mon+j);
            }
            if (svr->status) {
                write_report(svr,time,stderr);
            }
            n=(int)floor(time2gpst(time,NULL)/svr->cyc/3600.0);
            if (n!=m&&m>=0) gen_report(svr,time);
            m=n;
        }
        sleepms(SVR_CYCLE-(int)(tickget()-tick));
    }
    return 0;
}
/* start monitor server ------------------------------------------------------*/
static int monsvr_start(monsvr_t *svr, int status)
{
    int i,opt[8]={30000,30000,3000,32768,10};
    
    strsetopt(opt);
    
    for (i=0;i<svr->nmon;i++) {
        strmon_open(svr->mon+i);
    }
    svr->state=1;
    svr->ts=utc2gpst(timeget());
    svr->status=status;
    
    if (pthread_create(&svr->thread,NULL,monsvr_thread,svr)) {
        
        for (i=0;i<svr->nmon;i++) {
            strmon_close(svr->mon+i);
        }
        return 0;
    }
    return 1;
}
/* stop monitor server -------------------------------------------------------*/
static void monsvr_stop(monsvr_t *svr)
{
    int i;
    
    if (!svr->state) return;
    
    svr->state=0;
    pthread_join(svr->thread,NULL);
    
    for (i=0;i<svr->nmon;i++) {
        strmon_close(svr->mon+i);
    }
    svr->nmon=0;
}
/* signal handler ------------------------------------------------------------*/
static void sigint(int sig)
{
    intr=1;
    signal(sig,sigint);
}
/* main ------------------------------------------------------------------------
*  name:
*    strmon - ntrip stream monitor
*
*  synopsis:
*    strmon [options] [optfile]
*
*  description:
*    Receive NTRIP streams, log the streams and monitor statuses, latencies,
*    errors, availabity of the streams. The defintion of streams are read
*    from configuration file <optfile>. If <optfile> omitted, "strmon.conf" is
*    used.
*
*  options ([]: default) :
*    -c   cyc          monitor report cycle (hr) [1]
*    -r   file         monitor report file [strmon%Y%m%d%h.txt]
*    -x   script       script after generating monitor report []
*    -s                show realtime monitor [no]
*
*  configuration file format
*    A text line specify a record specifying a input stream. A record contains
*    the following fileds. Strings after # are dealed as comments.
*
*    record format:
*
*    STA  FORMAT   PATH_STR      PATH_LOG
*
*    (1) STA      : station or stream name
*
*    (2) FORMAT   : stream format
*
*        RTCM3 = RTCM 3
*        JAVAD = JAVAD JPS
*        BINEX = BINEX
*
*    (3) PATH_STR : ntrip steam path
*
*        [user:password@]ntrip_caster_address/mount_point
*        
*        keyword %S is replaced as follows
*
*    (3) PATH_LOG : log file path (optional)
*
*        keywords %Y,%y,%m,%d,%h,%H,%M,%n,%W,%D,%S are replaced as follows
*
*  keywords replaced in path
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
*    %S -> ssss        station or stream name
*
*-----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    monsvr_t *svr;
    char *report="strmon%Y%m%d%h.txt",*script="",*optfile="strmon.conf";
    int i,cyc=1,status=0;
    
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-c")&&i+1<argc) cyc=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-r")&&i+1<argc) report=argv[++i];
        else if (!strcmp(argv[i],"-x")&&i+1<argc) script=argv[++i];
        else if (!strcmp(argv[i],"-s")) status=1;
        else optfile=argv[i];
    }
    if (!(svr=monsvr_new(cyc,report,script,optfile))) return -1;
    
    if (!monsvr_start(svr,status)) {
        monsvr_free(svr);
        return -1;
    }
    signal(SIGINT,sigint);
    signal(SIGPIPE,SIG_IGN);
    
    while (!intr&&svr->state) sleepms(100);
    
    monsvr_stop(svr);
    monsvr_free(svr);
    
    return 0;
}
