/*
 * unixdomain.c : example implementation of custom stream for unix domain socket
 *
 * Initial implementation by eltorio (https://github.com/eltorio/RTKLIB/)
 * https://github.com/tomojitakasu/RTKLIB/pull/203
 *
 * Example of use:
 *
 *   extern strcustom_t unixdomain;
 *
 *   strsetcustom(&unixdomain);
 */
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include "rtklib.h"

#define MAXCLI              32          /* max client connection */

#define socket_t            int
#define closesocket         close

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen((ptr)->sun_path))
#endif

typedef struct {            /* unix domain socket control type */ 
    int state;              /* state (0:close,1:wait,2:connect) */ 
    char saddr[108];        /* address string */ 
    int port;               /* port */ 
    struct sockaddr_un addr; /* address resolved */ 
    socket_t sock;          /* socket descriptor */ 
    int tcon;               /* reconnect time (ms) (-1:never,0:now) */ 
    unsigned int tact;      /* data active tick */ 
    unsigned int tdis;      /* disconnect tick */ 
} unix_t; 

typedef struct {            /* unix domain socket server type */ 
    unix_t svr;              /* unix server control */ 
    unix_t cli[MAXCLI];      /* unix client controls */ 
} unixsvr_t;

static int ticonnect=10000; /* interval to re-connect (ms) */
static int buffsize =32768; /* receive/send buffer size (bytes) */

static int errsock(void) {return errno;}


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

/* generate unix socket -------------------------------------------------------*/
 static int genunix(unix_t *unx, char *msg)
 {
     tracet(3,"genunix\n");

     /* generate socket */
     if ((unx->sock=socket(AF_LOCAL,SOCK_STREAM,0))==(socket_t)-1) {
         sprintf(msg,"socket error (%d)",errsock());
         tracet(1,"genunix: socket error err=%d\n",errsock());
         unx->state=-1;
        return 0;
     }
     if (!setsock(unx->sock,msg)) {
         unx->state=-1;
         return 0;
     }
     memset(&unx->addr,0,sizeof(unx->addr));
     unx->addr.sun_family=AF_LOCAL;
     strncpy(unx->addr.sun_path, unx->saddr, sizeof(unx->addr.sun_path)-1);

     unlink(unx->addr.sun_path);

     if (bind(unx->sock,(struct sockaddr *)&unx->addr,SUN_LEN(&unx->addr))==-1) {
        sprintf(msg,"bind error (%d) ",errsock());
        tracet(1,"genunix: bind error path=%s err=%d\n",unx->addr.sun_path,errsock());
        closesocket(unx->sock);
        unx->state=-1;
        return 0;
     }
     listen(unx->sock,5);
     unx->state=1;
     unx->tact=tickget();
     tracet(5,"genunx: exit sock=%d\n",unx->sock);
     return 1;
 }
 /* disconnect unix socket ---------------------------------------------------*/
 static void disconunix(unix_t *unx, int tcon)
 {
     tracet(3,"disconunix: sock=%d tcon=%d\n",unx->sock,tcon);

     closesocket(unx->sock);
     unx->state=0;
     unx->tcon=tcon;
     unx->tdis=tickget();
 }

 /* open unix domain socket server --------------------------------------------*/
 static unixsvr_t *openunixsvr(const char *path, char *msg)
 {
     unixsvr_t *unixsvr,unixsvr0={{0}};

     tracet(3,"openunixsvr: path=%s\n",path);

     if (!(unixsvr=(unixsvr_t *)malloc(sizeof(unixsvr_t)))) return NULL;
     *unixsvr=unixsvr0;
     strncpy(unixsvr->svr.saddr, path, sizeof(unixsvr->svr.saddr)-1);
     if (!genunix(&unixsvr->svr,msg)) {
         free(unixsvr);
         return NULL;
     }
     unixsvr->svr.tcon=0;
     return unixsvr;
 }
 /* close unix server ----------------------------------------------------------*/
 static void closeunixsvr(unixsvr_t *unixsvr)
 {
     int i;

     tracet(3,"closeunixsvr:\n");

     for (i=0;i<MAXCLI;i++) {
         if (unixsvr->cli[i].state) closesocket(unixsvr->cli[i].sock);
     }
     closesocket(unixsvr->svr.sock);
     unlink(unixsvr->svr.addr.sun_path);
     free(unixsvr);
 }
 /* update unix server ---------------------------------------------------------*/
 static void updateunixsvr(unixsvr_t *unixsvr, char *msg)
 {
     char saddr[256]="";
     int i,j,n=0;

     tracet(3,"updateunixsvr: state=%d\n",unixsvr->svr.state);

     if (unixsvr->svr.state==0) return;

     for (i=0;i<MAXCLI;i++) {
         if (unixsvr->cli[i].state) continue;
         for (j=i+1;j<MAXCLI;j++) {
             if (!unixsvr->cli[j].state) continue;
             unixsvr->cli[i]=unixsvr->cli[j];
             unixsvr->cli[j].state=0;
             break;
         }
     }
     for (i=0;i<MAXCLI;i++) {
         if (!unixsvr->cli[i].state) continue;
         memcpy(saddr,unixsvr->cli[i].saddr, sizeof(unixsvr->cli[i].saddr));
         n++;
     }
     if (n==0) {
         unixsvr->svr.state=1;
         sprintf(msg,"waiting...");
         return;
     }
     unixsvr->svr.state=2;
     if (n==1) sprintf(msg,"%s",saddr); else sprintf(msg,"%d clients",n);
 }
 /* accept client connection --------------------------------------------------*/
 static int accunixsock(unixsvr_t *unixsvr, char *msg)
 {
     struct sockaddr_un addr;
     socket_t sock;
     socklen_t len=sizeof(addr);
     int i,err;

     tracet(3,"accunixsock: sock=%d\n",unixsvr->svr.sock);

     for (i=0;i<MAXCLI;i++) if (unixsvr->cli[i].state==0) break;
     if (i>=MAXCLI) return 0; /* too many client */

     if ((sock=accept_nb(unixsvr->svr.sock,(struct sockaddr *)&addr,&len))==(socket_t)-1) {
         err=errsock();
         sprintf(msg,"accept error (%d)",err);
         tracet(1,"accunixsock: accept error sock=%d err=%d\n",unixsvr->svr.sock,err);
         closesocket(unixsvr->svr.sock); unixsvr->svr.state=0;
 	unlink(unixsvr->svr.addr.sun_path);
         return 0;
     }
     if (sock==0) return 0;

     unixsvr->cli[i].sock=sock;
     if (!setsock(unixsvr->cli[i].sock,msg)) return 0;
     memcpy(&unixsvr->cli[i].addr,&addr,sizeof(addr));
     memcpy(unixsvr->cli[i].saddr,addr.sun_path, sizeof(addr.sun_path));
     sprintf(msg,"%s",unixsvr->cli[i].saddr);
     tracet(2,"accunixsock: connected sock=%d addr=%s\n",unixsvr->cli[i].sock,unixsvr->cli[i].saddr);
     unixsvr->cli[i].state=2;
     unixsvr->cli[i].tact=tickget();
     return 1;
 }
 /* wait socket accept --------------------------------------------------------*/
 static int waitunixsvr(unixsvr_t *unixsvr, char *msg)
 {
     tracet(4,"waitunixsvr: sock=%d state=%d\n",unixsvr->svr.sock,unixsvr->svr.state);

     if (unixsvr->svr.state<=0) return 0;

     while (accunixsock(unixsvr,msg)) ;

     updateunixsvr(unixsvr,msg);
     return unixsvr->svr.state==2;
 }
 /* read unix server ----------------------------------------------------------*/
 static int readunixsvr(unixsvr_t *unixsvr, unsigned char *buff, int n, char *msg)
 {
     int nr,err;

     tracet(4,"readunixsvr: state=%d n=%d\n",unixsvr->svr.state,n);

     if (!waitunixsvr(unixsvr,msg)||unixsvr->cli[0].state!=2) return 0;

     if ((nr=recv_nb(unixsvr->cli[0].sock,buff,n))==-1) {
         err=errsock();
         tracet(1,"readunixsvr: recv error sock=%d err=%d\n",unixsvr->cli[0].sock,err);
         sprintf(msg,"recv error (%d)",err);
         disconunix(&unixsvr->cli[0],ticonnect);
         updateunixsvr(unixsvr,msg);
         return 0;
     }
     if (nr>0) unixsvr->cli[0].tact=tickget();
     tracet(5,"readunixsvr: exit sock=%d nr=%d\n",unixsvr->cli[0].sock,nr);
     return nr;
 }
 /* write unix server ---------------------------------------------------------*/
 static int writeunixsvr(unixsvr_t *unixsvr, unsigned char *buff, int n, char *msg)
 {
     int i,ns=0,err;

     tracet(3,"writeunixsvr: state=%d n=%d\n",unixsvr->svr.state,n);

     if (!waitunixsvr(unixsvr,msg)) return 0;

     for (i=0;i<MAXCLI;i++) {
         if (unixsvr->cli[i].state!=2) continue;

         if ((ns=send_nb(unixsvr->cli[i].sock,buff,n))==-1) {
             err=errsock();
             tracet(1,"writeunixsvr: send error i=%d sock=%d err=%d\n",i,unixsvr->cli[i].sock,err);
             sprintf(msg,"send error (%d)",err);
             disconunix(&unixsvr->cli[i],ticonnect);
             updateunixsvr(unixsvr,msg);
             return 0;
         }
         if (ns>0) unixsvr->cli[i].tact=tickget();
         tracet(5,"writeunixsvr: send i=%d ns=%d\n",i,ns);
     }
     return ns;
 }
 /* get state unix server -----------------------------------------------------*/
 static int stateunixsvr(unixsvr_t *unixsvr)
 {
     return unixsvr?unixsvr->svr.state:0;
 }

/* print extended state tcp --------------------------------------------------*/
static int statexunix(unix_t *unx, char *msg)
{
  char *p=msg;

  p+=sprintf(p,"    state = %d\n",unx->state);
  p+=sprintf(p,"    saddr = %s\n",unx->saddr);
  p+=sprintf(p,"    port  = %d\n",unx->port);
  p+=sprintf(p,"    sock  = %d\n",(int)unx->sock);
#if 0
  p+=sprintf(p,"    tcon  = %d\n",tcp->tcon);
    p+=sprintf(p,"    tact  = %u\n",tcp->tact);
    p+=sprintf(p,"    tdis  = %u\n",tcp->tdis);
#endif
  return (int)(p-msg);
}

/* get extended state unix server --------------------------------------------*/
static int statexunixsvr(unixsvr_t *unixsvr, char *msg)
{
  char *p=msg;
  int i,state=unixsvr?unixsvr->svr.state:0;

  p+=sprintf(p,"unixsvr:\n");
  p+=sprintf(p,"  state   = %d\n",state);
  if (!state) return 0;
  p+=sprintf(p,"  svr:\n");
  p+=statexunix(&unixsvr->svr,p);
  for (i=0;i<MAXCLI;i++) {
    if (!unixsvr->cli[i].state) continue;
    p+=sprintf(p,"  cli#%d:\n",i);
    p+=statexunix(unixsvr->cli+i,p);
  }
  return state;
}

/* Wrappers for custom stream */
static void *c_openunixsvr(const char *path, int mode, char *msg)
{
  return openunixsvr(path,msg);
}
static void c_closeunixsvr(void *unixsvr)
{
  closeunixsvr(unixsvr);
}
static int c_readunixsvr(void *unixsvr, unsigned char *buff, int n, char *msg)
{
  return readunixsvr(unixsvr,buff,n,msg);
}
static int c_writeunixsvr(void *unixsvr, unsigned char *buff, int n, char *msg)
{
  return writeunixsvr(unixsvr,buff,n,msg);
}
static int c_stateunixsvr(void *unixsvr)
{
  return stateunixsvr(unixsvr);
}
static int c_statexunixsvr(void *unixsvr, char *msg)
{
  return statexunixsvr(unixsvr,msg);
}

strcustom_t unixdomain = {
    c_openunixsvr, c_closeunixsvr,
    c_readunixsvr, c_writeunixsvr,
    c_stateunixsvr, c_statexunixsvr
};
