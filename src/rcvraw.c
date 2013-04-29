/*------------------------------------------------------------------------------
* rcvraw.c : receiver raw data functions
*
*          Copyright (C) 2009-2013 by T.TAKASU, All rights reserved.
*
* references :
*     [1] IS-GPS-200D, Navstar GPS Space Segment/Navigation User Interfaces,
*         7 March, 2006
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/04/10 1.0  new
*           2009/06/02 1.1  support glonass
*           2010/07/31 1.2  support eph_t struct change
*           2010/12/06 1.3  add almanac decoding, support of GW10
*                           change api decode_frame()
*           2013/04/11 1.4  fix bug on decode fit interval
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* decode navigation data subframe 1 -----------------------------------------*/
static int decode_subfrm1(const unsigned char *buff, eph_t *eph)
{
    double tow,toc;
    int i=48,week,iodc0,iodc1;
    
    trace(4,"decode_subfrm1:\n");
    trace(5,"decode_subfrm1: buff="); traceb(5,buff,30);
    
    tow        =getbitu(buff,24,17)*6.0;           /* transmission time */
    week       =getbitu(buff,i,10);       i+=10;
    eph->code  =getbitu(buff,i, 2);       i+= 2;
    eph->sva   =getbitu(buff,i, 4);       i+= 4;   /* ura index */
    eph->svh   =getbitu(buff,i, 6);       i+= 6;
    iodc0      =getbitu(buff,i, 2);       i+= 2;
    eph->flag  =getbitu(buff,i, 1);       i+= 1+87;
    eph->tgd[0]=getbits(buff,i, 8)*P2_31; i+= 8;
    iodc1      =getbitu(buff,i, 8);       i+= 8;
    toc        =getbitu(buff,i,16)*16.0;  i+=16;
    eph->f2    =getbits(buff,i, 8)*P2_55; i+= 8;
    eph->f1    =getbits(buff,i,16)*P2_43; i+=16;
    eph->f0    =getbits(buff,i,22)*P2_31;
    
    eph->iodc=(iodc0<<8)+iodc1;
    eph->week=adjgpsweek(week); /* week of tow */
    eph->ttr=gpst2time(eph->week,tow);
    eph->toc=gpst2time(eph->week,toc);
    
    return 1;
}
/* decode navigation data subframe 2 -----------------------------------------*/
static int decode_subfrm2(const unsigned char *buff, eph_t *eph)
{
    double sqrtA;
    int i=48;
    
    trace(4,"decode_subfrm2:\n");
    trace(5,"decode_subfrm2: buff="); traceb(5,buff,30);
    
    eph->iode=getbitu(buff,i, 8);              i+= 8;
    eph->crs =getbits(buff,i,16)*P2_5;         i+=16;
    eph->deln=getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
    eph->M0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->cuc =getbits(buff,i,16)*P2_29;        i+=16;
    eph->e   =getbitu(buff,i,32)*P2_33;        i+=32;
    eph->cus =getbits(buff,i,16)*P2_29;        i+=16;
    sqrtA    =getbitu(buff,i,32)*P2_19;        i+=32;
    eph->toes=getbitu(buff,i,16)*16.0;         i+=16;
    eph->fit =getbitu(buff,i, 1)?0.0:4.0; /* 0:4hr,1:>4hr */
    
    eph->A=sqrtA*sqrtA;
    
    return 2;
}
/* decode navigation data subframe 3 -----------------------------------------*/
static int decode_subfrm3(const unsigned char *buff, eph_t *eph)
{
    double tow,toc;
    int i=48,iode;
    
    trace(4,"decode_subfrm3:\n");
    trace(5,"decode_subfrm3: buff="); traceb(5,buff,30);
    
    eph->cic =getbits(buff,i,16)*P2_29;        i+=16;
    eph->OMG0=getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->cis =getbits(buff,i,16)*P2_29;        i+=16;
    eph->i0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->crc =getbits(buff,i,16)*P2_5;         i+=16;
    eph->omg =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->OMGd=getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    iode     =getbitu(buff,i, 8);              i+= 8;
    eph->idot=getbits(buff,i,14)*P2_43*SC2RAD;
    
    /* check iode and iodc consistency */
    if (iode!=eph->iode||iode!=(eph->iodc&0xFF)) return 0;
    
    /* adjustment for week handover */
    tow=time2gpst(eph->ttr,&eph->week);
    toc=time2gpst(eph->toc,NULL);
    if      (eph->toes<tow-302400.0) {eph->week++; tow-=604800.0;}
    else if (eph->toes>tow+302400.0) {eph->week--; tow+=604800.0;}
    eph->toe=gpst2time(eph->week,eph->toes);
    eph->toc=gpst2time(eph->week,toc);
    eph->ttr=gpst2time(eph->week,tow);
    
    return 3;
}
/* decode almanac ------------------------------------------------------------*/
static void decode_almanac(const unsigned char *buff, alm_t *alm)
{
    gtime_t toa;
    double deltai,sqrtA,tt;
    int i=50,f0,sat=getbitu(buff,50,6);
    
    trace(4,"decode_almanac: sat=%2d\n",sat);
    
    if (!alm||sat<1||32<sat||alm[sat-1].week==0) return;
    
    alm[sat-1].sat =sat;
    alm[sat-1].e   =getbits(buff,i,16)*P2_21;        i+=16;
    alm[sat-1].toas=getbitu(buff,i, 8)*4096.0;       i+= 8;
    deltai         =getbits(buff,i,16)*P2_19*SC2RAD; i+=16;
    alm[sat-1].OMGd=getbits(buff,i,16)*P2_38*SC2RAD; i+=16;
    alm[sat-1].svh =getbitu(buff,i, 8);              i+= 8;
    sqrtA          =getbitu(buff,i,24)*P2_11;        i+=24;
    alm[sat-1].OMG0=getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm[sat-1].omg =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm[sat-1].M0  =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    f0             =getbitu(buff,i, 8);              i+= 8;
    alm[sat-1].f1  =getbits(buff,i,11)*P2_38;        i+=11;
    alm[sat-1].f0  =getbits(buff,i, 3)*P2_17+f0*P2_20;
    alm[sat-1].A   =sqrtA*sqrtA;
    alm[sat-1].i0  =0.3*SC2RAD+deltai;
    
    toa=gpst2time(alm[sat-1].week,alm[sat-1].toas);
    tt=timediff(toa,alm[sat-1].toa);
    if      (tt<302400.0) alm[sat-1].week--;
    else if (tt>302400.0) alm[sat-1].week++;
    alm[sat-1].toa=gpst2time(alm[sat-1].week,alm[sat-1].toas);
}
/* decode navigation data subframe 4 -----------------------------------------*/
static int decode_subfrm4(const unsigned char *buff, alm_t *alm, double *ion,
                          double *utc, int *leaps)
{
    int i,sat,svid=getbitu(buff,50,6);
    
    trace(4,"decode_subfrm4: svid=%d\n",svid);
    trace(5,"decode_subfrm4: buff="); traceb(5,buff,30);
    
    if (25<=svid&&svid<=32) { /* page 2,3,4,5,7,8,9,10 */
        
        /* decode almanac */
        decode_almanac(buff,alm);
    }
    else if (svid==63) { /* page 25 */
        
        /* decode as and sv config */
        i=56;
        for (sat=1;sat<=32;sat++) {
            if (alm) alm[sat-1].svconf=getbitu(buff,i,4); i+=4;
        }
        /* decode sv health */
        i=186;
        for (sat=25;sat<=32;sat++) {
            if (alm) alm[sat-1].svh   =getbitu(buff,i,6); i+=6;
        }
    }
    else if (svid==56) { /* page 18 */
        
        /* decode ion/utc parameters */
        if (ion) {
            i=56;
            ion[0]=getbits(buff,i, 8)*P2_30;     i+= 8;
            ion[1]=getbits(buff,i, 8)*P2_27;     i+= 8;
            ion[2]=getbits(buff,i, 8)*P2_24;     i+= 8;
            ion[3]=getbits(buff,i, 8)*P2_24;     i+= 8;
            ion[4]=getbits(buff,i, 8)*pow(2,11); i+= 8;
            ion[5]=getbits(buff,i, 8)*pow(2,14); i+= 8;
            ion[6]=getbits(buff,i, 8)*pow(2,16); i+= 8;
            ion[7]=getbits(buff,i, 8)*pow(2,16);
        }
        if (utc) {
            i=120;
            utc[1]=getbits(buff,i,24)*P2_50;     i+=24;
            utc[0]=getbits(buff,i,32)*P2_30;     i+=32;
            utc[2]=getbits(buff,i, 8)*pow(2,12); i+= 8;
            utc[3]=getbitu(buff,i, 8);
        }
        if (leaps) {
            i=192;
            *leaps=getbits(buff,i,8);
        }
    }
    return 4;
}
/* decode navigation data subframe 5 -----------------------------------------*/
static int decode_subfrm5(const unsigned char *buff, alm_t *alm)
{
    double toas;
    int i,sat,week,svid=getbitu(buff,50,6);
    
    trace(4,"decode_subfrm5: svid=%d\n",svid);
    trace(5,"decode_subfrm5: buff="); traceb(5,buff,30);
    
    if (1<=svid&&svid<=24) { /* page 1-24 */
        
        /* decode almanac */
        decode_almanac(buff,alm);
    }
    else if (svid==51) { /* page 25 */
        
        if (alm) {
            i=56;
            toas=getbitu(buff,i,8)*4096; i+=8;
            week=getbitu(buff,i,8);      i+=8;
            week=adjgpsweek(week);
            
            /* decode sv health */
            for (sat=1;sat<=24;sat++) {
                alm[sat-1].svh=getbitu(buff,i,6); i+=6;
            }
            for (sat=1;sat<=32;sat++) {
                alm[sat-1].toas=toas;
                alm[sat-1].week=week;
                alm[sat-1].toa=gpst2time(week,toas);
            }
        }
    }
    return 5;
}
/* decode navigation data frame ------------------------------------------------
* decode navigation data frame and extract ephemeris and ion/utc parameters
* args   : unsigned char *buff I navigation data frame without parity (8bitx30)
*          eph_t *eph    IO     ephemeris message      (NULL: no input)
*          alm_t *alm    IO     almanac                (NULL: no input)
*          double *ion   IO     ionospheric parameters (NULL: no input)
*          double *utc   IO     delta-utc parameters   (NULL: no input)
*          int   *leaps  IO     leap seconds (s)       (NULL: no input)
* return : status (0:no valid, 1-5:subframe id)
* notes  : use cpu time to resolve modulo 1024 ambiguity of the week number
*          see ref [1]
*          utc[3] reference week for utc parameter is truncated in 8 bits
*-----------------------------------------------------------------------------*/
extern int decode_frame(const unsigned char *buff, eph_t *eph, alm_t *alm,
                        double *ion, double *utc, int *leaps)
{
    int id=getbitu(buff,43,3); /* subframe id */
    
    trace(3,"decodefrm: id=%d\n",id);
    
    switch (id) {
        case 1: return decode_subfrm1(buff,eph);
        case 2: return decode_subfrm2(buff,eph);
        case 3: return decode_subfrm3(buff,eph);
        case 4: return decode_subfrm4(buff,alm,ion,utc,leaps);
        case 5: return decode_subfrm5(buff,alm);
    }
    return 0;
}
/* initialize receiver raw data control ----------------------------------------
* initialize receiver raw data control struct and reallocate obsevation and
* epheris buffer
* args   : raw_t  *raw   IO     receiver raw data control struct
* return : status (1:ok,0:memory allocation error)
*-----------------------------------------------------------------------------*/
extern int init_raw(raw_t *raw)
{
    const double lam_glo[NFREQ]={CLIGHT/FREQ1_GLO,CLIGHT/FREQ2_GLO};
    gtime_t time0={0};
    obsd_t data0={{0}};
    eph_t  eph0 ={0,-1,-1};
    alm_t  alm0 ={0,-1};
    geph_t geph0={0,-1};
    seph_t seph0={0};
    sbsmsg_t sbsmsg0={0};
    lexmsg_t lexmsg0={0};
    int i,j,sys;
    
    trace(3,"init_raw:\n");
    
    raw->time=raw->tobs=time0;
    raw->ephsat=0;
    raw->sbsmsg=sbsmsg0;
    raw->msgtype[0]='\0';
    for (i=0;i<MAXSAT;i++) {
        for (j=0;j<150  ;j++) raw->subfrm[i][j]=0;
        for (j=0;j<NFREQ;j++) raw->lockt[i][j]=0.0;
        for (j=0;j<NFREQ;j++) raw->halfc[i][j]=0;
        raw->icpp[i]=raw->off[i]=raw->prCA[i]=raw->dpCA[i]=0.0;
    }
    for (i=0;i<MAXOBS;i++) raw->freqn[i]=0;
    raw->lexmsg=lexmsg0;
    raw->icpc=0.0;
    raw->nbyte=raw->len=0;
    raw->iod=raw->flag=raw->tbase=raw->outtype=0;
    raw->tod=-1;
    for (i=0;i<MAXRAWLEN;i++) raw->buff[i]=0;
    raw->opt[0]='\0';
    
    raw->obs.data =NULL;
    raw->obuf.data=NULL;
    raw->nav.eph  =NULL;
    raw->nav.alm  =NULL;
    raw->nav.geph =NULL;
    raw->nav.seph =NULL;
    
    if (!(raw->obs.data =(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->obuf.data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->nav.eph  =(eph_t  *)malloc(sizeof(eph_t )*MAXSAT))||
        !(raw->nav.alm  =(alm_t  *)malloc(sizeof(alm_t )*MAXSAT))||
        !(raw->nav.geph =(geph_t *)malloc(sizeof(geph_t)*NSATGLO))||
        !(raw->nav.seph =(seph_t *)malloc(sizeof(seph_t)*NSATSBS*2))) {
        free_raw(raw);
        return 0;
    }
    raw->obs.n =0;
    raw->obuf.n=0;
    raw->nav.n =MAXSAT;
    raw->nav.na=MAXSAT;
    raw->nav.ng=NSATGLO;
    raw->nav.ns=NSATSBS*2;
    for (i=0;i<MAXOBS   ;i++) raw->obs.data [i]=data0;
    for (i=0;i<MAXOBS   ;i++) raw->obuf.data[i]=data0;
    for (i=0;i<MAXSAT   ;i++) raw->nav.eph  [i]=eph0;
    for (i=0;i<MAXSAT   ;i++) raw->nav.alm  [i]=alm0;
    for (i=0;i<NSATGLO  ;i++) raw->nav.geph [i]=geph0;
    for (i=0;i<NSATSBS*2;i++) raw->nav.seph [i]=seph0;
    for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
        if (!(sys=satsys(i+1,NULL))) continue;
        raw->nav.lam[i][j]=sys==SYS_GLO?lam_glo[j]:lam_carr[j];
    }
    raw->sta.name[0]=raw->sta.marker[0]='\0';
    raw->sta.antdes[0]=raw->sta.antsno[0]='\0';
    raw->sta.rectype[0]=raw->sta.recver[0]=raw->sta.recsno[0]='\0';
    raw->sta.antsetup=raw->sta.itrf=raw->sta.deltype=0;
    for (i=0;i<3;i++) {
        raw->sta.pos[i]=raw->sta.del[i]=0.0;
    }
    raw->sta.hgt=0.0;
    return 1;
}
/* free receiver raw data control ----------------------------------------------
* free observation and ephemeris buffer in receiver raw data control struct
* args   : raw_t  *raw   IO     receiver raw data control struct
* return : none
*-----------------------------------------------------------------------------*/
extern void free_raw(raw_t *raw)
{
    trace(3,"free_raw:\n");
    
    free(raw->obs.data ); raw->obs.data =NULL; raw->obs.n =0;
    free(raw->obuf.data); raw->obuf.data=NULL; raw->obuf.n=0;
    free(raw->nav.eph  ); raw->nav.eph  =NULL; raw->nav.n =0;
    free(raw->nav.alm  ); raw->nav.alm  =NULL; raw->nav.na=0;
    free(raw->nav.geph ); raw->nav.geph =NULL; raw->nav.ng=0;
    free(raw->nav.seph ); raw->nav.seph =NULL; raw->nav.ns=0;
}
/* input receiver raw data from stream -----------------------------------------
* fetch next receiver raw data and input a message from stream
* args   : raw_t  *raw   IO     receiver raw data control struct
*          int    format I      receiver raw data format (STRFMT_???)
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter, 31: input lex message)
*-----------------------------------------------------------------------------*/
extern int input_raw(raw_t *raw, int format, unsigned char data)
{
    trace(5,"input_raw: format=%d data=0x%02x\n",format,data);
    
    switch (format) {
        case STRFMT_OEM4 : return input_oem4 (raw,data);
        case STRFMT_OEM3 : return input_oem3 (raw,data);
        case STRFMT_UBX  : return input_ubx  (raw,data);
        case STRFMT_SS2  : return input_ss2  (raw,data);
        case STRFMT_CRES : return input_cres (raw,data);
        case STRFMT_STQ  : return input_stq  (raw,data);
        case STRFMT_GW10 : return input_gw10 (raw,data);
        case STRFMT_JAVAD: return input_javad(raw,data);
        case STRFMT_NVS  : return input_nvs  (raw,data);
        case STRFMT_BINEX: return input_bnx  (raw,data);
        case STRFMT_LEXR : return input_lexr (raw,data);
    }
    return 0;
}
/* input receiver raw data from file -------------------------------------------
* fetch next receiver raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          int    format I      receiver raw data format (STRFMT_???)
*          FILE   *fp    I      file pointer
* return : status(-2: end of file/format error, -1...31: same as above)
*-----------------------------------------------------------------------------*/
extern int input_rawf(raw_t *raw, int format, FILE *fp)
{
    trace(4,"input_rawf: format=%d\n",format);
    
    switch (format) {
        case STRFMT_OEM4 : return input_oem4f (raw,fp);
        case STRFMT_OEM3 : return input_oem3f (raw,fp);
        case STRFMT_UBX  : return input_ubxf  (raw,fp);
        case STRFMT_SS2  : return input_ss2f  (raw,fp);
        case STRFMT_CRES : return input_cresf (raw,fp);
        case STRFMT_STQ  : return input_stqf  (raw,fp);
        case STRFMT_GW10 : return input_gw10f (raw,fp);
        case STRFMT_JAVAD: return input_javadf(raw,fp);
        case STRFMT_NVS  : return input_nvsf  (raw,fp);
        case STRFMT_BINEX: return input_bnxf  (raw,fp);
        case STRFMT_LEXR : return input_lexrf (raw,fp);
    }
    return -2;
}
