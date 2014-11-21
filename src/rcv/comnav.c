/*------------------------------------------------------------------------------
* comnav.c : ComNav receiver functions
*
*          Copyright (C) 2014-2014 by YT.TIAN, All rights reserved.
*
* reference :
*     [1] ComNav, OEM Card Reference Manual 1.3, 2014
*
* version : $Revision: 1.0 $ $Date: 2014/11/20 14:00:00 $
* history : 2014/11/20 1.0 new
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id: comnav.c,v 1.0 2014/11/20 14:00:00 TYT Exp $";

#define SYNC1   0xAA        /* comnav message start sync code 1 */
#define SYNC2   0x44        /* comnav message start sync code 2 */
#define SYNC3   0x12        /* comnav message start sync code 3 */

#define HLEN    28          /* comnav message header length (bytes) */

#define ID_RANGECMP 140         /* message id: comnav range compressed */
#define ID_RAWEPHEM 41          /* message id: comnav raw ephemeris */
#define ID_GPSBDSEPHEM 71       /* message id: comnav decoded gps/bds ephemeris */
#define ID_GLOEPHEMERIS 723     /* message id: comnav glonass ephemeris */

#define MAXVAL      8388608.0

#define OFF_FRQNO   -7          /* */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}

/* extend sign ---------------------------------------------------------------*/
static int exsign(unsigned int v, int bits)
{
    return (int)(v&(1<<(bits-1))?v|(~0u<<bits):v);
}

/* get observation data index ------------------------------------------------*/
static int obsindex(obs_t *obs, gtime_t time, int sat)
{
    int i,j;
    
    if (obs->n>=MAXOBS) return -1;
    for (i=0;i<obs->n;i++) {
        if (obs->data[i].sat==sat) return i;
    }
    obs->data[i].time=time;
    obs->data[i].sat=sat;
    for (j=0;j<NFREQ+NEXOBS;j++) {
        obs->data[i].L[j]=obs->data[i].P[j]=0.0;
        obs->data[i].D[j]=0.0;
        obs->data[i].SNR[j]=obs->data[i].LLI[j]=0;
        obs->data[i].code[j]=CODE_NONE;
    }
    obs->n++;
    return i;
}

/* decode comnav tracking status -------------------------------------------------
* deocode comnav tracking status
* args   : unsigned int stat I  tracking status field
*          int    *sys   O      system (SYS_???)
*          int    *code  O      signal code (CODE_L??)
*          int    *track O      tracking state
*                         (comnav)
*                         0=idle                      7=freq-lock loop
*                         2=wide freq band pull-in    9=channel alignment
*                         3=narrow freq band pull-in 10=code search
*                         4=phase lock loop          11=aided phase lock loop
*          int    *plock O      phase-lock flag   (0=not locked, 1=locked)
*          int    *clock O      code-lock flag    (0=not locked, 1=locked)
*          int    *parity O     parity known flag (0=not known,  1=known)
*          int    *halfc O      phase measurement (0=half-cycle not added,
*                                                  1=added)
* return : signal frequency (0:L1/B1,1:L2/B2,2:L5/B3,3:L6,4:L7,5:L8,-1:error)
* notes  : refer [1]
*-----------------------------------------------------------------------------*/
static int decode_trackstat(unsigned int stat, int *sys, int *code, int *track,
                            int *plock, int *clock, int *parity, int *halfc)
{
    int satsys,sigtype,freq=0;
    
    *track  = stat&0x1F;
    *plock  = (stat>>10)&1;
    *parity = (stat>>11)&1;
    *clock  = (stat>>12)&1;
    satsys  = (stat>>16)&7;
    sigtype = (stat>>21)&0x1F;
    *halfc  = (stat>>28)&1;
    
    switch (satsys) {
        case 0: *sys = SYS_GPS; break;
        case 1: *sys = SYS_GLO; break;
        case 2: *sys = SYS_SBS; break;
        //case 3: *sys = SYS_GAL; break; // not defined
        case 4: *sys = SYS_CMP; break;
        default:
            trace(2, "comnav unknown system: sys=%d\n", satsys);
            return -1;
    }
    if (*sys == SYS_GPS) {
        switch (sigtype) {
            case  0: freq=0; *code=CODE_L1C; break; /* L1C/A */
            case  2: freq=2; *code=CODE_L5Q; break; /* L5Q */
            case  5: freq=1; *code=CODE_L2P; break; /* L2P */
            case  9: freq=1; *code=CODE_L2D; break; /* L2Pcodeless */
            case 17: freq=1; *code=CODE_L2C; break; /* L2C */
            default: freq=-1; break;
        }
    }
    else if (*sys==SYS_GLO) {
        switch (sigtype) {
            case  0: freq=0; *code=CODE_L1C; break; /* L1C/A */
            case  1: freq=1; *code=CODE_L2C; break; /* L2C/A */
            case  5: freq=1; *code=CODE_L2P; break; /* L2P */
            default: freq=-1; break;
        }
    }
    else if (*sys==SYS_CMP) {
        switch (sigtype) {
            case  0: freq=0; *code=CODE_L1I; break; /* B1 */
            case  2: freq=2; *code=CODE_L6I; break; /* B3 */
            case 17: freq=1; *code=CODE_L7I; break; /* B2 */
            default: freq=-1; break;
        }
    }
    else if (*sys==SYS_SBS) {
        switch (sigtype) {
            case  0: freq=0; *code=CODE_L1C; break; /* L1C/A */
            default: freq=-1; break;
        }
    }
    if (freq<0) {
        trace(2,"comnav signal type error: sys=%d sigtype=%d\n",*sys,sigtype);
        return -1;
    }
    return freq;
}

/* check code priority and return obs position -------------------------------*/
static int checkpri(const char *opt, int sys, int code, int freq)
{
    int nex=NEXOBS; /* number of extended obs data */
    
    if (sys==SYS_GPS) {
        if (strstr(opt,"-GL1P")&&freq==0) return code==CODE_L1P?0:-1;
        if (strstr(opt,"-GL2X")&&freq==1) return code==CODE_L2X?1:-1;
        if (code==CODE_L1P) return nex<1?-1:NFREQ;
        if (code==CODE_L2X) return nex<2?-1:NFREQ+1;
    }
    else if (sys==SYS_GLO) {
        if (strstr(opt,"-RL2C")&&freq==1) return code==CODE_L2C?1:-1;
        if (code==CODE_L2C) return nex<1?-1:NFREQ;
    }
    else if (sys==SYS_GAL) {
        if (strstr(opt,"-EL1B")&&freq==0) return code==CODE_L1B?0:-1;
        if (code==CODE_L1B) return nex<1?-1:NFREQ;
        if (code==CODE_L7Q) return nex<2?-1:NFREQ+1;
        if (code==CODE_L8Q) return nex<3?-1:NFREQ+2;
    }
    return freq<NFREQ?freq:-1;
}

/* decode rangecmpb ----------------------------------------------------------*/
static int decode_rangecmpb(raw_t *raw)
{
    double psr,adr,adr_rolls,lockt,tt,dop,snr,wavelen;
    int i,index,nobs,prn,sat,sys,code,freq,pos;
    int track,plock,clock,parity,halfc,lli;
    char *msg;
    unsigned char *p = raw->buff + HLEN;
    
    trace(3, "decode_rangecmpb: len=%d\n", raw->len);
    
    nobs = U4(p);
    
    if (raw->outtype) {
        msg = raw->msgtype + strlen(raw->msgtype);
        sprintf(msg, " nobs=%2d", nobs);
    }
    if (raw->len < HLEN+4+nobs*24) {
        trace(2, "comnav rangecmpb length error: len=%d nobs=%d\n", raw->len,nobs);
        return -1;
    }
    for (i=0,p+=4; i<nobs; i++,p+=24) {
        
        /* decode tracking status */
        if ((freq = decode_trackstat(U4(p),&sys,&code,&track,&plock,&clock,
                                   &parity,&halfc)) < 0) continue;
        
        /* obs position */
        if ((pos = checkpri(raw->opt,sys,code,freq)) < 0) continue;
        
        prn = U1(p+17);
        if      (sys == SYS_GLO) prn -= 37;
        else if (sys == SYS_CMP) prn -= 140;
        
        if (!(sat = satno(sys, prn))) {
            trace(3, "comnav rangecmpb satellite number error: sys=%d,prn=%d\n", sys, prn);
            continue;
        }
        if (sys==SYS_GLO && !parity) continue; /* invalid if GLO parity unknown */
        
        dop = exsign(U4(p+4)&0xFFFFFFF,28)/256.0;
        psr = (U4(p+7)>>4)/128.0 + U1(p+11)*2097152.0;
        
        if ((wavelen=satwavelen(sat,freq,&raw->nav)) <= 0.0) {
            if (sys==SYS_GLO) wavelen = CLIGHT/(freq==0?FREQ1_GLO:FREQ2_GLO);
            else wavelen = lam_carr[freq];
        }
        adr = I4(p+12)/256.0;
        adr_rolls = (psr/wavelen+adr)/MAXVAL;
        adr = -adr + MAXVAL*floor(adr_rolls+(adr_rolls<=0?-0.5:0.5));
        
        lockt = (U4(p+18)&0x1FFFFF)/32.0; /* lock time */
        
        tt = timediff(raw->time, raw->tobs);
        if (raw->tobs.time != 0) {
            lli = (lockt<65535.968 && lockt-raw->lockt[sat-1][pos]+0.05<=tt) ||
                halfc != raw->halfc[sat-1][pos];
        }
        else {
            lli = 0;
        }
        if (!parity) lli |= 2;
        raw->lockt[sat-1][pos] = lockt;
        raw->halfc[sat-1][pos] = halfc;
        
        snr = ((U2(p+20)&0x3FF)>>5) + 20.0;
        if (!clock) psr = 0.0;     /* code unlock */
        if (!plock) adr = dop = 0.0; /* phase unlock */
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time)) > 1E-9) {
            raw->obs.n = 0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat)) >= 0) {
            raw->obs.data[index].L  [pos] = adr;
            raw->obs.data[index].P  [pos] = psr;
            raw->obs.data[index].D  [pos] = (float)dop;
            raw->obs.data[index].SNR[pos] =
                0.0<=snr&&snr<255.0 ? (unsigned char)(snr*4.0+0.5) : 0;
            raw->obs.data[index].LLI[pos] = (unsigned char)lli;
            raw->obs.data[index].code[pos] = code;
#if 0
            /* L2C phase shift correction (L2C->L2P) */
            if (code==CODE_L2X) {
                raw->obs.data[index].L[pos]+=0.25;
                trace(3,"comnav L2C phase shift corrected: prn=%2d\n",prn);
            }
#endif
        }
    }
    raw->tobs = raw->time;
    return 1;
}

/* decode rawephemb ----------------------------------------------------------*/
static int decode_rawephemb(raw_t *raw)
{
    unsigned char *p = raw->buff + HLEN;
    eph_t eph = {0};
    int prn, sat;
    
    trace(3, "decode_rawephemb: len=%d\n", raw->len);
    
    if (raw->len < HLEN+102) {
        trace(2, "comnav rawephemb length error: len=%d\n", raw->len);
        return -1;
    }
    prn = U4(p);
    if (!(sat = satno(SYS_GPS,prn))) {
        trace(2, "comnav rawephemb satellite number error: prn=%d\n", prn);
        return -1;
    }
    if (decode_frame(p+ 12, &eph, NULL, NULL, NULL, NULL) !=1 ||
        decode_frame(p+ 42, &eph, NULL, NULL, NULL, NULL) !=2 ||
        decode_frame(p+ 72, &eph, NULL, NULL, NULL, NULL) !=3 ) {
        trace(2, "comnav rawephemb subframe error: prn=%d\n", prn);
        return -1;
    }
    if (!strstr(raw->opt, "-EPHALL")) {
        if (eph.iode == raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat = sat;
    raw->nav.eph[sat-1] = eph;
    raw->ephsat = sat;
    trace(4, "decode_rawephemb: sat=%2d\n", sat);
    return 2;
}

/* decode gpsbdsephemb -------------------------------------------------------*/
static int decode_gpsbdsephemb(raw_t *raw)
{
    eph_t eph = {0};
    unsigned char *p = raw->buff + HLEN;
    double sqrtA;
    char *msg;
    int prn, data_size;
    short ura;
    unsigned char sate_id, sate_health, data_valid, iono_valid;
    double toc;
    bool is_gps;
    
    trace(3, "decode_gpsbdsephemb: len=%d\n", raw->len);
    
    if (raw->len < HLEN+264) {
        trace(2, "comnav gpsephemb length error: len=%d\n", raw->len);
        return -1;
    }
    
    data_size   = U2(p);    p += 2;
    data_valid  = U1(p);    p += 1;
    sate_health = U1(p);    p += 1;
    sate_id     = U1(p);    p += 1;
    iono_valid  = U1(p);    p += 1;
    /* skip 4 bytes */      p += 4;
    eph.iodc    = I2(p);    p += 2; /* AODC */
    eph.sva     = I2(p);    p += 2;
    eph.week    = U2(p);    p += 2;
    eph.iode    = I4(p);    p += 4; /* AODE */
    /* skip 4 bytes */      p += 4;
    eph.toes    = R8(p);    p += 8;
    toc         = R8(p);    p += 8;
    eph.f2      = R8(p);    p += 8;
    eph.f1      = R8(p);    p += 8;
    eph.f0      = R8(p);    p += 8;
    eph.M0      = R8(p);    p += 8;
    eph.deln    = R8(p);    p += 8;
    eph.e       = R8(p);    p += 8;
    sqrtA       = R8(p);    p += 8;
    eph.OMG0    = R8(p);    p += 8;
    eph.i0      = R8(p);    p += 8;
    eph.omg     = R8(p);    p += 8;
    eph.OMGd    = R8(p);    p += 8;
    eph.idot    = R8(p);    p += 8;
    eph.cuc     = R8(p);    p += 8;
    eph.cus     = R8(p);    p += 8;
    eph.crc     = R8(p);    p += 8;
    eph.crs     = R8(p);    p += 8;
    eph.cic     = R8(p);    p += 8;
    eph.cis     = R8(p);    p += 8;
    eph.tgd[0]  = R8(p);    p += 8; /* TGD1 for B1 (s) */
    eph.tgd[1]  = R8(p);    p += 8; /* TGD2 for B2 (s) */
    
    eph.svh     = sate_health;
    eph.A       = sqrtA*sqrtA;
    
    if (1 <= sate_id && sate_id <= 32) {            // GPS 1 ~ 32
        is_gps = true;
        prn = sate_id;
        eph.toe = gpst2time(eph.week, eph.toes);
        eph.toc = gpst2time(eph.week, toc);
    } else if (141 <= sate_id && sate_id <= 177) {  // BDS 141 ~ 177
        is_gps = false;
        prn = sate_id - 140;
        eph.toe = bdt2gpst(bdt2time(eph.week, eph.toes)); /* bdt -> gpst */
        eph.toc = bdt2gpst(bdt2time(eph.week, toc));      /* bdt -> gpst */
    }
    
    if (!(eph.sat=satno((is_gps?SYS_GPS:SYS_CMP), prn))) {
        trace(2, "comnav gpsephemb satellite error: prn=%d\n", prn);
        return -1;
    }
    
    if (raw->outtype) {
        msg = raw->msgtype + strlen(raw->msgtype);
        sprintf(msg, " prn=%3d iod=%3d toes=%6.0f", prn, eph.iode, eph.toes);
    }
    
    eph.ttr = raw->time;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(raw->nav.eph[eph.sat-1].toe,eph.toe)==0.0) return 0; /* unchanged */
    }
    raw->nav.eph[eph.sat-1] = eph;
    raw->ephsat = eph.sat;
    return 2;
}

/* decode gloephemerisb ------------------------------------------------------*/
static int decode_gloephemerisb(raw_t *raw)
{
    unsigned char *p = raw->buff + HLEN;
    geph_t geph = {0};
    char *msg;
    double tow, tof, toff;
    int prn, sat, week;
    
    trace(3, "decode_gloephemerisb: len=%d\n", raw->len);
    
    if (raw->len < HLEN+144) {
        trace(2, "comnav gloephemerisb length error: len=%d\n", raw->len);
        return -1;
    }
    prn         = U2(p) - 37;
    
    if (raw->outtype) {
        msg = raw->msgtype + strlen(raw->msgtype);
        sprintf(msg, " prn=%3d", prn);
    }
    if (!(sat = satno(SYS_GLO,prn))) {
        trace(2, "comnav gloephemerisb prn error: prn=%d\n", prn);
        return -1;
    }
    
    geph.frq    = U2(p+  2) + OFF_FRQNO;
    week        = U2(p+  6);
    tow         = floor(U4(p+8)/1000.0+0.5); /* rounded to integer sec */
    toff        = U4(p+ 12);
    geph.iode   = U4(p+ 20)&0x7F;
    geph.svh    = U4(p+ 24);
    geph.pos[0] = R8(p+ 28);
    geph.pos[1] = R8(p+ 36);
    geph.pos[2] = R8(p+ 44);
    geph.vel[0] = R8(p+ 52);
    geph.vel[1] = R8(p+ 60);
    geph.vel[2] = R8(p+ 68);
    geph.acc[0] = R8(p+ 76);
    geph.acc[1] = R8(p+ 84);
    geph.acc[2] = R8(p+ 92);
    geph.taun   = R8(p+100);
    geph.gamn   = R8(p+116);
    tof         = U4(p+124)-toff; /* glonasst->gpst */
    geph.age    = U4(p+136);
    geph.toe = gpst2time(week,tow);
    tof += floor(tow/86400.0)*86400;
    if      (tof<tow-43200.0) tof += 86400.0;
    else if (tof>tow+43200.0) tof -= 86400.0;
    geph.tof = gpst2time(week,tof);
    
    if (!strstr(raw->opt, "-EPHALL")) {
        if (fabs(timediff(geph.toe,raw->nav.geph[prn-1].toe))<1.0&&
            geph.svh==raw->nav.geph[prn-1].svh) return 0; /* unchanged */
    }
    
    geph.sat = sat;
    raw->nav.geph[prn-1] = geph;
    raw->ephsat = sat;
    return 2;
}

/* decode comnav message -----------------------------------------------------*/
static int decode_comnav(raw_t *raw)
{
    double tow;
    int msg,week,type=U2(raw->buff+4);
    
    trace(3,"decode_comnav: type=%3d len=%d\n",type,raw->len);
    
    /* check crc32 */
    if (crc32(raw->buff,raw->len)!=U4(raw->buff+raw->len)) {
        trace(2,"comnav crc error: type=%3d len=%d\n",type,raw->len);
        return -1;
    }
    msg =(U1(raw->buff+6)>>4)&0x3;
    week=adjgpsweek(U2(raw->buff+14));
    tow =U4(raw->buff+16)*0.001;
    raw->time=gpst2time(week,tow);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"ComNav %4d (%4d): msg=%d %s",type,raw->len,msg,
                time_str(gpst2time(week,tow),2));
    }
    if (msg!=0) return 0; /* message type: 0=binary,1=ascii */
    
    switch (type) {
        case ID_RANGECMP     : return decode_rangecmpb     (raw);
        case ID_RAWEPHEM     : return decode_rawephemb     (raw);
        case ID_GPSBDSEPHEM  : return decode_gpsbdsephemb  (raw);
        case ID_GLOEPHEMERIS : return decode_gloephemerisb (raw);
    }
    return 0;
}

/* sync header ---------------------------------------------------------------*/
static int sync_comnav(unsigned char *buff, unsigned char data)
{
    buff[0] = buff[1]; buff[1] = buff[2]; buff[2] = data;
    return buff[0] == SYNC1 && buff[1] == SYNC2 && buff[2] == SYNC3;
}

/* input comnav raw data from stream -------------------------------------------
* fetch next comnav raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options for comnav, set raw->opt to the following
*          option strings separated by spaces.
*
*          -EPHALL : input all ephemerides
*          -GL1P   : select 1P for GPS L1 (default 1C)
*          -GL2X   : select 2X for GPS L2 (default 2W)
*          -RL2C   : select 2C for GLO L2 (default 2P)
*          -EL2C   : select 2C for GAL L2 (default 2C)
*
*-----------------------------------------------------------------------------*/
extern int input_cnb(raw_t *raw, unsigned char data)
{
    trace(5,"input_comnav: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (sync_comnav(raw->buff,data)) raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==10&&(raw->len=U2(raw->buff+8)+HLEN)>MAXRAWLEN-4) {
        trace(2,"comnav length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<10||raw->nbyte<raw->len+4) return 0;
    raw->nbyte=0;
    
    /* decode comnav message */
    return decode_comnav(raw);
}

/* input comnav raw data from file ---------------------------------------------
* fetch next comnav raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          int    format I      receiver raw data format (STRFMT_???)
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_cnbf(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_comnavf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_comnav(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,7,1,fp)<1) return -2;
    raw->nbyte=10;
    
    if ((raw->len=U2(raw->buff+8)+HLEN)>MAXRAWLEN-4) {
        trace(2,"comnav length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+10,raw->len-6,1,fp)<1) return -2;
    raw->nbyte=0;
    
    /* decode comnav message */
    return decode_comnav(raw);
}
