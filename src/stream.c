/*------------------------------------------------------------------------------
* stream.c : stream input/output functions
*
*          Copyright (C) 2008-2018 by T.TAKASU, All rights reserved.
*
* options : -DWIN32    use WIN32 API
*           -DSVR_REUSEADDR reuse tcp server address
*
* references :
*     [1] RTCM Recommendaed Standards for Networked Transport for RTCM via
*         Internet Protocol (Ntrip), Version 1.0, Semptember 30, 2004
*     [2] H.Niksic and others, GNU Wget 1.12, The non-iteractive download
*         utility, 4 September 2009
*
* version : $Revision:$ $Date:$
* history : 2009/01/16 1.0  new
*           2009/04/02 1.1  support nmea request in ntrip request
*                           support time-tag of file as stream
*           2009/09/04 1.2  ported to linux environment
*                           add fflush() to save file stream
*           2009/10/10 1.3  support multiple connection for tcp server
*                           add keyword replacement in file path
*                           add function strsendnmea(), strsendcmd()
*           2010/07/18 1.4  support ftp/http stream types
*                           add keywords replacement of %ha,%hb,%hc in path
*                           add api: strsetdir(),strsettimeout()
*           2010/08/31 1.5  reconnect after error of ntrip client
*                           fix bug on no file swap at week start (2.4.0_p6)
*           2011/05/29 1.6  add fast stream replay mode
*                           add time margin to swap file
*                           change api strsetopt()
*                           introduce non_block send for send socket
*                           add api: strsetproxy()
*           2011/12/21 1.7  fix bug decode tcppath (rtklib_2.4.1_p5)
*           2012/06/09 1.8  fix problem if user or password contains /
*                           (rtklib_2.4.1_p7)
*           2012/12/25 1.9  compile option SVR_REUSEADDR added
*           2013/03/10 1.10 fix problem with ntrip mountpoint containing "/"
*           2013/04/15 1.11 fix bug on swapping files if swapmargin=0
*           2013/05/28 1.12 fix bug on playback of file with 64 bit size_t
*           2014/05/23 1.13 retry to connect after gethostbyname() error
*                           fix bug on malloc size in openftp()
*           2014/06/21 1.14 add general hex message rcv command by !HEX ...
*           2014/10/16 1.15 support stdin/stdou for input/output from/to file
*           2014/11/08 1.16 fix getconfig error (87) with bluetooth device
*           2015/01/12 1.15 add rcv command to change bitrate by !BRATE
*           2016/01/16 1.16 add constant CRTSCTS for non-CRTSCTS-defined env.
*                           fix serial status for non-windows systems
*           2016/06/09 1.17 fix bug on !BRATE rcv command always failed
*                           fix program on struct alignment in time tag header
*           2016/06/21 1.18 reverse time-tag handler of file to previous
*           2016/07/23 1.19 add output of received stream to tcp port for serial
*           2016/08/20 1.20 modify api strsendnmea()
*           2016/08/29 1.21 fix bug on starting serial thread for windows
*           2016/09/03 1.22 add ntrip caster functions
*                           add api strstatx(),strsetsrctbl()
*                           add api strsetsel(),strgetsel()
*           2016/09/06 1.23 fix bug on ntrip caster socket and request handling
*           2016/09/27 1.24 support udp server and client
*           2016/10/10 1.25 support ::P={4|8} option in path for STR_FILE
*           2018/11/05 1.26 fix bug on default playback speed (= 0)
*                           fix bug on file playback as slave mode
*                           fix bug on timeset() in gpst instead of utc
*                           update trace levels and buffer sizes
*-----------------------------------------------------------------------------*/
#include <ctype.h>
#include "rtklib.h"
#ifndef WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#define __USE_MISC
#ifndef CRTSCTS
#define CRTSCTS  020000000000
#endif
#include <errno.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

/* constants -----------------------------------------------------------------*/

#define TINTACT             200         /* period for stream active (ms) */
#define SERIBUFFSIZE        4096        /* serial buffer size (bytes) */
#define TIMETAGH_LEN        64          /* time tag file header length */
#define MAXCLI              32          /* max client connection for tcp svr */
#define MAXSTATMSG          32          /* max length of status message */
#define DEFAULT_MEMBUF_SIZE 4096        /* default memory buffer size (bytes) */

#define NTRIP_AGENT         "RTKLIB/" VER_RTKLIB
#define NTRIP_CLI_PORT      2101        /* default ntrip-client connection port */
#define NTRIP_SVR_PORT      80          /* default ntrip-server connection port */
#define NTRIP_MAXRSP        32768       /* max size of ntrip response */
#define NTRIP_MAXSTR        256         /* max length of mountpoint string */
#define NTRIP_RSP_OK_CLI    "ICY 200 OK\r\n" /* ntrip response: client */
#define NTRIP_RSP_OK_SVR    "OK\r\n"    /* ntrip response: server */
#define NTRIP_RSP_SRCTBL    "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
#define NTRIP_RSP_TBLEND    "ENDSOURCETABLE"
#define NTRIP_RSP_HTTP      "HTTP/"     /* ntrip response: http */
#define NTRIP_RSP_ERROR     "ERROR"     /* ntrip response: error */
#define NTRIP_RSP_UNAUTH    "HTTP/1.0 401 Unauthorized\r\n"
#define NTRIP_RSP_ERR_PWD   "ERROR - Bad Pasword\r\n"
#define NTRIP_RSP_ERR_MNTP  "ERROR - Bad Mountpoint\r\n"

#define FTP_CMD             "wget"      /* ftp/http command */
#define FTP_TIMEOUT         30          /* ftp/http timeout (s) */

#define MIN(x,y)            ((x)<(y)?(x):(y))

/* macros --------------------------------------------------------------------*/

#ifdef WIN32
#define dev_t               HANDLE
#define socket_t            SOCKET
typedef int socklen_t;
#else
#define dev_t               int
#define socket_t            int
#define closesocket         close
#endif

/* type definition -----------------------------------------------------------*/

typedef struct {            /* file control type */
    FILE *fp;               /* file pointer */
    FILE *fp_tag;           /* file pointer of tag file */
    FILE *fp_tmp;           /* temporary file pointer for swap */
    FILE *fp_tag_tmp;       /* temporary file pointer of tag file for swap */
    char path[MAXSTRPATH];  /* file path */
    char openpath[MAXSTRPATH]; /* open file path */
    int mode;               /* file mode */
    int timetag;            /* time tag flag (0:off,1:on) */
    int repmode;            /* replay mode (0:master,1:slave) */
    int offset;             /* time offset (ms) for slave */
    int size_fpos;          /* file position size (bytes) */
    gtime_t time;           /* start time */
    gtime_t wtime;          /* write time */
    unsigned int tick;      /* start tick */
    unsigned int tick_f;    /* start tick in file */
    size_t fpos_n;          /* next file position */
    unsigned int tick_n;    /* next tick */
    double start;           /* start offset (s) */
    double speed;           /* replay speed (time factor) */
    double swapintv;        /* swap interval (hr) (0: no swap) */
    lock_t lock;            /* lock flag */
} file_t;

typedef struct {            /* tcp control type */
    int state;              /* state (0:close,1:wait,2:connect) */
    char saddr[256];        /* address string */
    int port;               /* port */
    struct sockaddr_in addr; /* address resolved */
    socket_t sock;          /* socket descriptor */
    int tcon;               /* reconnect time (ms) (-1:never,0:now) */
    unsigned int tact;      /* data active tick */
    unsigned int tdis;      /* disconnect tick */
} tcp_t;

typedef struct tcpsvr_tag { /* tcp server type */
    tcp_t svr;              /* tcp server control */
    tcp_t cli[MAXCLI];      /* tcp client controls */
} tcpsvr_t;

typedef struct {            /* tcp cilent type */
    tcp_t svr;              /* tcp server control */
    int toinact;            /* inactive timeout (ms) (0:no timeout) */
    int tirecon;            /* reconnect interval (ms) (0:no reconnect) */
} tcpcli_t;

typedef struct {            /* serial control type */
    dev_t dev;              /* serial device */
    int error;              /* error state */
#ifdef WIN32
    int state,wp,rp;        /* state,write/read pointer */
    int buffsize;           /* write buffer size (bytes) */
    HANDLE thread;          /* write thread */
    lock_t lock;            /* lock flag */
    unsigned char *buff;    /* write buffer */
#endif
    tcpsvr_t *tcpsvr;       /* tcp server for received stream */
} serial_t;

typedef struct {            /* ntrip control type */
    int state;              /* state (0:close,1:wait,2:connect) */
    int type;               /* type (0:server,1:client) */
    int nb;                 /* response buffer size */
    char url[256];          /* url for proxy */
    char mntpnt[256];       /* mountpoint */
    char user[256];         /* user */
    char passwd[256];       /* password */
    char str[NTRIP_MAXSTR]; /* mountpoint string for server */
    unsigned char buff[NTRIP_MAXRSP]; /* response buffer */
    tcpcli_t *tcp;          /* tcp client */
} ntrip_t;

typedef struct {            /* ntrip caster connection type */
    int state;              /* state (0:close,1:connect) */
    char mntpnt[256];       /* mountpoint */
    char str[NTRIP_MAXSTR]; /* mountpoint string for server */
    int nb;                 /* request buffer size */
    unsigned char buff[NTRIP_MAXRSP]; /* request buffer */
} ntripc_con_t;

typedef struct {            /* ntrip caster control type */
    int state;              /* state (0:close,1:wait,2:connect) */
    int type;               /* type (0:server,1:client) */
    char mntpnt[256];       /* selected mountpoint */
    char user[256];         /* user */
    char passwd[256];       /* password */
    char *srctbl;           /* source table */
    lock_t lock_srctbl;     /* lock flag for source table */
    tcpsvr_t *tcp;          /* tcp server */
    ntripc_con_t con[MAXCLI]; /* ntrip caster connections */
} ntripc_t;

typedef struct {            /* udp type */
    int state;              /* state (0:close,1:open) */
    int type;               /* type (0:server,1:client) */
    int port;               /* port */
    char saddr[256];        /* address (server:filter,client:server) */
    struct sockaddr_in addr; /* address resolved */
    socket_t sock;          /* socket descriptor */
} udp_t;

typedef struct {            /* ftp download control type */
    int state;              /* state (0:close,1:download,2:complete,3:error) */
    int proto;              /* protocol (0:ftp,1:http) */
    int error;              /* error code (0:no error,1-10:wget error, */
                            /*            11:no temp dir,12:uncompact error) */
    char addr[1024];        /* download address */
    char file[1024];        /* download file path */
    char user[256];         /* user for ftp */
    char passwd[256];       /* password for ftp */
    char local[1024];       /* local file path */
    int topts[4];           /* time options {poff,tint,toff,tretry} (s) */
    gtime_t tnext;          /* next retry time (gpst) */
    thread_t thread;        /* download thread */
} ftp_t;

typedef struct {            /* memory buffer type */
    int state,wp,rp;        /* state,write/read pointer */
    int bufsize;            /* buffer size (bytes) */
    lock_t lock;            /* lock flag */
    unsigned char *buf;     /* write buffer */
} membuf_t;

/* proto types for static functions ------------------------------------------*/

static tcpsvr_t *opentcpsvr(const char *path, char *msg);
static void closetcpsvr(tcpsvr_t *tcpsvr);
static int writetcpsvr(tcpsvr_t *tcpsvr, unsigned char *buff, int n, char *msg);

/* global options ------------------------------------------------------------*/

static int toinact  =10000; /* inactive timeout (ms) */
static int ticonnect=10000; /* interval to re-connect (ms) */
static int tirate   =1000;  /* avraging time for data rate (ms) */
static int buffsize =32768; /* receive/send buffer size (bytes) */
static char localdir[1024]=""; /* local directory for ftp/http */
static char proxyaddr[256]=""; /* http/ntrip/ftp proxy address */
static unsigned int tick_master=0; /* time tick master for replay */
static int fswapmargin=30;  /* file swap margin (s) */

/* read/write serial buffer --------------------------------------------------*/
#ifdef WIN32
static int readseribuff(serial_t *serial, unsigned char *buff, int nmax)
{
    int ns;
    
    tracet(5,"readseribuff: dev=%d\n",serial->dev);
    
    lock(&serial->lock);
    for (ns=0;serial->rp!=serial->wp&&ns<nmax;ns++) {
       buff[ns]=serial->buff[serial->rp];
       if (++serial->rp>=serial->buffsize) serial->rp=0;
    }
    unlock(&serial->lock);
    tracet(5,"readseribuff: ns=%d rp=%d wp=%d\n",ns,serial->rp,serial->wp);
    return ns;
}
static int writeseribuff(serial_t *serial, unsigned char *buff, int n)
{
    int ns,wp;
    
    tracet(5,"writeseribuff: dev=%d n=%d\n",serial->dev,n);
    
    lock(&serial->lock);
    for (ns=0;ns<n;ns++) {
        serial->buff[wp=serial->wp]=buff[ns];
        if (++wp>=serial->buffsize) wp=0;
        if (wp!=serial->rp) serial->wp=wp;
        else {
            tracet(2,"serial buffer overflow: size=%d\n",serial->buffsize);
            break;
        }
    }
    unlock(&serial->lock);
    tracet(5,"writeseribuff: ns=%d rp=%d wp=%d\n",ns,serial->rp,serial->wp);
    return ns;
}
#endif /* WIN32 */

/* write serial thread -------------------------------------------------------*/
#ifdef WIN32
static DWORD WINAPI serialthread(void *arg)
{
    serial_t *serial=(serial_t *)arg;
    unsigned char buff[128];
    unsigned int tick;
    DWORD ns;
    int n;
    
    tracet(3,"serialthread:\n");
    
    for (;;) {
        tick=tickget();
        while ((n=readseribuff(serial,buff,sizeof(buff)))>0) {
            if (!WriteFile(serial->dev,buff,n,&ns,NULL)) serial->error=1;
        }
        if (!serial->state) break;
        sleepms(10-(int)(tickget()-tick)); /* cycle=10ms */
    }
    free(serial->buff);
    return 0;
}
#endif /* WIN32 */

/* open serial ---------------------------------------------------------------*/
static serial_t *openserial(const char *path, int mode, char *msg)
{
    const int br[]={
        300,600,1200,2400,4800,9600,19200,38400,57600,115200,230400
    };
    serial_t *serial;
    int i,brate=9600,bsize=8,stopb=1,tcp_port=0;
    char *p,parity='N',dev[128],port[128],fctr[64]="",path_tcp[32],msg_tcp[128];
#ifdef WIN32
    DWORD error,rw=0,siz=sizeof(COMMCONFIG);
    COMMCONFIG cc={0};
    COMMTIMEOUTS co={MAXDWORD,0,0,0,0}; /* non-block-read */
    char dcb[64]="";
#else
    const speed_t bs[]={
        B300,B600,B1200,B2400,B4800,B9600,B19200,B38400,B57600,B115200,B230400
    };
    struct termios ios={0};
    int rw=0;
#endif
    tracet(3,"openserial: path=%s mode=%d\n",path,mode);
    
    if (!(serial=(serial_t *)calloc(1,sizeof(serial_t)))) return NULL;
    
    if ((p=strchr(path,':'))) {
        strncpy(port,path,p-path); port[p-path]='\0';
        sscanf(p,":%d:%d:%c:%d:%s",&brate,&bsize,&parity,&stopb,fctr);
    }
    else strcpy(port,path);
    
    if ((p=strchr(path,'#'))) {
        sscanf(p,"#%d",&tcp_port);
    }
    for (i=0;i<11;i++) if (br[i]==brate) break;
    if (i>=12) {
        sprintf(msg,"bitrate error (%d)",brate);
        tracet(1,"openserial: %s path=%s\n",msg,path);
        free(serial);
        return NULL;
    }
    parity=(char)toupper((int)parity);
    
#ifdef WIN32
    sprintf(dev,"\\\\.\\%s",port);
    if (mode&STR_MODE_R) rw|=GENERIC_READ;
    if (mode&STR_MODE_W) rw|=GENERIC_WRITE;
    
    serial->dev=CreateFile(dev,rw,0,0,OPEN_EXISTING,0,NULL);
    if (serial->dev==INVALID_HANDLE_VALUE) {
        sprintf(msg,"%s open error (%d)",port,(int)GetLastError());
        tracet(1,"openserial: %s path=%s\n",msg,path);
        free(serial);
        return NULL;
    }
    if (!GetCommConfig(serial->dev,&cc,&siz)) {
        sprintf(msg,"%s getconfig error (%d)",port,(int)GetLastError());
        tracet(1,"openserial: %s\n",msg);
        CloseHandle(serial->dev);
        free(serial);
        return NULL;
    }
    sprintf(dcb,"baud=%d parity=%c data=%d stop=%d",brate,parity,bsize,stopb);
    if (!BuildCommDCB(dcb,&cc.dcb)) {
        sprintf(msg,"%s buiddcb error (%d)",port,(int)GetLastError());
        tracet(1,"openserial: %s\n",msg);
        CloseHandle(serial->dev);
        free(serial);
        return NULL;
    }
    if (!strcmp(fctr,"rts")) {
        cc.dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;
    }
    SetCommConfig(serial->dev,&cc,siz); /* ignore error to support novatel */
    SetCommTimeouts(serial->dev,&co);
    ClearCommError(serial->dev,&error,NULL);
    PurgeComm(serial->dev,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    
    /* create write thread */
    initlock(&serial->lock);
    serial->state=serial->wp=serial->rp=serial->error=0;
    serial->buffsize=buffsize;
    if (!(serial->buff=(unsigned char *)malloc(buffsize))) {
        CloseHandle(serial->dev);
        free(serial);
        return NULL;
    }
    serial->state=1;
    if (!(serial->thread=CreateThread(NULL,0,serialthread,serial,0,NULL))) {
        sprintf(msg,"%s serial thread error (%d)",port,(int)GetLastError());
        tracet(1,"openserial: %s\n",msg);
        CloseHandle(serial->dev);
        serial->state=0;
        free(serial);
        return NULL;
    }
    sprintf(msg,"%s",port);
#else
    sprintf(dev,"/dev/%s",port);
    
    if ((mode&STR_MODE_R)&&(mode&STR_MODE_W)) rw=O_RDWR;
    else if (mode&STR_MODE_R) rw=O_RDONLY;
    else if (mode&STR_MODE_W) rw=O_WRONLY;
    
    if ((serial->dev=open(dev,rw|O_NOCTTY|O_NONBLOCK))<0) {
        sprintf(msg,"%s open error (%d)",dev,errno);
        tracet(1,"openserial: %s dev=%s\n",msg,dev);
        free(serial);
        return NULL;
    }
    tcgetattr(serial->dev,&ios);
    ios.c_iflag=0;
    ios.c_oflag=0;
    ios.c_lflag=0;     /* non-canonical */
    ios.c_cc[VMIN ]=0; /* non-block-mode */
    ios.c_cc[VTIME]=0;
    cfsetospeed(&ios,bs[i]);
    cfsetispeed(&ios,bs[i]);
    ios.c_cflag|=bsize==7?CS7:CS8;
    ios.c_cflag|=parity=='O'?(PARENB|PARODD):(parity=='E'?PARENB:0);
    ios.c_cflag|=stopb==2?CSTOPB:0;
    ios.c_cflag|=!strcmp(fctr,"rts")?CRTSCTS:0;
    tcsetattr(serial->dev,TCSANOW,&ios);
    tcflush(serial->dev,TCIOFLUSH);
    sprintf(msg,"%s",dev);
#endif
    serial->tcpsvr=NULL;
    
    /* open tcp sever to output received stream */
    if (tcp_port>0) {
        sprintf(path_tcp,":%d",tcp_port);
        serial->tcpsvr=opentcpsvr(path_tcp,msg_tcp);
    }
    tracet(3,"openserial: dev=%d\n",serial->dev);
    return serial;
}
/* close serial --------------------------------------------------------------*/
static void closeserial(serial_t *serial)
{
    tracet(3,"closeserial: dev=%d\n",serial->dev);
    
    if (!serial) return;
#ifdef WIN32
    serial->state=0;
    WaitForSingleObject(serial->thread,10000);
    CloseHandle(serial->dev);
    CloseHandle(serial->thread);
#else
    close(serial->dev);
#endif
    if (serial->tcpsvr) {
        closetcpsvr(serial->tcpsvr);
    }
    free(serial);
}
/* read serial ---------------------------------------------------------------*/
static int readserial(serial_t *serial, unsigned char *buff, int n, char *msg)
{
    char msg_tcp[128];
#ifdef WIN32
    DWORD nr;
#else
    int nr;
#endif
    tracet(4,"readserial: dev=%d n=%d\n",serial->dev,n);
    if (!serial) return 0;
#ifdef WIN32
    if (!ReadFile(serial->dev,buff,n,&nr,NULL)) return 0;
#else
    if ((nr=read(serial->dev,buff,n))<0) return 0;
#endif
    tracet(5,"readserial: exit dev=%d nr=%d\n",serial->dev,nr);
    
    /* write received stream to tcp server port */
    if (serial->tcpsvr&&nr>0) {
        writetcpsvr(serial->tcpsvr,buff,(int)nr,msg_tcp);
    }
    return nr;
}
/* write serial --------------------------------------------------------------*/
static int writeserial(serial_t *serial, unsigned char *buff, int n, char *msg)
{
    int ns;
    
    tracet(3,"writeserial: dev=%d n=%d\n",serial->dev,n);
    
    if (!serial) return 0;
#ifdef WIN32
    if ((ns=writeseribuff(serial,buff,n))<n) serial->error=1;
#else
    if (write(serial->dev,buff,n)<0) {
        serial->error=1;
        ns=0;
    }
#endif
    tracet(5,"writeserial: exit dev=%d ns=%d\n",serial->dev,ns);
    return ns;
}
/* get state serial ----------------------------------------------------------*/
static int stateserial(serial_t *serial)
{
    return !serial?0:(serial->error?-1:2);
}
/* get extended state serial -------------------------------------------------*/
static int statexserial(serial_t *serial, char *msg)
{
    char *p=msg;
    int state=!serial?0:(serial->error?-1:2);
    
    p+=sprintf(p,"serial:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  dev     = %d\n",(int)serial->dev);
    p+=sprintf(p,"  error   = %d\n",serial->error);
#ifdef WIN32
    p+=sprintf(p,"  buffsize= %d\n",serial->buffsize);
    p+=sprintf(p,"  wp      = %d\n",serial->wp);
    p+=sprintf(p,"  rp      = %d\n",serial->rp);
#endif
    return state;
}
/* open file -----------------------------------------------------------------*/
static int openfile_(file_t *file, gtime_t time, char *msg)
{    
    FILE *fp;
    char *rw,tagpath[MAXSTRPATH+4]="";
    char tagh[TIMETAGH_LEN+1]="";
    
    tracet(3,"openfile_: path=%s time=%s\n",file->path,time_str(time,0));
    
    file->time=utc2gpst(timeget());
    file->tick=file->tick_f=tickget();
    file->fpos_n=0;
    file->tick_n=0;
    
    /* use stdin or stdout if file path is null */
    if (!*file->path) {
        file->fp=file->mode&STR_MODE_R?stdin:stdout;
        return 1;
    }
    /* replace keywords */
    reppath(file->path,file->openpath,time,"","");
    
    /* create directory */
    if ((file->mode&STR_MODE_W)&&!(file->mode&STR_MODE_R)) {
        createdir(file->openpath);
    }
    if (file->mode&STR_MODE_R) rw="rb"; else rw="wb";
    
    if (!(file->fp=fopen(file->openpath,rw))) {
        sprintf(msg,"file open error: %s",file->openpath);
        tracet(1,"openfile: %s\n",msg);
        return 0;
    }
    tracet(4,"openfile_: open file %s (%s)\n",file->openpath,rw);
    
    sprintf(tagpath,"%s.tag",file->openpath);
    
    if (file->timetag) { /* output/sync time-tag */
        
        if (!(file->fp_tag=fopen(tagpath,rw))) {
            sprintf(msg,"tag open error: %s",tagpath);
            tracet(1,"openfile: %s\n",msg);
            fclose(file->fp);
            return 0;
        }
        tracet(4,"openfile_: open tag file %s (%s)\n",tagpath,rw);
        
        if (file->mode&STR_MODE_R) {
            if (fread(&tagh,TIMETAGH_LEN,1,file->fp_tag)==1&&
                fread(&file->time,sizeof(file->time),1,file->fp_tag)==1) {
                memcpy(&file->tick_f,tagh+TIMETAGH_LEN-4,sizeof(file->tick_f));
                file->wtime=file->time;
            }
            else {
                file->tick_f=0;
            }
            /* adust time to read playback file */
            timeset(gpst2utc(file->time));
        }
        else {
            sprintf(tagh,"TIMETAG RTKLIB %s",VER_RTKLIB);
            memcpy(tagh+TIMETAGH_LEN-4,&file->tick_f,sizeof(file->tick_f));
            fwrite(&tagh,1,TIMETAGH_LEN,file->fp_tag);
            fwrite(&file->time,1,sizeof(file->time),file->fp_tag);
            /* time tag file structure   */
            /*   HEADER(60)+TICK(4)+TIME(12)+ */
            /*   TICK0(4)+FPOS0(4/8)+    */
            /*   TICK1(4)+FPOS1(4/8)+... */
        }
    }
    else if (file->mode&STR_MODE_W) { /* remove time-tag */
        if ((fp=fopen(tagpath,"rb"))) {
            fclose(fp);
            remove(tagpath);
        }
    }
    return 1;
}
/* close file ----------------------------------------------------------------*/
static void closefile_(file_t *file)
{
    tracet(3,"closefile_: path=%s\n",file->path);
    
    if (file->fp) fclose(file->fp);
    if (file->fp_tag) fclose(file->fp_tag);
    if (file->fp_tmp) fclose(file->fp_tmp);
    if (file->fp_tag_tmp) fclose(file->fp_tag_tmp);
    file->fp=file->fp_tag=file->fp_tmp=file->fp_tag_tmp=NULL;
}
/* open file (path=filepath[::T[::+<off>][::x<speed>]][::S=swapintv][::P={4|8}] */
static file_t *openfile(const char *path, int mode, char *msg)
{
    file_t *file;
    gtime_t time,time0={0};
    double speed=1.0,start=0.0,swapintv=0.0;
    char *p;
    int timetag=0,size_fpos=(int)sizeof(size_t);
    
    tracet(3,"openfile: path=%s mode=%d\n",path,mode);
    
    if (!(mode&(STR_MODE_R|STR_MODE_W))) return NULL;
    
    /* file options */
    for (p=(char *)path;(p=strstr(p,"::"));p+=2) { /* file options */
        if      (*(p+2)=='T') timetag=1;
        else if (*(p+2)=='+') sscanf(p+2,"+%lf",&start);
        else if (*(p+2)=='x') sscanf(p+2,"x%lf",&speed);
        else if (*(p+2)=='S') sscanf(p+2,"S=%lf",&swapintv);
        else if (*(p+2)=='P') sscanf(p+2,"P=%d",&size_fpos);
    }
    if (start<=0.0) start=0.0;
    if (swapintv<=0.0) swapintv=0.0;
    
    if (!(file=(file_t *)malloc(sizeof(file_t)))) return NULL;
    
    file->fp=file->fp_tag=file->fp_tmp=file->fp_tag_tmp=NULL;
    strcpy(file->path,path);
    if ((p=strstr(file->path,"::"))) *p='\0';
    file->openpath[0]='\0';
    file->mode=mode;
    file->timetag=timetag;
    file->repmode=0;
    file->offset=0;
    file->size_fpos=size_fpos;
    file->time=file->wtime=time0;
    file->tick=file->tick_f=file->tick_n=file->fpos_n=0;
    file->start=start;
    file->speed=speed;
    file->swapintv=swapintv;
    initlock(&file->lock);
    
    time=utc2gpst(timeget());
    
    /* open new file */
    if (!openfile_(file,time,msg)) {
        free(file);
        return NULL;
    }
    return file;
}
/* close file ----------------------------------------------------------------*/
static void closefile(file_t *file)
{
    tracet(3,"closefile: fp=%d\n",file->fp);
    
    if (!file) return;
    closefile_(file);
    free(file);
}
/* open new swap file --------------------------------------------------------*/
static void swapfile(file_t *file, gtime_t time, char *msg)
{
    char openpath[MAXSTRPATH];
    
    tracet(3,"swapfile: fp=%d time=%s\n",file->fp,time_str(time,0));
    
    /* return if old swap file open */
    if (file->fp_tmp||file->fp_tag_tmp) return;
    
    /* check path of new swap file */
    reppath(file->path,openpath,time,"","");
    
    if (!strcmp(openpath,file->openpath)) {
        tracet(2,"swapfile: no need to swap %s\n",openpath);
        return;
    }
    /* save file pointer to temporary pointer */
    file->fp_tmp=file->fp;
    file->fp_tag_tmp=file->fp_tag;
    
    /* open new swap file */
    openfile_(file,time,msg);
}
/* close old swap file -------------------------------------------------------*/
static void swapclose(file_t *file)
{
    tracet(3,"swapclose: fp_tmp=%d\n",file->fp_tmp);
    
    if (file->fp_tmp    ) fclose(file->fp_tmp    );
    if (file->fp_tag_tmp) fclose(file->fp_tag_tmp);
    file->fp_tmp=file->fp_tag_tmp=NULL;
}
/* get state file ------------------------------------------------------------*/
static int statefile(file_t *file)
{
    return file?2:0;
}
/* get extended state file ---------------------------------------------------*/
static int statexfile(file_t *file, char *msg)
{
    char *p=msg,tstr1[32],tstr2[32];
    int state=file?2:0;
    
    p+=sprintf(p,"file:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    time2str(file->time ,tstr1,3);
    time2str(file->wtime,tstr2,3);
    p+=sprintf(p,"  path    = %s\n",file->path);
    p+=sprintf(p,"  openpath= %s\n",file->openpath);
    p+=sprintf(p,"  mode    = %d\n",file->mode);
    p+=sprintf(p,"  timetag = %d\n",file->timetag);
    p+=sprintf(p,"  repmode = %d\n",file->repmode);
    p+=sprintf(p,"  offsete = %d\n",file->offset);
    p+=sprintf(p,"  time    = %s\n",tstr1);
    p+=sprintf(p,"  wtime   = %s\n",tstr2);
    p+=sprintf(p,"  tick    = %u\n",file->tick);
    p+=sprintf(p,"  tick_f  = %u\n",file->tick_f);
    p+=sprintf(p,"  start   = %.3f\n",file->start);
    p+=sprintf(p,"  speed   = %.3f\n",file->speed);
    p+=sprintf(p,"  swapintv= %.3f\n",file->swapintv);
    return state;
}
/* read file -----------------------------------------------------------------*/
static int readfile(file_t *file, unsigned char *buff, int nmax, char *msg)
{
    struct timeval tv={0};
    fd_set rs;
    unsigned long fpos_8B;
    unsigned int t,tick,fpos_4B;
    long pos,n;
    int nr=0;
    
    tracet(4,"readfile: fp=%d nmax=%d\n",file->fp,nmax);
    
    if (!file) return 0;
    
    if (file->fp==stdin) {
#ifndef WIN32
        /* input from stdin */
        FD_ZERO(&rs); FD_SET(0,&rs);
        if (!select(1,&rs,NULL,NULL,&tv)) return 0;
        if ((nr=(int)read(0,buff,nmax))<0) return 0;
        return nr;
#else
        return 0;
#endif
    }
    if (file->fp_tag) {
        
        /* target tick */
        if (file->repmode) { /* slave */
            t=(unsigned int)(tick_master+file->offset);
        }
        else { /* master */
            t=(unsigned int)((tickget()-file->tick)*file->speed+file->start*1000.0);
            tick_master=t;
        }
        /* seek time-tag file to get next tick and file position */
        while ((int)(file->tick_n-t)<=0) {
            
            if (fread(&file->tick_n,sizeof(tick),1,file->fp_tag)<1||
                fread(file->size_fpos==4?(void *)&fpos_4B:(void *)&fpos_8B,
                      file->size_fpos,1,file->fp_tag)<1) {
                file->tick_n=(unsigned int)(-1);
                pos=ftell(file->fp);
                fseek(file->fp,0L,SEEK_END);
                file->fpos_n=(size_t)ftell(file->fp);
                fseek(file->fp,pos,SEEK_SET);
                break;
            }
            file->fpos_n=file->size_fpos==4?(size_t)fpos_4B:(size_t)fpos_8B;
        }
        if (file->tick_n==(unsigned int)(-1)) {
            sprintf(msg,"end");
        }
        else {
            sprintf(msg,"T%+.1fs",(int)t*0.001);
        }
        file->wtime = timeadd(file->time,(int)t*0.001);
        timeset(timeadd(gpst2utc(file->time),(int)file->tick_n*0.001));
        
        if ((n=file->fpos_n-ftell(file->fp))<nmax) {
            nmax=n;
        }
    }
    if (nmax>0) {
        nr=(int)fread(buff,1,nmax,file->fp);
    }
    if (feof(file->fp)) {
        sprintf(msg,"end");
    }
    tracet(5,"readfile: fp=%d nr=%d\n",file->fp,nr);
    return nr;
}
/* write file ----------------------------------------------------------------*/
static int writefile(file_t *file, unsigned char *buff, int n, char *msg)
{
    gtime_t wtime;
    unsigned int tick=tickget();
    int week1,week2,ns;
    double tow1,tow2,intv;
    size_t fpos,fpos_tmp;
    
    tracet(4,"writefile: fp=%d n=%d\n",file->fp,n);
    
    if (!file) return 0;
    
    wtime=utc2gpst(timeget()); /* write time in gpst */
    
    /* swap writing file */
    if (file->swapintv>0.0&&file->wtime.time!=0) {
        intv=file->swapintv*3600.0;
        tow1=time2gpst(file->wtime,&week1);
        tow2=time2gpst(wtime,&week2);
        tow2+=604800.0*(week2-week1);
        
        /* open new swap file */
        if (floor((tow1+fswapmargin)/intv)<floor((tow2+fswapmargin)/intv)) {
            swapfile(file,timeadd(wtime,fswapmargin),msg);
        }
        /* close old swap file */
        if (floor((tow1-fswapmargin)/intv)<floor((tow2-fswapmargin)/intv)) {
            swapclose(file);
        }
    }
    if (!file->fp) return 0;
    
    ns=(int)fwrite(buff,1,n,file->fp);
    fpos=ftell(file->fp);
    fflush(file->fp);
    file->wtime=wtime;
    
    if (file->fp_tmp) {
        fwrite(buff,1,n,file->fp_tmp);
        fpos_tmp=ftell(file->fp_tmp);
        fflush(file->fp_tmp);
    }
    if (file->fp_tag) {
        tick-=file->tick;
        fwrite(&tick,1,sizeof(tick),file->fp_tag);
        fwrite(&fpos,1,sizeof(fpos),file->fp_tag);
        fflush(file->fp_tag);
        
        if (file->fp_tag_tmp) {
            fwrite(&tick,1,sizeof(tick),file->fp_tag_tmp);
            fwrite(&fpos_tmp,1,sizeof(fpos_tmp),file->fp_tag_tmp);
            fflush(file->fp_tag_tmp);
        }
    }
    tracet(5,"writefile: fp=%d ns=%d tick=%5d fpos=%d\n",file->fp,ns,tick,fpos);
    
    return ns;
}
/* sync files by time-tag ----------------------------------------------------*/
static void syncfile(file_t *file1, file_t *file2)
{
    if (!file1->fp_tag||!file2->fp_tag) return;
    file1->repmode=0;
    file2->repmode=1;
    file2->offset=(int)(file1->tick_f-file2->tick_f);
}
/* decode tcp/ntrip path (path=[user[:passwd]@]addr[:port][/mntpnt[:str]]) ---*/
static void decodetcppath(const char *path, char *addr, char *port, char *user,
                          char *passwd, char *mntpnt, char *str)
{
    char buff[MAXSTRPATH],*p,*q;
    
    tracet(4,"decodetcpepath: path=%s\n",path);
    
    if (port) *port='\0';
    if (user) *user='\0';
    if (passwd) *passwd='\0';
    if (mntpnt) *mntpnt='\0';
    if (str) *str='\0';
    
    strcpy(buff,path);
    
    if (!(p=strrchr(buff,'@'))) p=buff;
    
    if ((p=strchr(p,'/'))) {
        if ((q=strchr(p+1,':'))) {
            *q='\0'; if (str) strcpy(str,q+1);
        }
        *p='\0'; if (mntpnt) strcpy(mntpnt,p+1);
    }
    if ((p=strrchr(buff,'@'))) {
        *p++='\0';
        if ((q=strchr(buff,':'))) {
             *q='\0'; if (passwd) strcpy(passwd,q+1);
        }
        if (user) strcpy(user,buff);
    }
    else p=buff;
    
    if ((q=strchr(p,':'))) {
        *q='\0'; if (port) strcpy(port,q+1);
    }
    if (addr) strcpy(addr,p);
}
/* get socket error ----------------------------------------------------------*/
#ifdef WIN32
static int errsock(void) {return WSAGetLastError();}
#else
static int errsock(void) {return errno;}
#endif

/* set socket option ---------------------------------------------------------*/
static int setsock(socket_t sock, char *msg)
{
    int bs=buffsize,mode=1;
#ifdef WIN32
    int tv=0;
#else
    struct timeval tv={0};
#endif
    tracet(3,"setsock: sock=%d\n",sock);
    
    if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(const char *)&tv,sizeof(tv))==-1||
        setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(const char *)&tv,sizeof(tv))==-1) {
        sprintf(msg,"sockopt error: notimeo");
        tracet(1,"setsock: setsockopt error 1 sock=%d err=%d\n",sock,errsock());
        closesocket(sock);
        return 0;
    }
    if (setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char *)&bs,sizeof(bs))==-1||
        setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char *)&bs,sizeof(bs))==-1) {
        tracet(1,"setsock: setsockopt error 2 sock=%d err=%d bs=%d\n",sock,errsock(),bs);
        sprintf(msg,"sockopt error: bufsiz");
    }
    if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(const char *)&mode,sizeof(mode))==-1) {
        tracet(1,"setsock: setsockopt error 3 sock=%d err=%d\n",sock,errsock());
        sprintf(msg,"sockopt error: nodelay");
    }
    return 1;
}
/* non-block accept ----------------------------------------------------------*/
static socket_t accept_nb(socket_t sock, struct sockaddr *addr, socklen_t *len)
{
    struct timeval tv={0};
    fd_set rs;
    int ret;
    
    FD_ZERO(&rs); FD_SET(sock,&rs);
    ret=select(sock+1,&rs,NULL,NULL,&tv);
    if (ret<=0) return (socket_t)ret;
    return accept(sock,addr,len);
}
/* non-block connect ---------------------------------------------------------*/
static int connect_nb(socket_t sock, struct sockaddr *addr, socklen_t len)
{
#ifdef WIN32
    u_long mode=1; 
    int err;
    
    ioctlsocket(sock,FIONBIO,&mode);
    if (connect(sock,addr,len)==-1) {
        err=errsock();
        if (err==WSAEWOULDBLOCK||err==WSAEINPROGRESS||
            err==WSAEALREADY   ||err==WSAEINVAL) return 0;
        if (err!=WSAEISCONN) return -1;
    }
#else
    struct timeval tv={0};
    fd_set rs,ws;
    int err,flag;
    
    flag=fcntl(sock,F_GETFL,0);
    fcntl(sock,F_SETFL,flag|O_NONBLOCK);
    if (connect(sock,addr,len)==-1) {
        err=errsock();
        if (err!=EISCONN&&err!=EINPROGRESS&&err!=EALREADY) return -1;
        FD_ZERO(&rs); FD_SET(sock,&rs); ws=rs;
        if (select(sock+1,&rs,&ws,NULL,&tv)==0) return 0;
    }
#endif
    return 1;
}
/* non-block receive ---------------------------------------------------------*/
static int recv_nb(socket_t sock, unsigned char *buff, int n)
{
    struct timeval tv={0};
    fd_set rs;
    int ret,nr;
    
    FD_ZERO(&rs); FD_SET(sock,&rs);
    ret=select(sock+1,&rs,NULL,NULL,&tv);
    if (ret<=0) return ret;
    nr=recv(sock,(char *)buff,n,0);
    return nr<=0?-1:nr;
}
/* non-block send ------------------------------------------------------------*/
static int send_nb(socket_t sock, unsigned char *buff, int n)
{
    struct timeval tv={0};
    fd_set ws;
    int ret,ns;
    
    FD_ZERO(&ws); FD_SET(sock,&ws);
    ret=select(sock+1,NULL,&ws,NULL,&tv);
    if (ret<=0) return ret;
    ns=send(sock,(char *)buff,n,0);
    return ns<n?-1:ns;
}
/* generate tcp socket -------------------------------------------------------*/
static int gentcp(tcp_t *tcp, int type, char *msg)
{
    struct hostent *hp;
#ifdef SVR_REUSEADDR
    int opt=1;
#endif
    
    tracet(3,"gentcp: type=%d\n",type);
    
    /* generate socket */
    if ((tcp->sock=socket(AF_INET,SOCK_STREAM,0))==(socket_t)-1) {
        sprintf(msg,"socket error (%d)",errsock());
        tracet(1,"gentcp: socket error err=%d\n",errsock());
        tcp->state=-1;
        return 0;
    }
    if (!setsock(tcp->sock,msg)) {
        tcp->state=-1;
        return 0;
    }
    memset(&tcp->addr,0,sizeof(tcp->addr));
    tcp->addr.sin_family=AF_INET;
    tcp->addr.sin_port=htons(tcp->port);
    
    if (type==0) { /* server socket */
    
#ifdef SVR_REUSEADDR
        /* multiple-use of server socket */
        setsockopt(tcp->sock,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,
                   sizeof(opt));
#endif
        if (bind(tcp->sock,(struct sockaddr *)&tcp->addr,sizeof(tcp->addr))==-1) {
            sprintf(msg,"bind error (%d) : %d",errsock(),tcp->port);
            tracet(1,"gentcp: bind error port=%d err=%d\n",tcp->port,errsock());
            closesocket(tcp->sock);
            tcp->state=-1;
            return 0;
        }
        listen(tcp->sock,5);
    }
    else { /* client socket */
        if (!(hp=gethostbyname(tcp->saddr))) {
            sprintf(msg,"address error (%s)",tcp->saddr);
            tracet(1,"gentcp: gethostbyname error addr=%s err=%d\n",tcp->saddr,errsock());
            closesocket(tcp->sock);
            tcp->state=0;
            tcp->tcon=ticonnect;
            tcp->tdis=tickget();
            return 0;
        }
        memcpy(&tcp->addr.sin_addr,hp->h_addr,hp->h_length);
    }
    tcp->state=1;
    tcp->tact=tickget();
    tracet(5,"gentcp: exit sock=%d\n",tcp->sock);
    return 1;
}
/* disconnect tcp ------------------------------------------------------------*/
static void discontcp(tcp_t *tcp, int tcon)
{
    tracet(3,"discontcp: sock=%d tcon=%d\n",tcp->sock,tcon);
    
    closesocket(tcp->sock);
    tcp->state=0;
    tcp->tcon=tcon;
    tcp->tdis=tickget();
}
/* open tcp server -----------------------------------------------------------*/
static tcpsvr_t *opentcpsvr(const char *path, char *msg)
{
    tcpsvr_t *tcpsvr,tcpsvr0={{0}};
    char port[256]="";
    
    tracet(3,"opentcpsvr: path=%s\n",path);
    
    if (!(tcpsvr=(tcpsvr_t *)malloc(sizeof(tcpsvr_t)))) return NULL;
    *tcpsvr=tcpsvr0;
    decodetcppath(path,tcpsvr->svr.saddr,port,NULL,NULL,NULL,NULL);
    if (sscanf(port,"%d",&tcpsvr->svr.port)<1) {
        sprintf(msg,"port error: %s",port);
        tracet(1,"opentcpsvr: port error port=%s\n",port);
        free(tcpsvr);
        return NULL;
    }
    if (!gentcp(&tcpsvr->svr,0,msg)) {
        free(tcpsvr);
        return NULL;
    }
    tcpsvr->svr.tcon=0;
    return tcpsvr;
}
/* close tcp server ----------------------------------------------------------*/
static void closetcpsvr(tcpsvr_t *tcpsvr)
{
    int i;
    
    tracet(3,"closetcpsvr:\n");
    
    for (i=0;i<MAXCLI;i++) {
        if (tcpsvr->cli[i].state) closesocket(tcpsvr->cli[i].sock);
    }
    closesocket(tcpsvr->svr.sock);
    free(tcpsvr);
}
/* update tcp server ---------------------------------------------------------*/
static void updatetcpsvr(tcpsvr_t *tcpsvr, char *msg)
{
    char saddr[256]="";
    int i,n=0;
    
    tracet(4,"updatetcpsvr: state=%d\n",tcpsvr->svr.state);
    
    if (tcpsvr->svr.state==0) return;
    
    for (i=0;i<MAXCLI;i++) {
        if (!tcpsvr->cli[i].state) continue;
        strcpy(saddr,tcpsvr->cli[i].saddr);
        n++;
    }
    if (n==0) {
        tcpsvr->svr.state=1;
        sprintf(msg,"waiting...");
        return;
    }
    tcpsvr->svr.state=2;
    if (n==1) sprintf(msg,"%s",saddr); else sprintf(msg,"%d clients",n);
}
/* accept client connection --------------------------------------------------*/
static int accsock(tcpsvr_t *tcpsvr, char *msg)
{
    struct sockaddr_in addr;
    socket_t sock;
    socklen_t len=sizeof(addr);
    int i,err;
    
    tracet(4,"accsock: sock=%d\n",tcpsvr->svr.sock);
    
    for (i=0;i<MAXCLI;i++) if (tcpsvr->cli[i].state==0) break;
    if (i>=MAXCLI) return 0; /* too many client */
    
    if ((sock=accept_nb(tcpsvr->svr.sock,(struct sockaddr *)&addr,&len))==(socket_t)-1) {
        err=errsock();
        sprintf(msg,"accept error (%d)",err);
        tracet(1,"accsock: accept error sock=%d err=%d\n",tcpsvr->svr.sock,err);
        closesocket(tcpsvr->svr.sock);
        tcpsvr->svr.state=0;
        return 0;
    }
    if (sock==0) return 0;
    if (!setsock(sock,msg)) return 0;
    
    tcpsvr->cli[i].sock=sock;
    memcpy(&tcpsvr->cli[i].addr,&addr,sizeof(addr));
    strcpy(tcpsvr->cli[i].saddr,inet_ntoa(addr.sin_addr));
    sprintf(msg,"%s",tcpsvr->cli[i].saddr);
    tracet(3,"accsock: connected sock=%d addr=%s i=%d\n",
           tcpsvr->cli[i].sock,tcpsvr->cli[i].saddr,i);
    tcpsvr->cli[i].state=2;
    tcpsvr->cli[i].tact=tickget();
    return 1;
}
/* wait socket accept --------------------------------------------------------*/
static int waittcpsvr(tcpsvr_t *tcpsvr, char *msg)
{
    tracet(4,"waittcpsvr: sock=%d state=%d\n",tcpsvr->svr.sock,tcpsvr->svr.state);
    
    if (tcpsvr->svr.state<=0) return 0;
    
    while (accsock(tcpsvr,msg)) ;
    
    updatetcpsvr(tcpsvr,msg);
    return tcpsvr->svr.state==2;
}
/* read tcp server -----------------------------------------------------------*/
static int readtcpsvr(tcpsvr_t *tcpsvr, unsigned char *buff, int n, char *msg)
{
    int i,nr,err;
    
    tracet(4,"readtcpsvr: state=%d\n",tcpsvr->svr.state);
    
    if (!waittcpsvr(tcpsvr,msg)) return 0;
    
    for (i=0;i<MAXCLI;i++) {
        if (tcpsvr->cli[i].state!=2) continue;
        
        if ((nr=recv_nb(tcpsvr->cli[i].sock,buff,n))==-1) {
            if ((err=errsock())) {
                tracet(1,"readtcpsvr: recv error sock=%d err=%d\n",
                       tcpsvr->cli[i].sock,err);
            }
            discontcp(&tcpsvr->cli[i],ticonnect);
            updatetcpsvr(tcpsvr,msg);
            return 0;
        }
        if (nr>0) {
            tcpsvr->cli[i].tact=tickget();
            return nr;
        }
    }
    return 0;
}
/* write tcp server ----------------------------------------------------------*/
static int writetcpsvr(tcpsvr_t *tcpsvr, unsigned char *buff, int n, char *msg)
{
    int i,ns=0,err;
    
    tracet(4,"writetcpsvr: state=%d n=%d\n",tcpsvr->svr.state,n);
    
    if (!waittcpsvr(tcpsvr,msg)) return 0;
    
    for (i=0;i<MAXCLI;i++) {
        if (tcpsvr->cli[i].state!=2) continue;
        
        if ((ns=send_nb(tcpsvr->cli[i].sock,buff,n))==-1) {
            if ((err=errsock())) {
                tracet(1,"writetcpsvr: send error i=%d sock=%d err=%d\n",i,
                       tcpsvr->cli[i].sock,err);
            }
            discontcp(&tcpsvr->cli[i],ticonnect);
            updatetcpsvr(tcpsvr,msg);
            return 0;
        }
        if (ns>0) tcpsvr->cli[i].tact=tickget();
    }
    return ns;
}
/* get state tcp server ------------------------------------------------------*/
static int statetcpsvr(tcpsvr_t *tcpsvr)
{
    return tcpsvr?tcpsvr->svr.state:0;
}
/* print extended state tcp --------------------------------------------------*/
static int statextcp(tcp_t *tcp, char *msg)
{
    char *p=msg;
    
    p+=sprintf(p,"    state = %d\n",tcp->state);
    p+=sprintf(p,"    saddr = %s\n",tcp->saddr);
    p+=sprintf(p,"    port  = %d\n",tcp->port);
    p+=sprintf(p,"    sock  = %d\n",(int)tcp->sock);
#if 0
    p+=sprintf(p,"    tcon  = %d\n",tcp->tcon);
    p+=sprintf(p,"    tact  = %u\n",tcp->tact);
    p+=sprintf(p,"    tdis  = %u\n",tcp->tdis);
#endif
    return (int)(p-msg);
}
/* get extended state tcp server ---------------------------------------------*/
static int statextcpsvr(tcpsvr_t *tcpsvr, char *msg)
{
    char *p=msg;
    int i,state=tcpsvr?tcpsvr->svr.state:0;
    
    p+=sprintf(p,"tcpsvr:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  svr:\n");
    p+=statextcp(&tcpsvr->svr,p);
    for (i=0;i<MAXCLI;i++) {
        if (!tcpsvr->cli[i].state) continue;
        p+=sprintf(p,"  cli#%d:\n",i);
        p+=statextcp(tcpsvr->cli+i,p);
    }
    return state;
}
/* connect server ------------------------------------------------------------*/
static int consock(tcpcli_t *tcpcli, char *msg)
{
    int stat,err;
    
    tracet(4,"consock: sock=%d\n",tcpcli->svr.sock);
    
    /* wait re-connect */
    if (tcpcli->svr.tcon<0||(tcpcli->svr.tcon>0&&
        (int)(tickget()-tcpcli->svr.tdis)<tcpcli->svr.tcon)) {
        return 0;
    }
    /* non-block connect */
    if ((stat=connect_nb(tcpcli->svr.sock,(struct sockaddr *)&tcpcli->svr.addr,
                         sizeof(tcpcli->svr.addr)))==-1) {
        err=errsock();
        sprintf(msg,"connect error (%d)",err);
        tracet(2,"consock: connect error sock=%d err=%d\n",tcpcli->svr.sock,err);
        closesocket(tcpcli->svr.sock);
        tcpcli->svr.state=0;
        return 0;
    }
    if (!stat) { /* not connect */
        sprintf(msg,"connecting...");
        return 0;
    }
    sprintf(msg,"%s",tcpcli->svr.saddr);
    tracet(3,"consock: connected sock=%d addr=%s\n",tcpcli->svr.sock,tcpcli->svr.saddr);
    tcpcli->svr.state=2;
    tcpcli->svr.tact=tickget();
    return 1;
}
/* open tcp client -----------------------------------------------------------*/
static tcpcli_t *opentcpcli(const char *path, char *msg)
{
    tcpcli_t *tcpcli,tcpcli0={{0}};
    char port[256]="";
    
    tracet(3,"opentcpcli: path=%s\n",path);
    
    if (!(tcpcli=(tcpcli_t *)malloc(sizeof(tcpcli_t)))) return NULL;
    *tcpcli=tcpcli0;
    decodetcppath(path,tcpcli->svr.saddr,port,NULL,NULL,NULL,NULL);
    if (sscanf(port,"%d",&tcpcli->svr.port)<1) {
        sprintf(msg,"port error: %s",port);
        tracet(2,"opentcp: port error port=%s\n",port);
        free(tcpcli);
        return NULL;
    }
    tcpcli->svr.tcon=0;
    tcpcli->toinact=toinact;
    tcpcli->tirecon=ticonnect;
    return tcpcli;
}
/* close tcp client ----------------------------------------------------------*/
static void closetcpcli(tcpcli_t *tcpcli)
{
    tracet(3,"closetcpcli: sock=%d\n",tcpcli->svr.sock);
    
    closesocket(tcpcli->svr.sock);
    free(tcpcli);
}
/* wait socket connect -------------------------------------------------------*/
static int waittcpcli(tcpcli_t *tcpcli, char *msg)
{
    tracet(4,"waittcpcli: sock=%d state=%d\n",tcpcli->svr.sock,tcpcli->svr.state);
    
    if (tcpcli->svr.state<0) return 0;
    
    if (tcpcli->svr.state==0) { /* close */
        if (!gentcp(&tcpcli->svr,1,msg)) return 0;
    }
    if (tcpcli->svr.state==1) { /* wait */
        if (!consock(tcpcli,msg)) return 0;
    }
    if (tcpcli->svr.state==2) { /* connect */
        if (tcpcli->toinact>0&&
            (int)(tickget()-tcpcli->svr.tact)>tcpcli->toinact) {
            sprintf(msg,"timeout");
            tracet(2,"waittcpcli: inactive timeout sock=%d\n",tcpcli->svr.sock);
            discontcp(&tcpcli->svr,tcpcli->tirecon);
            return 0;
        }
    }
    return 1;
}
/* read tcp client -----------------------------------------------------------*/
static int readtcpcli(tcpcli_t *tcpcli, unsigned char *buff, int n, char *msg)
{
    int nr,err;
    
    tracet(4,"readtcpcli: sock=%d\n",tcpcli->svr.sock);
    
    if (!waittcpcli(tcpcli,msg)) return 0;
    
    if ((nr=recv_nb(tcpcli->svr.sock,buff,n))==-1) {
        if ((err=errsock())) {
            tracet(2,"readtcpcli: recv error sock=%d err=%d\n",tcpcli->svr.sock,err);
            sprintf(msg,"recv error (%d)",err);
        }
        else {
            sprintf(msg,"disconnected");
        }
        discontcp(&tcpcli->svr,tcpcli->tirecon);
        return 0;
    }
    if (nr>0) tcpcli->svr.tact=tickget();
    tracet(5,"readtcpcli: exit sock=%d nr=%d\n",tcpcli->svr.sock,nr);
    return nr;
}
/* write tcp client ----------------------------------------------------------*/
static int writetcpcli(tcpcli_t *tcpcli, unsigned char *buff, int n, char *msg)
{
    int ns,err;
    
    tracet(3,"writetcpcli: sock=%d state=%d n=%d\n",tcpcli->svr.sock,tcpcli->svr.state,n);
    
    if (!waittcpcli(tcpcli,msg)) return 0;
    
    if ((ns=send_nb(tcpcli->svr.sock,buff,n))==-1) {
        if ((err=errsock())) {
            tracet(2,"writetcp: send error sock=%d err=%d\n",tcpcli->svr.sock,err);
            sprintf(msg,"send error (%d)",err);
        }
        else {
            sprintf(msg,"disconnected");
        }
        discontcp(&tcpcli->svr,tcpcli->tirecon);
        return 0;
    }
    if (ns>0) tcpcli->svr.tact=tickget();
    tracet(5,"writetcpcli: exit sock=%d ns=%d\n",tcpcli->svr.sock,ns);
    return ns;
}
/* get state tcp client ------------------------------------------------------*/
static int statetcpcli(tcpcli_t *tcpcli)
{
    return tcpcli?tcpcli->svr.state:0;
}
/* get extended state tcp client ---------------------------------------------*/
static int statextcpcli(tcpcli_t *tcpcli, char *msg)
{
    return tcpcli?tcpcli->svr.state:0;
}
/* base64 encoder ------------------------------------------------------------*/
static int encbase64(char *str, const unsigned char *byte, int n)
{
    const char table[]=
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i,j,k,b;
    
    tracet(4,"encbase64: n=%d\n",n);
    
    for (i=j=0;i/8<n;) {
        for (k=b=0;k<6;k++,i++) {
            b<<=1; if (i/8<n) b|=(byte[i/8]>>(7-i%8))&0x1;
        }
        str[j++]=table[b];
    }
    while (j&0x3) str[j++]='=';
    str[j]='\0';
    tracet(5,"encbase64: str=%s\n",str);
    return j;
}
/* send ntrip server request -------------------------------------------------*/
static int reqntrip_s(ntrip_t *ntrip, char *msg)
{
    char buff[1024+NTRIP_MAXSTR],*p=buff;
    
    tracet(3,"reqntrip_s: state=%d\n",ntrip->state);
    
    p+=sprintf(p,"SOURCE %s %s\r\n",ntrip->passwd,ntrip->mntpnt);
    p+=sprintf(p,"Source-Agent: NTRIP %s\r\n",NTRIP_AGENT);
    p+=sprintf(p,"STR: %s\r\n",ntrip->str);
    p+=sprintf(p,"\r\n");
    
    if (writetcpcli(ntrip->tcp,(unsigned char *)buff,p-buff,msg)!=p-buff) return 0;
    
    tracet(3,"reqntrip_s: send request state=%d ns=%d\n",ntrip->state,p-buff);
    tracet(5,"reqntrip_s: n=%d buff=\n%s\n",p-buff,buff);
    ntrip->state=1;
    return 1;
}
/* send ntrip client request -------------------------------------------------*/
static int reqntrip_c(ntrip_t *ntrip, char *msg)
{
    char buff[1024],user[512],*p=buff;
    
    tracet(3,"reqntrip_c: state=%d\n",ntrip->state);
    
    p+=sprintf(p,"GET %s/%s HTTP/1.0\r\n",ntrip->url,ntrip->mntpnt);
    p+=sprintf(p,"User-Agent: NTRIP %s\r\n",NTRIP_AGENT);
    
    if (!*ntrip->user) {
        p+=sprintf(p,"Accept: */*\r\n");
        p+=sprintf(p,"Connection: close\r\n");
    }
    else {
        sprintf(user,"%s:%s",ntrip->user,ntrip->passwd);
        p+=sprintf(p,"Authorization: Basic ");
        p+=encbase64(p,(unsigned char *)user,strlen(user));
        p+=sprintf(p,"\r\n");
    }
    p+=sprintf(p,"\r\n");
    
    if (writetcpcli(ntrip->tcp,(unsigned char *)buff,p-buff,msg)!=p-buff) return 0;
    
    tracet(3,"reqntrip_c: send request state=%d ns=%d\n",ntrip->state,p-buff);
    tracet(5,"reqntrip_c: n=%d buff=\n%s\n",p-buff,buff);
    ntrip->state=1;
    return 1;
}
/* test ntrip server response ------------------------------------------------*/
static int rspntrip_s(ntrip_t *ntrip, char *msg)
{
    int i,nb;
    char *p,*q;
    
    tracet(3,"rspntrip_s: state=%d nb=%d\n",ntrip->state,ntrip->nb);
    ntrip->buff[ntrip->nb]='0';
    tracet(5,"rspntrip_s: n=%d buff=\n%s\n",ntrip->nb,ntrip->buff);
    
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_OK_SVR))) { /* ok */
        q=(char *)ntrip->buff;
        p+=strlen(NTRIP_RSP_OK_SVR);
        ntrip->nb-=p-q;
        for (i=0;i<ntrip->nb;i++) *q++=*p++;
        ntrip->state=2;
        sprintf(msg,"%s/%s",ntrip->tcp->svr.saddr,ntrip->mntpnt);
        tracet(3,"rspntrip_s: response ok nb=%d\n",ntrip->nb);
        return 1;
    }
    else if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_ERROR))) { /* error */
        nb=ntrip->nb<MAXSTATMSG?ntrip->nb:MAXSTATMSG;
        strncpy(msg,(char *)ntrip->buff,nb); msg[nb]=0;
        if ((p=strchr(msg,'\r'))) *p='\0';
        tracet(3,"rspntrip_s: %s nb=%d\n",msg,ntrip->nb);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if (ntrip->nb>=NTRIP_MAXRSP) { /* buffer overflow */
        sprintf(msg,"response overflow");
        tracet(3,"rspntrip_s: response overflow nb=%d\n",ntrip->nb);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    tracet(5,"rspntrip_s: exit state=%d nb=%d\n",ntrip->state,ntrip->nb);
    return 0;
}
/* test ntrip client response ------------------------------------------------*/
static int rspntrip_c(ntrip_t *ntrip, char *msg)
{
    int i;
    char *p,*q;
    
    tracet(3,"rspntrip_c: state=%d nb=%d\n",ntrip->state,ntrip->nb);
    ntrip->buff[ntrip->nb]='0';
    tracet(5,"rspntrip_c: n=%d buff=\n%s\n",ntrip->nb,ntrip->buff);
    
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_OK_CLI))) { /* ok */
        q=(char *)ntrip->buff;
        p+=strlen(NTRIP_RSP_OK_CLI);
        ntrip->nb-=p-q;
        for (i=0;i<ntrip->nb;i++) *q++=*p++;
        ntrip->state=2;
        sprintf(msg,"%s/%s",ntrip->tcp->svr.saddr,ntrip->mntpnt);
        tracet(3,"rspntrip_c: response ok nb=%d\n",ntrip->nb);
        return 1;
    }
    if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_SRCTBL))) { /* source table */
        if (!*ntrip->mntpnt) { /* source table request */
            ntrip->state=2;
            sprintf(msg,"source table received");
            tracet(3,"rspntrip_c: receive source table nb=%d\n",ntrip->nb);
            return 1;
        }
        sprintf(msg,"no mountp. reconnect...");
        tracet(2,"rspntrip_c: no mount point nb=%d\n",ntrip->nb);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if ((p=strstr((char *)ntrip->buff,NTRIP_RSP_HTTP))) { /* http response */
        if ((q=strchr(p,'\r'))) *q='\0'; else ntrip->buff[128]='\0';
        strcpy(msg,p);
        tracet(3,"rspntrip_s: %s nb=%d\n",msg,ntrip->nb);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    else if (ntrip->nb>=NTRIP_MAXRSP) { /* buffer overflow */
        sprintf(msg,"response overflow");
        tracet(2,"rspntrip_s: response overflow nb=%d\n",ntrip->nb);
        ntrip->nb=0;
        ntrip->buff[0]='\0';
        ntrip->state=0;
        discontcp(&ntrip->tcp->svr,ntrip->tcp->tirecon);
    }
    tracet(5,"rspntrip_c: exit state=%d nb=%d\n",ntrip->state,ntrip->nb);
    return 0;
}
/* wait ntrip request/response -----------------------------------------------*/
static int waitntrip(ntrip_t *ntrip, char *msg)
{
    int n;
    char *p;
    
    tracet(4,"waitntrip: state=%d nb=%d\n",ntrip->state,ntrip->nb);
    
    if (ntrip->state<0) return 0; /* error */
    
    if (ntrip->tcp->svr.state<2) ntrip->state=0; /* tcp disconnected */
    
    if (ntrip->state==0) { /* send request */
        if (!(ntrip->type==0?reqntrip_s(ntrip,msg):reqntrip_c(ntrip,msg))) {
            return 0;
        }
        tracet(3,"waitntrip: state=%d nb=%d\n",ntrip->state,ntrip->nb);
    }
    if (ntrip->state==1) { /* read response */
        p=(char *)ntrip->buff+ntrip->nb;
        if ((n=readtcpcli(ntrip->tcp,(unsigned char *)p,NTRIP_MAXRSP-ntrip->nb-1,msg))==0) {
            tracet(5,"waitntrip: readtcp n=%d\n",n);
            return 0;
        }
        ntrip->nb+=n; ntrip->buff[ntrip->nb]='\0';
        
        /* wait response */
        return ntrip->type==0?rspntrip_s(ntrip,msg):rspntrip_c(ntrip,msg);
    }
    return 1;
}
/* open ntrip ----------------------------------------------------------------*/
static ntrip_t *openntrip(const char *path, int type, char *msg)
{
    ntrip_t *ntrip;
    int i;
    char addr[256]="",port[256]="",tpath[MAXSTRPATH];
    
    tracet(3,"openntrip: path=%s type=%d\n",path,type);
    
    if (!(ntrip=(ntrip_t *)malloc(sizeof(ntrip_t)))) return NULL;
    
    ntrip->state=0;
    ntrip->type=type; /* 0:server,1:client */
    ntrip->nb=0;
    ntrip->url[0]='\0';
    ntrip->mntpnt[0]=ntrip->user[0]=ntrip->passwd[0]=ntrip->str[0]='\0';
    for (i=0;i<NTRIP_MAXRSP;i++) ntrip->buff[i]=0;
    
    /* decode tcp/ntrip path */
    decodetcppath(path,addr,port,ntrip->user,ntrip->passwd,ntrip->mntpnt,
                  ntrip->str);
    
    /* use default port if no port specified */
    if (!*port) {
        sprintf(port,"%d",type?NTRIP_CLI_PORT:NTRIP_SVR_PORT);
    }
    sprintf(tpath,"%s:%s",addr,port);
    
    /* ntrip access via proxy server */
    if (*proxyaddr) {
        sprintf(ntrip->url,"http://%s",tpath);
        strcpy(tpath,proxyaddr);
    }
    /* open tcp client stream */
    if (!(ntrip->tcp=opentcpcli(tpath,msg))) {
        tracet(2,"openntrip: opentcp error\n");
        free(ntrip);
        return NULL;
    }
    return ntrip;
}
/* close ntrip ---------------------------------------------------------------*/
static void closentrip(ntrip_t *ntrip)
{
    tracet(3,"closentrip: state=%d\n",ntrip->state);
    
    closetcpcli(ntrip->tcp);
    free(ntrip);
}
/* read ntrip ----------------------------------------------------------------*/
static int readntrip(ntrip_t *ntrip, unsigned char *buff, int n, char *msg)
{
    int nb;
    
    tracet(4,"readntrip:\n");
    
    if (!waitntrip(ntrip,msg)) return 0;
    
    if (ntrip->nb>0) { /* read response buffer first */
        nb=ntrip->nb<=n?ntrip->nb:n;
        memcpy(buff,ntrip->buff+ntrip->nb-nb,nb);
        ntrip->nb=0;
        return nb;
    }
    return readtcpcli(ntrip->tcp,buff,n,msg);
}
/* write ntrip ---------------------------------------------------------------*/
static int writentrip(ntrip_t *ntrip, unsigned char *buff, int n, char *msg)
{
    tracet(3,"writentrip: n=%d\n",n);
    
    if (!waitntrip(ntrip,msg)) return 0;
    
    return writetcpcli(ntrip->tcp,buff,n,msg);
}
/* get state ntrip -----------------------------------------------------------*/
static int statentrip(ntrip_t *ntrip)
{
    return !ntrip?0:(ntrip->state==0?ntrip->tcp->svr.state:ntrip->state);
}
/* get extended state ntrip --------------------------------------------------*/
static int statexntrip(ntrip_t *ntrip, char *msg)
{
    char *p=msg;
    int state=!ntrip?0:(ntrip->state==0?ntrip->tcp->svr.state:ntrip->state);
    
    p+=sprintf(p,"ntrip:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  state   = %d\n",state);
    p+=sprintf(p,"  type    = %d\n",ntrip->type);
    p+=sprintf(p,"  nb      = %d\n",ntrip->nb);
    p+=sprintf(p,"  url     = %s\n",ntrip->url);
    p+=sprintf(p,"  mntpnt  = %s\n",ntrip->mntpnt);
    p+=sprintf(p,"  user    = %s\n",ntrip->user);
    p+=sprintf(p,"  passwd  = %s\n",ntrip->passwd);
    p+=sprintf(p,"  str     = %s\n",ntrip->str);
    p+=sprintf(p,"  svr:\n");
    p+=statextcp(&ntrip->tcp->svr,p);
    return state;
}
/* open ntrip-caster ---------------------------------------------------------*/
static ntripc_t *openntripc(const char *path, int type, char *msg)
{
    ntripc_t *ntripc;
    int i,j;
    char port[256]="",tpath[MAXSTRPATH];
    
    tracet(3,"openntripc: path=%s type=%d\n",path,type);
    
    if (!(ntripc=(ntripc_t *)malloc(sizeof(ntripc_t)))) return NULL;
    
    ntripc->state=0;
    ntripc->type=type; /* 0:server,1:client */
    ntripc->mntpnt[0]=ntripc->user[0]=ntripc->passwd[0]='\0';
    for (i=0;i<MAXCLI;i++) {
        ntripc->con[i].state=0;
        ntripc->con[i].mntpnt[0]='\0';
        ntripc->con[i].str[0]='\0';
        ntripc->con[i].nb=0;
        for (j=0;j<NTRIP_MAXRSP;j++) ntripc->con[i].buff[j]=0;
    }
    initlock(&ntripc->lock_srctbl);
    
    /* decode tcp/ntrip path */
    decodetcppath(path,NULL,port,ntripc->user,ntripc->passwd,NULL,NULL);
    
    /* use default port if no port specified */
    if (!*port) {
        sprintf(port,"%d",type?NTRIP_CLI_PORT:NTRIP_SVR_PORT);
    }
    sprintf(tpath,":%s",port);
    
    /* open tcp server stream */
    if (!(ntripc->tcp=opentcpsvr(tpath,msg))) {
        tracet(2,"openntripc: opentcpsvr error port=%d\n",port);
        free(ntripc);
        return NULL;
    }
    return ntripc;
}
/* close ntrip-caster --------------------------------------------------------*/
static void closentripc(ntripc_t *ntripc)
{
    tracet(3,"closentripc: state=%d\n",ntripc->state);
    
    closetcpsvr(ntripc->tcp);
    free(ntripc->srctbl);
    free(ntripc);
}
/* disconnect ntrip-caster connection ----------------------------------------*/
static void discon_ntripc(ntripc_t *ntripc, int i)
{
    tracet(3,"discon_ntripc: i=%d\n",i);
    
    discontcp(&ntripc->tcp->cli[i],ticonnect);
    ntripc->con[i].nb=0;
    ntripc->con[i].buff[0]='\0';
    ntripc->con[i].state=0;
}
/* test mountpoint in source table -------------------------------------------*/
static int test_mntpnt(ntripc_t *ntripc, const char *mntpnt)
{
    char *p,str[256];
    
    lock(&ntripc->lock_srctbl);
    
    if (!ntripc->srctbl) {
        unlock(&ntripc->lock_srctbl);
        return 1;
    }
    for (p=ntripc->srctbl;(p=strstr(p,"STR;"));p++) {
        if (sscanf(p,"STR;%255[^;]",str)&&!strcmp(str,mntpnt)) break;
    }
    unlock(&ntripc->lock_srctbl);
    
    return p!=NULL;
}
/* send ntrip source table ---------------------------------------------------*/
static void send_srctbl(ntripc_t *ntripc, socket_t sock)
{
    char buff[1024],*p=buff;
    int len;
    
    lock(&ntripc->lock_srctbl);
    
    len=ntripc->srctbl?strlen(ntripc->srctbl):0;
    p+=sprintf(p,"%s",NTRIP_RSP_SRCTBL);
    p+=sprintf(p,"Server: %s %s %s\r\n","RTKLIB",VER_RTKLIB,PATCH_LEVEL);
    p+=sprintf(p,"Date: %s UTC\r\n",time_str(timeget(),0));
    p+=sprintf(p,"Connection: close\r\n");
    p+=sprintf(p,"Content-Type: text/plain\r\n");
    p+=sprintf(p,"Content-Length: %d\r\n\r\n",len);
    send_nb(sock,(unsigned char *)buff,strlen(buff));
    if (len>0) {
        send_nb(sock,(unsigned char *)ntripc->srctbl,len);
    }
    unlock(&ntripc->lock_srctbl);
}
/* test ntrip-caster client request ------------------------------------------*/
static void rsp_ntripc_c(ntripc_t *ntripc, int i)
{
    const char *rsp1=NTRIP_RSP_UNAUTH,*rsp2=NTRIP_RSP_OK_CLI;
    ntripc_con_t *con=ntripc->con+i;
    char url[256]="",mntpnt[256]="",proto[256]="",user[513],user_pwd[256],*p,*q;
    
    tracet(3,"rspntripc_c i=%d\n",i);
    con->buff[con->nb]='\0';
    tracet(5,"rspntripc_c: n=%d,buff=\n%s\n",con->nb,con->buff);
    
    if (con->nb>=NTRIP_MAXRSP-1) { /* buffer overflow */
        tracet(2,"rsp_ntripc_c: request buffer overflow\n");
        discon_ntripc(ntripc,i);
        return;
    }
    /* test GET and User-Agent */
    if (!(p=strstr((char *)con->buff,"GET"))||!(q=strstr(p,"\r\n"))||
        !(q=strstr(q,"User-Agent:"))||!strstr(q,"\r\n")) {
        tracet(2,"rsp_ntripc_c: NTRIP request error\n");
        discon_ntripc(ntripc,i);
        return;
    }
    /* test protocol */
    if (sscanf(p,"GET %255s %255s",url,proto)<2||strcmp(proto,"HTTP/1.0")) {
        tracet(2,"rsp_ntripc_c: NTRIP request error proto=%s\n",proto);
        discon_ntripc(ntripc,i);
        return;
    }
    if ((p=strchr(url,'/'))) strcpy(mntpnt,p+1);
    
    /* test mountpoint */
    if (!*mntpnt||!test_mntpnt(ntripc,mntpnt)) {
        tracet(2,"rsp_ntripc_c: no mountpoint %s\n",mntpnt);
        
        /* send source table */
        send_srctbl(ntripc,ntripc->tcp->cli[i].sock);
        discon_ntripc(ntripc,i);
        return;
    }
    /* test authentication */
    if (*ntripc->passwd) {
        sprintf(user,"%s:%s",ntripc->user,ntripc->passwd);
        q=user_pwd;
        q+=sprintf(q,"Authorization: Basic ");
        q+=encbase64(q,(unsigned char *)user,strlen(user));
        if (!(p=strstr((char *)con->buff,"Authorization:"))||
            strncmp(p,user_pwd,strlen(user_pwd))) {
            tracet(2,"rsp_ntripc_c: authroziation error\n");
            send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp1,
                    strlen(rsp1));
            discon_ntripc(ntripc,i);
            return;
        }
    }
    /* send OK response */
    send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp2,strlen(rsp2));
    
    con->state=1;
    strcpy(con->mntpnt,mntpnt);
}
/* test ntrip-caster server request ------------------------------------------*/
static void rsp_ntripc_s(ntripc_t *ntripc, int i)
{
    const char *rsp1=NTRIP_RSP_ERR_MNTP,*rsp2=NTRIP_RSP_ERR_PWD;
    const char *rsp3=NTRIP_RSP_OK_CLI;
    ntripc_con_t *con=ntripc->con+i;
    char passwd[256]="",mntpnt[256]="",str[NTRIP_MAXSTR]="",*p,*q;
    int j,n;
    
    tracet(3,"rspntripc_s i=%d\n",i);
    con->buff[con->nb]='\0';
    tracet(5,"rsp_ntripc_s: n=%d,buff=\n%s\n",con->nb,con->buff);
    
    if (con->nb>=NTRIP_MAXRSP-1) { /* buffer overflow */
        tracet(1,"rspntripc_s: request buffer overflow\n");
        discon_ntripc(ntripc,i);
        return;
    }
    /* test SOURCE and Source-Agent */
    if (!(p=strstr((char *)con->buff,"SOURCE"))||!(q=strstr(p,"\r\n"))||
        !(q=strstr(q,"Source-Agent:"))||!strstr(q,"\r\n\r\n")) {
        tracet(2,"rsp_ntripc_s: NTRIP request error\n");
        discon_ntripc(ntripc,i);
        return;
    }
    sscanf(p,"SOURCE %255s %255s",passwd,mntpnt);
    
    if ((p=strstr((char *)con->buff,"STR: "))&&(q=strstr(p,"\r\n"))) {
        n=MIN(q-(p+5),255);
        strncpy(str,p+5,n);
        str[n]='\0';
    }
    /* test mountpoint */
    if (!*mntpnt||!test_mntpnt(ntripc,mntpnt)) {
        tracet(2,"rsp_ntripc_s: no mountpoint\n");
        send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp1,strlen(rsp1));
        discon_ntripc(ntripc,i);
        return;
    }
    /* test password */
    if (*ntripc->passwd&&strcmp(passwd,ntripc->passwd)) {
        tracet(2,"rsp_ntripc_s: bad password %s\n",passwd);
        send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp2,strlen(rsp2));
        discon_ntripc(ntripc,i);
        return;
    }
    /* test mountpoint busy */
    for (j=0;j<MAXCLI;j++) {
        if (ntripc->con[j].state&&!strcmp(mntpnt,ntripc->con[j].mntpnt)) {
            tracet(2,"rsp_ntripc_s: bad password %s\n",passwd);
            send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp1,strlen(rsp1));
            discon_ntripc(ntripc,i);
            return;
        }
    }
    /* send OK response */
    send_nb(ntripc->tcp->cli[i].sock,(unsigned char *)rsp3,strlen(rsp3));
    
    con->state=1;
    strcpy(con->mntpnt,mntpnt);
    strcpy(con->str,str);
}
/* handle ntrip-caster connect request ---------------------------------------*/
static void wait_ntripc(ntripc_t *ntripc, char *msg)
{
    unsigned char *buff;
    int i,n,nmax,err;
    
    tracet(4,"wait_ntripc\n");
    
    ntripc->state=ntripc->tcp->svr.state;
    
    if (!waittcpsvr(ntripc->tcp,msg)) return;
    
    for (i=0;i<MAXCLI;i++) {
        if (ntripc->tcp->cli[i].state!=2||ntripc->con[i].state) continue;
        
        /* receive ntrip-caster request */
        buff=ntripc->con[i].buff+ntripc->con[i].nb;
        nmax=NTRIP_MAXRSP-ntripc->con[i].nb-1;
        
        if ((n=recv_nb(ntripc->tcp->cli[i].sock,buff,nmax))==-1) {
            if ((err=errsock())) {
                tracet(2,"accept_ntripc: recv error sock=%d err=%d\n",
                       ntripc->tcp->cli[i].sock,err);
            }
            discon_ntripc(ntripc,i);
            continue;
        }
        if (n<=0) continue;
        
        /* test ntrip-caster request */
        ntripc->con[i].nb+=n;
        if (ntripc->type) {
            rsp_ntripc_c(ntripc,i);
        }
        else {
            rsp_ntripc_s(ntripc,i);
        }
    }
}
/* read ntrip-caster ---------------------------------------------------------*/
static int readntripc(ntripc_t *ntripc, unsigned char *buff, int n, char *msg)
{
    int i,nr,err;
    
    tracet(4,"readntripc:\n");
    
    wait_ntripc(ntripc,msg);
    
    for (i=0;i<MAXCLI;i++) {
        if (!ntripc->con[i].state) continue;
        
        nr=recv_nb(ntripc->tcp->cli[i].sock,buff,n);
        
        if (nr<0) {
            if ((err=errsock())) {
                tracet(2,"readntripc: recv error i=%d sock=%d err=%d\n",i,
                       ntripc->tcp->cli[i].sock,err);
            }
            discon_ntripc(ntripc,i);
        }
        else if (nr>0) {
            ntripc->tcp->cli[i].tact=tickget();
            
            /* record received mountpoint */
            strcpy(ntripc->mntpnt,ntripc->con[i].mntpnt);
            return nr;
        }
    }
    return 0;
}
/* write ntrip-caster --------------------------------------------------------*/
static int writentripc(ntripc_t *ntripc, unsigned char *buff, int n, char *msg)
{
    int i,ns=0,err;
    
    tracet(4,"writentripc: n=%d\n",n);
    
    wait_ntripc(ntripc,msg);
    
    for (i=0;i<MAXCLI;i++) {
        if (!ntripc->con[i].state) continue;
        
        /* skip if not selected mountpoint */
        if (*ntripc->mntpnt&&strcmp(ntripc->mntpnt,ntripc->con[i].mntpnt)) {
            continue;
        }
        ns=send_nb(ntripc->tcp->cli[i].sock,buff,n);
        
        if (ns<n) {
            if ((err=errsock())) {
                tracet(2,"writentripc: send error i=%d sock=%d err=%d\n",i,
                       ntripc->tcp->cli[i].sock,err);
            }
            discon_ntripc(ntripc,i);
        }
        else {
            ntripc->tcp->cli[i].tact=tickget();
        }
    }
    return ns;
}
/* get state ntrip-caster ----------------------------------------------------*/
static int statentripc(ntripc_t *ntripc)
{
    return !ntripc?0:ntripc->state;
}
/* get extended state ntrip-caster -------------------------------------------*/
static int statexntripc(ntripc_t *ntripc, char *msg)
{
    char *p=msg;
    int i,state=!ntripc?0:ntripc->state;
    
    p+=sprintf(p,"ntripc:\n");
    p+=sprintf(p,"  state   = %d\n",ntripc->state);
    if (!state) return 0;
    p+=sprintf(p,"  type    = %d\n",ntripc->type);
    p+=sprintf(p,"  mntpnt  = %s\n",ntripc->mntpnt);
    p+=sprintf(p,"  user    = %s\n",ntripc->user);
    p+=sprintf(p,"  passwd  = %s\n",ntripc->passwd);
    p+=sprintf(p,"  svr:\n");
    p+=statextcp(&ntripc->tcp->svr,p);
    for (i=0;i<MAXCLI;i++) {
        if (!ntripc->tcp->cli[i].state) continue;
        p+=sprintf(p,"  cli#%d:\n",i);
        p+=statextcp(ntripc->tcp->cli+i,p);
        p+=sprintf(p,"    mntpnt= %s\n",ntripc->con[i].mntpnt);
        p+=sprintf(p,"    str   = %s\n",ntripc->con[i].str);
        p+=sprintf(p,"    nb    = %d\n",ntripc->con[i].nb);
    }
    return state;
}
/* generate udp socket -------------------------------------------------------*/
static udp_t *genudp(int type, int port, const char *saddr, char *msg)
{
    udp_t *udp;
    struct hostent *hp;
    int bs=buffsize,opt=1;
    
    tracet(3,"genudp: type=%d\n",type);
    
    if (!(udp=(udp_t *)malloc(sizeof(udp_t)))) return NULL;
    udp->state=2;
    udp->type=type;
    udp->port=port;
    strcpy(udp->saddr,saddr);
    
    if ((udp->sock=socket(AF_INET,SOCK_DGRAM,0))==(socket_t)-1) {
        sprintf(msg,"socket error (%d)",errsock());
        return NULL;
    }
    if (setsockopt(udp->sock,SOL_SOCKET,SO_RCVBUF,(const char *)&bs,sizeof(bs))==-1||
        setsockopt(udp->sock,SOL_SOCKET,SO_SNDBUF,(const char *)&bs,sizeof(bs))==-1) {
        tracet(2,"genudp: setsockopt error sock=%d err=%d bs=%d\n",udp->sock,errsock(),bs);
        sprintf(msg,"sockopt error: bufsiz");
    }
    memset(&udp->addr,0,sizeof(udp->addr));
    udp->addr.sin_family=AF_INET;
    udp->addr.sin_port=htons(port);
    
    if (!udp->type) { /* udp server */
        udp->addr.sin_addr.s_addr=htonl(INADDR_ANY);
#ifdef SVR_REUSEADDR
        setsockopt(udp->sock,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt, sizeof(opt));
#endif
        if (bind(udp->sock,(struct sockaddr *)&udp->addr,sizeof(udp->addr))==-1) {
            tracet(2,"genudp: bind error sock=%d port=%d err=%d\n",udp->sock,port,errsock());
            sprintf(msg,"bind error (%d): %d",errsock(),port);
            closesocket(udp->sock);
            free(udp);
            return NULL;
        }
    }
    else { /* udp client */
        if (!strcmp(saddr,"255.255.255.255")&&
            setsockopt(udp->sock,SOL_SOCKET,SO_BROADCAST,(const char *)&opt,
                       sizeof(opt))==-1) {
            tracet(2,"genudp: setsockopt error sock=%d err=%d\n",udp->sock,errsock());
            sprintf(msg,"sockopt error: broadcast");
        }
        if (!(hp=gethostbyname(saddr))) {
            sprintf(msg,"address error (%s)",saddr);
            closesocket(udp->sock);
            free(udp);
            return NULL;
        }
        memcpy(&udp->addr.sin_addr,hp->h_addr,hp->h_length);
    }
    return udp;
}
/* open udp server -----------------------------------------------------------*/
static udp_t *openudpsvr(const char *path, char *msg)
{
    char sport[256]="";
    int port;
    
    tracet(3,"openudpsvr: path=%s\n",path);
    
    decodetcppath(path,NULL,sport,NULL,NULL,NULL,NULL);
    
    if (sscanf(sport,"%d",&port)<1) {
        sprintf(msg,"port error: %s",sport);
        tracet(2,"openudpsvr: port error port=%s\n",port);
        return NULL;
    }
    return genudp(0,port,"",msg);
}
/* close udp server ----------------------------------------------------------*/
static void closeudpsvr(udp_t *udpsvr)
{
    tracet(3,"closeudpsvr: sock=%d\n",udpsvr->sock);
    
    closesocket(udpsvr->sock);
    free(udpsvr);
}
/* read udp server -----------------------------------------------------------*/
static int readudpsvr(udp_t *udpsvr, unsigned char *buff, int n, char *msg)
{
    struct timeval tv={0};
    fd_set rs;
    int ret,nr;
    
    tracet(4,"readudpsvr: sock=%d n=%d\n",udpsvr->sock,n);
    
    FD_ZERO(&rs); FD_SET(udpsvr->sock,&rs);
    ret=select(udpsvr->sock+1,&rs,NULL,NULL,&tv);
    if (ret<=0) return ret;
    nr=recvfrom(udpsvr->sock,(char *)buff,n,0,NULL,NULL);
    return nr<=0?-1:nr;
}
/* get state udp server ------------------------------------------------------*/
static int stateudpsvr(udp_t *udpsvr)
{
    return udpsvr?udpsvr->state:0;
}
/* get extended state udp server ---------------------------------------------*/
static int statexudpsvr(udp_t *udpsvr, char *msg)
{
    char *p=msg;
    int state=udpsvr?udpsvr->state:0;
    
    p+=sprintf(p,"udpsvr:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  type    = %d\n",udpsvr->type);
    p+=sprintf(p,"  sock    = %d\n",(int)udpsvr->sock);
    p+=sprintf(p,"  port    = %d\n",udpsvr->port);
    return state;
}
/* open udp client -----------------------------------------------------------*/
static udp_t *openudpcli(const char *path, char *msg)
{
    char sport[256]="",saddr[256]="";
    int port;
    
    tracet(3,"openudpsvr: path=%s\n",path);
    
    decodetcppath(path,saddr,sport,NULL,NULL,NULL,NULL);
    
    if (sscanf(sport,"%d",&port)<1) {
        sprintf(msg,"port error: %s",sport);
        tracet(2,"openudpcli: port error port=%s\n",sport);
        return NULL;
    }
    return genudp(1,port,saddr,msg);
}
/* close udp client ----------------------------------------------------------*/
static void closeudpcli(udp_t *udpcli)
{
    tracet(3,"closeudpcli: sock=%d\n",udpcli->sock);
    
    closesocket(udpcli->sock);
    free(udpcli);
}
/* write udp client -----------------------------------------------------------*/
static int writeudpcli(udp_t *udpcli, unsigned char *buff, int n, char *msg)
{
    tracet(4,"writeudpcli: sock=%d n=%d\n",udpcli->sock,n);
    
    return (int)sendto(udpcli->sock,(char *)buff,n,0,
                       (struct sockaddr *)&udpcli->addr,sizeof(udpcli->addr));
}
/* get state udp client ------------------------------------------------------*/
static int stateudpcli(udp_t *udpcli)
{
    return udpcli?udpcli->state:0;
}
/* get extended state udp client ---------------------------------------------*/
static int statexudpcli(udp_t *udpcli, char *msg)
{
    char *p=msg;
    int state=udpcli?udpcli->state:0;
    
    p+=sprintf(p,"udpsvr:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  type    = %d\n",udpcli->type);
    p+=sprintf(p,"  sock    = %d\n",(int)udpcli->sock);
    p+=sprintf(p,"  addr    = %s\n",udpcli->saddr);
    p+=sprintf(p,"  port    = %d\n",udpcli->port);
    return state;
}
/* decode ftp path -----------------------------------------------------------*/
static void decodeftppath(const char *path, char *addr, char *file, char *user,
                          char *passwd, int *topts)
{
    char buff[MAXSTRPATH],*p,*q;
    
    tracet(4,"decodeftpath: path=%s\n",path);
    
    if (user) *user='\0';
    if (passwd) *passwd='\0';
    if (topts) {
        topts[0]=0;    /* time offset in path (s) */
        topts[1]=3600; /* download interval (s) */
        topts[2]=0;    /* download time offset (s) */
        topts[3]=0;    /* retry interval (s) (0: no retry) */
    }
    strcpy(buff,path);
    
    if ((p=strchr(buff,'/'))) {
        if ((q=strstr(p+1,"::"))) {
            *q='\0';
            if (topts) sscanf(q+2,"T=%d,%d,%d,%d",topts,topts+1,topts+2,topts+3);
        }
        strcpy(file,p+1);
        *p='\0';
    }
    else file[0]='\0';
    
    if ((p=strrchr(buff,'@'))) {
        *p++='\0';
        if ((q=strchr(buff,':'))) {
             *q='\0'; if (passwd) strcpy(passwd,q+1);
        }
        *q='\0'; if (user) strcpy(user,buff); 
    }
    else p=buff;
    
    strcpy(addr,p);
}
/* next download time --------------------------------------------------------*/
static gtime_t nextdltime(const int *topts, int stat)
{
    gtime_t time;
    double tow;
    int week,tint;
    
    tracet(3,"nextdltime: topts=%d %d %d %d stat=%d\n",topts[0],topts[1],
           topts[2],topts[3],stat);
    
    /* current time (gpst) */
    time=utc2gpst(timeget());
    tow=time2gpst(time,&week);
    
    /* next retry time */
    if (stat==0&&topts[3]>0) {
        tow=(floor((tow-topts[2])/topts[3])+1.0)*topts[3]+topts[2];
        return gpst2time(week,tow);
    }
    /* next interval time */
    tint=topts[1]<=0?3600:topts[1];
    tow=(floor((tow-topts[2])/tint)+1.0)*tint+topts[2];
    time=gpst2time(week,tow);
    
    return time;
}
/* ftp thread ----------------------------------------------------------------*/
#ifdef WIN32
static DWORD WINAPI ftpthread(void *arg)
#else
static void *ftpthread(void *arg)
#endif
{
    ftp_t *ftp=(ftp_t *)arg;
    FILE *fp;
    gtime_t time;
    char remote[1024],local[1024],tmpfile[1024],errfile[1024],*p;
    char cmd[2048],env[1024]="",opt[1024],*proxyopt="",*proto;
    int ret;
    
    tracet(3,"ftpthread:\n");
    
    if (!*localdir) {
        tracet(2,"no local directory\n");
        ftp->error=11;
        ftp->state=3;
        return 0;
    }
    /* replace keyword in file path and local path */
    time=timeadd(utc2gpst(timeget()),ftp->topts[0]);
    reppath(ftp->file,remote,time,"","");
    
    if ((p=strrchr(remote,'/'))) p++; else p=remote;
    sprintf(local,"%s%c%s",localdir,FILEPATHSEP,p);
    sprintf(errfile,"%s.err",local);
    
    /* if local file exist, skip download */
    strcpy(tmpfile,local);
    if ((p=strrchr(tmpfile,'.'))&&
        (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
         !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP"))) {
        *p='\0';
    }
    if ((fp=fopen(tmpfile,"rb"))) {
        fclose(fp);
        strcpy(ftp->local,tmpfile);
        tracet(3,"ftpthread: file exists %s\n",ftp->local);
        ftp->state=2;
        return 0;
    }
    /* proxy settings for wget (ref [2]) */
    if (*proxyaddr) {
        proto=ftp->proto?"http":"ftp";
        sprintf(env,"set %s_proxy=http://%s & ",proto,proxyaddr);
        proxyopt="--proxy=on ";
    }
    /* download command (ref [2]) */
    if (ftp->proto==0) { /* ftp */
        sprintf(opt,"--ftp-user=%s --ftp-password=%s --glob=off --passive-ftp %s-t 1 -T %d -O \"%s\"",
                ftp->user,ftp->passwd,proxyopt,FTP_TIMEOUT,local);
        sprintf(cmd,"%s%s %s \"ftp://%s/%s\" 2> \"%s\"\n",env,FTP_CMD,opt,ftp->addr,
                remote,errfile);
    }
    else { /* http */
        sprintf(opt,"%s-t 1 -T %d -O \"%s\"",proxyopt,FTP_TIMEOUT,local);
        sprintf(cmd,"%s%s %s \"http://%s/%s\" 2> \"%s\"\n",env,FTP_CMD,opt,ftp->addr,
                remote,errfile);
    }
    /* execute download command */
    if ((ret=execcmd(cmd))) {
        remove(local);
        tracet(2,"execcmd error: cmd=%s ret=%d\n",cmd,ret);
        ftp->error=ret;
        ftp->state=3;
        return 0;
    }
    remove(errfile);
    
    /* uncompress downloaded file */
    if ((p=strrchr(local,'.'))&&
        (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
         !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP"))) {
        
        if (rtk_uncompress(local,tmpfile)) {
            remove(local);
            strcpy(local,tmpfile);
        }
        else {
            tracet(2,"file uncompact error: %s\n",local);
            ftp->error=12;
            ftp->state=3;
            return 0;
        }
    }
    strcpy(ftp->local,local);
    ftp->state=2; /* ftp completed */
    
    tracet(3,"ftpthread: complete cmd=%s\n",cmd);
    return 0;
}
/* open ftp ------------------------------------------------------------------*/
static ftp_t *openftp(const char *path, int type, char *msg)
{
    ftp_t *ftp;
    
    tracet(3,"openftp: path=%s type=%d\n",path,type);
    
    msg[0]='\0';
    
    if (!(ftp=(ftp_t *)malloc(sizeof(ftp_t)))) return NULL;
    
    ftp->state=0;
    ftp->proto=type;
    ftp->error=0;
    ftp->thread=0;
    ftp->local[0]='\0';
    
    /* decode ftp path */
    decodeftppath(path,ftp->addr,ftp->file,ftp->user,ftp->passwd,ftp->topts);
    
    /* set first download time */
    ftp->tnext=timeadd(timeget(),10.0);
    
    return ftp;
}
/* close ftp -----------------------------------------------------------------*/
static void closeftp(ftp_t *ftp)
{
    tracet(3,"closeftp: state=%d\n",ftp->state);
    
    if (ftp->state!=1) free(ftp);
}
/* read ftp ------------------------------------------------------------------*/
static int readftp(ftp_t *ftp, unsigned char *buff, int n, char *msg)
{
    gtime_t time;
    unsigned char *p,*q;
    
    tracet(4,"readftp: n=%d\n",n);
    
    time=utc2gpst(timeget());
    
    if (timediff(time,ftp->tnext)<0.0) { /* until download time? */
        return 0;
    }
    if (ftp->state<=0) { /* ftp/http not executed? */
        ftp->state=1;
        sprintf(msg,"%s://%s",ftp->proto?"http":"ftp",ftp->addr);
    
#ifdef WIN32
        if (!(ftp->thread=CreateThread(NULL,0,ftpthread,ftp,0,NULL))) {
#else
        if (pthread_create(&ftp->thread,NULL,ftpthread,ftp)) {
#endif
            tracet(2,"readftp: ftp thread create error\n");
            ftp->state=3;
            strcpy(msg,"ftp thread error");
            return 0;
        }
    }
    if (ftp->state<=1) return 0; /* ftp/http on going? */
    
    if (ftp->state==3) { /* ftp error */
        sprintf(msg,"%s error (%d)",ftp->proto?"http":"ftp",ftp->error);
        
        /* set next retry time */
        ftp->tnext=nextdltime(ftp->topts,0);
        ftp->state=0;
        return 0;
    }
    /* return local file path if ftp completed */
    p=buff;
    q=(unsigned char *)ftp->local;
    while (*q&&(int)(p-buff)<n) *p++=*q++;
    p+=sprintf((char *)p,"\r\n");
    
    /* set next download time */
    ftp->tnext=nextdltime(ftp->topts,1);
    ftp->state=0;
    
    strcpy(msg,"");
    
    return (int)(p-buff);
}
/* get state ftp -------------------------------------------------------------*/
static int stateftp(ftp_t *ftp)
{
    return !ftp?0:(ftp->state==0?2:(ftp->state<=2?3:-1));
}
/* get extended state ftp ----------------------------------------------------*/
static int statexftp(ftp_t *ftp, char *msg)
{
    return !ftp?0:(ftp->state==0?2:(ftp->state<=2?3:-1));
}
/* open memory buffer --------------------------------------------------------*/
static membuf_t *openmembuf(const char *path, char *msg)
{
    membuf_t *membuf;
    int bufsize=DEFAULT_MEMBUF_SIZE;
    
    tracet(3,"openmembuf: path=%s\n",path);
    
    msg[0]='\0';
    
    sscanf(path,"%d",&bufsize);
    
    if (!(membuf=(membuf_t *)malloc(sizeof(membuf_t)))) return NULL;
    membuf->state=1;
    membuf->rp=0;
    membuf->wp=0;
    if (!(membuf->buf=(unsigned char *)malloc(bufsize))) {
        free(membuf);
        return NULL;
    }
    membuf->bufsize=bufsize;
    initlock(&membuf->lock);
    
    sprintf(msg,"membuf sizebuf=%d",bufsize);
    
    return membuf;
}
/* close memory buffer -------------------------------------------------------*/
static void closemembuf(membuf_t *membuf)
{
    tracet(3,"closemembufp\n");
    
    free(membuf->buf);
    free(membuf);
}
/* read memory buffer --------------------------------------------------------*/
static int readmembuf(membuf_t *membuf, unsigned char *buff, int n, char *msg)
{
    int i,nr=0;
    
    tracet(4,"readmembuf: n=%d\n",n);
    
    if (!membuf) return 0;
    
    lock(&membuf->lock);
    
    for (i=membuf->rp;i!=membuf->wp&&nr<n;i++) {
        if (i>=membuf->bufsize) i=0;
        buff[nr++]=membuf->buf[i];
    }
    membuf->rp=i;
    unlock(&membuf->lock);
    return nr;
}
/* write memory buffer -------------------------------------------------------*/
static int writemembuf(membuf_t *membuf, unsigned char *buff, int n, char *msg)
{
    int i;
    
    tracet(3,"writemembuf: n=%d\n",n);
    
    if (!membuf) return 0;
    
    lock(&membuf->lock);
    
    for (i=0;i<n;i++) {
        membuf->buf[membuf->wp++]=buff[i];
        if (membuf->wp>=membuf->bufsize) membuf->wp=0;
        if (membuf->wp==membuf->rp) {
           strcpy(msg,"mem-buffer overflow");
           membuf->state=-1;
           unlock(&membuf->lock);
           return i+1;
        }
    }
    unlock(&membuf->lock);
    return i;
}
/* get state memory buffer ---------------------------------------------------*/
static int statemembuf(membuf_t *membuf)
{
    return !membuf?0:membuf->state;
}
/* get extended state memory buffer ------------------------------------------*/
static int statexmembuf(membuf_t *membuf, char *msg)
{
    char *p=msg;
    int state=!membuf?0:membuf->state;
    
    p+=sprintf(p,"membuf:\n");
    p+=sprintf(p,"  state   = %d\n",state);
    if (!state) return 0;
    p+=sprintf(p,"  buffsize= %d\n",membuf->bufsize);
    p+=sprintf(p,"  wp      = %d\n",membuf->wp);
    p+=sprintf(p,"  rp      = %d\n",membuf->rp);
    return state;
}
/* initialize stream environment -----------------------------------------------
* initialize stream environment
* args   : none
* return : none
*-----------------------------------------------------------------------------*/
extern void strinitcom(void)
{
#ifdef WIN32
    WSADATA data;
#endif
    tracet(3,"strinitcom:\n");

#ifdef WIN32
    WSAStartup(MAKEWORD(2,0),&data);
#endif
}
/* initialize stream -----------------------------------------------------------
* initialize stream struct
* args   : stream_t *stream IO  stream
* return : none
*-----------------------------------------------------------------------------*/
extern void strinit(stream_t *stream)
{
    tracet(3,"strinit:\n");
    
    stream->type=0;
    stream->mode=0;
    stream->state=0;
    stream->inb=stream->inr=stream->outb=stream->outr=0;
    stream->tick_i=stream->tick_o=stream->tact=stream->inbt=stream->outbt=0;
    initlock(&stream->lock);
    stream->port=NULL;
    stream->path[0]='\0';
    stream->msg [0]='\0';
}
/* open stream -----------------------------------------------------------------
*
* open stream to read or write data from or to virtual devices.
*
* args   : stream_t *stream IO  stream
*          int type         I   stream type
*                                 STR_SERIAL   = serial device
*                                 STR_FILE     = file (record and playback)
*                                 STR_TCPSVR   = TCP server
*                                 STR_TCPCLI   = TCP client
*                                 STR_NTRIPSVR = NTRIP server
*                                 STR_NTRIPCLI = NTRIP client
*                                 STR_NTRIPC_S = NTRIP caster server
*                                 STR_NTRIPC_C = NTRIP caster client
*                                 STR_UDPSVR   = UDP server (read only)
*                                 STR_UDPCLI   = UDP client (write only)
*                                 STR_MEMBUF   = memory buffer (FIFO)
*                                 STR_FTP      = download by FTP (raed only)
*                                 STR_HTTP     = download by HTTP (raed only)
*          int mode         I   stream mode (STR_MODE_???)
*                                 STR_MODE_R   = read only
*                                 STR_MODE_W   = write only
*                                 STR_MODE_RW  = read and write
*          char *path       I   stream path (see below)
*
* return : status (0:error,1:ok)
*
* notes  : see reference [1] for NTRIP
*          STR_FTP/HTTP needs "wget" in command search paths
*
* stream path ([] options):
*
*   STR_SERIAL   port[:brate[:bsize[:parity[:stopb[:fctr[#port]]]]]]
*                    port  = COM??  (windows)
*                            tty??? (linuex, omit /dev/)
*                    brate = bit rate     (bps)
*                    bsize = bit size     (7|8)
*                    parity= parity       (n|o|e)
*                    stopb = stop bits    (1|2)
*                    fctr  = flow control (off|rts)
*                    port  = tcp server port to output received stream
*
*   STR_FILE     path[::T][::+start][::xseppd][::S=swap][::P={4|8}]
*                    path  = file path
*                            (can include keywords defined by )
*                    ::T   = enable time tag
*                    start = replay start offset (s)
*                    speed = replay speed factor
*                    swap  = output swap interval (hr) (0: no swap)
*                    ::P={4|8} = file pointer size (4:32bit,8:64bit)
*
*   STR_TCPSVR   :port
*                    port  = TCP server port to accept
*
*   STR_TCPCLI   addr:port
*                    addr  = TCP server address to connect
*                    port  = TCP server port to connect
*
*   STR_NTRIPSVR [:passwd@]addr[:port]/mponit[:string]
*                    addr  = NTRIP caster address to connect
*                    port  = NTRIP caster server port to connect
*                    passwd= NTRIP caster server password to connect
*                    mpoint= NTRIP mountpoint
*                    string= NTRIP server string
*
*   STR_NTRIPCLI [user[:passwd]@]addr[:port]/mpoint
*                    addr  = NTRIP caster address to connect
*                    port  = NTRIP caster client port to connect
*                    user  = NTRIP caster client user to connect
*                    passwd= NTRIP caster client password to connect
*                    mpoint= NTRIP mountpoint
*
*   STR_NTRIPC_S [:passwd@][:port]/mpoint
*                    port  = NTRIP caster server port to accept
*                    passwd= NTRIP caster server password to accept
*                    mpoint= NTRIP mountpoint
*
*   STR_NTRIPC_C [user[:passwd]@][:port]/mpoint
*                    port  = NTRIP caster client port to accept
*                    user  = NTRIP caster client user to accept
*                    passwd= NTRIP caster client password to accept
*                    mpoint= NTRIP mountpoint
*
*   STR_UDPSVR   :port
*                    port  = UDP server port to receive
*
*   STR_UDPCLI   addr:port
*                    addr  = UDP server or broadcast address to send
*                    port  = UDP server or broadcast port to send
*
*   STR_MEMBUF   [size]
*                    size  = FIFO size (bytes) ("":4096)
*
*   STR_FTP      [user[:passwd]@]addr/path[::T=poff[,tint[,toff,tret]]]]
*                    user  = FTP server user
*                    passwd= FTP server password
*                    addr  = FTP server address
*                    path  = FTP server file path
*                    poff  = time offset for path extension (s)
*                    tint  = download interval (s)
*                    toff  = download time offset (s)
*                    tret  = download retry interval (s) (0:no retry)
*
*   STR_HTTP     addr/path[::T=poff[,tint[,toff,tret]]]]
*                    addr  = HTTP server address
*                    path  = HTTP server file path
*                    poff  = time offset for path extension (s)
*                    tint  = download interval (s)
*                    toff  = download time offset (s)
*                    tret  = download retry interval (s) (0:no retry)
*
*-----------------------------------------------------------------------------*/
extern int stropen(stream_t *stream, int type, int mode, const char *path)
{
    tracet(3,"stropen: type=%d mode=%d path=%s\n",type,mode,path);
    
    stream->type=type;
    stream->mode=mode;
    strcpy(stream->path,path);
    stream->inb=stream->inr=stream->outb=stream->outr=0;
    stream->tick_i=stream->tick_o=tickget();
    stream->inbt=stream->outbt=0;
    stream->msg[0]='\0';
    stream->port=NULL;
    switch (type) {
        case STR_SERIAL  : stream->port=openserial(path,mode,stream->msg); break;
        case STR_FILE    : stream->port=openfile  (path,mode,stream->msg); break;
        case STR_TCPSVR  : stream->port=opentcpsvr(path,     stream->msg); break;
        case STR_TCPCLI  : stream->port=opentcpcli(path,     stream->msg); break;
        case STR_NTRIPSVR: stream->port=openntrip (path,0,   stream->msg); break;
        case STR_NTRIPCLI: stream->port=openntrip (path,1,   stream->msg); break;
        case STR_NTRIPC_S: stream->port=openntripc(path,0,   stream->msg); break;
        case STR_NTRIPC_C: stream->port=openntripc(path,1,   stream->msg); break;
        case STR_UDPSVR  : stream->port=openudpsvr(path,     stream->msg); break;
        case STR_UDPCLI  : stream->port=openudpcli(path,     stream->msg); break;
        case STR_MEMBUF  : stream->port=openmembuf(path,     stream->msg); break;
        case STR_FTP     : stream->port=openftp   (path,0,   stream->msg); break;
        case STR_HTTP    : stream->port=openftp   (path,1,   stream->msg); break;
        default: stream->state=0; return 1;
    }
    stream->state=!stream->port?-1:1;
    return stream->port!=NULL;
}
/* close stream ----------------------------------------------------------------
* close stream
* args   : stream_t *stream IO  stream
* return : none
*-----------------------------------------------------------------------------*/
extern void strclose(stream_t *stream)
{
    tracet(3,"strclose: type=%d mode=%d\n",stream->type,stream->mode);
    
    strlock(stream);
    
    if (stream->port) {
        switch (stream->type) {
            case STR_SERIAL  : closeserial((serial_t *)stream->port); break;
            case STR_FILE    : closefile  ((file_t   *)stream->port); break;
            case STR_TCPSVR  : closetcpsvr((tcpsvr_t *)stream->port); break;
            case STR_TCPCLI  : closetcpcli((tcpcli_t *)stream->port); break;
            case STR_NTRIPSVR: closentrip ((ntrip_t  *)stream->port); break;
            case STR_NTRIPCLI: closentrip ((ntrip_t  *)stream->port); break;
            case STR_NTRIPC_S: closentripc((ntripc_t *)stream->port); break;
            case STR_NTRIPC_C: closentripc((ntripc_t *)stream->port); break;
            case STR_UDPSVR  : closeudpsvr((udp_t    *)stream->port); break;
            case STR_UDPCLI  : closeudpcli((udp_t    *)stream->port); break;
            case STR_MEMBUF  : closemembuf((membuf_t *)stream->port); break;
            case STR_FTP     : closeftp   ((ftp_t    *)stream->port); break;
            case STR_HTTP    : closeftp   ((ftp_t    *)stream->port); break;
        }
    }
    else {
        trace(2,"no port to close stream: type=%d\n",stream->type);
    }
    stream->type=0;
    stream->mode=0;
    stream->state=0;
    stream->inr=stream->outr=0;
    stream->path[0]='\0';
    stream->msg[0]='\0';
    stream->port=NULL;
    
    strunlock(stream);
}
/* sync streams ----------------------------------------------------------------
* sync time for streams
* args   : stream_t *stream1 IO stream 1
*          stream_t *stream2 IO stream 2
* return : none
* notes  : for replay files with time tags
*-----------------------------------------------------------------------------*/
extern void strsync(stream_t *stream1, stream_t *stream2)
{
    file_t *file1,*file2;
    if (stream1->type!=STR_FILE||stream2->type!=STR_FILE) return;
    file1=(file_t*)stream1->port;
    file2=(file_t*)stream2->port;
    if (file1&&file2) syncfile(file1,file2);
}
/* lock/unlock stream ----------------------------------------------------------
* lock/unlock stream
* args   : stream_t *stream I  stream
* return : none
*-----------------------------------------------------------------------------*/
extern void strlock  (stream_t *stream) {lock  (&stream->lock);}
extern void strunlock(stream_t *stream) {unlock(&stream->lock);}

/* read stream -----------------------------------------------------------------
* read data from stream (unblocked)
* args   : stream_t *stream I  stream
*          unsinged char *buff O data buffer
*          int    n         I  maximum data length
* return : read data length
* notes  : if no data, return immediately with no data
*-----------------------------------------------------------------------------*/
extern int strread(stream_t *stream, unsigned char *buff, int n)
{
    unsigned int tick;
    char *msg=stream->msg;
    int nr;
    
    tracet(4,"strread: n=%d\n",n);
    
    if (!(stream->mode&STR_MODE_R)||!stream->port) return 0;
    
    strlock(stream);
    
    switch (stream->type) {
        case STR_SERIAL  : nr=readserial((serial_t *)stream->port,buff,n,msg); break;
        case STR_FILE    : nr=readfile  ((file_t   *)stream->port,buff,n,msg); break;
        case STR_TCPSVR  : nr=readtcpsvr((tcpsvr_t *)stream->port,buff,n,msg); break;
        case STR_TCPCLI  : nr=readtcpcli((tcpcli_t *)stream->port,buff,n,msg); break;
        case STR_NTRIPSVR:
        case STR_NTRIPCLI: nr=readntrip ((ntrip_t  *)stream->port,buff,n,msg); break;
        case STR_NTRIPC_S:
        case STR_NTRIPC_C: nr=readntripc((ntripc_t *)stream->port,buff,n,msg); break;
        case STR_UDPSVR  : nr=readudpsvr((udp_t    *)stream->port,buff,n,msg); break;
        case STR_MEMBUF  : nr=readmembuf((membuf_t *)stream->port,buff,n,msg); break;
        case STR_FTP     : nr=readftp   ((ftp_t    *)stream->port,buff,n,msg); break;
        case STR_HTTP    : nr=readftp   ((ftp_t    *)stream->port,buff,n,msg); break;
        default:
            strunlock(stream);
            return 0;
    }
    stream->inb+=nr;
    tick=tickget(); if (nr>0) stream->tact=tick;
    
    if ((int)(tick-stream->tick_i)>=tirate) {
        stream->inr=(stream->inb-stream->inbt)*8000/(tick-stream->tick_i);
        stream->tick_i=tick; stream->inbt=stream->inb;
    }
    strunlock(stream);
    return nr;
}
/* write stream ----------------------------------------------------------------
* write data to stream (unblocked)
* args   : stream_t *stream I   stream
*          unsinged char *buff I data buffer
*          int    n         I   data length
* return : status (0:error,1:ok)
* notes  : write data to buffer and return immediately
*-----------------------------------------------------------------------------*/
extern int strwrite(stream_t *stream, unsigned char *buff, int n)
{
    unsigned int tick;
    char *msg=stream->msg;
    int ns;
    
    tracet(4,"strwrite: n=%d\n",n);
    
    if (!(stream->mode&STR_MODE_W)||!stream->port) return 0;
    
    strlock(stream);
    
    switch (stream->type) {
        case STR_SERIAL  : ns=writeserial((serial_t *)stream->port,buff,n,msg); break;
        case STR_FILE    : ns=writefile  ((file_t   *)stream->port,buff,n,msg); break;
        case STR_TCPSVR  : ns=writetcpsvr((tcpsvr_t *)stream->port,buff,n,msg); break;
        case STR_TCPCLI  : ns=writetcpcli((tcpcli_t *)stream->port,buff,n,msg); break;
        case STR_NTRIPSVR:
        case STR_NTRIPCLI: ns=writentrip ((ntrip_t  *)stream->port,buff,n,msg); break;
        case STR_NTRIPC_S:
        case STR_NTRIPC_C: ns=writentripc((ntripc_t *)stream->port,buff,n,msg); break;
        case STR_UDPCLI  : ns=writeudpcli((udp_t    *)stream->port,buff,n,msg); break;
        case STR_MEMBUF  : ns=writemembuf((membuf_t *)stream->port,buff,n,msg); break;
        case STR_FTP     :
        case STR_HTTP    :
        default:
            strunlock(stream);
            return 0;
    }
    stream->outb+=ns;
    tick=tickget(); if (ns>0) stream->tact=tick;
    
    if ((int)(tick-stream->tick_o)>tirate) {
        stream->outr=(int)((double)(stream->outb-stream->outbt)*8000/(tick-stream->tick_o));
        stream->tick_o=tick; stream->outbt=stream->outb;
    }
    strunlock(stream);
    return ns;
}
/* set stream selection --------------------------------------------------------
* set stream selection for ntrip-caster
* args   : stream_t *stream I   stream
*          char   *sel      I   selected mountpoint of ntrip-caster ("":all)
* return : status (0:error,1:ok)
*-----------------------------------------------------------------------------*/
extern int strsetsel(stream_t *stream, const char *sel)
{
    if (!stream->port||
        (stream->type!=STR_NTRIPC_S&&stream->type!=STR_NTRIPC_C)) {
        return 0;
    }
    strlock(stream);
    
    strcpy(((ntripc_t *)stream->port)->mntpnt,sel);
    
    strunlock(stream);
    return 1;
}
/* get stream selection --------------------------------------------------------
* get stream selection for ntrip-caster
* args   : stream_t *stream I   stream
*          char   *sel      O   selected mountpoint of ntrip-caster
* return : status (0:error,1:ok)
*-----------------------------------------------------------------------------*/
extern int strgetsel(stream_t *stream, char *sel)
{
    *sel='\0';
    
    if (!stream->port||
        (stream->type!=STR_NTRIPC_S&&stream->type!=STR_NTRIPC_C)) {
        return 0;
    }
    strlock(stream);
    
    strcpy(sel,((ntripc_t *)stream->port)->mntpnt);
    
    strunlock(stream);
    return 1;
}
/* get stream status -----------------------------------------------------------
* get stream status
* args   : stream_t *stream I   stream
*          char   *msg      IO  status message (NULL: no output)
* return : status (-1:error,0:close,1:wait,2:connect,3:active)
*-----------------------------------------------------------------------------*/
extern int strstat(stream_t *stream, char *msg)
{
    int state;
    
    tracet(4,"strstat:\n");
    
    strlock(stream);
    if (msg) {
        strncpy(msg,stream->msg,MAXSTRMSG-1); msg[MAXSTRMSG-1]='\0';
    }
    if (!stream->port) {
        strunlock(stream);
        return stream->state;
    }
    switch (stream->type) {
        case STR_SERIAL  : state=stateserial((serial_t *)stream->port); break;
        case STR_FILE    : state=statefile  ((file_t   *)stream->port); break;
        case STR_TCPSVR  : state=statetcpsvr((tcpsvr_t *)stream->port); break;
        case STR_TCPCLI  : state=statetcpcli((tcpcli_t *)stream->port); break;
        case STR_NTRIPSVR:
        case STR_NTRIPCLI: state=statentrip ((ntrip_t  *)stream->port); break;
        case STR_NTRIPC_C:
        case STR_NTRIPC_S: state=statentripc((ntripc_t *)stream->port); break;
        case STR_UDPSVR  : state=stateudpsvr((udp_t    *)stream->port); break;
        case STR_UDPCLI  : state=stateudpcli((udp_t    *)stream->port); break;
        case STR_MEMBUF  : state=statemembuf((membuf_t *)stream->port); break;
        case STR_FTP     : state=stateftp   ((ftp_t    *)stream->port); break;
        case STR_HTTP    : state=stateftp   ((ftp_t    *)stream->port); break;
        default:
            strunlock(stream);
            return 0;
    }
    if (state==2&&(int)(tickget()-stream->tact)<=TINTACT) state=3;
    strunlock(stream);
    return state;
}
/* get extended stream status ---------------------------------------------------
* get extended stream status
* args   : stream_t *stream I   stream
*          char   *msg      IO  extended status message
* return : status (-1:error,0:close,1:wait,2:connect,3:active)
*-----------------------------------------------------------------------------*/
extern int strstatx(stream_t *stream, char *msg)
{
    int state;
    
    tracet(4,"strstatx:\n");
    
    strlock(stream);
    
    if (!stream->port) {
        strunlock(stream);
        return stream->state;
    }
    switch (stream->type) {
        case STR_SERIAL  : state=statexserial((serial_t *)stream->port,msg); break;
        case STR_FILE    : state=statexfile  ((file_t   *)stream->port,msg); break;
        case STR_TCPSVR  : state=statextcpsvr((tcpsvr_t *)stream->port,msg); break;
        case STR_TCPCLI  : state=statextcpcli((tcpcli_t *)stream->port,msg); break;
        case STR_NTRIPSVR:
        case STR_NTRIPCLI: state=statexntrip ((ntrip_t  *)stream->port,msg); break;
        case STR_NTRIPC_C:
        case STR_NTRIPC_S: state=statexntripc((ntripc_t *)stream->port,msg); break;
        case STR_UDPSVR  : state=statexudpsvr((udp_t    *)stream->port,msg); break;
        case STR_UDPCLI  : state=statexudpcli((udp_t    *)stream->port,msg); break;
        case STR_MEMBUF  : state=statexmembuf((membuf_t *)stream->port,msg); break;
        case STR_FTP     : state=statexftp   ((ftp_t    *)stream->port,msg); break;
        case STR_HTTP    : state=statexftp   ((ftp_t    *)stream->port,msg); break;
        default:
            msg[0]='\0';
            strunlock(stream);
            return 0;
    }
    if (state==2&&(int)(tickget()-stream->tact)<=TINTACT) state=3;
    strunlock(stream);
    return state;
}
/* get stream statistics summary -----------------------------------------------
* get stream statistics summary
* args   : stream_t *stream I   stream
*          int    *inb      IO   bytes of input  (NULL: no output)
*          int    *inr      IO   bps of input    (NULL: no output)
*          int    *outb     IO   bytes of output (NULL: no output)
*          int    *outr     IO   bps of output   (NULL: no output)
* return : none
*-----------------------------------------------------------------------------*/
extern void strsum(stream_t *stream, int *inb, int *inr, int *outb, int *outr)
{
    tracet(4,"strsum:\n");
    
    strlock(stream);
    if (inb)  *inb =stream->inb;
    if (inr)  *inr =stream->inr;
    if (outb) *outb=stream->outb;
    if (outr) *outr=stream->outr;
    strunlock(stream);
}
/* set ntrip source table to ntrip-caster stream -------------------------------
* set ntrip source table to ntrip-caster stream
* args   : stream_t *stream I   stream
*          char   *file     I   source table file
* return : status (0:error,1:ok)
*-----------------------------------------------------------------------------*/
extern int strsetsrctbl(stream_t *stream, const char *file)
{
    FILE *fp;
    ntripc_t *ntripc;
    char *srctbl=NULL,*srctbl_n;
    int n=0,nmax;
    
    tracet(3,"strsetsrctbl: file=%s\n",file);
    
    strlock(stream);
    
    if (stream->type!=STR_NTRIPC_C&&stream->type!=STR_NTRIPC_S) {
        strunlock(stream);
        return 1;
    }
    if (!(ntripc=(ntripc_t *)stream->port)) {
        strunlock(stream);
        return 0;
    }
    if (!(fp=fopen(file,"rb"))) {
        tracet(2,"strsetsrctbl: file open error %s\n",file);
        strunlock(stream);
        return 0;
    }
    for (nmax=4096;;nmax+=4096) {
        if (!(srctbl_n=(char *)realloc(srctbl,nmax))) {
            free(srctbl); fclose(fp);
            strunlock(stream);
            return 0;
        }
        srctbl=srctbl_n;
        n+=fread(srctbl+n,1,nmax-1-n,fp);
        if (n<nmax-1) break;
    }
    srctbl[n++]='\0';
    fclose(fp);
    tracet(3,"strsetsrctbl: n=%d\n",n);
    
    lock(&ntripc->lock_srctbl);
    
    free(ntripc->srctbl);
    ntripc->srctbl=srctbl;
    
    unlock(&ntripc->lock_srctbl);
    strunlock(stream);
    return 1;
}
/* set global stream options ---------------------------------------------------
* set global stream options
* args   : int    *opt      I   options
*              opt[0]= inactive timeout (ms) (0: no timeout)
*              opt[1]= interval to reconnect (ms)
*              opt[2]= averaging time of data rate (ms)
*              opt[3]= receive/send buffer size (bytes);
*              opt[4]= file swap margin (s)
*              opt[5]= reserved
*              opt[6]= reserved
*              opt[7]= reserved
* return : none
*-----------------------------------------------------------------------------*/
extern void strsetopt(const int *opt)
{
    tracet(3,"strsetopt: opt=%d %d %d %d %d %d %d %d\n",opt[0],opt[1],opt[2],
           opt[3],opt[4],opt[5],opt[6],opt[7]);
    
    toinact    =0<opt[0]&&opt[0]<1000?1000:opt[0]; /* >=1s */
    ticonnect  =opt[1]<1000?1000:opt[1]; /* >=1s */
    tirate     =opt[2]<100 ?100 :opt[2]; /* >=0.1s */
    buffsize   =opt[3]<4096?4096:opt[3]; /* >=4096byte */
    fswapmargin=opt[4]<0?0:opt[4];
}
/* set timeout time ------------------------------------------------------------
* set timeout time
* args   : stream_t *stream I   stream (STR_TCPCLI,STR_NTRIPCLI,STR_NTRIPSVR)
*          int     toinact  I   inactive timeout (ms) (0: no timeout)
*          int     tirecon  I   reconnect interval (ms) (0: no reconnect)
* return : none
*-----------------------------------------------------------------------------*/
extern void strsettimeout(stream_t *stream, int toinact, int tirecon)
{
    tcpcli_t *tcpcli;
    
    tracet(3,"strsettimeout: toinact=%d tirecon=%d\n",toinact,tirecon);
    
    if (stream->type==STR_TCPCLI) {
        tcpcli=(tcpcli_t *)stream->port;
    }
    else if (stream->type==STR_NTRIPCLI||stream->type==STR_NTRIPSVR) {
        tcpcli=((ntrip_t *)stream->port)->tcp;
    }
    else return;
    
    tcpcli->toinact=toinact;
    tcpcli->tirecon=tirecon;
}
/* set local directory ---------------------------------------------------------
* set local directory path for ftp/http download
* args   : char   *dir      I   directory for download files
* return : none
*-----------------------------------------------------------------------------*/
extern void strsetdir(const char *dir)
{
    tracet(3,"strsetdir: dir=%s\n",dir);
    
    strcpy(localdir,dir);
}
/* set http/ntrip proxy address ------------------------------------------------
* set http/ntrip proxy address
* args   : char   *addr     I   http/ntrip proxy address <address>:<port>
* return : none
*-----------------------------------------------------------------------------*/
extern void strsetproxy(const char *addr)
{
    tracet(3,"strsetproxy: addr=%s\n",addr);
    
    strcpy(proxyaddr,addr);
}
/* get stream time -------------------------------------------------------------
* get stream time
* args   : stream_t *stream I   stream
* return : current time or replay time for playback file
*-----------------------------------------------------------------------------*/
extern gtime_t strgettime(stream_t *stream)
{
    file_t *file;
    if (stream->type==STR_FILE&&(stream->mode&STR_MODE_R)&&
        (file=(file_t *)stream->port)) {
        return timeadd(file->time,file->start); /* replay start time */
    }
    return utc2gpst(timeget());
}
/* send nmea request -----------------------------------------------------------
* send nmea gpgga message to stream
* args   : stream_t *stream I   stream
*          sol_t *sol       I   solution
* return : none
*-----------------------------------------------------------------------------*/
extern void strsendnmea(stream_t *stream, const sol_t *sol)
{
    unsigned char buff[1024];
    int n;
    
    tracet(3,"strsendnmea: rr=%.3f %.3f %.3f\n",sol->rr[0],sol->rr[1],sol->rr[2]);
    
    n=outnmea_gga(buff,sol);
    strwrite(stream,buff,n);
}
/* generate general hex message ----------------------------------------------*/
static int gen_hex(const char *msg, unsigned char *buff)
{
    unsigned char *q=buff;
    char mbuff[1024]="",*args[256],*p;
    unsigned int byte;
    int i,narg=0;
    
    trace(4,"gen_hex: msg=%s\n",msg);
    
    strncpy(mbuff,msg,1023);
    for (p=strtok(mbuff," ");p&&narg<256;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    for (i=0;i<narg;i++) {
        if (sscanf(args[i],"%x",&byte)) *q++=(unsigned char)byte;
    }
    return (int)(q-buff);
}
/* set bitrate ---------------------------------------------------------------*/
static int set_brate(stream_t *str, int brate)
{
    char path[1024],buff[1024]="",*p,*q;
    int type=str->type,mode=str->mode;
    
    if (type!=STR_SERIAL) return 0;
    
    strcpy(path,str->path);
    
    if (!(p=strchr(path,':'))) {
        sprintf(path+strlen(path),":%d",brate);
    }
    else {
        if ((q=strchr(p+1,':'))) strcpy(buff,q);
        sprintf(p,":%d%s",brate,buff);
    }
    strclose(str);
    return stropen(str,type,mode,path);
}
/* send receiver command -------------------------------------------------------
* send receiver commands to stream
* args   : stream_t *stream I   stream
*          char   *cmd      I   receiver command strings
* return : none
*-----------------------------------------------------------------------------*/
extern void strsendcmd(stream_t *str, const char *cmd)
{
    unsigned char buff[1024];
    const char *p=cmd,*q;
    char msg[1024],cmdend[]="\r\n";
    int n,m,ms,brate;
    
    tracet(3,"strsendcmd: cmd=%s\n",cmd);
    
    for (;;) {
        for (q=p;;q++) if (*q=='\r'||*q=='\n'||*q=='\0') break;
        n=(int)(q-p); strncpy(msg,p,n); msg[n]='\0';
        
        if (!*msg||*msg=='#') { /* null or comment */
            ;
        }
        else if (*msg=='!') { /* binary escape */
            
            if (!strncmp(msg+1,"WAIT",4)) { /* wait */
                if (sscanf(msg+5,"%d",&ms)<1) ms=100;
                if (ms>3000) ms=3000; /* max 3 s */
                sleepms(ms);
            }
            else if (!strncmp(msg+1,"BRATE",5)) { /* set bitrate */
                if (sscanf(msg+6,"%d",&brate)<1) brate=9600;
                set_brate(str,brate);
                sleepms(500);
            }
            else if (!strncmp(msg+1,"UBX",3)) { /* ublox */
                if ((m=gen_ubx(msg+4,buff))>0) strwrite(str,buff,m);
            }
            else if (!strncmp(msg+1,"STQ",3)) { /* skytraq */
                if ((m=gen_stq(msg+4,buff))>0) strwrite(str,buff,m);
            }
            else if (!strncmp(msg+1,"NVS",3)) { /* nvs */
                if ((m=gen_nvs(msg+4,buff))>0) strwrite(str,buff,m);
            }
            else if (!strncmp(msg+1,"LEXR",4)) { /* lex receiver */
                if ((m=gen_lexr(msg+5,buff))>0) strwrite(str,buff,m);
            }
            else if (!strncmp(msg+1,"HEX",3)) { /* general hex message */
                if ((m=gen_hex(msg+4,buff))>0) strwrite(str,buff,m);
            }
        }
        else {
            strcat(msg,cmdend);
            strwrite(str,(unsigned char *)msg,n+2);
        }
        if (*q=='\0') break; else p=q+1;
    }
}