/*------------------------------------------------------------------------------
* rtkrcv.c : rtk-gps/gnss receiver console ap
*
*          Copyright (C) 2009-2015 by T.TAKASU, All rights reserved.
*
* notes   :
*     current version does not support win32 without pthread library
*
* version : $Revision:$ $Date:$
* history : 2009/12/13 1.0  new
*           2010/07/18 1.1  add option -m
*           2010/08/12 1.2  fix bug on ftp/http
*           2011/01/22 1.3  add option misc-proxyaddr,misc-fswapmargin
*           2011/08/19 1.4  fix bug on size of arg solopt arg for rtksvrstart()
*           2012/11/03 1.5  fix bug on setting output format
*           2013/06/30 1.6  add "nvs" option for inpstr*-format
*           2014/02/10 1.7  fix bug on printing obs data
*                           add print of status, glonass nav data
*                           ignore SIGHUP
*           2014/04/27 1.8  add "binex" option for inpstr*-format
*           2014/08/10 1.9  fix cpu overload with abnormal telnet shutdown
*           2014/08/26 1.10 support input format "rt17"
*                           change file paths of solution status and debug trace
*           2015/01/10 1.11 add line editting and command history
*                           separate codes for virtual console to vt.c
*           2015/05/22 1.12 fix bug on sp3 id in inpstr*-format options
*           2015/07/31 1.13 accept 4:stat for outstr1-format or outstr2-format
*                           add reading satellite dcb
*           2015/12/14 1.14 add option -sta for station name (#339)
*           2015/12/25 1.15 fix bug on -sta option (#339)
*           2015/01/26 1.16 support septentrio
*           2016/07/01 1.17 support CMR/CMR+
*           2016/08/20 1.18 add output of patch level with version
*           2016/09/05 1.19 support ntrip caster for output stream
*           2016/09/19 1.20 support multiple remote console connections
*                           add option -w
*           2017/09/01 1.21 add command ssr
*-----------------------------------------------------------------------------*/
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "rtklib.h"
#include "vt.h"

#define PRGNAME     "rtkrcv"            /* program name */
#define CMDPROMPT   "rtkrcv> "          /* command prompt */
#define MAXCON      32                  /* max number of consoles */
#define MAXARG      10                  /* max number of args in a command */
#define MAXCMD      256                 /* max length of a command */
#define MAXSTR      1024                /* max length of a stream */
#define OPTSDIR     "."                 /* default config directory */
#define OPTSFILE    "rtkrcv.conf"       /* default config file */
#define NAVIFILE    "rtkrcv.nav"        /* navigation save file */
#define STATFILE    "rtkrcv_%Y%m%d%h%M.stat"  /* solution status file */
#define TRACEFILE   "rtkrcv_%Y%m%d%h%M.trace" /* debug trace file */
#define INTKEEPALIVE 1000               /* keep alive interval (ms) */

#define ESC_CLEAR   "\033[H\033[2J"     /* ansi/vt100 escape: erase screen */
#define ESC_RESET   "\033[0m"           /* ansi/vt100: reset attribute */
#define ESC_BOLD    "\033[1m"           /* ansi/vt100: bold */

#define SQRT(x)     ((x)<=0.0||(x)!=(x)?0.0:sqrt(x))

/* type defintions -----------------------------------------------------------*/

typedef struct {                       /* console type */
    int state;                         /* state (0:stop,1:run) */
    vt_t *vt;                          /* virtual terminal */
    pthread_t thread;                  /* console thread */
} con_t;

/* function prototypes -------------------------------------------------------*/
extern FILE *popen(const char *, const char *);
extern int pclose(FILE *);

/* global variables ----------------------------------------------------------*/
static rtksvr_t svr;                    /* rtk server struct */
static stream_t moni;                   /* monitor stream */

static int intflg       =0;             /* interrupt flag (2:shtdown) */

static char passwd[MAXSTR]="admin";     /* login password */
static int timetype     =0;             /* time format (0:gpst,1:utc,2:jst,3:tow) */
static int soltype      =0;             /* sol format (0:dms,1:deg,2:xyz,3:enu,4:pyl) */
static int solflag      =2;             /* sol flag (1:std+2:age/ratio/ns) */
static int strtype[]={                  /* stream types */
    STR_SERIAL,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE
};
static char strpath[8][MAXSTR]={"","","","","","","",""}; /* stream paths */
static int strfmt[]={                   /* stream formats */
    STRFMT_UBX,STRFMT_RTCM3,STRFMT_SP3,SOLF_LLH,SOLF_NMEA
};
static int svrcycle     =10;            /* server cycle (ms) */
static int timeout      =10000;         /* timeout time (ms) */
static int reconnect    =10000;         /* reconnect interval (ms) */
static int nmeacycle    =5000;          /* nmea request cycle (ms) */
static int buffsize     =32768;         /* input buffer size (bytes) */
static int navmsgsel    =0;             /* navigation mesaage select */
static char proxyaddr[256]="";          /* http/ntrip proxy */
static int nmeareq      =0;             /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] ={0,0,0};       /* nmea position (lat/lon/height) (deg,m) */
static char rcvcmds[3][MAXSTR]={""};    /* receiver commands files */
static char startcmd[MAXSTR]="";        /* start command */
static char stopcmd [MAXSTR]="";        /* stop command */
static int modflgr[256] ={0};           /* modified flags of receiver options */
static int modflgs[256] ={0};           /* modified flags of system options */
static int moniport     =0;             /* monitor port */
static int keepalive    =0;             /* keep alive flag */
static int fswapmargin  =30;            /* file swap margin (s) */
static char sta_name[256]="";           /* station name */

static prcopt_t prcopt;                 /* processing options */
static solopt_t solopt[2]={{0}};        /* solution options */
static filopt_t filopt  ={""};          /* file options */

/* help text -----------------------------------------------------------------*/
static const char *usage[]={
    "usage: rtkrcv [-s][-p port][-d dev][-o file][-w pwd][-r level][-t level][-sta sta]",
    "options",
    "  -s         start RTK server on program startup",
    "  -p port    port number for telnet console",
    "  -m port    port number for monitor stream",
    "  -d dev     terminal device for console",
    "  -o file    processing options file",
    "  -w pwd     login password for remote console (\"\": no password)",
    "  -r level   output solution status file (0:off,1:states,2:residuals)",
    "  -t level   debug trace level (0:off,1-5:on)",
    "  -sta sta   station name for receiver dcb"
};
static const char *helptxt[]={
    "start                 : start rtk server",
    "stop                  : stop rtk server",
    "restart               : restart rtk sever",
    "solution [cycle]      : show solution",
    "status [cycle]        : show rtk status",
    "satellite [-n] [cycle]: show satellite status",
    "observ [-n] [cycle]   : show observation data",
    "navidata [cycle]      : show navigation data",
    "stream [cycle]        : show stream status",
    "ssr [cycle]           : show ssr corrections",
    "error                 : show error/warning messages",
    "option [opt]          : show option(s)",
    "set opt [val]         : set option",
    "load [file]           : load options from file",
    "save [file]           : save options to file",
    "log [file|off]        : start/stop log to file",
    "help|? [path]         : print help",
    "exit|ctr-D            : logout console (only for telnet)",
    "shutdown              : shutdown rtk server",
    "!command [arg...]     : execute command in shell",
    ""
};
static const char *pathopts[]={         /* path options help */
    "stream path formats",
    "serial   : port[:bit_rate[:byte[:parity(n|o|e)[:stopb[:fctr(off|on)]]]]]",
    "file     : path[::T[::+offset][::xspeed]]",
    "tcpsvr   : :port",
    "tcpcli   : addr:port",
    "ntripsvr : user:passwd@addr:port/mntpnt[:str]",
    "ntripcli : user:passwd@addr:port/mntpnt",
    "ntripc_s : :passwd@:port",
    "ntripc_c : user:passwd@:port",
    "ftp      : user:passwd@addr/path[::T=poff,tint,off,rint]",
    "http     : addr/path[::T=poff,tint,off,rint]",
    ""
};
/* receiver options table ----------------------------------------------------*/
#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr,11:ntripc_c"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,10:nvs,11:binex,12:rt17,13:sbf,14:cmr,15:tersus,18:sp3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea,4:stat"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[]={
    {"console-passwd",  2,  (void *)passwd,              ""     },
    {"console-timetype",3,  (void *)&timetype,           TIMOPT },
    {"console-soltype", 3,  (void *)&soltype,            CONOPT },
    {"console-solflag", 0,  (void *)&solflag,            FLGOPT },
    
    {"inpstr1-type",    3,  (void *)&strtype[0],         ISTOPT },
    {"inpstr2-type",    3,  (void *)&strtype[1],         ISTOPT },
    {"inpstr3-type",    3,  (void *)&strtype[2],         ISTOPT },
    {"inpstr1-path",    2,  (void *)strpath [0],         ""     },
    {"inpstr2-path",    2,  (void *)strpath [1],         ""     },
    {"inpstr3-path",    2,  (void *)strpath [2],         ""     },
    {"inpstr1-format",  3,  (void *)&strfmt [0],         FMTOPT },
    {"inpstr2-format",  3,  (void *)&strfmt [1],         FMTOPT },
    {"inpstr3-format",  3,  (void *)&strfmt [2],         FMTOPT },
    {"inpstr2-nmeareq", 3,  (void *)&nmeareq,            NMEOPT },
    {"inpstr2-nmealat", 1,  (void *)&nmeapos[0],         "deg"  },
    {"inpstr2-nmealon", 1,  (void *)&nmeapos[1],         "deg"  },
    {"inpstr2-nmeahgt", 1,  (void *)&nmeapos[2],         "m"    },
    {"outstr1-type",    3,  (void *)&strtype[3],         OSTOPT },
    {"outstr2-type",    3,  (void *)&strtype[4],         OSTOPT },
    {"outstr1-path",    2,  (void *)strpath [3],         ""     },
    {"outstr2-path",    2,  (void *)strpath [4],         ""     },
    {"outstr1-format",  3,  (void *)&strfmt [3],         SOLOPT },
    {"outstr2-format",  3,  (void *)&strfmt [4],         SOLOPT },
    {"logstr1-type",    3,  (void *)&strtype[5],         OSTOPT },
    {"logstr2-type",    3,  (void *)&strtype[6],         OSTOPT },
    {"logstr3-type",    3,  (void *)&strtype[7],         OSTOPT },
    {"logstr1-path",    2,  (void *)strpath [5],         ""     },
    {"logstr2-path",    2,  (void *)strpath [6],         ""     },
    {"logstr3-path",    2,  (void *)strpath [7],         ""     },
    
    {"misc-svrcycle",   0,  (void *)&svrcycle,           "ms"   },
    {"misc-timeout",    0,  (void *)&timeout,            "ms"   },
    {"misc-reconnect",  0,  (void *)&reconnect,          "ms"   },
    {"misc-nmeacycle",  0,  (void *)&nmeacycle,          "ms"   },
    {"misc-buffsize",   0,  (void *)&buffsize,           "bytes"},
    {"misc-navmsgsel",  3,  (void *)&navmsgsel,          MSGOPT },
    {"misc-proxyaddr",  2,  (void *)proxyaddr,           ""     },
    {"misc-fswapmargin",0,  (void *)&fswapmargin,        "s"    },
    
    {"misc-startcmd",   2,  (void *)startcmd,            ""     },
    {"misc-stopcmd",    2,  (void *)stopcmd,             ""     },
    
    {"file-cmdfile1",   2,  (void *)rcvcmds[0],          ""     },
    {"file-cmdfile2",   2,  (void *)rcvcmds[1],          ""     },
    {"file-cmdfile3",   2,  (void *)rcvcmds[2],          ""     },
    
    {"",0,NULL,""}
};
/* print usage ---------------------------------------------------------------*/
static void printusage(void)
{
    int i;
    for (i=0;i<(int)(sizeof(usage)/sizeof(*usage));i++) {
        fprintf(stderr,"%s\n",usage[i]);
    }
    exit(0);
}
/* external stop signal ------------------------------------------------------*/
static void sigshut(int sig)
{
    trace(3,"sigshut: sig=%d\n",sig);
    
    intflg=1;
}
/* discard space characters at tail ------------------------------------------*/
static void chop(char *str)
{
    char *p;
    for (p=str+strlen(str)-1;p>=str&&!isgraph((int)*p);p--) *p='\0';
}
/* thread to send keep alive for monitor port --------------------------------*/
static void *sendkeepalive(void *arg)
{
    trace(3,"sendkeepalive: start\n");
    
    while (keepalive) {
        strwrite(&moni,(unsigned char *)"\r",1);
        sleepms(INTKEEPALIVE);
    }
    trace(3,"sendkeepalive: stop\n");
    return NULL;
}
/* open monitor port ---------------------------------------------------------*/
static int openmoni(int port)
{
    pthread_t thread;
    char path[64];
    
    trace(3,"openmomi: port=%d\n",port);
    
    sprintf(path,":%d",port);
    if (!stropen(&moni,STR_TCPSVR,STR_MODE_RW,path)) return 0;
    strsettimeout(&moni,timeout,reconnect);
    keepalive=1;
    pthread_create(&thread,NULL,sendkeepalive,NULL);
    return 1;
}
/* close monitor port --------------------------------------------------------*/
static void closemoni(void)
{
    trace(3,"closemoni:\n");
    keepalive=0;
    
    /* send disconnect message */
    strwrite(&moni,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
    
    /* wait fin from clients */
    sleepms(1000);
    
    strclose(&moni);
}
/* confirm overwrite ---------------------------------------------------------*/
static int confwrite(vt_t *vt, const char *file)
{
    FILE *fp;
    char buff[MAXSTR],*p;
    
    strcpy(buff,file);
    if ((p=strstr(buff,"::"))) *p='\0'; /* omit options in path */
    if (!vt->state||!(fp=fopen(buff,"r"))) return 1; /* no existing file */
    fclose(fp);
    vt_printf(vt,"overwrite %-16s ? (y/n): ",buff);
    if (!vt_gets(vt,buff,sizeof(buff))||vt->brk) return 0;
    return toupper((int)buff[0])=='Y';
}
/* login ---------------------------------------------------------------------*/
static int login(vt_t *vt)
{
    char buff[256];
    
    trace(3,"login: passwd=%s type=%d\n",passwd,vt->type);
    
    if (!*passwd||!vt->type) return 1;
    
    while (!(intflg&2)) {
        if (!vt_printf(vt,"password: ",PRGNAME)) return 0;
        vt->blind=1;
        if (!vt_gets(vt,buff,sizeof(buff))||vt->brk) {
            vt->blind=0;
            return 0;
        }
        vt->blind=0;
        if (!strcmp(buff,passwd)) break;
        vt_printf(vt,"\ninvalid password\n");
    }
    return 1;
}
/* read receiver commands ----------------------------------------------------*/
static int readcmd(const char *file, char *cmd, int type)
{
    FILE *fp;
    char buff[MAXSTR],*p=cmd;
    int i=0;
    
    trace(3,"readcmd: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) return 0;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (*buff=='@') i++;
        else if (i==type&&p+strlen(buff)+1<cmd+MAXRCVCMD) {
            p+=sprintf(p,"%s",buff);
        }
    }
    fclose(fp);
    return 1;
}
/* read antenna file ---------------------------------------------------------*/
static void readant(vt_t *vt, prcopt_t *opt, nav_t *nav)
{
    const pcv_t pcv0={0};
    pcvs_t pcvr={0},pcvs={0};
    pcv_t *pcv;
    gtime_t time=timeget();
    int i;
    
    trace(3,"readant:\n");
    
    opt->pcvr[0]=opt->pcvr[1]=pcv0;
    if (!*filopt.rcvantp) return;
    
    if (readpcv(filopt.rcvantp,&pcvr)) {
        for (i=0;i<2;i++) {
            if (!*opt->anttype[i]) continue;
            if (!(pcv=searchpcv(0,opt->anttype[i],time,&pcvr))) {
                vt_printf(vt,"no antenna %s in %s",opt->anttype[i],filopt.rcvantp);
                continue;
            }
            opt->pcvr[i]=*pcv;
        }
    }
    else vt_printf(vt,"antenna file open error %s",filopt.rcvantp);
    
    if (readpcv(filopt.satantp,&pcvs)) {
        for (i=0;i<MAXSAT;i++) {
            if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
            nav->pcvs[i]=*pcv;
        }
    }
    else vt_printf(vt,"antenna file open error %s",filopt.satantp);
    
    free(pcvr.pcv); free(pcvs.pcv);
}
/* start rtk server ----------------------------------------------------------*/
static int startsvr(vt_t *vt)
{
    static sta_t sta[MAXRCV]={{""}};
    double pos[3],npos[3];
    char s1[3][MAXRCVCMD]={"","",""},*cmds[]={NULL,NULL,NULL};
    char s2[3][MAXRCVCMD]={"","",""},*cmds_periodic[]={NULL,NULL,NULL};
    char *ropts[]={"","",""};
    char *paths[]={
        strpath[0],strpath[1],strpath[2],strpath[3],strpath[4],strpath[5],
        strpath[6],strpath[7]
    };
    char errmsg[2048]="";
    int i,ret,stropt[8]={0};
    
    trace(3,"startsvr:\n");
    
    /* read start commads from command files */
    for (i=0;i<3;i++) {
        if (!*rcvcmds[i]) continue;
        if (!readcmd(rcvcmds[i],s1[i],0)) {
            vt_printf(vt,"no command file: %s\n",rcvcmds[i]);
        }
        else cmds[i]=s1[i];
        if (!readcmd(rcvcmds[i],s2[i],2)) {
            vt_printf(vt,"no command file: %s\n",rcvcmds[i]);
        }
        else cmds_periodic[i]=s2[i];
    }
    /* confirm overwrite */
    for (i=3;i<8;i++) {
        if (strtype[i]==STR_FILE&&!confwrite(vt,strpath[i])) return 0;
    }
    if (prcopt.refpos==4) { /* rtcm */
        for (i=0;i<3;i++) prcopt.rb[i]=0.0;
    }
    pos[0]=nmeapos[0]*D2R;
    pos[1]=nmeapos[1]*D2R;
    pos[2]=nmeapos[2];
    pos2ecef(pos,npos);
    
    /* read antenna file */
    readant(vt,&prcopt,&svr.nav);
    
    /* read dcb file */
    if (filopt.dcb) {
        strcpy(sta[0].name,sta_name);
        readdcb(filopt.dcb,&svr.nav,sta);
    }
    /* open geoid data file */
    if (solopt[0].geoid>0&&!opengeoid(solopt[0].geoid,filopt.geoid)) {
        trace(2,"geoid data open error: %s\n",filopt.geoid);
        vt_printf(vt,"geoid data open error: %s\n",filopt.geoid);
    }
    for (i=0;*rcvopts[i].name;i++) modflgr[i]=0;
    for (i=0;*sysopts[i].name;i++) modflgs[i]=0;
    
    /* set stream options */
    stropt[0]=timeout;
    stropt[1]=reconnect;
    stropt[2]=1000;
    stropt[3]=buffsize;
    stropt[4]=fswapmargin;
    strsetopt(stropt);
    
    if (strfmt[2]==8) strfmt[2]=STRFMT_SP3;
    
    /* set ftp/http directory and proxy */
    strsetdir(filopt.tempdir);
    strsetproxy(proxyaddr);
    
    /* execute start command */
    if (*startcmd&&(ret=system(startcmd))) {
        trace(2,"command exec error: %s (%d)\n",startcmd,ret);
        vt_printf(vt,"command exec error: %s (%d)\n",startcmd,ret);
    }
    solopt[0].posf=strfmt[3];
    solopt[1].posf=strfmt[4];
    
    /* start rtk server */
    if (!rtksvrstart(&svr,svrcycle,buffsize,strtype,paths,strfmt,navmsgsel,
                     cmds,cmds_periodic,ropts,nmeacycle,nmeareq,npos,&prcopt,
                     solopt,&moni,errmsg)) {
        trace(2,"rtk server start error (%s)\n",errmsg);
        vt_printf(vt,"rtk server start error (%s)\n",errmsg);
        return 0;
    }
    return 1;
}
/* stop rtk server -----------------------------------------------------------*/
static void stopsvr(vt_t *vt)
{
    char s[3][MAXRCVCMD]={"","",""},*cmds[]={NULL,NULL,NULL};
    int i,ret;
    
    trace(3,"stopsvr:\n");
    
    if (!svr.state) return;
    
    /* read stop commads from command files */
    for (i=0;i<3;i++) {
        if (!*rcvcmds[i]) continue;
        if (!readcmd(rcvcmds[i],s[i],1)) {
            vt_printf(vt,"no command file: %s\n",rcvcmds[i]);
        }
        else cmds[i]=s[i];
    }
    /* stop rtk server */
    rtksvrstop(&svr,cmds);
    
    /* execute stop command */
    if (*stopcmd&&(ret=system(stopcmd))) {
        trace(2,"command exec error: %s (%d)\n",stopcmd,ret);
        vt_printf(vt,"command exec error: %s (%d)\n",stopcmd,ret);
    }
    if (solopt[0].geoid>0) closegeoid();
    
    vt_printf(vt,"stop rtk server\n");
}
/* print time ----------------------------------------------------------------*/
static void prtime(vt_t *vt, gtime_t time)
{
    double tow;
    int week;
    char tstr[64]="";
    
    if (timetype==1) {
        time2str(gpst2utc(time),tstr,2);
    }
    else if (timetype==2) {
        time2str(timeadd(gpst2utc(time),9*3600.0),tstr,2);
    }
    else if (timetype==3) {
        tow=time2gpst(time,&week); sprintf(tstr,"  %04d %9.2f",week,tow);
    }
    else time2str(time,tstr,1);
    vt_printf(vt,"%s ",tstr);
}
/* print solution ------------------------------------------------------------*/
static void prsolution(vt_t *vt, const sol_t *sol, const double *rb)
{
    const char *solstr[]={"------","FIX","FLOAT","SBAS","DGPS","SINGLE","PPP",""};
    double pos[3]={0},Qr[9],Qe[9]={0},dms1[3]={0},dms2[3]={0},bl[3]={0};
    double enu[3]={0},pitch=0.0,yaw=0.0,len;
    int i;
    
    trace(4,"prsolution:\n");
    
    if (sol->time.time==0||!sol->stat) return;
    prtime(vt,sol->time);
    vt_printf(vt,"(%-6s)",solstr[sol->stat]);
    
    if (norm(sol->rr,3)>0.0&&norm(rb,3)>0.0) {
        for (i=0;i<3;i++) bl[i]=sol->rr[i]-rb[i];
    }
    len=norm(bl,3);
    Qr[0]=sol->qr[0];
    Qr[4]=sol->qr[1];
    Qr[8]=sol->qr[2];
    Qr[1]=Qr[3]=sol->qr[3];
    Qr[5]=Qr[7]=sol->qr[4];
    Qr[2]=Qr[6]=sol->qr[5];
    
    if (soltype==0) {
        if (norm(sol->rr,3)>0.0) {
            ecef2pos(sol->rr,pos);
            covenu(pos,Qr,Qe);
            deg2dms(pos[0]*R2D,dms1,4);
            deg2dms(pos[1]*R2D,dms2,4);
            if (solopt[0].height==1) pos[2]-=geoidh(pos); /* geodetic */
        }       
        vt_printf(vt," %s:%2.0f %02.0f %07.4f",pos[0]<0?"S":"N",fabs(dms1[0]),dms1[1],dms1[2]);
        vt_printf(vt," %s:%3.0f %02.0f %07.4f",pos[1]<0?"W":"E",fabs(dms2[0]),dms2[1],dms2[2]);
        vt_printf(vt," H:%8.3f",pos[2]);
        if (solflag&1) {
            vt_printf(vt," (N:%6.3f E:%6.3f U:%6.3f)",SQRT(Qe[4]),SQRT(Qe[0]),SQRT(Qe[8]));
        }
    }
    else if (soltype==1) {
        if (norm(sol->rr,3)>0.0) {
            ecef2pos(sol->rr,pos);
            covenu(pos,Qr,Qe);
            if (solopt[0].height==1) pos[2]-=geoidh(pos); /* geodetic */
        }       
        vt_printf(vt," %s:%11.8f",pos[0]<0.0?"S":"N",fabs(pos[0])*R2D);
        vt_printf(vt," %s:%12.8f",pos[1]<0.0?"W":"E",fabs(pos[1])*R2D);
        vt_printf(vt," H:%8.3f",pos[2]);
        if (solflag&1) {
            vt_printf(vt," (E:%6.3f N:%6.3f U:%6.3fm)",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
        }
    }
    else if (soltype==2) {
        vt_printf(vt," X:%12.3f",sol->rr[0]);
        vt_printf(vt," Y:%12.3f",sol->rr[1]);
        vt_printf(vt," Z:%12.3f",sol->rr[2]);
        if (solflag&1) {
            vt_printf(vt," (X:%6.3f Y:%6.3f Z:%6.3f)",SQRT(Qr[0]),SQRT(Qr[4]),SQRT(Qr[8]));
        }
    }
    else if (soltype==3) {
        if (len>0.0) {
            ecef2pos(rb,pos);
            ecef2enu(pos,bl,enu);
            covenu(pos,Qr,Qe);
        }       
        vt_printf(vt," E:%12.3f",enu[0]);
        vt_printf(vt," N:%12.3f",enu[1]);
        vt_printf(vt," U:%12.3f",enu[2]);
        if (solflag&1) {
            vt_printf(vt," (E:%6.3f N:%6.3f U:%6.3f)",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
        }
    }
    else if (soltype==4) {
        if (len>0.0) {
            ecef2pos(rb,pos);
            ecef2enu(pos,bl,enu);
            covenu(pos,Qr,Qe);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
        vt_printf(vt," P:%12.3f",pitch*R2D);
        vt_printf(vt," Y:%12.3f",yaw*R2D);
        vt_printf(vt," L:%12.3f",len);
        if (solflag&1) {
            vt_printf(vt," (E:%6.3f N:%6.3f U:%6.3f)",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
        }
    }
    if (solflag&2) {
        vt_printf(vt," A:%4.1f R:%5.1f N:%2d",sol->age,sol->ratio,sol->ns);
    }
    vt_printf(vt,"\n");
}
/* print status --------------------------------------------------------------*/
static void prstatus(vt_t *vt)
{
    rtk_t rtk;
    const char *svrstate[]={"stop","run"},*type[]={"rover","base","corr"};
    const char *sol[]={"-","fix","float","SBAS","DGPS","single","PPP",""};
    const char *mode[]={
         "single","DGPS","kinematic","static","moving-base","fixed",
         "PPP-kinema","PPP-static"
    };
    const char *freq[]={"-","L1","L1+L2","L1+L2+L5","","",""};
    rtcm_t rtcm[3];
    int i,j,n,thread,cycle,state,rtkstat,nsat0,nsat1,prcout,nave;
    int cputime,nb[3]={0},nmsg[3][10]={{0}};
    char tstr[64],s[1024],*p;
    double runtime,rt[3]={0},dop[4]={0},rr[3],bl1=0.0,bl2=0.0;
    double azel[MAXSAT*2],pos[3],vel[3],*del;
    
    trace(4,"prstatus:\n");
    
    rtksvrlock(&svr);
    rtk=svr.rtk;
    thread=(int)svr.thread;
    cycle=svr.cycle;
    state=svr.state;
    rtkstat=svr.rtk.sol.stat;
    nsat0=svr.obs[0][0].n;
    nsat1=svr.obs[1][0].n;
    cputime=svr.cputime;
    prcout=svr.prcout;
    nave=svr.nave;
    for (i=0;i<3;i++) nb[i]=svr.nb[i];
    for (i=0;i<3;i++) for (j=0;j<10;j++) {
        nmsg[i][j]=svr.nmsg[i][j];
    }
    if (svr.state) {
        runtime=(double)(tickget()-svr.tick)/1000.0;
        rt[0]=floor(runtime/3600.0); runtime-=rt[0]*3600.0;
        rt[1]=floor(runtime/60.0); rt[2]=runtime-rt[1]*60.0;
    }
    for (i=0;i<3;i++) rtcm[i]=svr.rtcm[i];
    rtksvrunlock(&svr);
    
    for (i=n=0;i<MAXSAT;i++) {
        if (rtk.opt.mode==PMODE_SINGLE&&!rtk.ssat[i].vs) continue;
        if (rtk.opt.mode!=PMODE_SINGLE&&!rtk.ssat[i].vsat[0]) continue;
        azel[  n*2]=rtk.ssat[i].azel[0];
        azel[1+n*2]=rtk.ssat[i].azel[1];
        n++;
    }
    dops(n,azel,0.0,dop);
    
    vt_printf(vt,"\n%s%-28s: %s%s\n",ESC_BOLD,"Parameter","Value",ESC_RESET);
    vt_printf(vt,"%-28s: %s %s\n","rtklib version",VER_RTKLIB,PATCH_LEVEL);
    vt_printf(vt,"%-28s: %d\n","rtk server thread",thread);
    vt_printf(vt,"%-28s: %s\n","rtk server state",svrstate[state]);
    vt_printf(vt,"%-28s: %d\n","processing cycle (ms)",cycle);
    vt_printf(vt,"%-28s: %s\n","positioning mode",mode[rtk.opt.mode]);
    vt_printf(vt,"%-28s: %s\n","frequencies",freq[rtk.opt.nf]);
    vt_printf(vt,"%-28s: %02.0f:%02.0f:%04.1f\n","accumulated time to run",rt[0],rt[1],rt[2]);
    vt_printf(vt,"%-28s: %d\n","cpu time for a cycle (ms)",cputime);
    vt_printf(vt,"%-28s: %d\n","missing obs data count",prcout);
    vt_printf(vt,"%-28s: %d,%d\n","bytes in input buffer",nb[0],nb[1]);
    for (i=0;i<3;i++) {
        sprintf(s,"# of input data %s",type[i]);
        vt_printf(vt,"%-28s: obs(%d),nav(%d),gnav(%d),ion(%d),sbs(%d),pos(%d),dgps(%d),ssr(%d),err(%d)\n",
                s,nmsg[i][0],nmsg[i][1],nmsg[i][6],nmsg[i][2],nmsg[i][3],
                nmsg[i][4],nmsg[i][5],nmsg[i][7],nmsg[i][9]);
    }
    for (i=0;i<3;i++) {
        p=s; *p='\0';
        for (j=1;j<100;j++) {
            if (rtcm[i].nmsg2[j]==0) continue;
            p+=sprintf(p,"%s%d(%d)",p>s?",":"",j,rtcm[i].nmsg2[j]);
        }
        if (rtcm[i].nmsg2[0]>0) {
            sprintf(p,"%sother2(%d)",p>s?",":"",rtcm[i].nmsg2[0]);
        }
        for (j=1;j<300;j++) {
            if (rtcm[i].nmsg3[j]==0) continue;
            p+=sprintf(p,"%s%d(%d)",p>s?",":"",j+1000,rtcm[i].nmsg3[j]);
        }
        if (rtcm[i].nmsg3[0]>0) {
            sprintf(p,"%sother3(%d)",p>s?",":"",rtcm[i].nmsg3[0]);
        }
        vt_printf(vt,"%-15s %-9s: %s\n","# of rtcm messages",type[i],s);
    }
    vt_printf(vt,"%-28s: %s\n","solution status",sol[rtkstat]);
    time2str(rtk.sol.time,tstr,9);
    vt_printf(vt,"%-28s: %s\n","time of receiver clock rover",rtk.sol.time.time?tstr:"-");
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f,%.3f\n","time sys offset (ns)",rtk.sol.dtr[1]*1e9,
              rtk.sol.dtr[2]*1e9,rtk.sol.dtr[3]*1e9,rtk.sol.dtr[4]*1e9);
    vt_printf(vt,"%-28s: %.3f\n","solution interval (s)",rtk.tt);
    vt_printf(vt,"%-28s: %.3f\n","age of differential (s)",rtk.sol.age);
    vt_printf(vt,"%-28s: %.3f\n","ratio for ar validation",rtk.sol.ratio);
    vt_printf(vt,"%-28s: %d\n","# of satellites rover",nsat0);
    vt_printf(vt,"%-28s: %d\n","# of satellites base",nsat1);
    vt_printf(vt,"%-28s: %d\n","# of valid satellites",rtk.sol.ns);
    vt_printf(vt,"%-28s: %.1f,%.1f,%.1f,%.1f\n","GDOP/PDOP/HDOP/VDOP",dop[0],dop[1],dop[2],dop[3]);
    vt_printf(vt,"%-28s: %d\n","# of real estimated states",rtk.na);
    vt_printf(vt,"%-28s: %d\n","# of all estimated states",rtk.nx);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz single (m) rover",
            rtk.sol.rr[0],rtk.sol.rr[1],rtk.sol.rr[2]);
    if (norm(rtk.sol.rr,3)>0.0) ecef2pos(rtk.sol.rr,pos); else pos[0]=pos[1]=pos[2]=0.0;
    vt_printf(vt,"%-28s: %.8f,%.8f,%.3f\n","pos llh single (deg,m) rover",
            pos[0]*R2D,pos[1]*R2D,pos[2]);
    ecef2enu(pos,rtk.sol.rr+3,vel);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","vel enu (m/s) rover",vel[0],vel[1],vel[2]);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz float (m) rover",
            rtk.x?rtk.x[0]:0,rtk.x?rtk.x[1]:0,rtk.x?rtk.x[2]:0);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz float std (m) rover",
            rtk.P?SQRT(rtk.P[0]):0,rtk.P?SQRT(rtk.P[1+1*rtk.nx]):0,rtk.P?SQRT(rtk.P[2+2*rtk.nx]):0);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz fixed (m) rover",
            rtk.xa?rtk.xa[0]:0,rtk.xa?rtk.xa[1]:0,rtk.xa?rtk.xa[2]:0);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz fixed std (m) rover",
            rtk.Pa?SQRT(rtk.Pa[0]):0,rtk.Pa?SQRT(rtk.Pa[1+1*rtk.na]):0,rtk.Pa?SQRT(rtk.Pa[2+2*rtk.na]):0);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","pos xyz (m) base",
            rtk.rb[0],rtk.rb[1],rtk.rb[2]);
    if (norm(rtk.rb,3)>0.0) ecef2pos(rtk.rb,pos); else pos[0]=pos[1]=pos[2]=0.0;
    vt_printf(vt,"%-28s: %.8f,%.8f,%.3f\n","pos llh (deg,m) base",
            pos[0]*R2D,pos[1]*R2D,pos[2]);
    vt_printf(vt,"%-28s: %d\n","# of average single pos base",nave);
    vt_printf(vt,"%-28s: %s\n","ant type rover",rtk.opt.pcvr[0].type);
    del=rtk.opt.antdel[0];
    vt_printf(vt,"%-28s: %.3f %.3f %.3f\n","ant delta rover",del[0],del[1],del[2]);
    vt_printf(vt,"%-28s: %s\n","ant type base" ,rtk.opt.pcvr[1].type);
    del=rtk.opt.antdel[1];
    vt_printf(vt,"%-28s: %.3f %.3f %.3f\n","ant delta base",del[0],del[1],del[2]);
    ecef2enu(pos,rtk.rb+3,vel);
    vt_printf(vt,"%-28s: %.3f,%.3f,%.3f\n","vel enu (m/s) base",
            vel[0],vel[1],vel[2]);
    if (rtk.opt.mode>0&&rtk.x&&norm(rtk.x,3)>0.0) {
        for (i=0;i<3;i++) rr[i]=rtk.x[i]-rtk.rb[i];
        bl1=norm(rr,3);
    }
    if (rtk.opt.mode>0&&rtk.xa&&norm(rtk.xa,3)>0.0) {
        for (i=0;i<3;i++) rr[i]=rtk.xa[i]-rtk.rb[i];
        bl2=norm(rr,3);
    }
    vt_printf(vt,"%-28s: %.3f\n","baseline length float (m)",bl1);
    vt_printf(vt,"%-28s: %.3f\n","baseline length fixed (m)",bl2);
    vt_printf(vt,"%-28s: %d\n","monitor port",moniport);
}
/* print satellite -----------------------------------------------------------*/
static void prsatellite(vt_t *vt, int nf)
{
    rtk_t rtk;
    double az,el;
    char id[32];
    int i,j,fix,frq[]={1,2,5,7,8,6};
    
    trace(4,"prsatellite:\n");
    
    rtksvrlock(&svr);
    rtk=svr.rtk;
    rtksvrunlock(&svr);
    if (nf<=0||nf>NFREQ) nf=NFREQ;
    vt_printf(vt,"\n%s%3s %2s %5s %4s",ESC_BOLD,"SAT","C1","Az","El");
    for (j=0;j<nf;j++) vt_printf(vt," L%d"    ,frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt,"  Fix%d" ,frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt,"  P%dRes",frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt,"   L%dRes",frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt,"  Sl%d"  ,frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt,"  Lock%d",frq[j]);
    for (j=0;j<nf;j++) vt_printf(vt," Rj%d"   ,frq[j]);
    vt_printf(vt,"%s\n",ESC_RESET);
    
    for (i=0;i<MAXSAT;i++) {
        if (rtk.ssat[i].azel[1]<=0.0) continue;
        satno2id(i+1,id);
        vt_printf(vt,"%3s %2s",id,rtk.ssat[i].vs?"OK":"-");
        az=rtk.ssat[i].azel[0]*R2D; if (az<0.0) az+=360.0;
        el=rtk.ssat[i].azel[1]*R2D;
        vt_printf(vt," %5.1f %4.1f",az,el);
        for (j=0;j<nf;j++) vt_printf(vt," %2s",rtk.ssat[i].vsat[j]?"OK":"-");
        for (j=0;j<nf;j++) {
            fix=rtk.ssat[i].fix[j];
            vt_printf(vt," %5s",fix==1?"FLOAT":(fix==2?"FIX":(fix==3?"HOLD":"-")));
        }
        for (j=0;j<nf;j++) vt_printf(vt,"%7.3f",rtk.ssat[i].resp[j]);
        for (j=0;j<nf;j++) vt_printf(vt,"%8.4f",rtk.ssat[i].resc[j]);
        for (j=0;j<nf;j++) vt_printf(vt," %4d",rtk.ssat[i].slipc[j]);
        for (j=0;j<nf;j++) vt_printf(vt," %6d",rtk.ssat[i].lock [j]);
        for (j=0;j<nf;j++) vt_printf(vt," %3d",rtk.ssat[i].rejc [j]);
        vt_printf(vt,"\n");
    }
}
/* print observation data ----------------------------------------------------*/
static void probserv(vt_t *vt, int nf)
{
    obsd_t obs[MAXOBS*2];
    char tstr[64],id[32];
    int i,j,n=0,frq[]={1,2,5,7,8,6,9};
    
    trace(4,"probserv:\n");
    
    rtksvrlock(&svr);
    for (i=0;i<svr.obs[0][0].n&&n<MAXOBS*2;i++) {
        obs[n++]=svr.obs[0][0].data[i];
    }
    for (i=0;i<svr.obs[1][0].n&&n<MAXOBS*2;i++) {
        obs[n++]=svr.obs[1][0].data[i];
    }
    rtksvrunlock(&svr);
    
    if (nf<=0||nf>NFREQ) nf=NFREQ;
    vt_printf(vt,"\n%s%-22s %3s %s",ESC_BOLD,"      TIME(GPST)","SAT","R");
    for (i=0;i<nf;i++) vt_printf(vt,"        P%d(m)" ,frq[i]);
    for (i=0;i<nf;i++) vt_printf(vt,"       L%d(cyc)",frq[i]);
    for (i=0;i<nf;i++) vt_printf(vt,"  D%d(Hz)"      ,frq[i]);
    for (i=0;i<nf;i++) vt_printf(vt," S%d"           ,frq[i]);
    vt_printf(vt," LLI%s\n",ESC_RESET);
    for (i=0;i<n;i++) {
        time2str(obs[i].time,tstr,2);
        satno2id(obs[i].sat,id);
        vt_printf(vt,"%s %3s %d",tstr,id,obs[i].rcv);
        for (j=0;j<nf;j++) vt_printf(vt,"%13.3f",obs[i].P[j]);
        for (j=0;j<nf;j++) vt_printf(vt,"%14.3f",obs[i].L[j]);
        for (j=0;j<nf;j++) vt_printf(vt,"%8.1f" ,obs[i].D[j]);
        for (j=0;j<nf;j++) vt_printf(vt,"%3.0f" ,obs[i].SNR[j]*0.25);
        for (j=0;j<nf;j++) vt_printf(vt,"%2d"   ,obs[i].LLI[j]);
        vt_printf(vt,"\n");
    }
}
/* print navigation data -----------------------------------------------------*/
static void prnavidata(vt_t *vt)
{
    eph_t eph[MAXSAT];
    geph_t geph[MAXPRNGLO];
    double ion[8],utc[4];
    gtime_t time;
    char id[32],s1[64],s2[64],s3[64];
    int i,valid,prn,leaps;
    
    trace(4,"prnavidata:\n");
    
    rtksvrlock(&svr);
    time=svr.rtk.sol.time;
    for (i=0;i<MAXSAT;i++) eph[i]=svr.nav.eph[i];
    for (i=0;i<MAXPRNGLO;i++) geph[i]=svr.nav.geph[i];
    for (i=0;i<8;i++) ion[i]=svr.nav.ion_gps[i];
    for (i=0;i<4;i++) utc[i]=svr.nav.utc_gps[i];
    leaps=svr.nav.leaps;
    rtksvrunlock(&svr);
    
    vt_printf(vt,"\n%s%3s %3s %3s %3s %3s %3s %3s %19s %19s %19s %3s %3s%s\n",
              ESC_BOLD,"SAT","S","IOD","IOC","FRQ","A/A","SVH","Toe","Toc",
              "Ttr/Tof","L2C","L2P",ESC_RESET);
    for (i=0;i<MAXSAT;i++) {
        if (!(satsys(i+1,&prn)&(SYS_GPS|SYS_GAL|SYS_QZS|SYS_CMP))||
            eph[i].sat!=i+1) continue;
        valid=eph[i].toe.time!=0&&!eph[i].svh&&
              fabs(timediff(time,eph[i].toe))<=MAXDTOE;
        satno2id(i+1,id);
        if (eph[i].toe.time!=0) time2str(eph[i].toe,s1,0); else strcpy(s1,"-");
        if (eph[i].toc.time!=0) time2str(eph[i].toc,s2,0); else strcpy(s2,"-");
        if (eph[i].ttr.time!=0) time2str(eph[i].ttr,s3,0); else strcpy(s3,"-");
        vt_printf(vt,"%3s %3s %3d %3d %3d %3d %03X %19s %19s %19s %3d %3d\n",
                id,valid?"OK":"-",eph[i].iode,eph[i].iodc,0,eph[i].sva,
                eph[i].svh,s1,s2,s3,eph[i].code,eph[i].flag);
    }
    for (i=0;i<MAXSAT;i++) {
        if (!(satsys(i+1,&prn)&SYS_GLO)||geph[prn-1].sat!=i+1) continue;
        valid=geph[prn-1].toe.time!=0&&!geph[prn-1].svh&&
              fabs(timediff(time,geph[prn-1].toe))<=MAXDTOE_GLO;
        satno2id(i+1,id);
        if (geph[prn-1].toe.time!=0) time2str(geph[prn-1].toe,s1,0); else strcpy(s1,"-");
        if (geph[prn-1].tof.time!=0) time2str(geph[prn-1].tof,s2,0); else strcpy(s2,"-");
        vt_printf(vt,"%3s %3s %3d %3d %3d %3d  %02X %19s %19s %19s %3d %3d\n",
                id,valid?"OK":"-",geph[prn-1].iode,0,geph[prn-1].frq,
                geph[prn-1].age,geph[prn].svh,s1,"-",s2,0,0);
    }
    vt_printf(vt,"ION: %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E\n",
            ion[0],ion[1],ion[2],ion[3],ion[4],ion[5],ion[6],ion[7]);
    vt_printf(vt,"UTC: %9.2E %9.2E %9.2E %9.2E  LEAPS: %d\n",utc[0],utc[1],utc[2],
            utc[3],leaps);
}
/* print error/warning messages ----------------------------------------------*/
static void prerror(vt_t *vt)
{
    int n;
    
    trace(4,"prerror:\n");
    
    rtksvrlock(&svr);
    if ((n=svr.rtk.neb)>0) {
        svr.rtk.errbuf[n]='\0';
        vt_puts(vt,svr.rtk.errbuf);
        svr.rtk.neb=0;
    }
    rtksvrunlock(&svr);
}
/* print stream --------------------------------------------------------------*/
static void prstream(vt_t *vt)
{
    const char *ch[]={
        "input rover","input base","input corr","output sol1","output sol2",
        "log rover","log base","log corr","monitor"
    };
    const char *type[]={
        "-","serial","file","tcpsvr","tcpcli","udp","ntrips","ntripc","ftp",
        "http","ntripc_s","ntripc_c"
    };
    const char *fmt[]={"rtcm2","rtcm3","oem4","oem3","ubx","ss2","hemis","skytreq",
                       "gw10","javad","nvs","binex","rt17","sbf","cmr","","","sp3",""};
    const char *sol[]={"llh","xyz","enu","nmea","stat","-"};
    stream_t stream[9];
    int i,format[9]={0};
    
    trace(4,"prstream:\n");
    
    rtksvrlock(&svr);
    for (i=0;i<8;i++) stream[i]=svr.stream[i];
    for (i=0;i<3;i++) format[i]=svr.format[i];
    for (i=3;i<5;i++) format[i]=svr.solopt[i-3].posf;
    stream[8]=moni;
    format[8]=SOLF_LLH;
    rtksvrunlock(&svr);
    
    vt_printf(vt,"\n%s%-12s %-8s %-5s %s %10s %7s %10s %7s %-24s %s%s\n",ESC_BOLD,
              "Stream","Type","Fmt","S","In-byte","In-bps","Out-byte","Out-bps",
              "Path","Message",ESC_RESET);
    for (i=0;i<9;i++) {
        vt_printf(vt,"%-12s %-8s %-5s %s %10d %7d %10d %7d %-24.24s %s\n",
            ch[i],type[stream[i].type],i<3?fmt[format[i]]:(i<5||i==8?sol[format[i]]:"-"),
            stream[i].state<0?"E":(stream[i].state?"C":"-"),
            stream[i].inb,stream[i].inr,stream[i].outb,stream[i].outr,
            stream[i].path,stream[i].msg);
    }
}
/* print ssr correction ------------------------------------------------------*/
static void prssr(vt_t *vt)
{
    static char buff[128*MAXSAT];
    gtime_t time;
    ssr_t ssr[MAXSAT];
    int i,valid;
    char tstr[64],id[32],*p=buff;
    
    rtksvrlock(&svr);
    time=svr.rtk.sol.time;
    for (i=0;i<MAXSAT;i++) {
        ssr[i]=svr.nav.ssr[i];
    }
    rtksvrunlock(&svr);
    
    p+=sprintf(p,"\n%s%3s %3s %3s %3s %3s %19s %6s %6s %6s %6s %6s %6s %8s "
               "%6s %6s %6s%s\n",
               ESC_BOLD,"SAT","S","UDI","IOD","URA","T0","D0-A","D0-C","D0-R",
               "D1-A","D1-C","D1-R","C0","C1","C2","C-HR",ESC_RESET);
    for (i=0;i<MAXSAT;i++) {
        if (!ssr[i].t0[0].time) continue;
        satno2id(i+1,id);
        valid=fabs(timediff(time,ssr[i].t0[0]))<=1800.0;
        time2str(ssr[i].t0[0],tstr,0);
        p+=sprintf(p,"%3s %3s %3.0f %3d %3d %19s %6.3f %6.3f %6.3f %6.3f %6.3f "
                   "%6.3f %8.3f %6.3f %6.4f %6.3f\n",
                   id,valid?"OK":"-",ssr[i].udi[0],ssr[i].iode,ssr[i].ura,tstr,
                   ssr[i].deph[0],ssr[i].deph[1],ssr[i].deph[2],
                   ssr[i].ddeph[0]*1E3,ssr[i].ddeph[1]*1E3,ssr[i].ddeph[2]*1E3,
                   ssr[i].dclk[0],ssr[i].dclk[1]*1E3,ssr[i].dclk[2]*1E3,
                   ssr[i].hrclk);
    }
    vt_puts(vt,buff);
}
/* start command -------------------------------------------------------------*/
static void cmd_start(char **args, int narg, vt_t *vt)
{
    trace(3,"cmd_start:\n");
    
    if (!startsvr(vt)) return;
    vt_printf(vt,"rtk server start\n");
}
/* stop command --------------------------------------------------------------*/
static void cmd_stop(char **args, int narg, vt_t *vt)
{
    trace(3,"cmd_stop:\n");
    
    stopsvr(vt);
    vt_printf(vt,"rtk server stop\n");
}
/* restart command -----------------------------------------------------------*/
static void cmd_restart(char **args, int narg, vt_t *vt)
{
    trace(3,"cmd_restart:\n");
    
    stopsvr(vt);
    if (!startsvr(vt)) return;
    vt_printf(vt,"rtk server restart\n");
}
/* solution command ----------------------------------------------------------*/
static void cmd_solution(char **args, int narg, vt_t *vt)
{
    int i,cycle=0;
    
    trace(3,"cmd_solution:\n");
    
    if (narg>1) cycle=(int)(atof(args[1])*1000.0);
    
    if (cycle>0) svr.nsol=0;
    
    while (!vt_chkbrk(vt)) {
        rtksvrlock(&svr);
        for (i=0;i<svr.nsol;i++) prsolution(vt,&svr.solbuf[i],svr.rtk.rb);
        svr.nsol=0;
        rtksvrunlock(&svr);
        if (cycle>0) sleepms(cycle); else return;
    }
}
/* status command ------------------------------------------------------------*/
static void cmd_status(char **args, int narg, vt_t *vt)
{
    int cycle=0;
    
    trace(3,"cmd_status:\n");
    
    if (narg>1) cycle=(int)(atof(args[1])*1000.0);
    
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        prstatus(vt);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* satellite command ---------------------------------------------------------*/
static void cmd_satellite(char **args, int narg, vt_t *vt)
{
    int i,nf=2,cycle=0;
    
    trace(3,"cmd_satellite:\n");
    
    for (i=1;i<narg;i++) {
        if (sscanf(args[i],"-%d",&nf)<1) cycle=(int)(atof(args[i])*1000.0);
    }
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        prsatellite(vt,nf);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* observ command ------------------------------------------------------------*/
static void cmd_observ(char **args, int narg, vt_t *vt)
{
    int i,nf=2,cycle=0;
    
    trace(3,"cmd_observ:\n");
    
    for (i=1;i<narg;i++) {
        if (sscanf(args[i],"-%d",&nf)<1) cycle=(int)(atof(args[i])*1000.0);
    }
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        probserv(vt,nf);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* navidata command ----------------------------------------------------------*/
static void cmd_navidata(char **args, int narg, vt_t *vt)
{
    int cycle=0;
    
    trace(3,"cmd_navidata:\n");
    
    if (narg>1) cycle=(int)(atof(args[1])*1000.0);
    
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        prnavidata(vt);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* error command -------------------------------------------------------------*/
static void cmd_error(char **args, int narg, vt_t *vt)
{
    trace(3,"cmd_error:\n");
    
    rtksvrlock(&svr);
    svr.rtk.neb=0;
    rtksvrunlock(&svr);
    
    while (!vt_chkbrk(vt)) {
        prerror(vt);
        sleepms(100);
    }
    vt_printf(vt,"\n");
}
/* stream command ------------------------------------------------------------*/
static void cmd_stream(char **args, int narg, vt_t *vt)
{
    int cycle=0;
    
    trace(3,"cmd_stream:\n");
    
    if (narg>1) cycle=(int)(atof(args[1])*1000.0);
    
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        prstream(vt);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* ssr command ---------------------------------------------------------------*/
static void cmd_ssr(char **args, int narg, vt_t *vt)
{
    int cycle=0;
    
    trace(3,"cmd_ssr:\n");
    
    if (narg>1) cycle=(int)(atof(args[1])*1000.0);
    
    while (!vt_chkbrk(vt)) {
        if (cycle>0) vt_printf(vt,ESC_CLEAR);
        prssr(vt);
        if (cycle>0) sleepms(cycle); else return;
    }
    vt_printf(vt,"\n");
}
/* option command ------------------------------------------------------------*/
static void cmd_option(char **args, int narg, vt_t *vt)
{
    char buff[MAXSTR],*p;
    int i,n;
    
    trace(3,"cmd_option:\n");
    
    for (i=0;*rcvopts[i].name;i++) {
        if (narg>=2&&!strstr(rcvopts[i].name,args[1])) continue;
        p=buff;
        p+=sprintf(p,"%-18s =",rcvopts[i].name);
        p+=opt2str(rcvopts+i,p);
        if (*rcvopts[i].comment) {
            if ((n=(int)(buff+30-p))>0) p+=sprintf(p,"%*s",n,"");
            p+=sprintf(p," # (%s)",rcvopts[i].comment);
        }
        vt_printf(vt,"%s%s\n",modflgr[i]?"*":" ",buff);
    }
    for (i=0;*sysopts[i].name;i++) {
        if (narg>=2&&!strstr(sysopts[i].name,args[1])) continue;
        p=buff;
        p+=sprintf(p,"%-18s =",sysopts[i].name);
        p+=opt2str(sysopts+i,p);
        if (*sysopts[i].comment) {
            if ((n=(int)(buff+30-p))>0) p+=sprintf(p,"%*s",n,"");
            p+=sprintf(p," # (%s)",sysopts[i].comment);
        }
        vt_printf(vt,"%s%s\n",modflgs[i]?"*":" ",buff);
    }
}
/* set command ---------------------------------------------------------------*/
static void cmd_set(char **args, int narg, vt_t *vt)
{
    opt_t *opt;
    int *modf;
    char buff[MAXSTR];
    
    trace(3,"cmd_set:\n");
    
    if (narg<2) {
        vt_printf(vt,"specify option type\n");
        return;
    }
    if ((opt=searchopt(args[1],rcvopts))) {
        modf=modflgr+(int)(opt-rcvopts);
    }
    else if ((opt=searchopt(args[1],sysopts))) {
        modf=modflgs+(int)(opt-sysopts);
    }
    else {
        vt_printf(vt,"no option type: %s\n",args[1]);
        return;
    }
    if (narg<3) {
        vt_printf(vt,"%s",opt->name);
        if (*opt->comment) vt_printf(vt," (%s)",opt->comment);
        vt_printf(vt,": ");
        if (!vt_gets(vt,buff,sizeof(buff))||vt->brk) return;
    }
    else strcpy(buff,args[2]);
    
    chop(buff);
    if (!str2opt(opt,buff)) {
        vt_printf(vt,"invalid option value: %s %s\n",opt->name,buff);
        return;
    }
    getsysopts(&prcopt,solopt,&filopt);
    
    vt_printf(vt,"option %s changed.",opt->name);
    if (strncmp(opt->name,"console",7)) {
        *modf=1;
        vt_printf(vt," restart to enable it");
    }
    vt_printf(vt,"\n");
}
/* load command --------------------------------------------------------------*/
static void cmd_load(char **args, int narg, vt_t *vt)
{
    char file[MAXSTR]="";
    
    trace(3,"cmd_load:\n");
    
    if (narg>=2) {
        strcpy(file,args[1]);
    }
    else {
        sprintf(file,"%s/%s",OPTSDIR,OPTSFILE);
    }
    resetsysopts();
    if (!loadopts(file,sysopts)) {
        vt_printf(vt,"no options file: %s\n",file);
        return;
    }
    getsysopts(&prcopt,solopt,&filopt);
    
    if (!loadopts(file,rcvopts)) {
        vt_printf(vt,"no options file: %s\n",file);
        return;
    }
    vt_printf(vt,"options loaded from %s. restart to enable them\n",file);
}
/* save command --------------------------------------------------------------*/
static void cmd_save(char **args, int narg, vt_t *vt)
{
    char file[MAXSTR]="",comment[256],s[64];
    
    trace(3,"cmd_save:\n");
    
    if (narg>=2) {
        strcpy(file,args[1]);
    }
    else {
        sprintf(file,"%s/%s",OPTSDIR,OPTSFILE);
    }
    if (!confwrite(vt,file)) return;
    time2str(utc2gpst(timeget()),s,0);
    sprintf(comment,"%s options (%s, v.%s %s)",PRGNAME,s,VER_RTKLIB,PATCH_LEVEL);
    setsysopts(&prcopt,solopt,&filopt);
    if (!saveopts(file,"w",comment,rcvopts)||!saveopts(file,"a",NULL,sysopts)) {
        vt_printf(vt,"options save error: %s\n",file);
        return;
    }
    vt_printf(vt,"options saved to %s\n",file);
}
/* log command ---------------------------------------------------------------*/
static void cmd_log(char **args, int narg, vt_t *vt)
{
    trace(3,"cmd_log:\n");
    
    if (narg<2) {
        vt_printf(vt,"specify log file\n");
        return;
    }
    if (!strcmp(args[1],"off")) {
        vt_closelog(vt);
        vt_printf(vt,"log off\n");
        return;
    } 
    if (!confwrite(vt,args[1])) return;
    
    if (!vt_openlog(vt,args[1])) {
        vt_printf(vt,"log open error: %s\n",args[1]);
        return;
    }
    vt_printf(vt,"log on: %s\n",args[1]);
}
/* help command --------------------------------------------------------------*/
static void cmd_help(char **args, int narg, vt_t *vt)
{
    char str[]="path";
    int i;
    
    if (narg<2) {
        vt_printf(vt,"%s (ver.%s %s)\n",PRGNAME,VER_RTKLIB,PATCH_LEVEL);
        for (i=0;*helptxt[i];i++) vt_printf(vt,"%s\n",helptxt[i]);
    }
    else if (strstr(str,args[1])==str) {
        for (i=0;*pathopts[i];i++) vt_printf(vt,"%s\n",pathopts[i]);
    }
    else {
        vt_printf(vt,"unknown help: %s\n",args[1]);
    }
}
/* exec command --------------------------------------------------------------*/
static int cmd_exec(const char *cmd, vt_t *vt)
{
    FILE *fp;
    int ret;
    char buff[MAXSTR];
    
    if (!(fp=popen(cmd,"r"))) {
        vt_printf(vt,"command exec error\n");
        return -1;
    }
    while (!vt_chkbrk(vt)) {
        if (!fgets(buff,sizeof(buff),fp)) break;
        vt_printf(vt,buff);
    }
    if ((ret=pclose(fp))) {
        vt_printf(vt,"command exec error (%d)\n",ret);
    }
    return ret;
}
/* console thread ------------------------------------------------------------*/
static void *con_thread(void *arg)
{
    const char *cmds[]={
        "start","stop","restart","solution","status","satellite","observ",
        "navidata","stream","ssr","error","option","set","load","save","log",
        "help","?","exit","shutdown",""
    };
    con_t *con=(con_t *)arg;
    int i,j,narg;
    char buff[MAXCMD],*args[MAXARG],*p;
    
    trace(3,"console_thread:\n");
    
    vt_printf(con->vt,"\n%s** %s ver.%s %s console (h:help) **%s\n",ESC_BOLD,
              PRGNAME,VER_RTKLIB,PATCH_LEVEL,ESC_RESET);
    
    if (!login(con->vt)) {
        vt_close(con->vt);
        con->state=0;
        return 0;
    }
    while (con->state) {
        
        /* output prompt */
        if (!vt_puts(con->vt,CMDPROMPT)) break;
        
        /* input command */
        if (!vt_gets(con->vt,buff,sizeof(buff))) break;
        
        if (buff[0]=='!') { /* shell escape */
            cmd_exec(buff+1,con->vt);
            continue;
        }
        /* parse command */
        narg=0;
        for (p=strtok(buff," \t\n");p&&narg<MAXARG;p=strtok(NULL," \t\n")) {
            args[narg++]=p;
        }
        if (narg==0) continue;
        
        for (i=0,j=-1;*cmds[i];i++) {
            if (strstr(cmds[i],args[0])==cmds[i]) j=i;
        }
        switch (j) {
            case  0: cmd_start    (args,narg,con->vt); break;
            case  1: cmd_stop     (args,narg,con->vt); break;
            case  2: cmd_restart  (args,narg,con->vt); break;
            case  3: cmd_solution (args,narg,con->vt); break;
            case  4: cmd_status   (args,narg,con->vt); break;
            case  5: cmd_satellite(args,narg,con->vt); break;
            case  6: cmd_observ   (args,narg,con->vt); break;
            case  7: cmd_navidata (args,narg,con->vt); break;
            case  8: cmd_stream   (args,narg,con->vt); break;
            case  9: cmd_ssr      (args,narg,con->vt); break;
            case 10: cmd_error    (args,narg,con->vt); break;
            case 11: cmd_option   (args,narg,con->vt); break;
            case 12: cmd_set      (args,narg,con->vt); break;
            case 13: cmd_load     (args,narg,con->vt); break;
            case 14: cmd_save     (args,narg,con->vt); break;
            case 15: cmd_log      (args,narg,con->vt); break;
            case 16: cmd_help     (args,narg,con->vt); break;
            case 17: cmd_help     (args,narg,con->vt); break;
            case 18: /* exit */
                if (con->vt->type) con->state=0;
                break;
            case 19: /* shutdown */
                if (!strcmp(args[0],"shutdown")) {
                    vt_printf(con->vt,"rtk server shutdown ...\n");
                    sleepms(1000);
                    intflg=1;
                    con->state=0;
                }
                break;
            default:
                vt_printf(con->vt,"unknown command: %s.\n",args[0]);
                break;
        }
    }
    vt_close(con->vt);
    return 0;
}
/* open console --------------------------------------------------------------*/
static con_t *con_open(int sock, const char *dev)
{
    con_t *con;
    
    trace(3,"con_open: sock=%d dev=%s\n",sock,dev);
    
    if (!(con=(con_t *)malloc(sizeof(con_t)))) return NULL;
    
    if (!(con->vt=vt_open(sock,dev))) {
        free(con);
        return NULL;
    }
    /* start console thread */
    con->state=1;
    if (pthread_create(&con->thread,NULL,con_thread,con)) {
        free(con);
        return NULL;
    }
    return con;
}
/* close console -------------------------------------------------------------*/
static void con_close(con_t *con)
{
    trace(3,"con_close:\n");
    
    if (!con) return;
    con->state=con->vt->state=0;
    pthread_join(con->thread,NULL);
    free(con);
}
/* open socket for remote console --------------------------------------------*/
static int open_sock(int port)
{
    struct sockaddr_in addr;
    int sock,on=1;
    
    trace(3,"open_sock: port=%d\n",port);
    
    if ((sock=socket(AF_INET,SOCK_STREAM,0))<0) {
        fprintf(stderr,"socket error (%d)\n",errno);
        return 0;
    }
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char *)&on,sizeof(on));
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    
    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr))<0) {
        fprintf(stderr,"bind error (%d)\n",errno);
        close(sock);
        return -1;
    }
    listen(sock,5);
    return sock;
}
/* accept remote console connection ------------------------------------------*/
static void accept_sock(int ssock, con_t **con)
{
    struct timeval tv={0};
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    fd_set rs;
    int i,sock;
    
    if (ssock<=0) return;
    
    trace(4,"accept_sock: ssock=%d\n",ssock);
    
    for (i=1;i<MAXCON;i++) {
        if (!con[i]||con[i]->state) continue;
        con_close(con[i]);
        con[i]=NULL;
    }
    FD_ZERO(&rs);
    FD_SET(ssock,&rs);
    if (select(ssock+1,&rs,NULL,NULL,&tv)<=0) {
        return;
    }
    if ((sock=accept(ssock,(struct sockaddr *)&addr,&len))<=0) {
        return;
    }
    for (i=1;i<MAXCON;i++) {
        if (con[i]) continue;
        
        con[i]=con_open(sock,"");
        
        trace(3,"remote console connected: addr=%s\n",
              inet_ntoa(addr.sin_addr));
        return;
    }
    close(sock);
    trace(2,"remote console connection refused. addr=%s\n",
         inet_ntoa(addr.sin_addr));
}
/* rtkrcv main -----------------------------------------------------------------
* sysnopsis
*     rtkrcv [-s][-p port][-d dev][-o file][-r level][-t level][-sta sta]
*
* description
*     A command line version of the real-time positioning AP by rtklib. To start
*     or stop RTK server, to configure options or to print solution/status,
*     login a console and input commands. As default, /dev/tty is used for the
*     console. Use -p option for network login with telnet protocol. To show
*     the available commands, type ? or help on the console. With -p option,
*     multiple telnet console logins are allowed. The initial processing options
*     are loaded from default file rtkrcv.conf. To change the file, use -o
*     option. To configure the processing options, edit the options file or use
*     set, load or save command on the console. To shutdown the program, use
*     shutdown command on the console or send USR2 signal to the process.
*
* option
*     -s         start RTK server on program startup
*     -p port    port number for telnet console
*     -m port    port number for monitor stream
*     -d dev     terminal device for console
*     -o file    processing options file
*     -w pwd     login password for remote console ("": no password)
*     -r level   output solution status file (0:off,1:states,2:residuals)
*     -t level   debug trace level (0:off,1-5:on)
*     -sta sta   station name for receiver dcb
*
* command
*     start
*       Start RTK server. No need the command if the program runs with -s
*       option.
*
*     stop
*       Stop RTK server.
*
*     restart
*       Restart RTK server. If the processing options are set, execute the
*       command to enable the changes.
*
*     solution [cycle]
*       Show solutions. Without option, only one solution is shown. With
*       option, the soluiton is displayed at intervals of cycle (s). To stop
*       cyclic display, send break (ctr-C).
*
*     status [cycle]
*       Show RTK status. Use option cycle for cyclic display.
*
*     satellite [-n] [cycle]
*       Show satellite status. Use option cycle for cyclic display. Option -n
*       specify number of frequencies.
*
*     observ [-n] [cycle]
*       Show observation data. Use option cycle for cyclic display. Option -n
*       specify number of frequencies.
*
*     navidata [cycle]
*       Show navigation data. Use option cycle for cyclic display.
*
*     stream [cycle]
*       Show stream status. Use option cycle for cyclic display.
*
*     error
*       Show error/warning messages. To stop messages, send break (ctr-C).
*
*     option [opt]
*       Show the values of processing options. Without option, all options are
*       displayed. With option, only pattern-matched options are displayed.
*
*     set opt [val]
*       Set the value of a processing option to val. With out option val,
*       prompt message is shown to input the value. The change of the 
*       processing option is not enabled before RTK server is restarted.
*
*     load [file]
*       Load processing options from file. Without option, default file
*       rtkrcv.conf is used. To enable the changes, restart RTK server.
*
*     save [file]
*       Save current processing optons to file. Without option, default file
*       rtkrcv.conf is used.
*
*     log [file|off]
*       Record console log to file. To stop recording the log, use option off.
*
*     help|? [path]
*       Show the command list. With option path, the stream path options are
*       shown.
*
*     exit
*       Exit and logout console. The status of RTK server is not affected by
*       the command.
*
*     shutdown
*       Shutdown RTK server and exit the program.
*
*     !command [arg...]
*       Execute command by the operating system shell. Do not use the
*       interactive command.
*
* notes
*     Short form of a command is allowed. In case of the short form, the
*     command is distinguished according to header characters.
*     
*-----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    con_t *con[MAXCON]={0};
    int i,start=0,port=0,outstat=0,trace=0,sock=0;
    char *dev="",file[MAXSTR]="";
    
    for (i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-s")) start=1;
        else if (!strcmp(argv[i],"-p")&&i+1<argc) port=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-m")&&i+1<argc) moniport=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-d")&&i+1<argc) dev=argv[++i];
        else if (!strcmp(argv[i],"-o")&&i+1<argc) strcpy(file,argv[++i]);
        else if (!strcmp(argv[i],"-w")&&i+1<argc) strcpy(passwd,argv[++i]);
        else if (!strcmp(argv[i],"-r")&&i+1<argc) outstat=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-t")&&i+1<argc) trace=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-sta")&&i+1<argc) strcpy(sta_name,argv[++i]);
        else printusage();
    }
    if (trace>0) {
        traceopen(TRACEFILE);
        tracelevel(trace);
    }
    /* initialize rtk server and monitor port */
    rtksvrinit(&svr);
    strinit(&moni);
    
    /* load options file */
    if (!*file) sprintf(file,"%s/%s",OPTSDIR,OPTSFILE);
    
    resetsysopts();
    if (!loadopts(file,rcvopts)||!loadopts(file,sysopts)) {
        fprintf(stderr,"no options file: %s. defaults used\n",file);
    }
    getsysopts(&prcopt,solopt,&filopt);
    
    /* read navigation data */
    if (!readnav(NAVIFILE,&svr.nav)) {
        fprintf(stderr,"no navigation data: %s\n",NAVIFILE);
    }
    if (outstat>0) {
        rtkopenstat(STATFILE,outstat);
    }
    /* open monitor port */
    if (moniport>0&&!openmoni(moniport)) {
        fprintf(stderr,"monitor port open error: %d\n",moniport);
    }
    if (port) {
        /* open socket for remote console */
        if ((sock=open_sock(port))<=0) {
            fprintf(stderr,"console open error port=%d\n",port);
            if (moniport>0) closemoni();
            if (outstat>0) rtkclosestat();
            traceclose();
            return -1;
        }
    }
    else {
        /* open device for local console */
        if (!(con[0]=con_open(0,dev))) {
            fprintf(stderr,"console open error dev=%s\n",dev);
            if (moniport>0) closemoni();
            if (outstat>0) rtkclosestat();
            traceclose();
            return -1;
        }
    }
    signal(SIGINT, sigshut); /* keyboard interrupt */
    signal(SIGTERM,sigshut); /* external shutdown signal */
    signal(SIGUSR2,sigshut);
    signal(SIGHUP ,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    
    /* start rtk server */
    if (start) {
        startsvr(NULL);
    }
    while (!intflg) {
        /* accept remote console connection */
        accept_sock(sock,con);
        sleepms(100);
    }
    /* stop rtk server */
    stopsvr(NULL);
    
    /* close consoles */
    for (i=0;i<MAXCON;i++) {
        con_close(con[i]);
    }
    if (moniport>0) closemoni();
    if (outstat>0) rtkclosestat();
    
    /* save navigation data */
    if (!savenav(NAVIFILE,&svr.nav)) {
        fprintf(stderr,"navigation data save error: %s\n",NAVIFILE);
    }
    traceclose();
    return 0;
}
