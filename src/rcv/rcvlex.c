/*------------------------------------------------------------------------------
* rcvlex.c : qzss lex receiver dependent functions
*
*          Copyright (C) 2011 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] LEX signal receiver (LPY-10000) protocol specification, Furuno Denki,
*         2010
*     [2] RFC 2083, PNG (Portable Network Graphics) Specification version 1.0,
*         March, 1997
*
* version : $Revision:$ $Date:$
* history : 2011/05/27 1.0 new
*           2013/06/02 1.1 fix bug on unable compile
*           2014/10/26 1.2 suppress warning on type-punning pointer
*           2017/04/11 1.3 (char *) -> (signed char *)
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#ifdef EXTLEX /* lex extention */

#define LEXFRMPREAMB 0x1ACFFC1Du /* lex message frame preamble */

#define LEXRSYNC1   0xAA        /* lex receiver message sync code 1 */
#define LEXRSYNC2   0x55        /* lex receiver message sync code 2 */

#define MAXLEXRLEN  8192        /* max length of lex receiver message */

#define ID_LEXRAW   0x0002      /* lex receiver message id: raw measurement */
#define ID_LEXMSG   0x0015      /* lex receiver message id: lex message */

/* extract field (big-endian) ------------------------------------------------*/
#define U1(p)       (*((unsigned char *)(p)))
#define I1(p)       (*((signed char *)(p)))

static unsigned short U2(unsigned char *p)
{
    union {unsigned short u2; unsigned char b[2];} buff;
    buff.b[0]=p[1]; buff.b[1]=p[0];
    return buff.u2;
}
static unsigned int U4(unsigned char *p)
{
    union {unsigned int u4; unsigned char b[4];} buff;
    buff.b[0]=p[3]; buff.b[1]=p[2]; buff.b[2]=p[1]; buff.b[3]=p[0];
    return buff.u4;
}
static double R8(unsigned char *p)
{
    union {double r8; unsigned char b[8];} buff;
    buff.b[0]=p[7]; buff.b[1]=p[6]; buff.b[2]=p[5]; buff.b[3]=p[4];
    buff.b[4]=p[3]; buff.b[5]=p[2]; buff.b[6]=p[1]; buff.b[7]=p[0];
    return buff.r8;
}
/* crc-32 parity (ref [2] 15) ------------------------------------------------*/
static unsigned int crc32r(const unsigned char *buff, int len)
{
    static unsigned int crcs[256]={0};
    unsigned int crc;
    int i,j;
    
    if (!crcs[1]) {
        for (i=0;i<256;i++) {
            crc=(unsigned int)i;
            for (j=0;j<8;j++) {
                if (crc&1) crc=(crc>>1)^0xEDB88320u; else crc>>=1;
            }
            crcs[i]=crc;
        }
    }
    for (crc=0xFFFFFFFFu,i=0;i<len;i++) {
        crc=(crc>>8)^crcs[(int)(crc^buff[i])&255];
    }
    return crc^0xFFFFFFFFu;
}
/* decode raw measurement ----------------------------------------------------*/
static int decode_lexraw(raw_t *raw)
{
    unsigned int ttt;
    unsigned short cn0,acc;
    unsigned char *p=raw->buff+16,lli;
    double clk,pr,dop,adr;
    int i,j,ncpu,cpuid,satid,nsig,type,node,prn,stat,sat,n=0;
    
    ncpu =U1(p);       p+=1;
    cpuid=U1(p);       p+=1;
    clk  =R8(p)*0.001; p+=8;
    
    trace(3,"decode_lexraw: len=%d ncpu=%d cpuid=%d clk=%.3f\n",raw->len,ncpu,
          cpuid,clk);
    
    for (i=0;i<17&&p+38<=raw->buff+raw->len-4;i++) {
        satid=U1(p); p+=1; /* satellite id */
        nsig =U1(p); p+=1; /* number of signals */
        type =U1(p); p+=1; /* singnal type */
                           /* (0x1A:lex short,0x1B:lex long,0x1C:combined) */
        node =U1(p); p+=1; /* cpu board node id */
        prn  =U1(p); p+=1; /* prn code number (193-197) */
        stat =U1(p); p+=1; /* tracking status (b0-b3) */
                           /* (0:waiting,1:searching,2:freq-tracking */
                           /*  3:phase-tracking,4:phase-lock amb not resolved */
                           /*  5:phase-lock,amb resolved,9:noise measured) */
        cn0  =U2(p); p+=2; /* C/N0 (0.01 dBHz) */
        pr   =R8(p); p+=8; /* pseudorange (m) */
        dop  =R8(p); p+=8; /* doppler frequency (Hz) */
        adr  =R8(p); p+=8; /* accumlated delta range (cycle) */
        acc  =U2(p); p+=2; /* accurated time of delta range (ms) */
        ttt  =U4(p); p+=4; /* tracking time (ms) */
        
        if (stat<2||5<stat) continue;
        
        if (raw->lexmsg.prn==prn) {
            raw->lexmsg.stat=stat;
            raw->lexmsg.snr=(unsigned char)(cn0*0.04+0.5);
            raw->lexmsg.ttt=ttt;
        }
        trace(4,"satid=%3d nsig=%d type=0x%02X node=%d prn=%3d stat=%d\n",
              satid,nsig,type,node,prn,stat);
        trace(4,"cn0=%4.1f pr=%13.3f dop=%8.3f adr=%13.3f acc=%6.3f ttt=%9.3f\n",
              cn0*0.01,pr,dop,adr,acc,ttt*0.001);
        
        if (!(sat=satno(SYS_QZS,prn))) {
            trace(2,"lexraw sat number error: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].time=raw->time;
        raw->obs.data[n].sat=sat;
        
        for (j=0;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
            
            if (j!=3) continue;
            
            lli=0;
            if (ttt<=0.0||ttt*0.001<raw->lockt[sat-1][3]) {
                trace(2,"lexraw loss of lock: t=%s ttt=%6.0f->%6.0f\n",
                      time_str(raw->time,3),raw->lockt[sat-1][3],ttt*0.001);
                lli=1;
            }
            raw->lockt[sat-1][3]=ttt*0.001;
            lli|=stat<=4?2:0;
            
            raw->obs.data[n].P[3]=pr;
            raw->obs.data[n].L[3]=adr;
            raw->obs.data[n].D[3]=dop;
            raw->obs.data[n].SNR[3]=(unsigned char)(cn0*0.04+0.5);
            raw->obs.data[n].LLI[3]=lli;
            raw->obs.data[n].code[3]=
                type==0x1A?CODE_L6S:(type==0x1B?CODE_L6L:CODE_NONE);
            n++;
        }
    }
    raw->obs.n=n;
    return 1;
}
/* decode lex message --------------------------------------------------------*/
static int decode_lexmsg(raw_t *raw)
{
    lexmsg_t msg={0};
    unsigned int preamb;
    int i,j,sat,ch,sig,prn,err,prnmsg,type,alert;
    unsigned char *p=raw->buff+16;
    
    if (raw->len<276) {
        trace(2,"lexr lexmsg length error: len=%d\n",raw->len);
        return -1;
    }
    sat =U1(p); p+=1; /* satellite id */
    ch  =U1(p); p+=1; /* channel number */
    sig =U1(p); p+=1; /* signal type (0x1A:lex short) */
    prn =U1(p); p+=1; /* prn number */
    err =U2(p); p+=2; /* err status */
    
    trace(3,"decode_lexmsg: len=%d sat=%d ch=%d sig=%d prn=%d err=%d\n",
          raw->len,sat,ch,sig,prn,err);
    
    if (err&1) {
        trace(2,"lex message decode error: sat=%d ch=%d prn=%d err=%02X\n",
              sat,ch,prn,err);
        return -1;
    }
    /* decode lex message header (49 bytes) */
    i=0;
    preamb=getbitu(p,i,32); i+=32; /* preamble */
    prnmsg=getbitu(p,i, 8); i+= 8; /* prn number */
    type  =getbitu(p,i, 8); i+= 8; /* lex message type */
    alert =getbitu(p,i, 1); i+= 1; /* alert flag */
    
    if (preamb!=LEXFRMPREAMB) {
        trace(2,"lex message preamble error: preamb=%08X\n",preamb);
        return -1;
    }
    if (prn!=prnmsg) {
        trace(2,"lex message prn inconsistent: prn=%d %d\n",prn,prnmsg);
        return -1;
    }
    raw->lexmsg.prn=prn;
    raw->lexmsg.type=type;
    raw->lexmsg.alert=alert;
    
    /* save data part (1695 bytes) */
    for (j=0;j<212;j++) {
        raw->lexmsg.msg[j]=(unsigned char)getbitu(p,i,8); i+=8;
    }
    raw->lexmsg.msg[211]&=0xFE;
    
    trace(4,"lexmsg: prn=%d type=%d aleart=%d\n",prn,type,alert);
    trace(4,"lexmsg: msg="); traceb(4,msg.msg,212);
    return 31;
}
/* decode lex raw message ---------------------------------------------------*/
static int decode_lexr(raw_t *raw)
{
    double tow;
    int stat,week,type=U2(raw->buff+2); /* message id */
    
    stat=U4(raw->buff+ 6); /* status */
            /*  b0    : timing valid (0:invalid,1:valid) */
            /*  b1    : timing auto steering (0:off,1:on) */
            /*  b2-3  : time status (00:inaccurate,01:approx,11:precise) */
            /*  b4    : 1pps synchronization (0:async,1:sync) */
            /*  b6    : position status (0:invalid,1:ok) */
            /*  b8-15 : position type (0x03:qzs single-freq) */
            /*  b28   : leap second insert (0:other,1:leap second insert) */
            /*  b31   : timing shift flag (0:other,1:timing shift) */
    tow =U4(raw->buff+10)/1000.0;
    week=U2(raw->buff+14);
    raw->time=gpst2time(week,tow);
    
    trace(3,"decode_lexr: type=%04X len=%3d stat=%08X time=%s\n",type,raw->len,
          stat,time_str(raw->time,3));
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"LEXR 0x%04X (%4d): stat=%08X week=%d tow=%10.3f",
                type,raw->len,stat,week,tow/1000.0);
    }
    switch (type) {
        case ID_LEXRAW: return decode_lexraw(raw);
        case ID_LEXMSG: return decode_lexmsg(raw);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_lexr(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==LEXRSYNC1&&buff[1]==LEXRSYNC2;
}
/* input lex receiver raw message from stream ----------------------------------
* fetch next lex receiver raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  31: input lex message)
*-----------------------------------------------------------------------------*/
extern int input_lexr(raw_t *raw, unsigned char data)
{
    trace(5,"input_lexr: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_lexr(raw->buff,data)) return 0;
        raw->nbyte=2;
        return 0;
    }
#if 0 /* omitted ver.2.4.1 */
    /* replpace 0xAAAA by 0xAA (ref [1] 4.1.4) */
    if (!raw->flag&&data==0xAA&&raw->buff[raw->nbyte-1]==0xAA) {
        trace(3,"replace 0xAAAA by 0xAA\n");
        raw->flag=1;
        return 0;
    }
    raw->flag=0;
#endif
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==6) {
        raw->len=U2(raw->buff+4);
        if (raw->len>MAXRAWLEN) {
            trace(2,"rcvlex message length error: len=%d\n",raw->len);
            raw->nbyte=0; raw->buff[0]=0;
            return -1;
        }
    }
    if (raw->nbyte<6||raw->nbyte<raw->len) return 0;
    
    /* check crc */
    if (crc32r(raw->buff,raw->len-4)!=U4(raw->buff+raw->len-4)) {
        trace(2,"rcvlex message crc error: len=%d\n",raw->len);
        raw->nbyte=0; raw->buff[0]=0;
        return -1;
    }
    raw->nbyte=0;
    
    /* decode lex receiver raw message */
    return decode_lexr(raw);
}
/* input lex receiver raw message from file ------------------------------------
* fetch next lex receiver raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_lexrf(raw_t *raw, FILE *fp)
{
    int i,data,ret;
    
    trace(4,"input_lexrf:\n");
    
    for (i=0;i<4096;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if ((ret=input_lexr(raw,(unsigned char)data))) return ret;
    }
    return 0; /* return at every 4k bytes */
}
/* generate lex receiver raw message ------------------------------------------*/
static int genmsg(unsigned char *buff, unsigned short id, unsigned char *data,
                  unsigned short len)
{
    unsigned char *p=buff;
    unsigned int crc;
    int i;
    len+=20;
    *p++=LEXRSYNC1;
    *p++=LEXRSYNC2;
    for (i=1;i>=0;i--) *p++=*((unsigned char *)&id +i);
    for (i=1;i>=0;i--) *p++=*((unsigned char *)&len+i);
    for (i=0;i<10;i++) *p++=0;
    for (i=0;i<len-20;i++) *p++=data[i];
    crc=crc32r(buff,len-4);
    for (i=3;i>=0;i--) *p++=*((unsigned char *)&crc+i);
    return (int)(p-buff);
}
/* generate lex receiver raw message -------------------------------------------
* generate lex receiver raw message from message string
* args   : char  *msg   IO     message string 
*            "CMD-RST [clear]"       : restart command
*            "CMD-REQ [msgid]"       : request message
*            "CMD-REQC"              : request of reference clock status
*            "CMD-CH sat1 sat2 sat3" : sv-channel assignment
*            "CMD-TIME [date time]"  : time input (default current time)
*                                      (date=YYYY/dd/mm, time=HH:MM:SS.SS)
*            "CMD-POS lat lon hgt"   : antenna position input
*            "CMD-BIT {on|off}"      : output-control of predecoding bit stream
*          unsigned char *buff O binary message
* return : length of binary message (0: error)
* note   : see ref [1] for details.
*-----------------------------------------------------------------------------*/
extern int gen_lexr(const char *msg, unsigned char *buff)
{
    double pos[3]={0},epoch[6];
    unsigned short id,ep[]={2010,1,1,0,0,0};
    char *args[32],mbuff[1024],*p;
    unsigned char data[32]={0},*q=data;
    int i,len,narg=0;
    
    trace(4,"gen_lexr: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    if (narg<1) return 0;
    
    if (!strcmp(args[0],"CMD-RST")) {
        if (narg>1&&!strcmp(args[1],"clear")) *q=3;
        return genmsg(buff,0x8000,data,1);
    }
    else if (!strcmp(args[0],"CMD-REQ")) {
        if (narg<2) return 0;
        id=(unsigned short)atoi(args[1])+0xC000;
        len=genmsg(buff,id,data,0);
    }
    else if (!strcmp(args[0],"CMD-REQC")) {
        len=genmsg(buff,0xC00F,data,0);
    }
    else if (!strcmp(args[0],"CMD-CH")) {
        if (narg>1) *q++=(unsigned char)atoi(args[1]);
        if (narg>2) *q++=(unsigned char)atoi(args[2]);
        if (narg>3) *q  =(unsigned char)atoi(args[3]);
        len=genmsg(buff,0x8020,data,16);
    }
    else if (!strcmp(args[0],"CMD-TIME")) {
        if (narg<2) {
            time2epoch(timeget(),epoch);
        }
        else {
            sscanf(args[1],"%lf/%lf/%lf",epoch  ,epoch+1,epoch+2);
            if (narg>2) sscanf(args[2],"%lf:%lf:%lf",epoch+3,epoch+4,epoch+5);
        }
        for (i=0;i<6;i++) ep[i]=(unsigned short)epoch[i];
        for (i=1;i>=0;i--) *q++=*((unsigned char *)&ep[0]+i);
        for (i=0;i<5;i++)  *q++=(unsigned char)ep[i+1];
        len=genmsg(buff,0x8030,data,7);
    }
    else if (!strcmp(args[0],"CMD-POS")) {
        for (i=0;i<3;i++) if (i+1<narg) pos[i]=atof(args[i+1])*(i<2?D2R:1.0);
        *q++=0;
        for (i=7;i>=0;i--) *q++=*((unsigned char *)&pos[0]+i);
        for (i=7;i>=0;i--) *q++=*((unsigned char *)&pos[1]+i);
        for (i=7;i>=0;i--) *q++=*((unsigned char *)&pos[2]+i);
        len=genmsg(buff,0x8032,data,25);
    }
    else if (!strcmp(args[0],"CMD-BITS")) {
        *q=narg>=2&&!strcmp(args[1],"on")?1:0;
        len=genmsg(buff,0x8052,data,1);
    }
    else {
        trace(2,"unknown lexr command: msg=%s\n",args[0]);
        return 0;
    }
    trace(5,"gen_lexr: buff=\n"); traceb(5,buff,len);
    return len;
}
#endif /* EXTLEX */
