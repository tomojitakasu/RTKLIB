/*------------------------------------------------------------------------------
* streamsvr.c : stream server functions
*
*          Copyright (C) 2010-2012 by T.TAKASU, All rights reserved.
*
* options : -DWIN32    use WIN32 API
*
* version : $Revision:$ $Date:$
* history : 2010/07/18 1.0  moved from stream.c
*           2011/01/18 1.1  change api strsvrstart()
*           2012/12/04 1.2  add stream conversion function
*           2012/12/25 1.3  fix bug on cyclic navigation data output
*                           suppress warnings
*           2013/05/08 1.4  fix bug on 1 s offset for javad -> rtcm conversion
*           2014/10/16 1.5  support input from stdout
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id$";

/* test observation data message ---------------------------------------------*/
static int is_obsmsg(int msg)
{
    return (1001<=msg&&msg<=1004)||(1009<=msg&&msg<=1012)||
           (1071<=msg&&msg<=1077)||(1081<=msg&&msg<=1087)||
           (1091<=msg&&msg<=1097)||(1101<=msg&&msg<=1107)||
           (1111<=msg&&msg<=1117)||(1121<=msg&&msg<=1127);
}
/* test navigataion data message ---------------------------------------------*/
static int is_navmsg(int msg)
{
    return msg==1019||msg==1020||msg==1044||msg==1045||msg==1046;
}
/* test station info message -------------------------------------------------*/
static int is_stamsg(int msg)
{
    return msg==1005||msg==1006||msg==1007||msg==1008||msg==1033;
}
/* test time interval --------------------------------------------------------*/
static int is_tint(gtime_t time, double tint)
{
    if (tint<=0.0) return 1;
    return fmod(time2gpst(time,NULL)+DTTOL,tint)<=2.0*DTTOL;
}
/* new stream converter --------------------------------------------------------
* generate new stream converter
* args   : int    itype     I   input stream type  (STR_???)
*          int    otype     I   output stream type (STR_???)
*          char   *msgs     I   output message type and interval (, separated)
*          int    staid     I   station id
*          int    stasel    I   station info selection (0:remote,1:local)
*          char   *opt      I   rtcm or receiver raw options
* return : stream generator (NULL:error)
*-----------------------------------------------------------------------------*/
extern strconv_t *strconvnew(int itype, int otype, const char *msgs, int staid,
                             int stasel, const char *opt)
{
    strconv_t *conv;
    double tint;
    char buff[1024],*p;
    int msg;
    
    if (!(conv=(strconv_t *)malloc(sizeof(strconv_t)))) return NULL;
    
    conv->nmsg=0;
    strcpy(buff,msgs);
    for (p=strtok(buff,",");p;p=strtok(NULL,",")) {
       tint=0.0;
       if (sscanf(p,"%d(%lf)",&msg,&tint)<1) continue;
       conv->msgs[conv->nmsg]=msg;
       conv->tint[conv->nmsg]=tint;
       conv->tick[conv->nmsg]=tickget();
       conv->ephsat[conv->nmsg++]=0;
       if (conv->nmsg>=32) break;
    }
    if (conv->nmsg<=0) {
        free(conv);
        return NULL;
    }
    conv->itype=itype;
    conv->otype=otype;
    conv->stasel=stasel;
    if (!init_rtcm(&conv->rtcm)||!init_rtcm(&conv->out)) {
        free(conv);
        return NULL;
    }
    if (!init_raw(&conv->raw)) {
        free_rtcm(&conv->rtcm);
        free_rtcm(&conv->out);
        free(conv);
        return NULL;
    }
    if (stasel) conv->out.staid=staid;
    sprintf(conv->rtcm.opt,"-EPHALL %s",opt);
    sprintf(conv->raw.opt ,"-EPHALL %s",opt);
    return conv;
}
/* free stream converter -------------------------------------------------------
* free stream converter
* args   : strconv_t *conv  IO  stream converter
* return : none
*-----------------------------------------------------------------------------*/
extern void strconvfree(strconv_t *conv)
{
    if (!conv) return;
    free_rtcm(&conv->rtcm);
    free_rtcm(&conv->out);
    free_raw(&conv->raw);
    free(conv);
}
/* copy received data from receiver raw to rtcm ------------------------------*/
static void raw2rtcm(rtcm_t *out, const raw_t *raw, int ret)
{
    int i,sat,prn;
    
    out->time=raw->time;
    
    if (ret==1) {
        for (i=0;i<raw->obs.n;i++) {
            out->time=raw->obs.data[i].time;
            out->obs.data[i]=raw->obs.data[i];
        }
        out->obs.n=raw->obs.n;
    }
    else if (ret==2) {
        sat=raw->ephsat;
        switch (satsys(sat,&prn)) {
            case SYS_GLO: out->nav.geph[prn-1]=raw->nav.geph[prn-1]; break;
            case SYS_GPS:
            case SYS_GAL:
            case SYS_QZS:
            case SYS_CMP: out->nav.eph [sat-1]=raw->nav.eph [sat-1]; break;
        }
        out->ephsat=sat;
    }
    else if (ret==9) {
        matcpy(out->nav.utc_gps,raw->nav.utc_gps,4,1);
        matcpy(out->nav.utc_glo,raw->nav.utc_glo,4,1);
        matcpy(out->nav.utc_gal,raw->nav.utc_gal,4,1);
        matcpy(out->nav.utc_qzs,raw->nav.utc_qzs,4,1);
        matcpy(out->nav.ion_gps,raw->nav.ion_gps,8,1);
        matcpy(out->nav.ion_gal,raw->nav.ion_gal,4,1);
        matcpy(out->nav.ion_qzs,raw->nav.ion_qzs,8,1);
        out->nav.leaps=raw->nav.leaps;
    }
}
/* copy received data from receiver rtcm to rtcm -----------------------------*/
static void rtcm2rtcm(rtcm_t *out, const rtcm_t *rtcm, int ret, int stasel)
{
    int i,sat,prn;
    
    out->time=rtcm->time;
    
    if (!stasel) out->staid=rtcm->staid;
    
    if (ret==1) {
        for (i=0;i<rtcm->obs.n;i++) {
            out->obs.data[i]=rtcm->obs.data[i];
        }
        out->obs.n=rtcm->obs.n;
    }
    else if (ret==2) {
        sat=rtcm->ephsat;
        switch (satsys(sat,&prn)) {
            case SYS_GLO: out->nav.geph[prn-1]=rtcm->nav.geph[prn-1]; break;
            case SYS_GPS:
            case SYS_GAL:
            case SYS_QZS:
            case SYS_CMP: out->nav.eph [sat-1]=rtcm->nav.eph [sat-1]; break;
        }
        out->ephsat=sat;
    }
    else if (ret==5) {
        if (!stasel) out->sta=rtcm->sta;
    }
    else if (ret==9) {
        matcpy(out->nav.utc_gps,rtcm->nav.utc_gps,4,1);
        matcpy(out->nav.utc_glo,rtcm->nav.utc_glo,4,1);
        matcpy(out->nav.utc_gal,rtcm->nav.utc_gal,4,1);
        matcpy(out->nav.utc_qzs,rtcm->nav.utc_qzs,4,1);
        matcpy(out->nav.ion_gps,rtcm->nav.ion_gps,8,1);
        matcpy(out->nav.ion_gal,rtcm->nav.ion_gal,4,1);
        matcpy(out->nav.ion_qzs,rtcm->nav.ion_qzs,8,1);
        out->nav.leaps=rtcm->nav.leaps;
    }
}
/* write obs data messages ---------------------------------------------------*/
static void write_obs(gtime_t time, stream_t *str, strconv_t *conv)
{
    int i,j=0;
    
    for (i=0;i<conv->nmsg;i++) {
        if (!is_obsmsg(conv->msgs[i])||!is_tint(time,conv->tint[i])) continue;
        
        j=i; /* index of last message */
    }
    for (i=0;i<conv->nmsg;i++) {
        if (!is_obsmsg(conv->msgs[i])||!is_tint(time,conv->tint[i])) continue;
        
        /* generate messages */
        if (conv->otype==STRFMT_RTCM2) {
            if (!gen_rtcm2(&conv->out,conv->msgs[i],i!=j)) continue;
        }
        else if (conv->otype==STRFMT_RTCM3) {
            if (!gen_rtcm3(&conv->out,conv->msgs[i],i!=j)) continue;
        }
        else continue;
        
        /* write messages to stream */
        strwrite(str,conv->out.buff,conv->out.nbyte);
    }
}
/* write nav data messages ---------------------------------------------------*/
static void write_nav(gtime_t time, stream_t *str, strconv_t *conv)
{
    int i;
    
    for (i=0;i<conv->nmsg;i++) {
        if (!is_navmsg(conv->msgs[i])||conv->tint[i]>0.0) continue;
        
        /* generate messages */
        if (conv->otype==STRFMT_RTCM2) {
            if (!gen_rtcm2(&conv->out,conv->msgs[i],0)) continue;
        }
        else if (conv->otype==STRFMT_RTCM3) {
            if (!gen_rtcm3(&conv->out,conv->msgs[i],0)) continue;
        }
        else continue;
        
        /* write messages to stream */
        strwrite(str,conv->out.buff,conv->out.nbyte);
    }
}
/* next ephemeris satellite --------------------------------------------------*/
static int nextsat(nav_t *nav, int sat, int msg)
{
    int sys,p,p0,p1,p2;
    
    switch (msg) {
        case 1019: sys=SYS_GPS; p1=MINPRNGPS; p2=MAXPRNGPS; break;
        case 1020: sys=SYS_GLO; p1=MINPRNGLO; p2=MAXPRNGLO; break;
        case 1044: sys=SYS_QZS; p1=MINPRNQZS; p2=MAXPRNQZS; break;
        case 1045:
        case 1046: sys=SYS_GAL; p1=MINPRNGAL; p2=MAXPRNGAL; break;
        default: return 0;
    }
    if (satsys(sat,&p0)!=sys) return satno(sys,p1);
    
    /* search next valid ephemeris */
    for (p=p0>p2?p1:p0+1;p!=p0;p=p>=p2?p1:p+1) {
        
        if (sys==SYS_GLO) {
            sat=satno(sys,p);
            if (nav->geph[p-1].sat==sat) return sat;
        }
        else {
            sat=satno(sys,p);
            if (nav->eph[sat-1].sat==sat) return sat;
        }
    }
    return 0;
}
/* write cyclic nav data messages --------------------------------------------*/
static void write_nav_cycle(stream_t *str, strconv_t *conv)
{
    unsigned int tick=tickget();
    int i,sat,tint;
    
    for (i=0;i<conv->nmsg;i++) {
        if (!is_navmsg(conv->msgs[i])||conv->tint[i]<=0.0) continue;
        
        /* output cycle */
        tint=(int)(conv->tint[i]*1000.0);
        if ((int)(tick-conv->tick[i])<tint) continue;
        conv->tick[i]=tick;
        
        /* next satellite */
        if (!(sat=nextsat(&conv->out.nav,conv->ephsat[i],conv->msgs[i]))) {
            continue;
        }
        conv->out.ephsat=conv->ephsat[i]=sat;
        
        /* generate messages */
        if (conv->otype==STRFMT_RTCM2) {
            if (!gen_rtcm2(&conv->out,conv->msgs[i],0)) continue;
        }
        else if (conv->otype==STRFMT_RTCM3) {
            if (!gen_rtcm3(&conv->out,conv->msgs[i],0)) continue;
        }
        else continue;
        
        /* write messages to stream */
        strwrite(str,conv->out.buff,conv->out.nbyte);
    }
}
/* write cyclic station info messages ----------------------------------------*/
static void write_sta_cycle(stream_t *str, strconv_t *conv)
{
    unsigned int tick=tickget();
    int i,tint;
    
    for (i=0;i<conv->nmsg;i++) {
        if (!is_stamsg(conv->msgs[i])) continue;
        
        /* output cycle */
        tint=conv->tint[i]==0.0?30000:(int)(conv->tint[i]*1000.0);
        if ((int)(tick-conv->tick[i])<tint) continue;
        conv->tick[i]=tick;
        
        /* generate messages */
        if (conv->otype==STRFMT_RTCM2) {
            if (!gen_rtcm2(&conv->out,conv->msgs[i],0)) continue;
        }
        else if (conv->otype==STRFMT_RTCM3) {
            if (!gen_rtcm3(&conv->out,conv->msgs[i],0)) continue;
        }
        else continue;
        
        /* write messages to stream */
        strwrite(str,conv->out.buff,conv->out.nbyte);
    }
}
/* convert stearm ------------------------------------------------------------*/
static void strconv(stream_t *str, strconv_t *conv, unsigned char *buff, int n)
{
    int i,ret;
    
    for (i=0;i<n;i++) {
        
        /* input rtcm 2 messages */
        if (conv->itype==STRFMT_RTCM2) {
            ret=input_rtcm2(&conv->rtcm,buff[i]);
            rtcm2rtcm(&conv->out,&conv->rtcm,ret,conv->stasel);
        }
        /* input rtcm 3 messages */
        else if (conv->itype==STRFMT_RTCM3) {
            ret=input_rtcm3(&conv->rtcm,buff[i]);
            rtcm2rtcm(&conv->out,&conv->rtcm,ret,conv->stasel);
        }
        /* input receiver raw messages */
        else {
            ret=input_raw(&conv->raw,conv->itype,buff[i]);
            raw2rtcm(&conv->out,&conv->raw,ret);
        }
        /* write obs and nav data messages to stream */
        switch (ret) {
            case 1: write_obs(conv->out.time,str,conv); break;
            case 2: write_nav(conv->out.time,str,conv); break;
        }
    }
    /* write cyclic nav data and station info messages to stream */
    write_nav_cycle(str,conv);
    write_sta_cycle(str,conv);
}
/* stearm server thread ------------------------------------------------------*/
#ifdef WIN32
static DWORD WINAPI strsvrthread(void *arg)
#else
static void *strsvrthread(void *arg)
#endif
{
    strsvr_t *svr=(strsvr_t *)arg;
    unsigned int tick,ticknmea;
    int i,n;
    
    tracet(3,"strsvrthread:\n");
    
    svr->state=1;
    svr->tick=tickget();
    ticknmea=svr->tick-1000;
    
    while (svr->state) {
        tick=tickget();
        
        /* read data from input stream */
        n=strread(svr->stream,svr->buff,svr->buffsize);
        
        /* write data to output streams */
        for (i=1;i<svr->nstr;i++) {
            if (svr->conv[i-1]) {
                strconv(svr->stream+i,svr->conv[i-1],svr->buff,n);
            }
            else {
                strwrite(svr->stream+i,svr->buff,n);
            }
        }
        /* write nmea messages to input stream */
        if (svr->nmeacycle>0&&(int)(tick-ticknmea)>=svr->nmeacycle) {
            strsendnmea(svr->stream,svr->nmeapos);
            ticknmea=tick;
        }
        lock(&svr->lock);
        for (i=0;i<n&&svr->npb<svr->buffsize;i++) {
            svr->pbuf[svr->npb++]=svr->buff[i];
        }
        unlock(&svr->lock);
        
        sleepms(svr->cycle-(int)(tickget()-tick));
    }
    for (i=0;i<svr->nstr;i++) strclose(svr->stream+i);
    svr->npb=0;
    free(svr->buff); svr->buff=NULL;
    free(svr->pbuf); svr->pbuf=NULL;
    
    return 0;
}
/* initialize stream server ----------------------------------------------------
* initialize stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    nout      I   number of output streams
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrinit(strsvr_t *svr, int nout)
{
    int i;
    
    tracet(3,"strsvrinit: nout=%d\n",nout);
    
    svr->state=0;
    svr->cycle=0;
    svr->buffsize=0;
    svr->nmeacycle=0;
    svr->npb=0;
    for (i=0;i<3;i++) svr->nmeapos[i]=0.0;
    svr->buff=svr->pbuf=NULL;
    svr->tick=0;
    for (i=0;i<nout+1&&i<16;i++) strinit(svr->stream+i);
    svr->nstr=i;
    for (i=0;i<16;i++) svr->conv[i]=NULL;
    svr->thread=0;
    initlock(&svr->lock);
}
/* start stream server ---------------------------------------------------------
* start stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    *opts     I   stream options
*              opts[0]= inactive timeout (ms)
*              opts[1]= interval to reconnect (ms)
*              opts[2]= averaging time of data rate (ms)
*              opts[3]= receive/send buffer size (bytes);
*              opts[4]= server cycle (ms)
*              opts[5]= nmea request cycle (ms) (0:no)
*              opts[6]= file swap margin (s)
*          int    *strs     I   stream types (STR_???)
*              strs[0]= input stream
*              strs[1]= output stream 1
*              strs[2]= output stream 2
*              strs[3]= output stream 3
*          char   **paths   I   stream paths
*              paths[0]= input stream
*              paths[1]= output stream 1
*              paths[2]= output stream 2
*              paths[3]= output stream 3
*          strcnv **conv    I   stream converter
*              conv[0]= output stream 1 converter
*              conv[1]= output stream 2 converter
*              conv[2]= output stream 3 converter
*          char   *cmd      I   input stream start command (NULL: no cmd)
*          double *nmeapos  I   nmea request position (ecef) (m) (NULL: no)
* return : status (0:error,1:ok)
*-----------------------------------------------------------------------------*/
extern int strsvrstart(strsvr_t *svr, int *opts, int *strs, char **paths,
                       strconv_t **conv, const char *cmd, const double *nmeapos)
{
    int i,rw,stropt[5]={0};
    char file1[MAXSTRPATH],file2[MAXSTRPATH],*p;
    
    tracet(3,"strsvrstart:\n");
    
    if (svr->state) return 0;
    
    strinitcom();
    
    for (i=0;i<4;i++) stropt[i]=opts[i];
    stropt[4]=opts[6];
    strsetopt(stropt);
    svr->cycle=opts[4];
    svr->buffsize=opts[3]<4096?4096:opts[3]; /* >=4096byte */
    svr->nmeacycle=0<opts[5]&&opts[5]<1000?1000:opts[5]; /* >=1s */
    for (i=0;i<3;i++) svr->nmeapos[i]=nmeapos?nmeapos[i]:0.0;
    
    for (i=0;i<svr->nstr-1;i++) svr->conv[i]=conv[i];
    
    if (!(svr->buff=(unsigned char *)malloc(svr->buffsize))||
        !(svr->pbuf=(unsigned char *)malloc(svr->buffsize))) {
        free(svr->buff); free(svr->pbuf);
        return 0;
    }
    /* open streams */
    for (i=0;i<svr->nstr;i++) {
        strcpy(file1,paths[0]); if ((p=strstr(file1,"::"))) *p='\0';
        strcpy(file2,paths[i]); if ((p=strstr(file2,"::"))) *p='\0';
        if (i>0&&*file1&&!strcmp(file1,file2)) {
            sprintf(svr->stream[i].msg,"output path error: %s",file2);
            for (i--;i>=0;i--) strclose(svr->stream+i);
            return 0;
        }
        rw=i==0?STR_MODE_R:STR_MODE_W;
        if (strs[i]!=STR_FILE) rw|=STR_MODE_W;
        if (stropen(svr->stream+i,strs[i],rw,paths[i])) continue;
        for (i--;i>=0;i--) strclose(svr->stream+i);
        return 0;
    }
    /* write start command to input stream */
    if (cmd) strsendcmd(svr->stream,cmd);
    
    /* create stream server thread */
#ifdef WIN32
    if (!(svr->thread=CreateThread(NULL,0,strsvrthread,svr,0,NULL))) {
#else
    if (pthread_create(&svr->thread,NULL,strsvrthread,svr)) {
#endif
        for (i=0;i<svr->nstr;i++) strclose(svr->stream+i);
        return 0;
    }
    return 1;
}
/* stop stream server ----------------------------------------------------------
* start stream server
* args   : strsvr_t *svr    IO  stream server struct
*          char  *cmd       I   input stop command (NULL: no cmd)
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrstop(strsvr_t *svr, const char *cmd)
{
    tracet(3,"strsvrstop:\n");
    
    if (cmd) strsendcmd(svr->stream,cmd);
    
    svr->state=0;
    
#ifdef WIN32
    WaitForSingleObject(svr->thread,10000);
    CloseHandle(svr->thread);
#else
    pthread_join(svr->thread,NULL);
#endif
}
/* get stream server status ----------------------------------------------------
* get status of stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    *stat     O   stream status
*          int    *byte     O   bytes received/sent
*          int    *bps      O   bitrate received/sent
*          char   *msg      O   messages
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrstat(strsvr_t *svr, int *stat, int *byte, int *bps, char *msg)
{
    char s[MAXSTRMSG]="",*p=msg;
    int i;
    
    tracet(4,"strsvrstat:\n");
    
    for (i=0;i<svr->nstr;i++) {
        if (i==0) {
            strsum(svr->stream,byte,bps,NULL,NULL);
            stat[i]=strstat(svr->stream,s);
        }
        else {
            strsum(svr->stream+i,NULL,NULL,byte+i,bps+i);
            stat[i]=strstat(svr->stream+i,s);
        }
        if (*s) p+=sprintf(p,"(%d) %s ",i,s);
    }
}
/* peek input/output stream ----------------------------------------------------
* peek input/output stream of stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          unsigend char *msg O stream buff
*          int    nmax      I  buffer size (bytes)
* return : stream size (bytes)
*-----------------------------------------------------------------------------*/
extern int strsvrpeek(strsvr_t *svr, unsigned char *buff, int nmax)
{
    int n;
    
    if (!svr->state) return 0;
    
    lock(&svr->lock);
    n=svr->npb<nmax?svr->npb:nmax;
    if (n>0) {
        memcpy(buff,svr->pbuf,n);
    }
    if (n<svr->npb) {
        memmove(svr->pbuf,svr->pbuf+n,svr->npb-n);
    }
    svr->npb-=n;
    unlock(&svr->lock);
    return n;
}
