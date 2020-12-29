/*------------------------------------------------------------------------------
* vt.c : virtual console
*
*          Copyright (C) 2014-2016 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2015/01/11 1.0  separated from rtkrcv.c
*           2016/09/19 1.1  change api vt_open()
*-----------------------------------------------------------------------------*/
#ifndef WIN32
#define _POSIX_C_SOURCE 2
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef WIN32
#include <winsock2.h>
#else
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
#include <termios.h>
#endif
#include "vt.h"

#define DEF_DEV     "/dev/tty"          /* default console device */

#define C_DEL       (char)0x7F          /* delete */
#define C_ESC       (char)0x1B          /* escape */
#define C_CTRC      (char)0x03          /* interrupt (ctrl-c) */
#define C_CTRD      (char)0x04          /* logout (ctrl-d) */
#define C_ECHO      (char)1             /* telnet echo */
#define C_SUPPGA    (char)3             /* telnet suppress go ahead */
#define C_BRK       (char)243           /* telnet break */
#define C_IP        (char)244           /* telnet interrupt */
#define C_EC        (char)247           /* telnet erase character */
#define C_EL        (char)248           /* telnet erase line */
#define C_WILL      (char)251           /* telnet option negotiation */
#define C_WONT      (char)252           /* telnet option negotiation */
#define C_DO        (char)253           /* telnet option negotiation */
#define C_DONT      (char)254           /* telnet option negotiation */
#define C_IAC       (char)255           /* telnet interpret as command */

/* open virtual console --------------------------------------------------------
* open virtual console
* args   : vt_t   *vt       I   virtual console
*          int    sock      I   socket (0:use device)
*          char   *dev      I   device ("": standard tty)
* return : virtual console (NULL: error)
*-----------------------------------------------------------------------------*/
extern vt_t *vt_open(int sock, const char *dev)
{
    const char mode[]={C_IAC,C_WILL,C_SUPPGA,C_IAC,C_WILL,C_ECHO};
    struct termios tio={0};
    vt_t *vt;
    int i;
    
    trace(3,"vt_open: sock=%d dev=%s\n",sock,dev);
    
    if (!(vt=(vt_t *)malloc(sizeof(vt_t)))) {
        return NULL;
    }
    vt->type=vt->n=vt->nesc=vt->cur=vt->cur_h=vt->brk=vt->blind=0;
    vt->logfp=NULL;
    for (i=0;i<MAXHIST;i++) {
        vt->hist[i]=NULL;
    }
    if (!sock) {
        if ((vt->in=vt->out=open(*dev?dev:DEF_DEV,O_RDWR))<0) {
            free(vt);
            return 0;
        }
        /* set terminal mode echo-off */
        tcgetattr(vt->in,&vt->tio);
        tcsetattr(vt->in,TCSANOW,&tio);
    }
    else {
        vt->type=1;
        vt->in=vt->out=sock;
        
        /* send telnet character mode */
        if (write(sock,mode,6)!=6) {
            free(vt);
            return NULL;
        }
    }
    vt->state=1;
    return vt;
}
/* close virtual console -------------------------------------------------------
* close virtual console
* args   : vt_t   *vt       I   virtual console
* return : none
*-----------------------------------------------------------------------------*/
extern void vt_close(vt_t *vt)
{
    int i;
    
    trace(3,"vt_close:\n");
    
    /* restore terminal mode */
    if (!vt->type) {
        tcsetattr(vt->in,TCSANOW,&vt->tio);
    }
    close(vt->in);
    if (vt->logfp) fclose(vt->logfp);
    for (i=0;i<MAXHIST;i++) {
        free(vt->hist[i]);
    }
    free(vt);
}
/* clear line buffer ---------------------------------------------------------*/
static int clear_buff(vt_t *vt)
{
    char buff[MAXBUFF*3],*p=buff;
    int i,len=strlen(vt->buff);
    for (i=0;i<vt->cur;i++) *p++='\b';
    for (i=0;i<len;i++) *p++=' ';
    for (i=0;i<len;i++) *p++='\b';
    vt->n=vt->nesc=vt->cur=0;
    return write(vt->out,buff,p-buff)==p-buff;
}
/* refresh line buffer -------------------------------------------------------*/
static int ref_buff(vt_t *vt)
{
    char buff[MAXBUFF*3],*p=buff;
    int i;
    for (i=vt->cur;i<vt->n;i++) *p++=vt->buff[i];
    *p++=' ';
    for (;i>=vt->cur;i--) *p++='\b';
    return write(vt->out,buff,p-buff)==p-buff;
}
/* move cursor right ---------------------------------------------------------*/
static int right_cur(vt_t *vt)
{
    if (vt->cur>=vt->n) return 1;
    if (write(vt->out,vt->buff+vt->cur,1)<1) return 0;
    vt->cur++;
    return 1;
}
/* move cursor left ----------------------------------------------------------*/
static int left_cur(vt_t *vt)
{
    if (vt->cur<=0) return 1;
    vt->cur--;
    return write(vt->out,"\b",1)==1;
}
/* delete character before cursor --------------------------------------------*/
static int del_cur(vt_t *vt)
{
    int i;
    if (vt->cur<=0) return 1;
    for (i=vt->cur;i<vt->n;i++) vt->buff[i-1]=vt->buff[i];
    vt->n--;
    return left_cur(vt)&&ref_buff(vt);
}
/* insert character after cursor ---------------------------------------------*/
static int ins_cur(vt_t *vt, char c)
{
    int i;
    if (vt->n>=MAXBUFF) return 1;
    for (i=vt->n++;i>vt->cur;i--) vt->buff[i]=vt->buff[i-1];
    vt->buff[vt->cur++]=c;
    if (write(vt->out,vt->blind?"*":&c,1)<1) return 0;
    return ref_buff(vt);
}
/* add history ---------------------------------------------------------------*/
static int hist_add(vt_t *vt, const char *buff)
{
    int i,len;
    if ((len=strlen(buff))<=0) return 1;
    free(vt->hist[MAXHIST-1]);
    for (i=MAXHIST-1;i>0;i--) vt->hist[i]=vt->hist[i-1];
    if (!(vt->hist[0]=(char *)malloc(len+1))) return 0;
    strcpy(vt->hist[0],buff);
    return 1;
}
/* call previous history -----------------------------------------------------*/
static int hist_prev(vt_t *vt)
{
    char *p;
    if (vt->cur_h>=MAXHIST||!vt->hist[vt->cur_h]) return 1;
    if (!clear_buff(vt)) return 0;
    for (p=vt->hist[vt->cur_h++];*p;p++) if (!ins_cur(vt,*p)) return 0;
    return 1;
}
/* call next history ---------------------------------------------------------*/
static int hist_next(vt_t *vt)
{
    char *p;
    if (!clear_buff(vt)) return 0;
    if (vt->cur_h==0||!vt->hist[vt->cur_h-1]) return 1;
    for (p=vt->hist[--vt->cur_h];*p;p++) if (!ins_cur(vt,*p)) return 0;
    return 1;
}
/* handle telnet sequence ----------------------------------------------------*/
static int seq_telnet(vt_t *vt)
{
    char msg[3]={C_IAC};
    
    if (vt->esc[1]==C_WILL) { /* option negotiation */
        if (vt->nesc<3) return 1;
        msg[1]=vt->esc[2]==C_ECHO||vt->esc[2]==C_SUPPGA?C_DO:C_DONT;
        msg[2]=vt->esc[2];
        if (write(vt->out,msg,3)<3) return 0;
    }
    else if (vt->esc[1]==C_DO) { /* option negotiation */
        if (vt->nesc<3) return 1;
        msg[1]=vt->esc[2]==C_ECHO||vt->esc[2]==C_SUPPGA?C_WILL:C_WONT;
        msg[2]=vt->esc[2];
        if (write(vt->out,msg,3)<3) return 0;
    }
    else if (vt->esc[1]==C_WONT||vt->esc[1]==C_DONT) { /* option negotiation */
        if (vt->nesc<3) return 1;
        msg[1]=vt->esc[1]==C_WONT?C_DONT:C_WONT;
        msg[2]=vt->esc[2];
        if (write(vt->out,msg,3)<3) return 0;
    }
    else if (vt->esc[1]==C_BRK||vt->esc[1]==C_IP) { /* break or interrupt */
        vt->brk=1;
    }
    else if (vt->esc[1]==C_EC) { /* erase character */
        del_cur(vt);
    }
    else if (vt->esc[1]==C_EL) { /* erase line */
        clear_buff(vt);
    }
    vt->nesc=0;
    return 1;
}
/* handle escape sequence ----------------------------------------------------*/
static int seq_esc(vt_t *vt)
{
    if (vt->nesc<3) return 1;
    vt->nesc=0;
    if (vt->esc[1]=='['||vt->esc[1]=='O') {
        if (vt->esc[2]=='A') return hist_prev(vt); /* cursor up    */
        if (vt->esc[2]=='B') return hist_next(vt); /* cursor down  */
        if (vt->esc[2]=='C') return right_cur(vt); /* cursor right */
        if (vt->esc[2]=='D') return left_cur (vt); /* cursor left  */
    }
    return 1;
}
/* get character from console --------------------------------------------------
* get a character from virtual console with timeout
* args   : vt_t   *vt       I   virtual console
*          char   *c        O   character
* return : status (1:ok,0:error)
* notes  : if no input, return ok with *c='\0'
*-----------------------------------------------------------------------------*/
extern int vt_getc(vt_t *vt, char *c)
{
    struct timeval tv={0,1000}; /* timeout (us) */
    fd_set rs;
    int stat;
    
    *c='\0';
    
    if (!vt||!vt->state) return 0;
    
    /* read character with timeout */
    FD_ZERO(&rs);
    FD_SET(vt->in,&rs);
    if (!(stat=select(vt->in+1,&rs,NULL,NULL,&tv))) return 1; /* no data */
    if (stat<0||read(vt->in,c,1)!=1) return 0; /* error */
    
    if ((vt->type&&*c==C_IAC)||*c==C_ESC) { /* escape or telnet */
        vt->esc[0]=*c; *c='\0';
        vt->nesc=1;
    }
    else if (vt->nesc>0&&vt->esc[0]==C_IAC) { /* telnet sequence */
        vt->esc[vt->nesc++]=*c; *c='\0';
        if (!seq_telnet(vt)) return 0;
    }
    else if (vt->nesc>0&&vt->esc[0]==C_ESC) { /* escape sequence */
        vt->esc[vt->nesc++]=*c; *c='\0';
        if (!seq_esc(vt)) return 0;
    }
    else if (*c=='\b'||*c==C_DEL) { /* backspace or delete */
        if (!del_cur(vt)) return 0;
    }
    else if (*c==C_CTRC) { /* interrupt (ctrl-c) */
        vt->brk=1;
        if (!vt_puts(vt,"^C\n")) return 0;
    }
    else if (isprint(*c)) { /* printable character */
        if (!ins_cur(vt,*c)) return 0;
    }
    return 1;
}
/* get line from console -------------------------------------------------------
* get line from virtual console
* args   : vt_t   *vt       I   virtual console
*          char   *buff     O   buffer
*          in     n         I   buffer size
* return : status (1:ok,0:no input)
*-----------------------------------------------------------------------------*/
extern int vt_gets(vt_t *vt, char *buff, int n)
{
    char c;
    
    buff[0]='\0';
    
    if (!vt||!vt->state) return 0;
    
    vt->n=vt->cur=vt->nesc=vt->brk=0;
    
    while (vt->state) {
        if (!vt_getc(vt,&c)) return 0;
        
        if (c==C_CTRD&&vt->n==0) { /* logout */
            vt->state=0;
        }
        else if (vt->brk) { /* break */
            return vt_puts(vt,"\n");
        }
        else if (c=='\r') { /* end of line */
            vt->buff[vt->n]='\0';
            strncpy(buff,vt->buff,n-1);
            buff[n-1]='\0';
            if (!vt->blind) hist_add(vt,buff);
            return vt_putc(vt,'\n');
        }
    }
    return 0;
}
/* put character to console --------------------------------------------------*/
static int vt_putchar(vt_t *vt, char c)
{
    if (!vt||!vt->state) return 0;
    if (vt->logfp) fwrite(&c,1,1,vt->logfp);
    return write(vt->out,&c,1)==1;
}
/* put character to console ----------------------------------------------------
* put a character to virtual console
* args   : vt_t   *vt       I   virtual console
*          char   c         I   character
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int vt_putc(vt_t *vt, char c)
{
    if (c=='\n'&&!vt_putchar(vt,'\r')) return 0;
    return vt_putchar(vt,c);
}
/* put strings to console ------------------------------------------------------
* put strings to virtual console
* args   : vt_t   *vt       I   virtual console
*          char   *buff     I   strings
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int vt_puts(vt_t *vt, const char *buff)
{
    const char *p;
    for (p=buff;*p;p++) if (!vt_putc(vt,*p)) return 0;
    return 1;
}
/* print to console with formatting --------------------------------------------
* print to virtual console with formatting
* args   : vt_t   *vt       I   virtual console
*          char   *format   I   format (same as sfprintf)
*          ...              I   variable arguments
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int vt_printf(vt_t *vt, const char *format, ...)
{
    va_list ap;
    char buff[MAXBUFF+1];
    va_start(ap,format);
    vsprintf(buff,format,ap);
    va_end(ap);
    return vt_puts(vt,buff);
}
/* check break on console ------------------------------------------------------
* check break on virtual console
* args   : vt_t   *vt       I   virtual console
* return : status (1:break,0:no break)
*-----------------------------------------------------------------------------*/
extern int vt_chkbrk(vt_t *vt)
{
    char c;
    vt->brk=0;
    return !vt_getc(vt,&c)||vt->brk;
}
/* open console log ------------------------------------------------------------
* open console log for virtual console
* args   : vt_t   *vt       I   virtual console
*          char   *file     I   log file path
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int vt_openlog(vt_t *vt, const char *file)
{
    if (!vt||!vt->state||!(vt->logfp=fopen(file,"w"))) return 0;
    return 1;
}
/* close console log -----------------------------------------------------------
* close console log for virtual console
* args   : vt_t   *vt       I   virtual console
* return : none
*-----------------------------------------------------------------------------*/
extern void vt_closelog(vt_t *vt)
{
    if (!vt||!vt->state||!vt->logfp) return;
    fclose(vt->logfp);
    vt->logfp=NULL;
}
