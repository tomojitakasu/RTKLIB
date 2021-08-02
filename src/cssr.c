/*------------------------------------------------------------------------------
* cssr.c : Compact SSR message decode functions
**
* references :
*     see rtcm3.c
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

/* GNSS definitions */
#define CSSR_SYS_GPS    0
#define CSSR_SYS_GLO    1
#define CSSR_SYS_GAL    2
#define CSSR_SYS_BDS    3
#define CSSR_SYS_QZS    4
#define CSSR_SYS_SBS    5
#define CSSR_SYS_IRN    6
#define CSSR_SYS_BDS3   7
#define CSSR_SYS_NONE   -1

/* Compact SSR Messages SubType */
#define CSSR_TYPE_TEST  0
#define CSSR_TYPE_MASK  1
#define CSSR_TYPE_OC    2
#define CSSR_TYPE_CC    3
#define CSSR_TYPE_CB    4
#define CSSR_TYPE_PB    5
#define CSSR_TYPE_BIAS  6
#define CSSR_TYPE_URA   7
#define CSSR_TYPE_STEC  8
#define CSSR_TYPE_GRID  9
#define CSSR_TYPE_SI    10
#define CSSR_TYPE_OCC   11
#define CSSR_TYPE_ATMOS 12

/* Service Information */
#define CSSR_SUBTYPE_SERVICE        1
#define CSSR_SUBTYPE_OPINFO         2
#define CSSR_SUBTYPE_GRID           3
#define CSSR_SUBTYPE_COORDINATE     4

#define INVALID_VALUE -10000

#define CSSR_TROP_HS_REF    2.3
#define CSSR_TROP_WET_REF   0.252

#define P2_S9_MAX  255
#define P2_S8_MAX  127

#define CSSR_MAX_SV_GNSS  40
#define CSSR_MAX_CELLMASK 64

#define ROUND(x)    ((int)floor((x)+0.5))
#define ROUND_U(x)  ((uint32_t)floor((x)+0.5))

/* constants -----------------------------------------------------------------*/

/* ssr update intervals ------------------------------------------------------*/
static const double ssrudint[16]={
    1,2,5,10,15,30,60,120,240,300,600,900,1800,3600,7200,10800
};

static double decode_sval(unsigned char *buff, int i, int n, double lsb)
{
    int slim=-((1<<(n-1))-1)-1,v;
    v = getbits(buff,i,n);
    return (v==slim) ? INVALID_VALUE:(double)v*lsb;
}

static int sat2gnss(int sat,int *prn_min)
{
    int id=CSSR_SYS_NONE,prn,sys,prn0=1;
    sys=satsys(sat,&prn);
    switch (sys) {
        case SYS_GPS: id = CSSR_SYS_GPS; break;
        case SYS_GLO: id = CSSR_SYS_GLO; break;
        case SYS_GAL: id = CSSR_SYS_GAL; break;
        case SYS_CMP: id = CSSR_SYS_BDS; break;
        case SYS_SBS: id = CSSR_SYS_SBS; prn0=120; break;
        case SYS_QZS: id = CSSR_SYS_QZS; prn0=193; break;
        case SYS_IRN: id = CSSR_SYS_IRN; break;
    }
    if (sys==SYS_CMP&&prn>=19&&prn<=58) {
        id=CSSR_SYS_BDS3;
        prn0=19;
    }
    if(prn_min) {
        *prn_min=prn0;
    }
    return id;
}

/* convert GNSS ID of cssr to system id of rtklib */
static int gnss2sys(int id, int *prn_min)
{
    int sys = SYS_NONE, prn0=1;
    switch (id) {
        case CSSR_SYS_GPS:  sys = SYS_GPS; break;
        case CSSR_SYS_GLO:  sys = SYS_GLO; break;
        case CSSR_SYS_GAL:  sys = SYS_GAL; break;
        case CSSR_SYS_BDS:  sys = SYS_CMP; break;
        case CSSR_SYS_SBS:  sys = SYS_SBS; prn0=120; break;
        case CSSR_SYS_QZS:  sys = SYS_QZS; prn0=193; break;
        case CSSR_SYS_IRN:  sys = SYS_IRN; break;
        case CSSR_SYS_BDS3: sys = SYS_CMP; prn0=19; break;
    }
    if (prn_min) *prn_min = prn0;
    return sys;
}

/*
 * count number of satellite in satellite mask
 */
static int svmask2nsat(uint64_t svmask)
{
    int j,nsat=0;
    for (j=0;j<CSSR_MAX_SV_GNSS;j++) {
        if ((svmask>>(CSSR_MAX_SV_GNSS-1-j))&1) {
            nsat++;
        }
    }
    return nsat;
}

/*
 * count number of signals in signal mask
 */
static int sigmask2nsig(uint16_t sigmask)
{
    int j,nsig=0;
    for (j=0;j<CSSR_MAX_SIG;j++) {
        if ((sigmask>>j)&1) {
            nsig++;
        }
    }
    return nsig;
}
/* convert from svmask to satellite list */
static int svmask2sat(uint64_t *svmask,int *sat)
{
    int j,id,nsat=0,sys,prn_min;
    for (id=0;id<CSSR_MAX_GNSS;id++) {
        sys = gnss2sys(id,&prn_min);
        for (j=0;j<CSSR_MAX_SV_GNSS;j++) {
            if ((svmask[id]>>(CSSR_MAX_SV_GNSS-1-j))&1) {
                if (sat)
                    sat[nsat]=satno(sys,prn_min+j);
                nsat++;
            }
        }
    }
    return nsat;
}
/*
 * decode quality indicator:
 *  a: URA_CLASS, b: URA_VALUE
 */
static float decode_cssr_quality(uint8_t q)
{
    uint8_t a=(q>>3)&0x7,b=q&0x7;
    float quality;
    if ((a==0 && b==0)||(a==7 && b==7)) {
        quality = 9999;
    } else {
        quality = ((1.0f+(float)b*0.25f)*pow(3.0f,(float)a)-1.0f)*1e-3;
    }
    return quality;
}

/* decode cssr message header */
static int decode_cssr_head(rtcm_t *rtcm, int *sync, int *tow,
       int *iod, double *udint, int *ngnss, int i0)
{
    int i=i0,udi,type,subtype=0;
    cssr_t *cssr=&rtcm->cssr;

    type=getbitu(rtcm->buff,i,12); i+=12;
    if (type==4073) {
        subtype = getbitu(rtcm->buff,i,4); i+=4;
    }
    if (subtype==CSSR_TYPE_MASK) {
        *tow=getbitu(rtcm->buff,i,20); i+=20; /* gps epoch time */
    } else {
        *tow=cssr->tow0+getbitu(rtcm->buff,i,12); i+=12; /* gps epoch time (hourly) */
    }
    trace(4,"decode_cssr_head: subtype=%d epoch=%4d\n",subtype,*tow);
    udi=getbitu(rtcm->buff,i,4); i+=4; /* update interval */
    *sync=getbitu(rtcm->buff,i,1); i+=1; /* multiple message indicator */
    *udint=ssrudint[udi];
    *iod=getbitu(rtcm->buff,i,4); i+=4; /* iod ssr */
    if (subtype==CSSR_TYPE_MASK) {
        cssr->iod=*iod;
        *ngnss=getbitu(rtcm->buff,i,4); i+=4; /* number of gnss */
    } else {
        if (cssr->iod!=*iod) {
            trace(4,"decode_cssr_head: iod mismatch subtype=%d epoch=%4d\n",subtype,*tow);
            return -1;
        }
    }
    return i;
}
/* decode mask message */
static int decode_cssr_mask(rtcm_t *rtcm, int i0)
{
    uint8_t cmi;
    int i,j,k,sync,tow,iod,nsat=0,id,ngnss,nsig,ncell=0;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    i = decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0);
    cssr->tow0 = floor(tow/3600.0)*3600.0;
    rtcm->time = gpst2time(cssr->week,tow);
    for (j=0;j<CSSR_MAX_GNSS;j++) {
        cssr->svmask[j] = 0;
        cssr->sigmask[j] = 0;
    }
    for (j=0;j<CSSR_MAX_SV;j++) {
        cssr->cellmask[j] = 0;
    }
    trace(3,"decode_cssr_mask: sync=%d tow=%d iod=%d ngnss=%d\n",sync,tow,cssr->iod,ngnss);
    for (k=0;k<ngnss;k++) {
        id = getbitu(rtcm->buff,i,4); i+= 4; /* gnss id */
        cssr->svmask[id] = (uint64_t)getbitu(rtcm->buff,i,8)<<32; i+= 8; /* sv mask */
        cssr->svmask[id] |= getbitu(rtcm->buff,i,32); i+= 32; /* sv mask */
        cssr->sigmask[id] = getbitu(rtcm->buff,i,16); i+= 16; /* signal mask */
        cmi = getbitu(rtcm->buff,i,1); i++; /* cell mask availability */
        nsig = sigmask2nsig(cssr->sigmask[id]);
        nsat = svmask2nsat(cssr->svmask[id]);
        trace(4,"mask-gnss: id=%d svmask=%x sigmask=%x cmi=%d\n",
                id,cssr->svmask[id],cssr->sigmask[id],cmi);
        for (j=0;j<nsat;j++) {
            if (cmi) {
                cssr->cellmask[ncell] = getbitu(rtcm->buff,i,nsig); i+=nsig;
            } else {
                cssr->cellmask[ncell] = (1<<nsig)-1;
            }
            trace(4,"mask-cell: cell=%d cellmask=%x\n",ncell,cssr->cellmask[ncell-1]);
            ncell++;
        }
    }
    cssr->nbit = i;
    return sync ? 0:10;
}
/* check if the buffer length is enough to decode the mask message */
static int check_bit_width_mask(rtcm_t *rtcm, int i0)
{
    int k,i=i0+12,ngnss=0,cmi=0,nsig,nsat,n;
    uint16_t sigmask;
    uint64_t svmask;

    if (i+37>rtcm->nbit) return -1;
    i+=33;
    ngnss = getbitu(rtcm->buff,i,4); i+=4;
    for (k=0;k<ngnss;k++) {
        if (i+61>rtcm->nbit) return -1;
        i+=4;
        svmask = (uint64_t)getbitu(rtcm->buff,i,8)<<32; i+=8;
        svmask |= (uint64_t)getbitu(rtcm->buff,i,32); i+=32;
        sigmask = getbitu(rtcm->buff,i,16); i+=16;
        nsat=svmask2nsat(svmask);
        nsig=sigmask2nsig(sigmask);
        cmi = getbitu(rtcm->buff,i,1); i++;
        if (cmi) {
            n=nsat*nsig;
            if (i+n>rtcm->nbit||n>128) return -1;
            i+=nsat*nsig;
        }
    }
    return i-i0;
}
/* decode orbit correction */
static int decode_cssr_oc(rtcm_t *rtcm, int i0)
{
    int i,j,k,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,iode;
    int prn,sys;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;
    if ((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    nsat = svmask2sat(cssr->svmask,sat);
    rtcm->time = gpst2time(cssr->week,tow);
    trace(3,"decode_cssr_oc:   sync=%d tow=%d iod=%d\n",sync,tow,iod);
    for (j=0;j<nsat;j++) {
        ssr = &rtcm->ssr[sat[j]-1];
        sys=satsys(sat[j],&prn);
        if (sys==SYS_GAL) {
            iode = getbitu(rtcm->buff,i,10); i+=10; /* iode */
        } else {
            iode = getbitu(rtcm->buff,i,8); i+=8; /* iode */
        }
        /* delta radial/along-track/cross-track */
        ssr->deph[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
        ssr->deph[1] = decode_sval(rtcm->buff,i,13,0.0064); i+=13;
        ssr->deph[2] = decode_sval(rtcm->buff,i,13,0.0064); i+=13;
        ssr->iode = iode;
        ssr->t0 [0]=rtcm->time;
        ssr->udi[0]=udint;
        ssr->iod[0]=cssr->iod;
        for (k=0;k<3;k++) ssr->ddeph[k]=0.0;
        ssr->update=1;
        trace(4, "ssr orbit: sys=%2d, prn=%3d, tow=%d, udi=%.1f, iod=%2d, orb=%10.4f,%10.4f,%10.4f\n",
                sys,prn,tow,udint,cssr->iod,ssr->deph[0],ssr->deph[1],ssr->deph[2]);
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is enough to decode the orbit correction message */
static int check_bit_width_oc(rtcm_t *rtcm, int i0)
{
    int i=i0+37,k,sat[CSSR_MAX_SV],nsat,prn;
    cssr_t *cssr=&rtcm->cssr;
    if (i>rtcm->nbit) return -1;
    nsat = svmask2sat(cssr->svmask,sat);
    for (k=0;k<nsat;k++) {
        i+=(satsys(sat[k],&prn)==SYS_GAL)?51:49;
        if (i>rtcm->nbit) return -1;
    }
    return i-i0;
}
/* decode clock correction */
static int decode_cssr_cc(rtcm_t *rtcm, int i0)
{
    int i,j,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;
    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);
    trace(3,"decode_cssr_cc:   sync=%d tow=%d iod=%d\n",sync,tow,iod);
    for (j=0;j<nsat;j++) {
        ssr = &rtcm->ssr[sat[j]-1];
        sys=satsys(sat[j], &prn);
        ssr->t0 [1]=rtcm->time;
        ssr->udi[1]=udint;
        ssr->iod[1]=cssr->iod;
        ssr->dclk[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
        ssr->dclk[1] = ssr->dclk[2] = 0.0;
        ssr->update=1;
        trace(4, "ssr clock: sys=%2d prn=%3d, tow=%d, udi=%.1f, iod=%2d, clk=%8.4f\n",
                sys,prn,tow,udint,cssr->iod,ssr->dclk[0]);
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the clock correction message */
static int check_bit_width_cc(rtcm_t *rtcm, int i0)
{
    int nsat,nbit;
    cssr_t *cssr=&rtcm->cssr;
    nsat = svmask2sat(cssr->svmask,NULL);
    nbit = 37+15*nsat;
    if (nbit+i0>rtcm->nbit) return -1;
    return nbit;
}
static int sig2code(int sys, int sig)
{
    uint8_t *codes=NULL;
    const uint8_t codes_gps[CSSR_MAX_SIG]={ /* GPS */
        CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1S,CODE_L1L,CODE_L1X,
        CODE_L2S,CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,
        CODE_L5I,CODE_L5Q,CODE_L5X
    };
    const uint8_t codes_glo[CSSR_MAX_SIG]={ /* GLONASS */
        CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P,CODE_L3I,CODE_L3Q,CODE_L3X
    };
    const uint8_t codes_gal[CSSR_MAX_SIG]={ /* Galileo */
        CODE_L1B,CODE_L1C,CODE_L1X,CODE_L5I,CODE_L5Q,
        CODE_L5X,CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L8I,CODE_L8Q,
        CODE_L8X,CODE_L6B,CODE_L6C
    };
    const uint8_t codes_qzs[CSSR_MAX_SIG]={ /* QZSS */
        CODE_L1C,CODE_L1S,CODE_L1L,CODE_L1X,CODE_L2S,CODE_L2L,CODE_L2X,
        CODE_L5I,CODE_L5Q,CODE_L5X,CODE_L6S,CODE_L6L,CODE_L6E,CODE_L1C
    };
    const uint8_t codes_bds[CSSR_MAX_SIG]={ /* BDS2/BDS3 */
        CODE_L2I,CODE_L2Q,CODE_L2X,CODE_L6I,CODE_L6Q,CODE_L6X,
        CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L1D,CODE_L1P,       0,
        CODE_L5D,CODE_L5P
    };
    const uint8_t codes_sbs[CSSR_MAX_SIG]={ /* SBAS */
        CODE_L1C,CODE_L5I,CODE_L5Q,CODE_L5X
    };
    const uint8_t codes_irn[CSSR_MAX_SIG]={ /* NavIC */
               0,       0,       0,CODE_L5A,       0,       0,CODE_L9A
    };

    if (sig>=CSSR_MAX_SIG) return 0;

    switch (sys) {
        case SYS_GPS: codes=(uint8_t *)codes_gps; break;
        case SYS_GLO: codes=(uint8_t *)codes_glo; break;
        case SYS_GAL: codes=(uint8_t *)codes_gal; break;
        case SYS_CMP: codes=(uint8_t *)codes_bds; break;
        case SYS_QZS: codes=(uint8_t *)codes_qzs; break;
        case SYS_SBS: codes=(uint8_t *)codes_sbs; break;
        case SYS_IRN: codes=(uint8_t *)codes_irn; break;
    }

    return codes[sig];
}
/* decode available signals from sigmask */
static int sigmask2sig(int nsat, int *sat, uint16_t *sigmask,
        uint16_t *cellmask, int *nsig, uint8_t *sig)
{
    int j,k,id,sys,sys_p=-1,ofst=0,nsig_s=0,prn;
    uint8_t code[CSSR_MAX_SIG];

    for (j=0;j<nsat;j++,ofst+=nsig_s) {
        sys = satsys(sat[j],&prn);
        if (sys != sys_p) {
            id = sat2gnss(sat[j],NULL);
            ofst=0;
            for (k=0,nsig_s=0;k<CSSR_MAX_SIG;k++) {
                if ((sigmask[id]>>(CSSR_MAX_SIG-1-k))&1) {
                    code[nsig_s] = sig2code(sys,k);
                    nsig_s++;
                }
            }
        }
        sys_p=sys;
        for (k=0,nsig[j]=0;k<nsig_s;k++) {
            if ((cellmask[j]>>(nsig_s-1-k))&1) {
                if (sig)
                    sig[j*CSSR_MAX_SIG+nsig[j]]=code[k];
                nsig[j]++;
            }
        }
    }
    return 1;
}
/* decode code bias message */
static int decode_cssr_cb(rtcm_t *rtcm, int i0)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys;
    int nsig[CSSR_MAX_SV];
    static uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;
    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time=gpst2time(cssr->week,tow);
    nsat=svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);
    trace(3,"decode_cssr_cb: sync=%d tow=%d iod=%d\n",sync,tow,iod);
    for (k=0;k<nsat;k++) {
        ssr=&rtcm->ssr[sat[k]-1];
        sys=satsys(sat[k],&prn);
        for (j=0;j<nsig[k];j++) {
            if ((s=sig[k*CSSR_MAX_SIG+j])>0) {
                ssr->cbias[s-1]=decode_sval(rtcm->buff,i,11,0.02); i+=11;
                trace(4, "ssr cbias: sys=%2d prn=%3d, tow=%d, s=%2d, cbias=%8.2f\n",
                        sys,prn,tow,s,ssr->cbias[s-1]);
            }
        }
        ssr->t0 [4]=rtcm->time;
        ssr->udi[4]=udint;
        ssr->iod[4]=iod;
        ssr->update=1;
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* decode phase bias message */
static int decode_cssr_pb(rtcm_t *rtcm, int i0)
{
    int i,j,k,s,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys,sdc;
    int nsig[CSSR_MAX_SV];
    static uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;
    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);
    trace(3,"decode_cssr_pb: sync=%d tow=%d iod=%d\n",sync,tow,iod);
    for (k=0;k<nsat;k++) {
        ssr = &rtcm->ssr[sat[k]-1];
        sys = satsys(sat[k],&prn);
        for (j=0;j<nsig[k];j++) {
            s = sig[k*CSSR_MAX_SIG+j];
            ssr->pbias[s-1] = decode_sval(rtcm->buff,i,15,0.001); i+=15;
            sdc = getbitu(rtcm->buff,i,2); i+=2;
            trace(4, "ssr pbias: sys=%2d prn=%3d, tow=%d, s=%2d, pbias=%10.3f, disc=%d\n",
                    sys,prn,tow,s,ssr->pbias[s-1],sdc);
        }
        ssr->t0 [5]=rtcm->time;
        ssr->udi[5]=udint;
        ssr->iod[5]=iod;
        ssr->update=1;
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the code bias message */
static int check_bit_width_cb(rtcm_t *rtcm, int i0)
{
    int nsig[CSSR_MAX_SV],k,sat[CSSR_MAX_SV],nsat,nsig_total=0,nbit;
    cssr_t *cssr=&rtcm->cssr;
    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,NULL);
    for (k=0;k<nsat;k++) {
        nsig_total+=nsig[k];
    }
    nbit = 37+nsig_total*11;
    if (i0+nbit>rtcm->nbit) return -1;
    return nbit;
}
/* check if the buffer length is sufficient to decode the phase bias message */
static int check_bit_width_pb(rtcm_t *rtcm, int i0)
{
    int nsig[CSSR_MAX_SV],k,sat[CSSR_MAX_SV],nsat,nsig_total=0,nbit;
    cssr_t *cssr=&rtcm->cssr;
    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,NULL);
    for (k=0;k<nsat;k++) {
        nsig_total+=nsig[k];
    }
    nbit=37+nsig_total*17;
    if (i0+nbit>rtcm->nbit) return -1;
    return nbit;
}
/* code bias correction */
static int decode_cssr_bias(rtcm_t *rtcm, int i0)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,sdc;
    int nsig[CSSR_MAX_SV],netmask=0,prn,sys;
    uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;
    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    cssr->flg_cb = getbitu(rtcm->buff,i,1); i+=1;
    cssr->flg_pb = getbitu(rtcm->buff,i,1); i+=1;
    cssr->flg_net = getbitu(rtcm->buff,i,1); i+=1;
    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);
    if (cssr->flg_net) {
        cssr->inet = getbitu(rtcm->buff,i,5); i+=5;
        netmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
        rtcm->staid = cssr->inet;
    } else {
        rtcm->staid = 0;
    }
    trace(3,"decode_cssr_bias: sync=%d tow=%d iod=%d net=%d\n",sync,tow,iod,cssr->inet);
    for (k=0;k<nsat;k++) {
        if (cssr->flg_net&&(!((netmask>>(nsat-1-k)) & 1))) continue;
        ssr = &rtcm->ssr[sat[k]-1];
        sys = satsys(sat[k],&prn);
        if (cssr->flg_cb) {
            ssr->t0 [4]=rtcm->time;
            ssr->udi[4]=udint;
            ssr->iod[4]=cssr->iod;
        }
        if (cssr->flg_pb) {
            ssr->t0 [5]=rtcm->time;
            ssr->udi[5]=udint;
            ssr->iod[5]=cssr->iod;
        }
        ssr->update=1;
        for (j=0;j<nsig[k];j++) {
            s = sig[k*CSSR_MAX_SIG+j];
            if (cssr->flg_cb) { /* code bias */
                ssr->cbias[s-1] = decode_sval(rtcm->buff,i,11,0.02); i+=11;
                trace(4, "ssr cbias: sys=%2d,prn=%3d, tow=%8d, net=%2d, s=%2d, cbias=%6.2f\n",
                        sys,prn,tow,cssr->inet,s,ssr->cbias[s-1]);
            }
            if (cssr->flg_pb) { /* phase bias */
                ssr->pbias[s-1] = decode_sval(rtcm->buff,i,15,0.001); i+=15;
                sdc = getbitu(rtcm->buff, i, 2); i+= 2;
                trace(4, "ssr pbias: sys=%2d,prn=%3d, tow=%8d, net=%2d, s=%2d, pbias=%8.3f, disc=%d\n",
                        sys,prn,tow,cssr->inet,s,ssr->pbias[s-1],sdc);
            }
        }
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the bias message */
static int check_bit_width_bias(rtcm_t *rtcm, int i0)
{
    int i=i0+37,j,k,nsat,slen=0,flg_cb,flg_pb,flg_net,netmask=0;
    static int sat[CSSR_MAX_SV],nsig[CSSR_MAX_SV];
    cssr_t *cssr=&rtcm->cssr;

    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,NULL);
    if (i+3>rtcm->nbit) return -1;
    flg_cb = getbitu(rtcm->buff,i,1); i+=1;
    flg_pb = getbitu(rtcm->buff,i,1); i+=1;
    flg_net = getbitu(rtcm->buff,i,1); i+=1;
    if (flg_net) {
        if (i+5+nsat>rtcm->nbit) return -1;
        i+=5;
        netmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
    }
    if (flg_cb) slen+=11;
    if (flg_pb) slen+=17;
    for (k=0;k<nsat;k++) {
        if (flg_net && !((netmask>>(nsat-1-k))&1)) continue;
        for(j=0;j<nsig[k];j++) {
            if (i+slen>rtcm->nbit) return -1;
            i+=slen;
        }
    }
    return i-i0;
}
/* decode ura correction */
static int decode_cssr_ura(rtcm_t *rtcm, int i0)
{
    int i,j,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,sys,prn;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr = NULL;

    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);
    trace(3,"decode_cssr_ura: sync=%d tow=%d iod=%d\n",sync, tow, iod);
    for (j=0;j<nsat;j++) {
        ssr = &rtcm->ssr[sat[j]-1];
        ssr->t0 [3]=rtcm->time;
        ssr->udi[3]=udint;
        ssr->iod[3]=iod;
        ssr->ura = getbitu(rtcm->buff,i, 6); i+= 6; /* ssr ura */
        ssr->update=1;
        sys = satsys(sat[j],&prn);
        trace(4, "decode_cssr_ura: sys=%2d, prn=%3d, tow=%8d, udi=%5.1f, iod=%2d, ura=%2d\n",
                sys,prn,tow,udint,iod,ssr->ura);
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the ura message */
static int check_bit_width_ura(rtcm_t *rtcm,int i0)
{
    int nsat,nbit;
    cssr_t *cssr=&rtcm->cssr;

    nsat = svmask2sat(cssr->svmask,NULL);
    nbit=37+6*nsat;
    if(i0+nbit>rtcm->nbit) return -1;
    return nbit;
}

/* decode stec correction */
static int decode_cssr_stec(rtcm_t *rtcm, int i0)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,inet,sys,prn;
    uint8_t stec_type;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    atmos_t *atmos=NULL;

    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask, sat);
    trace(3,"decode_cssr_stec: sync=%d tow=%d iod=%d\n",sync, tow, iod);
    stec_type = getbitu(rtcm->buff,i, 2); i+= 2; /* stec correction type */
    inet = getbitu(rtcm->buff,i, 5); i+= 5; /* network id */
    atmos = &rtcm->atmos[inet];
    cssr->net_svmask[inet] = getbitu(rtcm->buff,i, nsat); i+= nsat; /* stec correction type */
    atmos->time = rtcm->time;
    atmos->udi = udint;
    atmos->iod = iod;

    for (j=0,s=0;j<nsat;j++) {
        if ((cssr->net_svmask[inet]>>(nsat-1-j))&1) {
            sys = satsys(sat[j],&prn);
            atmos->sat[0][s] = sat[j];
            atmos->stec_quality[s] = getbitu(rtcm->buff,i,6); i+=6;
            for (k=0;k<4;k++) atmos->ci[s][k] = 0.0;
            atmos->ci[s][0] = decode_sval(rtcm->buff,i,14,0.05); i+=14;
            if (stec_type>0) {
                atmos->ci[s][1] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
                atmos->ci[s][2] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
            }
            if (stec_type>1) {
                atmos->ci[s][3] = decode_sval(rtcm->buff,i,10,0.02); i+=10;
            }
            s++;
            trace(4, "ssr stec: sys=%d, prn=%2d, tow=%d, udi=%.1f, iod=%2d, quality=%d a=(%4.2f,%4.2f,%4.2f,%4.2f)\n",
                    sys,prn,tow,udint,iod,
                    atmos->stec_quality,atmos->ci[s][0],atmos->ci[s][1],atmos->ci[s][2],atmos->ci[s][3]);
        }
    }
    atmos->inet=inet;
    atmos->nsat[0]=s;
    atmos->update=1;
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the stec message */
static int check_bit_width_stec(rtcm_t *rtcm, int i0)
{
    int i=i0+37,j,sat[CSSR_MAX_SV],nsat,stec_type,slen=0,nsat_local=0;
    uint64_t net_svmask;
    const int slen_t[4] = {20,44,54,0};
    cssr_t *cssr=&rtcm->cssr;

    nsat = svmask2sat(cssr->svmask, sat);
    if (i+7+nsat>rtcm->nbit) return -1;
    stec_type = getbitu(rtcm->buff,i,2); i+=2;
    i+=5;
    net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
    slen = slen_t[stec_type];
    for (j=0;j<nsat;j++) { /* number of local satellites */
        if ((net_svmask>>(nsat-1-j))&1) {
            nsat_local++;
        }
    }
    i+=nsat_local*slen;
    if (i>rtcm->nbit) return -1;
    return i-i0;
}
/* decode grid correction */
static int decode_cssr_grid(rtcm_t *rtcm, int i0)
{
    int i,j,k,ii,s,sync,iod,tow,ngnss,sat[CSSR_MAX_SV],nsat,ofst=0,sz;
    int trop_type,sz_idx,inet,hs,wet,sys,prn;
    double udint,stec0,dlat,dlon,iono,dstec;
    cssr_t *cssr=&rtcm->cssr;
    atmos_t *atmos=NULL;
    static char wetsign[CSSR_MAX_NET*CSSR_MAX_GP]={0},hydsign[CSSR_MAX_NET*CSSR_MAX_GP]={0};

    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
            return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);
    trop_type = getbitu(rtcm->buff,i,2); i+=2;  /* troposphere correction type */
    sz_idx = getbitu(rtcm->buff,i,1); i++; /* stec range */
    sz = (sz_idx)?16:7;
    inet = getbitu(rtcm->buff,i,5); i+=5; /* network id */
    trace(3,"decode_cssr_grid: sync=%d tow=%d iod=%d net=%d\n",sync,tow,iod,inet);
    atmos=&rtcm->atmos[inet];
    cssr->net_svmask[inet] = getbitu(rtcm->buff,i, nsat); i+=nsat; /* stec correction type */
    atmos->trop_quality=getbitu(rtcm->buff,i,6); i+=6;
    atmos->ng = getbitu(rtcm->buff,i,6); i+=6;
    atmos->time = rtcm->time;
    atmos->inet = inet;

    for (j=0;j<atmos->ng;j++) {
        ofst=inet*CSSR_MAX_GP+j;
        switch (trop_type) {
            case 0: break;
            case 1:
                hs = getbits(rtcm->buff,i,9); i+=9;
                wet = getbits(rtcm->buff,i,8); i+=8;
                if (hs==(-P2_S9_MAX-1)) {
                    hs=(hydsign[ofst]>=0)?P2_S9_MAX:-P2_S9_MAX;
                } else {
                    hydsign[ofst]=(hs>0)?1:-1;
                }
                if (wet==(-P2_S8_MAX-1)) {
                    wet=(wetsign[ofst]>=0)?P2_S8_MAX:-P2_S8_MAX;
                } else {
                    wetsign[ofst]=(wet>0)?1:-1;
                }
                atmos->trop_wet[j] = wet*0.004+CSSR_TROP_WET_REF;
                atmos->trop_total[j] = (hs+wet)*0.004+CSSR_TROP_WET_REF+CSSR_TROP_HS_REF;
                break;
        }

        dlat = atmos->pos[j][0]-atmos->pos[0][0];
        dlon = atmos->pos[j][1]-atmos->pos[0][1];

        for (k=0,s=0,ii=0;k<nsat;k++) {
            if ((cssr->net_svmask[inet]>>(nsat-1-k))&1) {
                sys = satsys(sat[k],&prn);
                dstec = decode_sval(rtcm->buff,i,sz,0.04); i+=sz;
                if (dstec==INVALID_VALUE) {
                    atmos->stec[j][s] = INVALID_VALUE;
                    iono = INVALID_VALUE;
                    trace(2,"dstec is invalid: tow=%d, inet=%d, grid=%d, sat=%d\n",tow,inet,j,sat[k]);
                } else {
                    stec0 = atmos->ci[ii][0]+atmos->ci[ii][1]*dlat+
                            atmos->ci[ii][2]*dlon+
                            atmos->ci[ii][3]*dlat*dlon;
                    atmos->stec[j][s] = stec0+dstec;
                    iono = 40.3E16/(FREQ1*FREQ2)*atmos->stec[j][s];
                }
                atmos->sat[j][s] = sat[k];

                trace(4,"decode_cssr_grid time=%s,net=%d,ig=%d,pos=%6.3f,%6.3f,sys=%d,prn=%d,iono=%.4f\n",
                        time_str(atmos->time,0),inet,j,atmos->pos[j][0],atmos->pos[j][1],
                        sys,prn,iono);
                s++;
                ii++;
            }
        }
        atmos->nsat[j]=s;
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the grid message */
static int check_bit_width_grid(rtcm_t *rtcm, int i0)
{
    int i=i0+37,k,nsat,trop_type,ngp,sz_trop,sz_idx,sz_stec,nsat_local=0;
    uint64_t net_svmask;
    cssr_t *cssr=&rtcm->cssr;

    nsat = svmask2sat(cssr->svmask,NULL);
    if (i+20+nsat>rtcm->nbit) return -1;
    trop_type = getbitu(rtcm->buff,i,2); i+=2;
    sz_idx = getbitu(rtcm->buff,i,1); i++;
    i+=5; /* network id */
    net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
    i+=6; /* trop quality indicator */
    ngp = getbitu(rtcm->buff,i,6); i+=6;
    sz_trop = (trop_type==0) ? 0:17;
    sz_stec = (sz_idx==0) ? 7:16;
    for (k=0;k<nsat;k++) {
        if ((net_svmask>>(nsat-1-k))&1) {
            nsat_local++;
        }
    }
    i+=ngp*(sz_trop+nsat_local*sz_stec);
    if (i>rtcm->nbit) return -1;
    return i-i0;
}
/* decode orbit/clock combination message */
static int decode_cssr_occ(rtcm_t *rtcm, int i0)
{
    int i,j,sync,iod,tow,ngnss,sat[CSSR_MAX_SV],nsat,iode,sz;
    int s,sys,prn;
    uint64_t net_svmask=0;
    double udint;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;

    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    /*check_week_ref(rtcm, tow, ref_combined);*/
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask, sat);
    
    trace(2, "decode_cssr_combo:sync=%d tow=%d iod=%d\n", sync, tow, iod);
    cssr->flg_orb = getbitu(rtcm->buff,i,1); i+=1;
    cssr->flg_clk = getbitu(rtcm->buff,i,1); i+=1;
    cssr->flg_net = getbitu(rtcm->buff,i,1); i+=1;
    if (cssr->flg_net) {
        cssr->inet = getbitu(rtcm->buff,i,5); i+=5;
        net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
        rtcm->staid = cssr->inet;
    }
    for (j=s=0;j<nsat;j++) {
        if (!cssr->flg_net||(cssr->flg_net && ((net_svmask>>(nsat-1-j))&1))) {
            ssr = &rtcm->ssr[sat[j]-1];
            sys = satsys(sat[j], &prn);
            if (cssr->flg_orb) {
                sz = (satsys(sat[j],NULL)==SYS_GAL) ? 10:8;
                iode = getbitu(rtcm->buff,i,sz); i+=sz; /* iode */
                /* delta radial,along-track,cross-track */
                ssr->deph[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
                ssr->deph[1] = decode_sval(rtcm->buff,i,13,0.0064); i+=13;
                ssr->deph[2] = decode_sval(rtcm->buff,i,13,0.0064); i+=13;
                ssr->t0 [2] = rtcm->time;
                ssr->udi[2] = udint;
                ssr->iod[2] = cssr->iod;
                ssr->iode = iode;
                trace(4, "combined orbit: net=%2d, tow=%8d, sys=%2d, prn=%3d, iode=%3d, deph=%10.4f, %10.4f, %10.4f\n",
                        cssr->inet,tow,sys,prn,ssr->iode,ssr->deph[0],ssr->deph[1],ssr->deph[2]);
            }
            if (cssr->flg_clk) {
                ssr->dclk[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
                ssr->t0 [3] = rtcm->time;
                ssr->udi[3] = udint;
                ssr->iod[3] = cssr->iod;
                trace(4, "combined clock: net=%2d, tow=%8d, sys=%2d, prn=%3d, dclk=%10.4f\n",
                        cssr->inet,tow,sys,prn,ssr->dclk[0]);
            }
            s++;
        }
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the orbit/clock combined message */
static int check_bit_width_occ(rtcm_t *rtcm, int i0)
{
    int i=i0+37,sat[CSSR_MAX_SV],nsat,j,flg_orbit,flg_clock,flg_net,sz;
    uint64_t net_svmask=0;
    cssr_t *cssr=&rtcm->cssr;

    nsat = svmask2sat(cssr->svmask,sat);
    if (i+3>rtcm->nbit) return -1;
    flg_orbit = getbitu(rtcm->buff,i,1); i+=1;
    flg_clock = getbitu(rtcm->buff,i,1); i+=1;
    flg_net = getbitu(rtcm->buff,i,1); i+=1;
    if (flg_net) {
        if (i+5+nsat>rtcm->nbit) return -1;
        i+=5; /* network id */
        net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
    }
    for (j=0;j<nsat;j++) {
        if (!flg_net||(flg_net&&((net_svmask>>(nsat-1-j))&1))) {
            if (flg_orbit) {
                sz = (satsys(sat[j],NULL)==SYS_GAL) ? 10:8;
                i+=sz+41;
                if (i>rtcm->nbit) return -1;
            }
            if (flg_clock) {
                if ((i+=15)>rtcm->nbit) return -1;
            }
        }
    }
    return i-i0;
}
/*
 * decode atmospheric correction message
 */
static int decode_cssr_atmos(rtcm_t *rtcm, int i0)
{
    int i,j,k,s,sync,tow,iod,ngnss,sat[CSSR_MAX_SV],nsat,sz_idx,sys,prn,sz;
    int stec_type,inet;
    uint8_t flg_trop,flg_stec;
    double udint,stec0,ci[6]={0},dlat,dlon,dstec;
    cssr_t *cssr=&rtcm->cssr;
    atmos_t *atmos;
    const float dstec_lsb_t[4] = {0.04f,0.12f,0.16f,0.24f};
    const int dstec_sz_t[4] = {4,4,5,7};
    
    if((i=decode_cssr_head(rtcm,&sync,&tow,&iod,&udint,&ngnss,i0))<0)
        return -1;
    rtcm->time = gpst2time(cssr->week,tow);
    nsat = svmask2sat(cssr->svmask,sat);
    flg_trop=getbitu(rtcm->buff,i,2); i+=2;   /* troposphere correction type */
    flg_stec=getbitu(rtcm->buff,i,2); i+=2;   /* stec correction type */
    inet=getbitu(rtcm->buff,i,5); i+=5;       /* network id */
    trace(3,"decode_cssr_atmos:tow=%d iod=%d net=%d flg-trop=%d flg-stec=%d\n",
            tow,iod,inet,flg_trop,flg_stec);
    rtcm->atmos[0].inet = inet;
    atmos = &rtcm->atmos[inet];
    atmos->ng = getbitu(rtcm->buff,i,6); i+=6;
    atmos->time = rtcm->time;
    atmos->inet = inet;
    for (j=0;j<CSSR_MAX_GP;j++) {
        atmos->nsat[j]=0;
    }
    
    if (flg_trop>0) {
        atmos->trop_quality=getbitu(rtcm->buff,i,6); i+=6;
        if (flg_trop&0x2) { /* trop functional term */
            atmos->trop_type = getbitu(rtcm->buff,i,2); i+=2;
            for (k=0;k<4;k++) atmos->ct[k]=0.0;
            atmos->ct[0]=decode_sval(rtcm->buff,i,9,0.004); i+=9;
            if (atmos->trop_type>0) {
                atmos->ct[1]=decode_sval(rtcm->buff,i,7,0.002); i+=7;
                atmos->ct[2]=decode_sval(rtcm->buff,i,7,0.002); i+=7;
            }
            if (atmos->trop_type>1) {
                atmos->ct[3]=decode_sval(rtcm->buff,i,7,0.001); i+=7;
            }
            trace(4, "atmos-trop-func: net=%d tow=%d quality=%d trop_type=%d ct=%.3f %.3f %.3f %.3f\n",
                 atmos->inet,tow,atmos->trop_quality,atmos->trop_type,
                 atmos->ct[0],atmos->ct[1],atmos->ct[2],atmos->ct[3]);
        }
        if (flg_trop&0x1) { /* trop residual term */
            sz_idx=getbitu(rtcm->buff,i,1); i+=1;
            sz=(sz_idx==0)?6:8;
            atmos->trop_ofst = getbitu(rtcm->buff,i,4)*0.02; i+=4;
            trace(4, "atmos-trop-res: net=%d tow=%d ng=%d quality=%d trop_type=%d sz=%d ofst=%.2f\n",
                 atmos->inet,tow,atmos->ng,atmos->trop_quality,atmos->trop_type,sz_idx,atmos->trop_ofst);
            for (j=0;j<atmos->ng;j++) {
                dlat=atmos->pos[j][0]-atmos->pos[0][0];
                dlon=atmos->pos[j][1]-atmos->pos[0][1];
                atmos->trop_total[j]=CSSR_TROP_HS_REF+atmos->ct[0];
                if (atmos->trop_type>0) {
                    atmos->trop_total[j]+=(atmos->ct[1]*dlat)+(atmos->ct[2]*dlon);
                }
                if (atmos->trop_type>1) {
                    atmos->trop_total[j]+=atmos->ct[3]*dlat*dlon;
                }
                atmos->trop_wet[j]=decode_sval(rtcm->buff,i,sz,0.004); i+=sz;
                if (atmos->trop_wet[j]==INVALID_VALUE) {
                    atmos->trop_total[j]=INVALID_VALUE;
                    trace(2,"invalid trop(wet): tow=%d, inet=%d, grid=%d\n",tow,inet,j);
                } else {
                    atmos->trop_wet[j]+=atmos->trop_ofst;
                    atmos->trop_total[j]+=atmos->trop_wet[j];
                }
                trace(4, "atmos-trop-grid: net=%2d tow=%d grid=%2d pos=%.3f %.3f %.3f total=%.3f wet=%.3f\n",
                    atmos->inet,tow,j,atmos->pos[j][0],atmos->pos[j][1],atmos->pos[j][2],
                    atmos->trop_total[j],atmos->trop_wet[j]);
            }
        }
    }
    
    if (flg_stec>0) {
        cssr->net_svmask[inet]=getbitu(rtcm->buff,i,nsat); i+=nsat; /* stec correction type */
        trace(4, "decode_cssr_atmos: mask=0x%x\n",cssr->net_svmask[inet]);
        for (j=s=0;j<nsat;j++) {
            if (!((cssr->net_svmask[inet]>>(nsat-1-j))&1)) {
                continue;
            }
            sys = satsys(sat[j], &prn);
            atmos->stec_quality[s] = getbitu(rtcm->buff,i,6); i+=6;
            for (k=0;k<6;k++) ci[k]=0.0;
            if (flg_stec&0x2) {
                stec_type=getbitu(rtcm->buff,i,2); i+=2;
                atmos->stec_type[s]=stec_type;
                for (k=0;k<6;k++) ci[k]=0.0;
                ci[0] = decode_sval(rtcm->buff,i,14,0.05); i+=14;
                if (stec_type>0) {
                    ci[1] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
                    ci[2] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
                }
                if (stec_type>1) {
                    ci[3] = decode_sval(rtcm->buff,i,10,0.02); i+=10;
                }
                if (stec_type>2) {
                    ci[4] = decode_sval(rtcm->buff,i,8,0.005); i+=8;
                    ci[5] = decode_sval(rtcm->buff,i,8,0.005); i+=8;
                }
                for (k=0;k<6;k++) atmos->ci[s][k]=ci[k];
                trace(4,"atmos-stec-func: tow=%d net=%d sys=%d prn=%d quality=%d stec_type=%d ci=%.2f %.2f %.2f %.2f %.3f %.3f\n",
                        tow,inet,sys,prn,atmos->stec_quality[s],atmos->stec_type[s],
                        ci[0],ci[1],ci[2],ci[3],ci[4],ci[5]);
            }
            if (flg_stec&0x1) {
                sz_idx = getbitu(rtcm->buff,i,2); i+=2;
                trace(4,"atmos-stec-res: tow=%d net=%d sys=%d prn=%d quality=%d size=%d\n",
                        tow,inet,sys,prn,atmos->stec_quality[s],sz_idx);
                for (k=0;k<atmos->ng;k++) {
                    dlat=atmos->pos[k][0]-atmos->pos[0][0];
                    dlon=atmos->pos[k][1]-atmos->pos[0][1];
                    dstec=decode_sval(rtcm->buff,i,dstec_sz_t[sz_idx],dstec_lsb_t[sz_idx]);
                    i+=dstec_sz_t[sz_idx];
                    if (dstec==INVALID_VALUE) {
                        atmos->stec[k][s] = INVALID_VALUE;
                        trace(2,"dstec is invalid: tow=%d, net=%2d, grid=%2d, sat=%d\n",tow,inet,k,sat[j]);
                    } else {
                        stec0=ci[0];
                        if (stec_type>0) {
                            stec0+=(ci[1]*dlat)+(ci[2]*dlon);
                        }
                        if (stec_type>1) {
                            stec0+=ci[3]*dlat*dlon;
                        }
                        if (stec_type>2) {
                            stec0+=(ci[4]*dlat*dlat)+(ci[5]*dlon*dlon);
                        }
                        atmos->stec[k][s]=stec0+dstec;
                    }
                    atmos->sat[k][s]=sat[j];
                    atmos->nsat[k]++;
                    trace(4,"atmos-stec-grid: net=%2d tow=%8d sys=%2d prn=%3d grid=%2d dstec=%8.3f stec=%8.3f\n",
                        inet,tow,sys,prn,k,dstec,atmos->stec[k][s]);
                }
                s++;
            }
            for (k=0;k<atmos->ng;k++) {
                atmos->nsat[k]=s;
                trace(4,"decode_cssr_atmos: grid=%d, nsv=%d\n",k+1,atmos->nsat[k]);
            }
        }
    }
    cssr->nbit=i;
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the atmospheric correction message */
static int check_bit_width_atmos(rtcm_t *rtcm, int i0)
{
    int i=i0+37,flg_trop,flg_stec,trop_type,stec_type,ngp,sz_idx,sz,j,nsat;
    uint64_t net_svmask;
    const int dstec_sz_t[4]={4,4,5,7},trop_sz_t[3]={9,23,30};
    const int stec_sz_t[4]={14,38,48,64};
    cssr_t *cssr=&rtcm->cssr;
    
    nsat = svmask2sat(cssr->svmask,NULL);
    if (i+15>rtcm->nbit) return -1;
    flg_trop = getbitu(rtcm->buff,i,2); i+=2;
    flg_stec = getbitu(rtcm->buff,i,2); i+=2;
    i+=5;
    ngp = getbitu(rtcm->buff,i,6); i+=6;
    if (flg_trop>0) {
        if (i+8>rtcm->nbit) return -1;
        i+=6;
        if (flg_trop&0x2) {
            trop_type = getbitu(rtcm->buff,i,2); i+=2;
            sz = trop_sz_t[trop_type];
            if (i+sz+5>rtcm->nbit) return -1;
            i+=sz;
        }
        if (flg_trop&0x1) {
            sz_idx = getbitu(rtcm->buff,i,1); i+=1;
            i+=4;
            sz = (sz_idx==0)?6:8;
            if (i+sz*ngp>rtcm->nbit) return -1;
            i+=sz*ngp;
        }
    }
    if (flg_stec>0) {
        if (i+nsat>rtcm->nbit) return -1;
        net_svmask=getbitu(rtcm->buff,i,nsat); i+=nsat;
        for (j=0;j<nsat;j++) {
            if (!((net_svmask>>(nsat-1-j))&1)) continue;
            if (i+6>rtcm->nbit) return -1;
            i+=6;
            if (flg_stec&0x2) {
                if (i+2>rtcm->nbit) return -1;
                stec_type=getbitu(rtcm->buff,i,2); i+=2;
                sz=stec_sz_t[stec_type];
                if (i+sz>rtcm->nbit) return -1;
                i+=sz;
            }
            if (flg_stec&0x1) {
                if (i+2>rtcm->nbit) return -1;
                sz_idx=getbitu(rtcm->buff,i,2); i+=2;
                sz=ngp*dstec_sz_t[sz_idx];
                if (i+sz>rtcm->nbit) return -1;
                i+=sz;
            }
        }
    }
    trace(4,"check_bit_width_atmos: i0=%d nbit=%d\n",i,rtcm->nbit);
    return i-i0;
}
/* decode grid definition */
static int decode_cssr_griddef(rtcm_t *rtcm, int i0)
{
    int i=i0,j,k,ii,ij,jj,sync,type,subtype,subsubtype;
    int ig,nb=0,idx,inet,gtype,nlat,nlon,narea,npart,ngrid;
    uint8_t iod,fmask;
    uint64_t mask;
    float lat0,lon0,dlat,dlon,slat,slon;
    cssr_t *cssr=&rtcm->cssr;
    atmos_t *atmos=NULL;

    type=getbitu(rtcm->buff,i,12);i+=12;
    if (type==4073) {
        subtype=getbitu(rtcm->buff,i,4);i+=4;
        if (subtype==CSSR_TYPE_TEST) {
            subsubtype=getbitu(rtcm->buff,i,4);i+=4;
            if (subsubtype!=CSSR_SUBTYPE_GRID) {
                return -1;
            }
            sync=getbitu(rtcm->buff,i,1);i+=1;/* multiple message indicator */
        } else if (subtype==CSSR_TYPE_SI) {
            sync=getbitu(rtcm->buff,i,1);i+=1;/* multiple message indicator */
            i+=7;   /* byte alignment */
            i+=4+3; /* skip si-type,iod-si */
        }
    }
    iod=getbitu(rtcm->buff,i,3);i+=3;
    narea=getbitu(rtcm->buff,i,5)+1;i+=5;
    trace(3,"decode_cssr_griddef: iod-grid=%d nnet=%d",iod,narea);
    for (j=0;j<narea;j++) {
        inet=getbitu(rtcm->buff,i,5);i+=5;
        atmos=&rtcm->atmos[inet];
        npart=getbitu(rtcm->buff,i,3)+1;i+=3;
        for (k=0,ii=0;k<npart;k++) {
            gtype=getbitu(rtcm->buff,i,2);i+=2;
            lat0=getbits(rtcm->buff,i,15)*0.01;i+=15;
            lon0=getbits(rtcm->buff,i,16)*0.01;i+=16;
            trace(4,"decode_cssr_griddef: np=%d net/part=%d/%d type=%d lat0=%.2f lon0=%.2f",
                    npart,j,k,gtype,lat0,lon0);
            if (gtype==0) {
                atmos->pos[ii][0]=lat0;
                atmos->pos[ii][1]=lon0;
                ii++;
                ngrid=getbitu(rtcm->buff,i,6);i+=6;
                for (ig=0;ig<ngrid;ig++,ii++) {
                    dlat=getbits(rtcm->buff,i,10)*0.01;i+=10;
                    dlon=getbits(rtcm->buff,i,11)*0.01;i+=11;
                    atmos->pos[ii][0]=atmos->pos[ii-1][0]+dlat;
                    atmos->pos[ii][1]=atmos->pos[ii-1][1]+dlon;
                    trace(4,"decode_cssr_griddef: type=%d ngrid=%d grid=%d dlat=%.2f dlon=%.2f",
                            gtype,ngrid,ig,dlat,dlon);
                }
            } else if (gtype==1||gtype==2) {
                nlat=getbitu(rtcm->buff,i,6);i+=6;
                nlon=getbitu(rtcm->buff,i,6);i+=6;
                slat=getbitu(rtcm->buff,i,9)*0.01;i+=9;
                slon=getbitu(rtcm->buff,i,10)*0.01;i+=10;
                fmask=getbitu(rtcm->buff,i,1);i+=1;
                if (fmask) {
                    nb=nlat*nlon;
                    if (nb>64) {
                        trace(2,"decode_cssr_grid: nb=%d>64\n",nb);
                        return -1;
                    }
                    if (nb>32) {
                        mask=(uint64_t)getbitu(rtcm->buff,i,nb-32)<<32;i+=nb-32;
                        mask|=getbitu(rtcm->buff,i,32);i+=32;
                    } else {
                        mask=getbitu(rtcm->buff,i,nb);i+=nb;
                    }
                } else {
                    mask=0;
                }
                trace(4,"decode_cssr_griddef: type=%d nlat=%d nlon=%d slat=%.2f slon=%.2f fmask=%d mask=%x",
                        gtype,nlat,nlon,slat,slon,fmask,mask);
                if (gtype==1) { /* grid-type: 1 */
                    for (ij=0;ij<nlat;ij++) {
                        for (jj=0;jj<nlon;jj++) {
                            idx=ij*nlon+jj;
                            if (!fmask||((mask>>(nb-idx-1))&0x1)) {
                                atmos->pos[ii][0]=lat0-slat*ij;
                                atmos->pos[ii][1]=lon0+slon*jj;
                                ii++;
                            }
                        }
                    }
                } else {    /* grid-type: 2 */
                    for (ij=0;ij<nlon;ij++) {
                        for (jj=0;jj<nlat;jj++) {
                            idx=ij*nlat+jj;
                            if (!fmask||((mask>>(nb-idx-1))&0x1)) {
                                atmos->pos[ii][0]=lat0+slat*jj;
                                atmos->pos[ii][1]=lon0+slon*ij;
                                ii++;
                            }
                        }
                    }
                }
            }
        }
        atmos->pos[ii][0]=atmos->pos[ii][1]=-1;
        atmos->ng=ii;
    }
    cssr->nbit=i;
    return sync?0:10;
}
/*
 * decode service information message
 */
static int decode_cssr_si(rtcm_t *rtcm, int i0)
{
    int i=i0,j,k,sync,j0,type,subtype,blen=0,si_cnt;
    static uint8_t flg_cssr_si=0,si_sz[8]={0,};
    cssr_t *cssr=&rtcm->cssr;

    type = getbitu(rtcm->buff,i,12); i+=12;
    subtype = getbitu(rtcm->buff,i,4); i+=4;

    if (type!=4073||subtype!=CSSR_TYPE_SI) {
        trace(2,"decode_cssr_si invalid type,subtype=%4d,%2d\n",
                type,subtype);
    }

    sync = getbitu(rtcm->buff,i,1); i+=1; /* multiple message indicator */
    si_cnt = getbitu(rtcm->buff,i,3); i+=3;  /* information message counter */
    si_sz[si_cnt] = getbitu(rtcm->buff,i,2)+1; i+=2; /* data size */

    trace(4,"decode_cssr_si: cnt=%d sz=%d\n",si_cnt,si_sz[si_cnt]);

    if (sync==0 && si_cnt>0) {
        j0=si_cnt*20;
        for (j=0,blen=0;j<=si_cnt;j++) {
            blen+=si_sz[j]*5;
        }
    } else {
        j0=si_cnt*si_sz[si_cnt]*5;
        blen=si_sz[si_cnt]*5;
    }
    for (j=0;j<si_sz[si_cnt];j++) {
        for (k=0;k<5;k++) {
            cssr->buff[j0+j*5+k] = getbitu(rtcm->buff,i,8);i+=8;
        }
    }
    flg_cssr_si|=1<<si_cnt;
    if (sync==0 && flg_cssr_si==(1<<(si_cnt+1))-1) {
        /* decode SI */
        memcpy(rtcm->buff+6,cssr->buff,blen);
        decode_cssr_griddef(rtcm,i0);
        flg_cssr_si = 0;
    }
    return sync?0:10;
}
/* check if the buffer length is sufficient to decode the service information message */
static int check_bit_width_si(rtcm_t *rtcm, int i0)
{
    int i=i0+16,data_sz=0;

    if (i+6>rtcm->nbit) return -1;
    i+=4;
    data_sz = getbitu(rtcm->buff,i,2); i+=2;
    i+=40*(data_sz+1);
    if (i>rtcm->nbit) return -1;
    return i-i0;
}
/* decode type 4073: Melco proprietary messages */
extern int decode_cssr(rtcm_t *rtcm, int i0, int head)
{
    int i=i0,ret=0,subtype,subsubtype;

    i+=(head)?24:0;
    subtype = getbitu(rtcm->buff,i+12,4);
    trace(4,"decode_cssr subtype=%d\n",subtype);

    switch (subtype) {
        case CSSR_TYPE_MASK:
            ret=decode_cssr_mask(rtcm,i);
            break;
        case CSSR_TYPE_OC:
            ret=decode_cssr_oc(rtcm,i);
            break;
        case CSSR_TYPE_CC:
            ret=decode_cssr_cc(rtcm,i);
            break;
        case CSSR_TYPE_CB:
            ret=decode_cssr_cb(rtcm,i);
            break;
        case CSSR_TYPE_PB:
            ret=decode_cssr_pb(rtcm,i);
            break;
        case CSSR_TYPE_BIAS:
            ret=decode_cssr_bias(rtcm,i);
            break;
        case CSSR_TYPE_URA:
            ret=decode_cssr_ura(rtcm,i);
            break;
        case CSSR_TYPE_STEC:
            ret=decode_cssr_stec(rtcm,i);
            break;
        case CSSR_TYPE_GRID:
            ret=decode_cssr_grid(rtcm,i);
            break;
        case CSSR_TYPE_OCC:
            ret=decode_cssr_occ(rtcm,i);
            break;
        case CSSR_TYPE_ATMOS:
            ret = decode_cssr_atmos(rtcm,i);
            break;
        case CSSR_TYPE_SI:
            ret = decode_cssr_si(rtcm,i);
            break;
        case CSSR_TYPE_TEST:    /* experimental */
            subsubtype = getbitu(rtcm->buff,i+12+4,4);
            if (subsubtype==CSSR_SUBTYPE_GRID) {
                ret = decode_cssr_griddef(rtcm,i);
            }
            break;
        default: break;
    }
    return ret;
}
/* check message length of CSSR */
extern int cssr_check_bitlen(rtcm_t *rtcm,int i0)
{
    int i=i0,type,subtype,nbit=0;
    cssr_t *cssr = &rtcm->cssr;
    type = getbitu(rtcm->buff,i,12);
    if (type!=4073) {
        trace(2,"invalid type:%d rtcm-nbit=%d nbit=%d\n",
                type,rtcm->nbit,cssr->nbit);
        return -1;
    }
    subtype = getbitu(rtcm->buff,i+12,4);
    switch (subtype) {
        case CSSR_TYPE_MASK:
            nbit=check_bit_width_mask(rtcm,i);
            break;
        case CSSR_TYPE_OC:
            nbit=check_bit_width_oc(rtcm,i);
            break;
        case CSSR_TYPE_CC:
            nbit=check_bit_width_cc(rtcm,i);
            break;
        case CSSR_TYPE_CB:
            nbit=check_bit_width_cb(rtcm,i);
            break;
        case CSSR_TYPE_PB:
            nbit=check_bit_width_pb(rtcm,i);
            break;
        case CSSR_TYPE_BIAS:
            nbit=check_bit_width_bias(rtcm,i);
            break;
        case CSSR_TYPE_URA:
            nbit=check_bit_width_ura(rtcm,i);
            break;
        case CSSR_TYPE_STEC:
            nbit=check_bit_width_stec(rtcm,i);
            break;
        case CSSR_TYPE_GRID:
            nbit=check_bit_width_grid(rtcm,i);
            break;
        case CSSR_TYPE_OCC:
            nbit=check_bit_width_occ(rtcm,i);
            break;
        case CSSR_TYPE_ATMOS:
            nbit=check_bit_width_atmos(rtcm,i);
            break;
        case CSSR_TYPE_SI:
            nbit=check_bit_width_si(rtcm,i);
            break;
        default:
            trace(2,"invalid subtype:%d nbit=%d\n",subtype,rtcm->nbit);
            return -1;
    }
    return nbit;
}
/* read list of grid position from ascii file */
extern int read_grid_def(rtcm_t *rtcm, const char *gridfile)
{
    int gridsel=0;
    int no;
    double lat,lon,alt;
    char buff[1024];
    int inet,grid[CSSR_MAX_NET]={0,},ret;
    atmos_t *atmos=rtcm->atmos;
    FILE *fp=NULL;

    for (inet=0;inet<CSSR_MAX_NET;inet++) {
        atmos[inet].pos[0][0]=atmos[inet].pos[0][1]=atmos[inet].pos[0][2]=-1.0;
    }

    trace(4,"read_grid_def: gridfile=%s\n",gridfile);
    if(!(fp=fopen(gridfile,"r"))) return -1;

    while (fgets(buff, sizeof(buff), fp)) {
        if (strstr(buff,"Compact Network ID    GRID No.  Latitude     Longitude   Ellipsoidal height")) {
            gridsel=3;
            trace(3,"grid definition: IS attached file version%d\n",gridsel);
            break;
        } else {
            trace(1,"grid definition: invalid format%d\n",gridsel);
            return -1;
        }
    }
    while ((ret=fscanf(fp,"%d %d %lf %lf %lf",&inet,&no,&lat,&lon,&alt))!=EOF) {
        if (inet>=0 && inet<CSSR_MAX_NET && ret==5) {
            atmos[inet].pos[grid[inet]][0]=lat;
            atmos[inet].pos[grid[inet]][1]=lon;
            atmos[inet].pos[grid[inet]][2]=alt;
            grid[inet]++;
            atmos[inet].ng=grid[inet];
            atmos[inet].pos[grid[inet]][0]=-1.0;
            atmos[inet].pos[grid[inet]][1]=-1.0;
            atmos[inet].pos[grid[inet]][2]=-1.0;
        }
        trace(4,"grid_info: %2d, %2d, %10.3f, %10.3f, %8.3f\n",
                ret,inet,no,lat,lon,alt);
    }
    fclose(fp);
    return 0;
}

/* Compact SSR encoder */

static int encode_cssr_head(rtcm_t *rtcm, int type, int subtype, int sync,
       int iod, double udint, int ngnss)
{
    double tow;
    int i=24,udi,msgno=type,epoch,week;

    setbitu(rtcm->buff,i,12,msgno); i+=12; /* message type */
    setbitu(rtcm->buff,i,4,subtype);i+=4;  /* message sub-type */

    tow=time2gpst(rtcm->time,&week);
    epoch=ROUND(tow)%604800;
    if (subtype==CSSR_TYPE_MASK) {
        setbitu(rtcm->buff,i,20,epoch); i+=20; /* GPS epoch time */
    } else {
        setbitu(rtcm->buff,i,12,epoch%3600); i+=12; /* GPS epoch time */
    }
    trace(4,"encode_cssr_head: subtype=%d epoch=%4d\n",subtype,epoch);

    for (udi=0;udi<15;udi++) {
        if (ssrudint[udi]>=udint) break;
    }
    setbitu(rtcm->buff,i,4,udi);    i+=4; /* update interval */
    setbitu(rtcm->buff,i,1,sync);   i+=1; /* multiple message indicator */
    setbitu(rtcm->buff,i,4,iod);    i+=4; /* iod ssr */

    if (subtype==CSSR_TYPE_MASK) {
        setbitu(rtcm->buff,i,4,ngnss); i+=4;
    }
    return i;
}
/* encode mask message */
static int encode_cssr_mask(rtcm_t *rtcm, int subtype, int sync)
{
    double udint=0.0;
    int i,j,ii,jj,iod=0,nsat,prn,sys,ngnss=0,type=4073;
    int id,id_p=-1,prn_min=1,k,sma[CSSR_MAX_GNSS]={0},nsig[CSSR_MAX_GNSS]={0},code=0;
    cssr_t *cssr=&rtcm->cssr;

    trace(3,"encode_cssr_mask: subtype=%d sync=%d\n",subtype,sync);

    for (id=0;id<CSSR_MAX_GNSS;id++) {
        cssr->svmask[id]=cssr->sigmask[id]=0;
    }

    /* number of satellites */
    for (j=nsat=0;j<MAXSAT;j++) {
        if (!rtcm->ssr[j].update) continue;
        if(nsat++>CSSR_MAX_SV) {
            trace(2,"encode_cssr_mask: nsat exceeds CSSR_MAX_SV: %d\n",nsat);
            return -1;
        }
        udint=rtcm->ssr[j].udi[0];
        iod  =rtcm->ssr[j].iod[0];
        sys=satsys(j+1,&prn);
        id=sat2gnss(j+1,&prn_min);
        if (id!=id_p) ngnss++;
        id_p=id;
        cssr->svmask[id]|=1<<(CSSR_MAX_SV_GNSS-(prn-prn_min)-1);

        for (k=0;k<CSSR_MAX_SIG;k++) {
            code=sig2code(sys,k);
            if (rtcm->ssr[j].cbias[code-1]!=0.0||rtcm->ssr[j].pbias[code-1]!=0.0) {
                cssr->sigmask[id]|=1<<(CSSR_MAX_SIG-k-1);
            }
        }
    }

    for (id=0;id<CSSR_MAX_GNSS;id++) {
        nsig[id]=sigmask2nsig(cssr->sigmask[id]);
    }

    cssr->iod=iod;
    for (j=0,ii=0;j<MAXSAT;j++) {
        if (!rtcm->ssr[j].update) continue;
        sys=satsys(j+1,&prn);
        id=sat2gnss(j+1,&prn_min);
        cssr->cellmask[ii]=0;
        for (k=0,jj=0;k<CSSR_MAX_SIG;k++) {
            if ((cssr->sigmask[id]>>(CSSR_MAX_SIG-k-1))&1) {
                code=sig2code(sys,k);
                if (rtcm->ssr[j].cbias[code-1]!=0.0||rtcm->ssr[j].pbias[code-1]!=0.0) {
                    cssr->cellmask[ii]|=1<<(nsig[id]-jj-1);
                }
                jj++;
            }
        }
        if (cssr->cellmask[ii]!=(1<<nsig[id])-1) sma[id]=1;
        ii++;
    }

    /* encode SSR header */
    i=encode_cssr_head(rtcm,type,subtype,sync,iod,udint,ngnss);

    for (id=0;id<CSSR_MAX_GNSS;id++) {
        if (cssr->svmask[id]==0) continue;
        setbitu(rtcm->buff,i,4,id);    i+=4;
        setbitu(rtcm->buff,i,32,cssr->svmask[id]>>8);    i+=32;
        setbitu(rtcm->buff,i,8,cssr->svmask[id]&0xff);    i+=8;
        setbitu(rtcm->buff,i,16,cssr->sigmask[id]);    i+=16;
        setbitu(rtcm->buff,i,1,sma[id]);    i+=1;
        if (sma[id]) {
            for (j=0;j<nsat;j++) {
                setbitu(rtcm->buff,i,nsig[id],cssr->cellmask[j]); i+=nsig[id];
            }
        }
    }

    rtcm->nbit=i;
    return 1;
}

/* encode orbit/clock correction message */
static int encode_cssr_occ(rtcm_t *rtcm, int subtype, int sync)
{
    double udint=0.0;
    int i,j,type=4073,iod,sys,nsat,sat[CSSR_MAX_SV],sz,deph[3],dclk;
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;

    /* number of satellites */
    for (j=0;j<MAXSAT;j++) {
        if (!rtcm->ssr[j].update) continue;
        udint=rtcm->ssr[j].udi[0];
        iod  =rtcm->ssr[j].iod[0];
    }
    /* encode SSR header */
    i=encode_cssr_head(rtcm,type,subtype,sync,iod,udint,0);

    setbitu(rtcm->buff,i,1,cssr->flg_orb); i++;
    setbitu(rtcm->buff,i,1,cssr->flg_clk); i++;
    setbitu(rtcm->buff,i,1,cssr->flg_net&1); i++;
    nsat=svmask2sat(cssr->svmask,sat);
    if (cssr->flg_net) {
        setbitu(rtcm->buff,i,5,cssr->inet); i+=5;
        setbitu(rtcm->buff,i,nsat,cssr->net_svmask[cssr->inet-1]); i+=nsat;
    }

    for (j=0;j<nsat;j++) {
        if (cssr->flg_net && ((cssr->net_svmask[cssr->inet-1]>>(nsat-j-1))&1)!=1)
            continue;
        sys=satsys(sat[j],NULL);
        ssr=&rtcm->ssr[sat[j]-1];
        if (cssr->flg_orb) {
            deph [0]=ROUND(rtcm->ssr[j].deph [0]/0.0016);
            deph [1]=ROUND(rtcm->ssr[j].deph [1]/0.0064);
            deph [2]=ROUND(rtcm->ssr[j].deph [2]/0.0064);
            sz=(sys==SYS_GAL)?10:8;
            setbitu(rtcm->buff,i,sz,ssr->iode); i+=sz;
            setbits(rtcm->buff,i,15,deph [0]); i+=15; /* delta radial */
            setbits(rtcm->buff,i,13,deph [1]); i+=13; /* delta along-track */
            setbits(rtcm->buff,i,13,deph [2]); i+=13; /* delta cross-track */
        }
        if (cssr->flg_clk) {
            dclk=ROUND(rtcm->ssr[j].dclk [0]/0.0016);
            setbits(rtcm->buff,i,15,dclk ); i+=15; /* delta clock */
        }
    }

    rtcm->nbit=i;
    return 1;
}

/* encode bias message */
static int encode_cssr_bias(rtcm_t *rtcm, int subtype, int sync)
{
    double udint=0.0;
    int i,j,k,type=4073,iod,nsat,sat[CSSR_MAX_SV];
    int nsig[CSSR_MAX_SV],s,sdc=0,pbias,cbias;
    static uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    cssr_t *cssr=&rtcm->cssr;
    ssr_t *ssr=NULL;

    nsat=svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);
    j=sat[0]-1;
    udint=rtcm->ssr[j].udi[2];
    iod=rtcm->ssr[j].iod[2];
    /* encode SSR header */
    i=encode_cssr_head(rtcm,type,subtype,sync,iod,udint,0);

    setbitu(rtcm->buff,i,1,cssr->flg_cb); i++;
    setbitu(rtcm->buff,i,1,cssr->flg_pb); i++;
    setbitu(rtcm->buff,i,1,cssr->flg_net&1); i++;

    if (cssr->flg_net) {
        setbitu(rtcm->buff,i,5,cssr->inet); i+=5;
        setbitu(rtcm->buff,i,nsat,cssr->net_svmask[cssr->inet-1]); i+=nsat;
    }

    for (k=0;k<nsat;k++) {
        if (cssr->flg_net&&(!((cssr->net_svmask[cssr->inet-1]>>(nsat-1-k))&1)))
            continue;
        ssr=&rtcm->ssr[sat[k]-1];
        for (j=0;j<nsig[k];j++) {
            s = sig[k*CSSR_MAX_SIG+j];
            if (cssr->flg_cb) { /* code bias */
                cbias=ROUND(ssr->cbias[s-1]/0.02);
                setbits(rtcm->buff,i,11,cbias); i+=11;
            }
            if (cssr->flg_pb) { /* phase bias */
                pbias=ROUND(ssr->pbias[s-1]/0.001);
                setbits(rtcm->buff,i,15,pbias); i+=15;
                setbitu(rtcm->buff,i,2,sdc); i+= 2; /* discont counter */
            }
        }
    }

    rtcm->nbit=i;
    return 1;
}

/* encode ura message */
static int encode_cssr_ura(rtcm_t *rtcm, int subtype, int sync)
{
    double udint=0.0;
    int i,k,type=4073,iod,sat[CSSR_MAX_SV],ura,nsat;
    cssr_t *cssr=&rtcm->cssr;

    nsat=svmask2sat(cssr->svmask,sat);
    k=sat[0]-1;
    udint=rtcm->ssr[k].udi[3];
    iod=rtcm->ssr[k].iod[3];
    /* encode SSR header */
    i=encode_cssr_head(rtcm,type,subtype,sync,iod,udint,0);

    for (k=0;k<nsat;k++) {
        ura=rtcm->ssr[k].ura;
        setbitu(rtcm->buff,i,6,ura); i+=6; /* ssr ura */
    }

    rtcm->nbit=i;
    return 1;
}

/* encode atmospheric correction message */
static int encode_cssr_atmos(rtcm_t *rtcm, int subtype, int sync)
{
    int type=4073;
    int i,j,k,s,iod,sat[CSSR_MAX_SV],nsat,sz_idx,sz,trop_ofst;
    int flg_trop,flg_stec,stec_type,inet,trop_wet,dstec_i;
    double udint,stec0,dlat,dlon,dstec;
    int ct[4],ci[6];
    cssr_t *cssr=&rtcm->cssr;
    atmos_t *atmos;
    const float dstec_lsb_t[4] = {0.04f,0.12f,0.16f,0.24f};
    const int dstec_sz_t[4] = {4,4,5,7};

    inet=cssr->inet;
    atmos = &rtcm->atmos[inet];
    nsat=svmask2sat(cssr->svmask,sat);
    k=sat[0]-1;
    udint=rtcm->ssr[k].udi[3];
    iod=rtcm->ssr[k].iod[3];
    /* encode SSR header */
    i=encode_cssr_head(rtcm,type,subtype,sync,iod,udint,0);
    flg_trop=cssr->flg_trop[inet-1];
    flg_stec=cssr->flg_stec[inet-1];

    setbitu(rtcm->buff,i,2,flg_trop); i+=2;
    setbitu(rtcm->buff,i,2,flg_stec); i+=2;
    setbitu(rtcm->buff,i,5,inet); i+=5;
    setbitu(rtcm->buff,i,6,atmos->ng); i+=6;

    if (flg_trop>0) {
        setbitu(rtcm->buff,i,6,atmos->trop_quality); i+=6;
        if (flg_trop&0x2) { /* trop functional term */
            setbitu(rtcm->buff,i,2,atmos->trop_type); i+=2;
            ct[0]=ROUND(atmos->ct[0]/0.004);
            setbits(rtcm->buff,i,9,ct[0]); i+=9;
            if (atmos->trop_type>0) {
                ct[1]=ROUND(atmos->ct[1]/0.002);
                ct[2]=ROUND(atmos->ct[2]/0.002);
                setbits(rtcm->buff,i,7,ct[1]); i+=7;
                setbits(rtcm->buff,i,7,ct[2]); i+=7;
            }
            if (atmos->trop_type>1) {
                ct[3]=ROUND(atmos->ct[3]/0.001);
                setbits(rtcm->buff,i,7,ct[3]); i+=7;
            }
        }
        if (flg_trop&0x1) { /* trop residual term */
            setbitu(rtcm->buff,i,1,atmos->sz_trop); i+=1;
            sz=(atmos->sz_trop==0)?6:8;
            trop_ofst=ROUND_U(atmos->trop_ofst/0.02);
            setbitu(rtcm->buff,i,4,trop_ofst); i+=4;
            for (j=0;j<atmos->ng;j++) {
                trop_wet=ROUND((atmos->trop_wet[j]-atmos->trop_ofst)/0.004);
                setbits(rtcm->buff,i,sz,trop_wet); i+=sz;
            }
        }
    }

    if (flg_stec>0) {
        cssr->net_svmask[inet]=getbitu(rtcm->buff,i,nsat); i+=nsat; /* stec correction type */
        for (j=s=0;j<nsat;j++) {
            if (!((cssr->net_svmask[inet]>>(nsat-1-j))&1)) {
                continue;
            }
            setbitu(rtcm->buff,i,6,atmos->stec_quality[s]); i+=6;
            if (flg_stec&0x2) {
                stec_type=atmos->stec_type[s];
                setbitu(rtcm->buff,i,2,stec_type); i+=2;
                ci[0]=ROUND(atmos->ci[s][0]/0.05);
                setbits(rtcm->buff,i,14,ci[0]); i+=14;
                if (stec_type>0) {
                    ci[1]=ROUND(atmos->ci[s][1]/0.02);
                    ci[2]=ROUND(atmos->ci[s][2]/0.02);
                    setbits(rtcm->buff,i,12,ci[1]); i+=12;
                    setbits(rtcm->buff,i,12,ci[2]); i+=12;
                }
                if (stec_type>1) {
                    ci[3]=ROUND(atmos->ci[s][3]/0.02);
                    setbits(rtcm->buff,i,10,ci[3]); i+=10;
                }
                if (stec_type>2) {
                    ci[4]=ROUND(atmos->ci[s][4]/0.005);
                    ci[5]=ROUND(atmos->ci[s][5]/0.005);
                    setbits(rtcm->buff,i,8,ci[4]); i+=8;
                    setbits(rtcm->buff,i,8,ci[5]); i+=8;
                }
            }
            if (flg_stec&0x1) {
                sz_idx=atmos->sz_stec[s];
                sz=dstec_sz_t[sz_idx];
                setbitu(rtcm->buff,i,2,sz_idx); i+=2;
                for (k=0;k<atmos->ng;k++) {
                    dlat=atmos->pos[k][0]-atmos->pos[0][0];
                    dlon=atmos->pos[k][1]-atmos->pos[0][1];
                    stec0=atmos->ci[s][0];
                    if (stec_type>0) {
                        stec0+=atmos->ci[s][1]*dlat+atmos->ci[s][2]*dlon;
                    }
                    if (stec_type>1) {
                        stec0+=atmos->ci[s][3]*dlat*dlon;
                    }
                    if (stec_type>2) {
                        stec0+=(atmos->ci[s][4]*dlat*dlat)+(atmos->ci[s][5]*dlon*dlon);
                    }
                    dstec=atmos->stec[k][s]-stec0;
                    dstec_i=ROUND(dstec/dstec_lsb_t[sz_idx]);
                    setbits(rtcm->buff,i,sz,dstec_i); i+=sz;
                }
                s++;
            }
        }
    }
    rtcm->nbit=i;
    return 1;
}

extern int encode_cssr(rtcm_t *rtcm, int subtype, int sync)
{
    switch (subtype) {
        case CSSR_TYPE_MASK:    return encode_cssr_mask(rtcm,subtype,sync);
        case CSSR_TYPE_OCC:     return encode_cssr_occ(rtcm,subtype,sync);
        case CSSR_TYPE_BIAS:    return encode_cssr_bias(rtcm,subtype,sync);
        case CSSR_TYPE_URA:     return encode_cssr_ura(rtcm,subtype,sync);
        case CSSR_TYPE_ATMOS:   return encode_cssr_atmos(rtcm,subtype,sync);
    }
    return 1;
}

