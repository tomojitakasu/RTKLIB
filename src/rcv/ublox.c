/*------------------------------------------------------------------------------
* ublox.c : ublox receiver dependent functions
*
*          Copyright (C) 2007-2013 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] ublox-AG, GPS.G3-X-03002-D, ANTARIS Positioning Engine NMEA and UBX
*         Protocol Specification, Version 5.00, 2003
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2007/10/08 1.0 new
*           2008/06/16 1.1 separate common functions to rcvcmn.c
*           2009/04/01 1.2 add range check of prn number
*           2009/04/10 1.3 refactored
*           2009/09/25 1.4 add function gen_ubx()
*           2010/01/17 1.5 add time tag adjustment option -tadj sec
*           2010/10/31 1.6 fix bug on playback disabled for raw data (2.4.0_p9)
*           2011/05/27 1.7 add almanac decoding
*                          add -EPHALL option
*                          fix problem with ARM compiler
*           2013/02/23 1.8 fix memory access violation problem on arm
*                          change options -tadj to -TADJ, -invcp to -INVCP
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define UBXSYNC1    0xB5        /* ubx message sync code 1 */
#define UBXSYNC2    0x62        /* ubx message sync code 2 */
#define UBXCFG      0x06        /* ubx message cfg-??? */

#define ID_RXMRAW   0x0210      /* ubx message id: raw measurement data */
#define ID_RXMSFRB  0x0211      /* ubx message id: subframe buffer */
#define FU1         1           /* ubx message field types */
#define FU2         2
#define FU4         3
#define FI1         4
#define FI2         5
#define FI4         6
#define FR4         7
#define FR8         8
#define FS32        9

static const char rcsid[]="$Id: ublox.c,v 1.2 2008/07/14 00:05:05 TTAKA Exp $";

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

/* set fields (little-endian) ------------------------------------------------*/
static void setU1(unsigned char *p, unsigned char  u) {*p=u;}
static void setU2(unsigned char *p, unsigned short u) {memcpy(p,&u,2);}
static void setU4(unsigned char *p, unsigned int   u) {memcpy(p,&u,4);}
static void setI1(unsigned char *p, char           i) {*p=(unsigned char)i;}
static void setI2(unsigned char *p, short          i) {memcpy(p,&i,2);}
static void setI4(unsigned char *p, int            i) {memcpy(p,&i,4);}
static void setR4(unsigned char *p, float          r) {memcpy(p,&r,4);}
static void setR8(unsigned char *p, double         r) {memcpy(p,&r,8);}

/* checksum ------------------------------------------------------------------*/
static int checksum(unsigned char *buff, int len)
{
    unsigned char cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    return cka==buff[len-2]&&ckb==buff[len-1];
}
static void setcs(unsigned char *buff, int len)
{
    unsigned char cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    buff[len-2]=cka;
    buff[len-1]=ckb;
}
/* decode ublox rxm-raw: raw measurement data --------------------------------*/
static int decode_rxmraw(raw_t *raw)
{
    gtime_t time;
    double tow,tt,tadj=0.0,toff=0.0,tn;
    int i,j,prn,sat,n=0,nsat,week;
    unsigned char *p=raw->buff+6;
    char *q,tstr[32];
    
    trace(4,"decode_rxmraw: len=%d\n",raw->len);
    
    /* time tag adjustment option (-TADJ) */
    if ((q=strstr(raw->opt,"-TADJ"))) {
        sscanf(q,"-TADJ=%lf",&tadj);
    }
    tow =U4(p  );
    week=U2(p+4);
    nsat=U1(p+6);
    if (raw->len<12+24*nsat) {
        trace(2,"ubx rxmraw length error: len=%d nsat=%d\n",raw->len,nsat);
        return -1;
    }
    time=gpst2time(week,tow*0.001);
    
    /* time tag adjustment */
    if (tadj>0.0) {
        tn=time2gpst(time,&week)/tadj;
        toff=(tn-floor(tn+0.5))*tadj;
        time=timeadd(time,-toff);
    }
    if (fabs(tt=timediff(time,raw->time))<=1e-3) {
        time2str(time,tstr,3);
        trace(2,"ubx rxmraw time tag duplicated: time=%s\n",tstr);
        return 0;
    }
    for (i=0,p+=8;i<nsat&&i<MAXOBS;i++,p+=24) {
        raw->obs.data[n].time=time;
        raw->obs.data[n].L[0]  =R8(p   )-toff*FREQ1;
        raw->obs.data[n].P[0]  =R8(p+ 8)-toff*CLIGHT;
        raw->obs.data[n].D[0]  =R4(p+16);
        prn                    =U1(p+20);
        raw->obs.data[n].SNR[0]=(unsigned char)(I1(p+22)*4.0+0.5);
        raw->obs.data[n].LLI[0]=U1(p+23);
        raw->obs.data[n].code[0]=CODE_L1C;
        
        /* phase polarity flip option (-INVCP) */
        if (strstr(raw->opt,"-INVCP")) {
            raw->obs.data[n].L[0]=-raw->obs.data[n].L[0];
        }
        if (!(sat=satno(MINPRNSBS<=prn?SYS_SBS:SYS_GPS,prn))) {
            trace(2,"ubx rxmraw sat number error: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].sat=sat;
        
        if (raw->obs.data[n].LLI[0]&1) raw->lockt[sat-1][0]=0.0;
        else if (tt<0.0||10.0<tt) raw->lockt[sat-1][0]=0.0;
        else raw->lockt[sat-1][0]+=tt;
        
        for (j=1;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}
/* save subframe -------------------------------------------------------------*/
static int save_subfrm(int sat, raw_t *raw)
{
    unsigned char *p=raw->buff+6,*q;
    int i,j,n,id=(U4(p+6)>>2)&0x7;
    
    trace(4,"save_subfrm: sat=%2d id=%d\n",sat,id);
    
    if (id<1||5<id) return 0;
    
    q=raw->subfrm[sat-1]+(id-1)*30;
    
    for (i=n=0,p+=2;i<10;i++,p+=4) {
        for (j=23;j>=0;j--) {
            *q=(*q<<1)+((U4(p)>>j)&1); if (++n%8==0) q++;
        }
    }
    return id;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_ephem(int sat, raw_t *raw)
{
    eph_t eph={0};
    
    trace(4,"decode_ephem: sat=%2d\n",sat);
    
    if (decode_frame(raw->subfrm[sat-1]   ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(raw->subfrm[sat-1]+30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(raw->subfrm[sat-1]+60,&eph,NULL,NULL,NULL,NULL)!=3) return 0;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode almanac and ion/utc ------------------------------------------------*/
static int decode_alm1(int sat, raw_t *raw)
{
    trace(4,"decode_alm1 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+90,NULL,raw->nav.alm,raw->nav.ion_gps,
                 raw->nav.utc_gps,&raw->nav.leaps);
    return 0;
}
/* decode almanac ------------------------------------------------------------*/
static int decode_alm2(int sat, raw_t *raw)
{
    trace(4,"decode_alm2 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+120,NULL,raw->nav.alm,NULL,NULL,NULL);
    return  0;
}
/* decode ublox rxm-sfrb: subframe buffer ------------------------------------*/
static int decode_rxmsfrb(raw_t *raw)
{
    unsigned int words[10];
    int i,prn,sat,sys,id;
    unsigned char *p=raw->buff+6;
    
    trace(4,"decode_rxmsfrb: len=%d\n",raw->len);
    
    if (raw->len<42) {
        trace(2,"ubx rxmsfrb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U1(p+1);
    if (!(sat=satno(MINPRNSBS<=prn?SYS_SBS:SYS_GPS,prn))) {
        trace(2,"ubx rxmsfrb satellite number error: prn=%d\n",prn);
        return -1;
    }
    sys=satsys(sat,&prn);
    
    if (sys==SYS_GPS) {
        id=save_subfrm(sat,raw);
        if (id==3) return decode_ephem(sat,raw);
        if (id==4) return decode_alm1 (sat,raw);
        if (id==5) return decode_alm2 (sat,raw);
        return 0;
    }
    else if (sys==SYS_SBS) {
        for (i=0,p+=2;i<10;i++,p+=4) words[i]=U4(p);
        return sbsdecodemsg(raw->time,prn,words,&raw->sbsmsg)?3:0;
    }
    return 0;
}
/* decode ublox raw message --------------------------------------------------*/
static int decode_ubx(raw_t *raw)
{
    int type=(U1(raw->buff+2)<<8)+U1(raw->buff+3);
    
    trace(3,"decode_ubx: type=%04x len=%d\n",type,raw->len);
    
    /* checksum */
    if (!checksum(raw->buff,raw->len)) {
        trace(2,"ubx checksum error: type=%04x len=%d\n",type,raw->len);
        return -1;
    }
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX 0x%04X (%4d):",type,raw->len);
    }
    switch (type) {
        case ID_RXMRAW : return decode_rxmraw(raw);
        case ID_RXMSFRB: return decode_rxmsfrb(raw);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_ubx(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==UBXSYNC1&&buff[1]==UBXSYNC2;
}
/* input ublox raw message from stream -----------------------------------------
* fetch next ublox raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL    : input all ephemerides
*          -INVCP     : invert polarity of carrier-phase
*          -TADJ=tint : adjust time tags to multiples of tint (sec)
*
*-----------------------------------------------------------------------------*/
extern int input_ubx(raw_t *raw, unsigned char data)
{
    trace(5,"input_ubx: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_ubx(raw->buff,data)) return 0;
        raw->nbyte=2;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==6) {
        if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
            trace(2,"ubx length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
    if (raw->nbyte<6||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode ublox raw message */
    return decode_ubx(raw);
}
/* input ublox raw message from file -------------------------------------------
* fetch next ublox raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_ubxf(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_ubxf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_ubx(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+2,1,4,fp)<4) return -2;
    raw->nbyte=6;
    
    if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
        trace(2,"ubx length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+6,1,raw->len-6,fp)<(size_t)(raw->len-6)) return -2;
    raw->nbyte=0;
    
    /* decode ubx raw message */
    return decode_ubx(raw);
}
/* generate ublox binary message -----------------------------------------------
* generate ublox binary message from message string
* args   : char  *msg   IO     message string 
*            "CFG-PRT   portid res0 res1 mode baudrate inmask outmask flags"
*            "CFG-USB   vendid prodid res1 res2 power flags vstr pstr serino"
*            "CFG-MSG   msgid rate0 rate1 rate2 rate3"
*            "CFG-NMEA  filter version numsv flags"
*            "CFG-RATE  meas nav time"
*            "CFG-CFG   clear_mask save_mask load_mask"
*            "CFG-TP    interval length status time_ref res adelay rdelay udelay"
*            "CFG-NAV2  ..."
*            "CFG-DAT   maja flat dx dy dz rotx roty rotz scale"
*            "CFG-INF   protocolid res0 res1 mask0 mask1 mask2 mask3"
*            "CFG-RST   navbbr reset res"
*            "CFG-RXM   gpsmode lpmode"
*            "CFG-ANT   flags pins"
*            "CFG-FXN   flags treacq tacq treacqoff tacqoff ton toff res basetow"
*            "CFG-SBAS  mode usage maxsbas res scanmode"
*            "CFG-LIC   key0 key1 key2 key3 key4 key5"
*            "CFG-TM    intid rate flags"
*            "CFG-TM2   ch res0 res1 rate flags"
*            "CFG-TMODE tmode posx posy posz posvar svinmindur svinvarlimit"
*            "CFG-EKF   ..."
*          unsigned char *buff O binary message
* return : length of binary message (0: error)
* note   : see reference [1] for details.
*-----------------------------------------------------------------------------*/
extern int gen_ubx(const char *msg, unsigned char *buff)
{
    const char *cmd[]={
        "PRT","USB","MSG","NMEA","RATE","CFG","TP","NAV2","DAT","INF",
        "RST","RXM","ANT","FXN","SBAS","LIC","TM","TM2","TMODE","EKF",""
    };
    const unsigned char id[]={
        0x00,0x1B,0x01,0x17,0x08,0x09,0x07,0x1A,0x06,0x02,
        0x04,0x11,0x13,0x0E,0x16,0x80,0x10,0x19,0x1D,0x12
    };
    const int prm[][32]={
        {FU1,FU1,FU2,FU4,FU4,FU2,FU2,FU2,FU2},    /* PRT */
        {FU2,FU2,FU2,FU2,FU2,FU2,FS32,FS32,FS32}, /* USB */
        {FU1,FU1,FU1,FU1,FU1,FU1},                /* MSG */
        {FU1,FU1,FU1,FU1},                        /* NMEA */
        {FU2,FU2,FU2},                            /* RATE */
        {FU4,FU4,FU4},                            /* CFG */
        {FU4,FU4,FI1,FU1,FU2,FI2,FI2,FI4},        /* TP */
        {FU1,FU1,FU2,FU1,FU1,FU1,FU1,FI4,FU1,FU1,FU1,FU1,FU1,FU1,FU2,FU2,FU2,FU2,
         FU2,FU1,FU1,FU2,FU4,FU4},                /* NAV2 */
        {FR8,FR8,FR4,FR4,FR4,FR4,FR4,FR4,FR4},    /* DAT */
        {FU1,FU1,FU2,FU1,FU1,FU1,FU1},            /* INF */
        {FU2,FU1,FU1},                            /* RST */
        {FU1,FU1},                                /* RXM */
        {FU2,FU2},                                /* ANT */
        {FU4,FU4,FU4,FU4,FU4,FU4,FU4,FU4},        /* FXN */
        {FU1,FU1,FU1,FU1,FU4},                    /* SBAS */
        {FU2,FU2,FU2,FU2,FU2,FU2},                /* LIC */
        {FU4,FU4,FU4},                            /* TM */
        {FU1,FU1,FU2,FU4,FU4},                    /* TM2 */
        {FU4,FI4,FI4,FI4,FU4,FU4,FU4},            /* TMODE */
        {FU1,FU1,FU1,FU1,FU4,FU2,FU2,FU1,FU1,FU2} /* EKF */
    };
    unsigned char *q=buff;
    char mbuff[1024],*args[32],*p;
    int i,j,n,narg=0;
    
    trace(4,"gen_ubxf: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    if (narg<1||strncmp(args[0],"CFG-",4)) return 0;
    
    for (i=0;*cmd[i];i++) {
        if (!strcmp(args[0]+4,cmd[i])) break;
    }
    if (!*cmd[i]) return 0;
    
    *q++=UBXSYNC1;
    *q++=UBXSYNC2;
    *q++=UBXCFG;
    *q++=id[i];
    q+=2;
    for (j=1;prm[i][j-1]>0;j++) {
        switch (prm[i][j-1]) {
            case FU1 : setU1(q,j<narg?(unsigned char )atoi(args[j]):0); q+=1; break;
            case FU2 : setU2(q,j<narg?(unsigned short)atoi(args[j]):0); q+=2; break;
            case FU4 : setU4(q,j<narg?(unsigned int  )atoi(args[j]):0); q+=4; break;
            case FI1 : setI1(q,j<narg?(char          )atoi(args[j]):0); q+=1; break;
            case FI2 : setI2(q,j<narg?(short         )atoi(args[j]):0); q+=2; break;
            case FI4 : setI4(q,j<narg?(int           )atoi(args[j]):0); q+=4; break;
            case FR4 : setR4(q,j<narg?(float         )atof(args[j]):0); q+=4; break;
            case FR8 : setR8(q,j<narg?(double)atof(args[j]):0); q+=8; break;
            case FS32: sprintf((char *)q,"%-32.32s",j<narg?args[j]:""); q+=32; break;
        }
    }
    n=(int)(q-buff)+2;
    setU2(buff+4,(unsigned short)(n-8));
    setcs(buff,n);
    
    trace(5,"gen_ubxf: buff=\n"); traceb(5,buff,n);
    return n;
}
