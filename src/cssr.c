/*------------------------------------------------------------------------------
* cssr.c : Compact SSR message decode functions
*
*          Copyright (C) 2015- by Mitsubishi Electric Corp., All rights reserved.
*
* references :
*     see rtcm3.c
*
*-----------------------------------------------------------------------------*/
#include "rtklib.h"
#include "cssr.h"

cssr_t _cssr = {0,};

/* constants -----------------------------------------------------------------*/

/* ssr update intervals ------------------------------------------------------*/
static const double ssrudint[16]={
    1,2,5,10,15,30,60,120,240,300,600,900,1800,3600,7200,10800
};

#define MAX_NGRID   4           /* number of grids for interpolation */
#define MAX_DIST    100.0       /* max distance to grid (km) */
#define MAX_AGE     300.0       /* max age of difference (s) */

#define SQR(x)      ((x)*(x))
#define MAX(x,y)    ((x)>(y)?(x):(y))
#define MIN(x,y)    ((x)<(y)?(x):(y))

static double decode_sval(unsigned char *buff, int i, int n, double lsb)
{
	int slim=-((1<<(n-1))-1)-1,v;
	v = getbits(buff,i,n);
	return (v==slim) ? INVALID_VALUE:(double)v*lsb;
}

static int sys2gnss(int sys, int *prn_min)
{
    int id = CSSR_SYS_NONE, prn0=1;

    switch (sys) {
        case SYS_GPS: id = CSSR_SYS_GPS; break;
        case SYS_GLO: id = CSSR_SYS_GLO; break;
        case SYS_GAL: id = CSSR_SYS_GAL; break;
        case SYS_CMP: id = CSSR_SYS_BDS; break;
        case SYS_SBS: id = CSSR_SYS_SBS; prn0 = 120; break;
        case SYS_QZS: id = CSSR_SYS_QZS; prn0 = 193; break;
        case SYS_IRN: id = CSSR_SYS_IRN; break;
    }

    if (prn_min) *prn_min = prn0;
    return id;
}

/* convert GNSS ID of cssr to system id of rtklib */
static int gnss2sys(int id, int *prn_min)
{
    int sys = SYS_NONE, prn0=1;

    switch (id) {
        case CSSR_SYS_GPS: sys = SYS_GPS; break;
        case CSSR_SYS_GLO: sys = SYS_GLO; break;
        case CSSR_SYS_GAL: sys = SYS_GAL; break;
        case CSSR_SYS_BDS: sys = SYS_CMP; break;
        case CSSR_SYS_SBS: sys = SYS_SBS; prn0=120; break;
        case CSSR_SYS_QZS: sys = SYS_QZS; prn0=193; break;
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

static int svmask2nsatlist(uint64_t svmask, int id, int *sat)
{
    int j,nsat=0,sys,prn_min;

    sys = gnss2sys(id, &prn_min);
    for (j=0;j<CSSR_MAX_SV_GNSS;j++) {
        if ((svmask>>(CSSR_MAX_SV_GNSS-1-j))&1) {
            sat[nsat++] = satno(sys, prn_min+j);
        }
    }
    return nsat;
}

/* convert from svmask to satellite list */
static int svmask2sat(uint64_t *svmask,int *sat)
{
    int j,id,nsat=0,sys,prn_min;

    for (id=0;id<CSSR_MAX_GNSS;id++) {
        sys = gnss2sys(id, &prn_min);
        for (j=0;j<CSSR_MAX_SV_GNSS;j++) {
            if ((svmask[id]>>(CSSR_MAX_SV_GNSS-1-j))&1) {
            	if (sat)
            		sat[nsat] = satno(sys,prn_min+j);
            	nsat++;
            }
        }
    }
    return nsat;
}
/* decode tropo quality indicator */
static float decode_cssr_quality(int a, int b)
{
    float quality;

    if ((a == 0 && b == 0) || (a == 7 && b == 7)) {
        quality = 9999;
    } else {
        quality = (1.0+b*0.25)*pow(3.0,a)-1.0;
    }

    return quality;
}

/* decode cssr message header */
static int decode_cssr_head(rtcm_t *rtcm, cssr_t *cssr, int *sync, int *tow,
       int *iod, int *iod_sv, double *udint, int *ngnss, int i0, int header)
{
    int i=i0,udi,subtype;

    subtype = getbitu(rtcm->buff,i,4); i+=4;
    if (subtype ==  CSSR_TYPE_MASK) {
        *tow = getbitu(rtcm->buff,i,20); i+=20; /* gps epoch time */
    } else {
        *tow = cssr->tow0 + getbitu(rtcm->buff,i,12); i+=12; /* gps epoch time (hourly) */
    }

    trace(4,"decode_cssr_head: subtype=%d epoch=%4d\n",subtype,*tow);

    udi = getbitu(rtcm->buff,i, 4); i+= 4; /* update interval */
    *sync = getbitu(rtcm->buff,i, 1); i+= 1; /* multiple message indicator */
    *udint = ssrudint[udi];
    *iod = getbitu(rtcm->buff,i, 4); i+= 4; /* iod ssr */

    if (subtype == CSSR_TYPE_MASK) {
        cssr->iod = *iod;
        *ngnss = getbitu(rtcm->buff,i, 4); i+= 4; /* number of gnss */
    } else {
    	if (cssr->iod!=*iod) {
    		trace(4,"decode_cssr_head: iod mismatch subtype=%d epoch=%4d\n",subtype,*tow);
    		return -1;
    	}
    }

    return i;
}

/* decode mask message */
static int decode_cssr_mask(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,sync,tow,iod,nsat_g=0,id,nsig,ncell=0,sat[CSSR_MAX_SV];
    double udint;

    i = decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&cssr->ngnss,i0,header);
    cssr->tow0 = floor(tow/3600.0)*3600.0;
    rtcm->time = gpst2time(cssr->week,tow);
    for (j=0;j<CSSR_MAX_GNSS;j++) {
        cssr->cmi[j] = 0;
        cssr->svmask[j] = 0;
        cssr->sigmask[j] = 0;
    }
    for (j=0;j<CSSR_MAX_SV;j++) {
        cssr->cellmask[j] = 0;
    }

    trace(3,"decode_cssr_mask: sync=%d tow=%d iod=%d\n",sync, tow, cssr->iod);

    for (k=0;k<cssr->ngnss;k++) {
        id = getbitu(rtcm->buff,i, 4); i+= 4; /* gnss id */
        cssr->svmask[id] = (uint64_t)getbitu(rtcm->buff,i,8)<<32; i+= 8; /* sv mask */
        cssr->svmask[id] |= getbitu(rtcm->buff,i,32); i+= 32; /* sv mask */
        cssr->sigmask[id] = getbitu(rtcm->buff,i,16); i+= 16; /* signal mask */
        cssr->cmi[id] = getbitu(rtcm->buff,i,1); i++; /* cell mask availability */

        nsig = sigmask2nsig(cssr->sigmask[id]);
        nsat_g = svmask2nsatlist(cssr->svmask[id], id, sat);
        for (j=0;j<nsat_g;j++) {
        	if (cssr->cmi[id]) {
        		cssr->cellmask[ncell] = getbitu(rtcm->buff,i,nsig); i+=nsig;
        	} else {
        		cssr->cellmask[ncell] = (1<<nsig)-1;
        	}
            ncell++;
        }
    }
    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is enough to decode the mask message */
static int check_bit_width_mask(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
	int k,i=i0,ngnss=0,cmi=0,nsig,nsat,n;
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
static int decode_cssr_oc(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,iode;
    int prn,gnss=0;
    double udint;
    ssr_t *ssr=NULL;

    if ((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    nsat = svmask2sat(cssr->svmask,sat);
    rtcm->time = gpst2time(cssr->week,tow);

    trace(2,"decode_cssr_oc:   sync=%d tow=%d iod=%d\n",sync,tow,iod);
    
    for (j=0;j<nsat;j++) {
    	ssr = &rtcm->ssr[sat[j]-1];
        if ((gnss=sys2gnss(satsys(sat[j],&prn),NULL))==CSSR_SYS_GAL) {
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
        trace(4, "ssr orbit: prn=%2d, tow=%d, udi=%.1f, iod=%2d, orb=%f,%f,%f\n",
        		sat[j],tow,udint,cssr->iod,ssr->deph[0],ssr->deph[1],ssr->deph[2]);
    }

    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is enough to decode the orbit correction message */
static int check_bit_width_oc(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
	int i=i0+25,k,sat[CSSR_MAX_SV],nsat,prn;

	if (i>rtcm->nbit) return -1;
	nsat = svmask2sat(cssr->svmask,sat);
	for (k=0;k<nsat;k++) {
		i+=(satsys(sat[k],&prn)==SYS_GAL)?51:49;
		if (i>rtcm->nbit) return -1;
	}
	return i-i0;
}

/* decode clock correction */
static int decode_cssr_cc(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys;
    double udint;
    ssr_t *ssr=NULL;

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);

    trace(2,"decode_cssr_cc:   sync=%d tow=%d iod=%d\n",sync,tow,iod);

    for (j=0;j<nsat;j++) {
    	ssr = &rtcm->ssr[sat[j]-1];
        sys=satsys(sat[j], &prn);
        ssr->t0 [1]=rtcm->time;
        ssr->udi[1]=udint;
        ssr->iod[1]=cssr->iod;

        ssr->dclk[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
        ssr->dclk[1] = ssr->dclk[2] = 0.0;
        ssr->update=1;
        trace(4, "ssr clock: sys=%d prn=%2d, tow=%d, udi=%.1f, iod=%2d, clk=%f\n",
        		sys,prn,tow,udint,cssr->iod,ssr->dclk[0]);
    }
    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the clock correction message */
static int check_bit_width_cc(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int nsat,nbit;

    nsat = svmask2sat(cssr->svmask,NULL);
    nbit = 25+15*nsat;
    if (nbit+i0>rtcm->nbit) return -1;
    return nbit;
}

/* decode available signals from sigmask */
static int sigmask2sig(int nsat, int *sat, uint16_t *sigmask,
		uint16_t *cellmask, int *nsig, uint8_t *sig)
{
    int j,k,id,sys,sys_p=-1,ofst=0,nsig_s=0;
    uint8_t *codes=NULL,code[CSSR_MAX_SIG];
    const uint8_t codes_gps[CSSR_MAX_SIG]={
        CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1S,CODE_L1L,CODE_L1X,
        CODE_L2S,CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,
        CODE_L5I,CODE_L5Q,CODE_L5X
    };
    const uint8_t codes_glo[CSSR_MAX_SIG]={
        CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P,CODE_L3I,CODE_L3Q,CODE_L3X
    };
    const uint8_t codes_gal[CSSR_MAX_SIG]={
        CODE_L1B,CODE_L1C,CODE_L1X,CODE_L5I,CODE_L5Q,
        CODE_L5X,CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L8I,CODE_L8Q,
        CODE_L8X,CODE_L6B,CODE_L6C
    };
    const uint8_t codes_qzs[CSSR_MAX_SIG]={
        CODE_L1C,CODE_L1S,CODE_L1L,CODE_L1X,CODE_L2S,CODE_L2L,CODE_L2X,
        CODE_L5I,CODE_L5Q,CODE_L5X,CODE_L6S,CODE_L6L,CODE_L6E,CODE_L1C
    };
    const uint8_t codes_bds[CSSR_MAX_SIG]={
        CODE_L2I,CODE_L2Q,CODE_L2X,CODE_L6I,CODE_L6Q,CODE_L6X,
        CODE_L7I,CODE_L7Q,CODE_L7X,CODE_L1D,CODE_L1P,       0,
		CODE_L5D,CODE_L5P
    };
    const uint8_t codes_sbs[CSSR_MAX_SIG]={
        CODE_L1C,CODE_L5I,CODE_L5Q,CODE_L5X
    };
    const uint8_t codes_irn[CSSR_MAX_SIG]={
               0,       0,       0,CODE_L5A,       0,       0,CODE_L9A
    };

    for (j=0;j<nsat;j++,ofst+=nsig_s) {
        sys = satsys(sat[j], NULL);
        if (sys != sys_p) {
            id = sys2gnss(sys, NULL);
            ofst = 0;
            switch (sys) {
                case SYS_GPS: codes = (uint8_t *)codes_gps; break;
                case SYS_GLO: codes = (uint8_t *)codes_glo; break;
                case SYS_GAL: codes = (uint8_t *)codes_gal; break;
                case SYS_CMP: codes = (uint8_t *)codes_bds; break;
                case SYS_QZS: codes = (uint8_t *)codes_qzs; break;
                case SYS_SBS: codes = (uint8_t *)codes_sbs; break;
                case SYS_IRN: codes = (uint8_t *)codes_irn; break;
            }
            for (k=0,nsig_s=0;k<CSSR_MAX_SIG;k++) {
                if ((sigmask[id]>>(CSSR_MAX_SIG-1-k))&1) {
                    code[nsig_s] = codes[k];
                    nsig_s++;
                }
            }
        }
        sys_p = sys;

        for (k=0, nsig[j]=0;k<nsig_s;k++) {
            if ((cellmask[j]>>(nsig_s-1-k))&1) {
            	if (sig)
            		sig[j*CSSR_MAX_SIG+nsig[j]] = code[k];
                nsig[j]++;
            }
        }
    }

    return 1;
}

/* decode code bias message */
static int decode_cssr_cb(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys;
    int nsig[CSSR_MAX_SV];
    static uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    ssr_t *ssr=NULL;

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    rtcm->time=gpst2time(cssr->week,tow);
    nsat=svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);

    trace(4,"decode_cssr_cb: sync=%d tow=%d iod=%d\n",sync,tow,iod);
    
    for (k=0;k<nsat;k++) {
    	ssr=&rtcm->ssr[sat[k]-1];
        sys=satsys(sat[k],&prn);
        for (j=0;j<nsig[k];j++) {
            if ((s=sig[k*CSSR_MAX_SIG+j])>0) {
            	ssr->cbias[s-1]=decode_sval(rtcm->buff,i,11,0.02); i+=11;
            	trace(4, "ssr cbias: sys=%d prn=%2d, tow=%d, cbias=%f\n",
            			sys,prn,tow,ssr->cbias[s-1]);
            }
        }
        ssr->t0 [4]=rtcm->time;
        ssr->udi[4]=udint;
        ssr->iod[4]=iod;
        ssr->update=1;
    }
    cssr->nbit = i;
    return sync ? 0:10;
}

/* decode phase bias message */
static int decode_cssr_pb(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,s,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,prn,sys,sdc;
    int nsig[CSSR_MAX_SV];
    uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    ssr_t *ssr=NULL;

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);

    trace(4,"decode_cssr_pb: sync=%d tow=%d iod=%d\n",sync,tow,iod);

    for (k=0;k<nsat;k++) {
    	ssr = &rtcm->ssr[sat[k]-1];
        sys = satsys(sat[k],&prn);
        for (j=0;j<nsig[k];j++) {
            s = sig[k*CSSR_MAX_SIG+j];
            ssr->pbias[s-1] = decode_sval(rtcm->buff,i,15,0.001); i+=15;
            sdc = getbitu(rtcm->buff,i,2); i+=2;
            trace(4, "ssr pbias: sys=%d prn=%2d, tow=%d, pbias=%f, disc=%d\n",
            		sys,prn,tow,ssr->pbias[s-1],sdc);
        }
        ssr->t0 [5]=rtcm->time;
        ssr->udi[5]=udint;
        ssr->iod[5]=iod;
        ssr->update=1;
    }
    cssr->nbit=i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the code bias message */
static int check_bit_width_cb(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int nsig[CSSR_MAX_SV],k,sat[CSSR_MAX_SV],nsat,nsig_total=0,nbit;

    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,NULL);
    for (k=0;k<nsat;k++) {
    	nsig_total+=nsig[k];
    }
    nbit = 25+nsig_total*11;
    if (i0+nbit>rtcm->nbit) return -1;
    return nbit;
}

static int check_bit_width_pb(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int nsig[CSSR_MAX_SV],k,sat[CSSR_MAX_SV],nsat,nsig_total=0,nbit;

    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,NULL);
    for (k=0;k<nsat;k++) {
    	nsig_total+=nsig[k];
    }
    nbit=25+nsig_total*17;
    if (i0+nbit>rtcm->nbit) return -1;
    return nbit;
}

/* code bias correction */
static int decode_cssr_bias(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,sdc;
    int nsig[CSSR_MAX_SV],flg_cb,flg_pb,flg_net,inet=0,netmask=0,prn,sys;
    uint8_t sig[CSSR_MAX_SV*CSSR_MAX_SIG];
    double udint;
    ssr_t *ssr=NULL;

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    rtcm->time = gpst2time(cssr->week, tow);

    flg_cb = getbitu(rtcm->buff,i,1); i+=1;
    flg_pb = getbitu(rtcm->buff,i,1); i+=1;
    flg_net = getbitu(rtcm->buff,i,1); i+=1;

    nsat = svmask2sat(cssr->svmask,sat);
    sigmask2sig(nsat,sat,cssr->sigmask,cssr->cellmask,nsig,sig);

    if (flg_net) {
        inet = getbitu(rtcm->buff,i,5); i+=5;
        netmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
        rtcm->staid = inet;
    } else {
    	rtcm->staid = 0;
    }

    trace(2,"decode_cssr_bias: sync=%d tow=%d iod=%d net=%d\n",sync,tow,iod,inet);

    for (k=0;k<nsat;k++) {
        if (flg_net&&(!((netmask>>(nsat-1-k)) & 1))) continue;
    	ssr = &rtcm->ssr[sat[k]-1];
        sys = satsys(sat[k],&prn);
        if (flg_cb) {
            ssr->t0 [4]=rtcm->time;
            ssr->udi[4]=udint;
            ssr->iod[4]=cssr->iod;
        }
        if (flg_pb) {
            ssr->t0 [5]=rtcm->time;
            ssr->udi[5]=udint;
            ssr->iod[5]=cssr->iod;
        }
        ssr->update=1;
        for (j=0;j<nsig[k];j++) {
            s = sig[k*CSSR_MAX_SIG+j];
            if (flg_cb) { /* code bias */
            	ssr->cbias[s-1] = decode_sval(rtcm->buff,i,11,0.02); i+=11;
                trace(4, "ssr cbias: sys=%d,prn=%2d, tow=%d, net=%d, s=%d, cbias=%f\n",
                		sys,prn,tow,inet,s,ssr->cbias[s-1]);
            }
            if (flg_pb) { /* phase bias */
            	ssr->pbias[s-1] = decode_sval(rtcm->buff,i,15,0.001); i+=15;
                sdc = getbitu(rtcm->buff, i, 2); i+= 2;
                trace(4, "ssr pbias: sys=%d,prn=%2d, tow=%d, net=%d, s=%d, pbias=%f, disc=%d\n",
                		sys,prn,tow,inet,s,ssr->pbias[s-1],sdc);
            }
        }
    }

    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the bias message */
static int check_bit_width_bias(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int i=i0+25,k,nsat,slen=0,flg_cb,flg_pb,flg_net,netmask=0;

    nsat = svmask2sat(cssr->svmask,NULL);
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
        if (i+slen>rtcm->nbit) return -1;
        i += slen;
    }
    return i-i0;
}

/* decode ura correction */
static int decode_cssr_ura(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,iod,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,sys,prn;
    double udint;
    ssr_t *ssr = NULL;

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);

    trace(4,"decode_cssr_ura: sync=%d tow=%d iod=%d\n",sync, tow, iod);

    for (j=0;j<nsat;j++) {
    	ssr = &rtcm->ssr[sat[j]-1];
        ssr->t0 [3]=rtcm->time;
        ssr->udi[3]=udint;
        ssr->iod[3]=iod;
        ssr->ura = getbitu(rtcm->buff,i, 6); i+= 6; /* ssr ura */
        ssr->update=1;
        sys = satsys(sat[j],&prn);
        trace(4, "decode_cssr_ura: sys=%d, prn=%2d, tow=%d, udi=%.1f, iod=%2d, ura=%d\n",
        		sys,prn,tow,udint,iod,ssr->ura);
    }
    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the ura message */
static int check_bit_width_ura(rtcm_t *rtcm,cssr_t *cssr,int i0)
{
    int nsat,nbit;

    nsat = svmask2sat(cssr->svmask,NULL);
    nbit=25+6*nsat;
    if(i0+nbit>rtcm->nbit) return -1;
    return nbit;
}

/* decode stec correction */
static int decode_cssr_stec(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,iod,s,sync,tow,ngnss,sat[CSSR_MAX_SV],nsat,inet,a,b,sys,prn;
    double udint;
    atmos_t *atmos=NULL;
    
    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
		return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask, sat);

    trace(3,"decode_cssr_stec: sync=%d tow=%d iod=%d\n",sync, tow, iod);

    cssr->opt.stec_type = getbitu(rtcm->buff,i, 2); i+= 2; /* stec correction type */
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
            a = getbitu(rtcm->buff,i, 3); i+= 3;
            b = getbitu(rtcm->buff,i, 3); i+= 3;
            atmos->stec_quality[s] = decode_cssr_quality(a,b);

            for (k=0;k<4;k++) atmos->ai[s][k] = 0.0;

            atmos->ai[s][0] = decode_sval(rtcm->buff,i,14,0.05); i+=14;
            if (cssr->opt.stec_type > 0) {
            	atmos->ai[s][1] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
            	atmos->ai[s][2] = decode_sval(rtcm->buff,i,12,0.02); i+=12;
            }
            if (cssr->opt.stec_type > 1) {
            	atmos->ai[s][3] = decode_sval(rtcm->buff,i,10,0.02); i+=10;
            }
            s++;

            trace(4, "ssr stec: sys=%d, prn=%2d, tow=%d, udi=%.1f, iod=%2d, quality=%d a=(%4.2f,%4.2f,%4.2f,%4.2f)\n",
            		sys,prn,tow,udint,iod,
					atmos->stec_quality,atmos->ai[s][0],atmos->ai[s][1],atmos->ai[s][2],atmos->ai[s][3]);
        }
    }
    cssr->inet = inet;
    atmos->nsat[0] = s;
    atmos->update = 1;
    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the stec message */
static int check_bit_width_stec(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int i=i0+25,j,sat[CSSR_MAX_SV],nsat,stec_type,slen=0,nsat_local=0;
    uint64_t net_svmask;
    const int slen_t[4] = {20,44,54,0};

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
static int decode_cssr_grid(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,ii,s,sync,iod,tow,ngnss,sat[CSSR_MAX_SV],nsat,ofst=0,sz;
    int trop_type,sz_idx,inet,a,b,hs,wet,sys,prn;
    double udint,stec0,dlat,dlon,iono,dstec;
    atmos_t *atmos=NULL;
    static char wetsign[CSSR_MAX_NETWORK*RTCM_SSR_MAX_GP]={0},hydsign[CSSR_MAX_NETWORK*RTCM_SSR_MAX_GP]={0};

    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    		return -1;
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask,sat);

    trop_type = getbitu(rtcm->buff,i,2); i+=2;  /* troposphere correction type */
    sz_idx = getbitu(rtcm->buff,i,1); i++; /* stec range */
    sz = (sz_idx)?16:7;
    inet = getbitu(rtcm->buff,i,5); i+=5; /* network id */

    trace(2,"decode_cssr_grid: sync=%d tow=%d iod=%d net=%d\n",sync,tow,iod,inet);

    atmos = &rtcm->atmos[inet];

    cssr->net_svmask[inet] = getbitu(rtcm->buff,i, nsat); i+= nsat; /* stec correction type */
    a = getbitu(rtcm->buff,i, 3); i+= 3;
    b = getbitu(rtcm->buff,i, 3); i+= 3;
    atmos->ng = getbitu(rtcm->buff,i, 6); i+= 6;
    atmos->time = rtcm->time;
    atmos->trop_quality = decode_cssr_quality(a,b);
    atmos->inet = inet;

    for (j=0;j<atmos->ng;j++) {
        ofst=inet*RTCM_SSR_MAX_GP+j;
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
                atmos->trop_wet[j] = wet*0.004+0.252;
                atmos->trop_total[j] = (hs+wet)*0.004+0.252+CSSR_TROP_HS_REF;

#if 0
                if (nav->rtcmmode == RTCMMODE_CSSR) {
                    add_grid_idx(nav,ssrg->gp[j].pos,ofst);
                    add_data_trop(nav->zwd+ofst, rtcm->time,
                                  ssrg->trop_wet[j], ssrg->trop_total[j],ssrg->quality,0,valid);
                    trace(5,"decode_cssr_grid time=%s,net=%d,ig=%d,wet=%.3f,total=%.3f\n",
                          time_str(ssrg->t0,0),inet,j,nav->zwd[ofst].data[0].zwd,
                          nav->zwd[ofst].data[0].ztd);
                }
#endif
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
                    stec0 = atmos->ai[ii][0]+atmos->ai[ii][1]*dlat+
                            atmos->ai[ii][2]*dlon+
                            atmos->ai[ii][3]*dlat*dlon;
                	atmos->stec[j][s] = stec0+dstec;
                	iono = 40.3E16/(FREQ1*FREQ2)*atmos->stec[j][s];
                }
                atmos->sat[j][s] = sat[k];

                trace(5,"decode_cssr_grid time=%s,net=%d,ig=%d,pos=%6.3f,%6.3f,sys=%d,prn=%d,iono=%.4f\n",
                		time_str(atmos->time,0),inet,j,atmos->pos[j][0]*R2D,atmos->pos[j][1]*R2D,
						sys,prn,iono);

#if 0
                if (nav->rtcmmode == RTCMMODE_CSSR) {
                    add_data_stec(nav->stec+ofst,rtcm->time,sat[k],0,iono,
                                  0.0,0,ssr_ion->stec.quality[ii]);
                    trace(5,"decode_cssr_grid time=%s,net=%d,ig=%d,pos=%6.3f,%6.3f,sat=%d,iono=%.4f\n",
                          time_str(ssrg->t0,0),inet,j,ssrg->gp[j].pos[0]*R2D,ssrg->gp[j].pos[1]*R2D,
                          ssrg->sat[j][s],iono);
                }
#endif
                s++;
                ii++;
            }
        }
        atmos->nsat[j] = s;
    }

    cssr->nbit = i;
    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the grid message */
static int check_bit_width_grid(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int i=i0+25,k,nsat,trop_type,ngp,sz_trop,sz_idx,sz_stec,nsat_local=0;
    uint64_t net_svmask;

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
static int decode_cssr_combo(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,sync,iod,tow,ngnss,sat[CSSR_MAX_SV],nsat,iode,sz;
    int flg_orbit,flg_clock,flg_net,netid=0,s,sys,prn;
    uint64_t net_svmask=0;
    double udint;
    ssr_t *ssr=NULL;
    
    if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
    	return -1;
    /*check_week_ref(rtcm, tow, ref_combined);*/
    rtcm->time = gpst2time(cssr->week, tow);
    nsat = svmask2sat(cssr->svmask, sat);
    
    trace(2, "decode_cssr_combo:sync=%d tow=%d iod=%d\n", sync, tow, iod);
    
    flg_orbit = getbitu(rtcm->buff,i,1); i+=1;
    flg_clock = getbitu(rtcm->buff,i,1); i+=1;
    flg_net = getbitu(rtcm->buff,i,1); i+=1;
    if (flg_net) {
    	netid = getbitu(rtcm->buff,i,5); i+=5;
    	net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
    	rtcm->staid = netid;
    }
    
    for (j=s=0;j<nsat;j++) {
        if (!flg_net||(flg_net && ((net_svmask>>(nsat-1-j))&1))) {
        	ssr = &rtcm->ssr[sat[j]-1];
            sys = satsys(sat[j], &prn);

            if (flg_orbit) {
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
                
                trace(4, "combined orbit: network=%d, tow=%d, sys=%d, prn=%d, iode=%d, deph=%f, %f, %f\n",
                		netid,tow,sys,prn,ssr->iode,ssr->deph[0],ssr->deph[1],ssr->deph[2]);
            }
            if (flg_clock) {
            	ssr->dclk[0] = decode_sval(rtcm->buff,i,15,0.0016); i+=15;
                
                ssr->t0 [3] = rtcm->time;
                ssr->udi[3] = udint;
                ssr->iod[3] = cssr->iod;

                trace(4, "combined clock: network=%d, tow=%d, sys=%d, prn=%d, dclk=%f\n",
                		netid,tow,sys,prn,ssr->dclk);
            }
            s++;
        }
    }

    cssr->nbit = i;
    return sync ? 0: 10;
}

/* check if the buffer length is sufficient to decode the orbit/clock combined message */
static int check_bit_width_combo(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int i=i0+25,sat[CSSR_MAX_SV],nsat,j,flg_orbit,flg_clock,flg_net,sz;
    uint64_t net_svmask=0;

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
static int decode_cssr_atmos(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
    int i,j,k,s,sync,tow,iod,ngnss,sat[CSSR_MAX_SV],nsat,sz_idx,sys,prn,sz;
    int flg_trop,flg_stec,trop_type,stec_type,inet,a,b;
    double udint,stec0,ct[4]={0},ci[6]={0},trop_ofst,dlat,dlon,dstec;
    atmos_t *atmos;
    const float dstec_lsb_t[4] = {0.04f,0.12f,0.16f,0.24f};
    const int dstec_sz_t[4] = {4,4,5,7};
    
	if((i=decode_cssr_head(rtcm,cssr,&sync,&tow,&iod,NULL,&udint,&ngnss,i0,header))<0)
		return -1;
    rtcm->time = gpst2time(cssr->week,tow);
    nsat = svmask2sat(cssr->svmask,sat);
    
    flg_trop = getbitu(rtcm->buff,i,2); i+=2;  /* troposphere correction type */
    flg_stec = getbitu(rtcm->buff,i,2); i+=2;  /* stec correction type */
    inet = getbitu(rtcm->buff,i,5); i+=5;       /* network id */
    
    trace(3, "decode_cssr_atmos:tow=%d iod=%d net=%d\n",tow,iod,inet);
    
    rtcm->atmos[0].inet = inet;
    atmos = &rtcm->atmos[inet];
    
    atmos->ng = getbitu(rtcm->buff,i,6); i+=6;
    atmos->time = rtcm->time;
    atmos->inet = inet;
    
    for (j = 0; j < RTCM_SSR_MAX_GP; ++j) {
        atmos->nsat[j] = 0;
    }
    
    if (flg_trop) {
        a = getbitu(rtcm->buff,i,3); i+=3;
        b = getbitu(rtcm->buff,i,3); i+=3;
        atmos->trop_quality = decode_cssr_quality(a,b);
        trop_type = getbitu(rtcm->buff, i, 2); i+=2;
        for (k=0;k<4;k++) ct[k] = 0.0;
        ct[0] = decode_sval(rtcm->buff,i,9,0.004); i+=9;
        if (trop_type>0) {
        	ct[1] = decode_sval(rtcm->buff,i,7,0.002); i+=7;
        	ct[2] = decode_sval(rtcm->buff,i,7,0.002); i+=7;
        }
        if (trop_type>1) {
        	ct[3] = decode_sval(rtcm->buff,i,7,0.001); i+=7;
        }

        sz_idx = getbitu(rtcm->buff, i, 1); i+=1;
        trop_ofst = getbitu(rtcm->buff, i, 4)*0.02; i+=4;
        trace(4, "decode_cssr_atmos: network=%d, tow=%d, trop=0x%02x, stec=0x%02x, ngp=%d, trop_type=%d, ct=%.3f %.3f %.3f %.3f, size=%d, offset=%.3f\n",
            atmos->inet,tow,flg_trop,flg_stec,atmos->ng,trop_type,ct[0],ct[1],ct[2],ct[3],sz_idx,trop_ofst);
        sz = (sz_idx==0) ? 6:8;
        
        for (j = 0; j < atmos->ng; ++j) {
            dlat = atmos->pos[j][0]-atmos->pos[0][0];
            dlon = atmos->pos[j][1]-atmos->pos[0][1];
            
            atmos->trop_total[j] = CSSR_TROP_HS_REF+ct[0];
            if (trop_type>0) {
                atmos->trop_total[j] += (ct[1]*dlat)+(ct[2]*dlon);
            }
            if (trop_type>1) {
                atmos->trop_total[j] += ct[3]*dlat*dlon;
            }
            atmos->trop_wet[j] = decode_sval(rtcm->buff,i,sz,0.004); i+=sz;
            if (atmos->trop_wet[j]==INVALID_VALUE) {
            	atmos->trop_total[j] = INVALID_VALUE;
            	trace(2,"trop(wet) is invalid: tow=%d, inet=%d, grid=%d\n",tow,inet,j);
            } else {
            	atmos->trop_wet[j] += trop_ofst;
            	atmos->trop_total[j] += atmos->trop_wet[j];
            }
#if 0
            add_grid_idx(nav, ssrg->gp[j].pos, ofst);
            add_data_trop(nav->zwd+ofst, rtcm->time,
                ssrg->trop_wet[j], ssrg->trop_total[j], ssrg->quality, 0, valid);
#endif
            trace(4, "decode_cssr_atmos: network=%d, tow=%d, quality=%.2f, index=%d, pos=%.3f %.3f %.3f, total=%.3f, wet=%.3f\n",
                atmos->inet,tow,atmos->trop_quality,j,atmos->pos[j][0]*R2D,atmos->pos[j][1]*R2D,
				atmos->pos[j][2],atmos->trop_total[j],atmos->trop_wet[j]);
        }
    }
    
    if (flg_stec) {
        cssr->net_svmask[inet] = getbitu(rtcm->buff, i, nsat); i += nsat; /* stec correction type */
        trace(4, "decode_cssr_atmos: mask=0x%x\n", cssr->net_svmask[inet]);
        
        for (j=s=0;j<nsat;j++) {
            if (!((cssr->net_svmask[inet]>>(nsat-1-j))&1)) {
                continue;
            }
			sys = satsys(sat[j], &prn);
            a = getbitu(rtcm->buff,i,3); i+=3;
            b = getbitu(rtcm->buff,i,3); i+=3;
            atmos->stec_quality[s] = decode_cssr_quality(a,b);

            stec_type = getbitu(rtcm->buff,i,2); i+=2;
            
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
            sz_idx = getbitu(rtcm->buff,i,2); i+=2;
            trace(4,"decode_cssr_atmos: stec_type=%d, ct=%.2f %.2f %.2f %.2f %.2f %.2f, size=%d\n",
                stec_type,ci[0],ci[1],ci[2],ci[3],ci[4],ci[5],sz_idx);
            
            for (k=0;k<atmos->ng;k++) {
                dlat = atmos->pos[k][0]-atmos->pos[0][0];
                dlon = atmos->pos[k][1]-atmos->pos[0][1];
                dstec = decode_sval(rtcm->buff,i,dstec_sz_t[sz_idx],dstec_lsb_t[sz_idx]);
                i+=dstec_sz_t[sz_idx];
                trace(5,"size=%d, dstec=%f\n",dstec_sz_t[sz_idx],dstec);
                if (dstec==INVALID_VALUE) {
                	atmos->stec[k][s] = INVALID_VALUE;
                    trace(2,"dstec is invalid: tow=%d, inet=%d, grid=%d, sat=%d\n",tow,inet,k,sat[j]);
                } else {
                    stec0 = ci[0];
                    if (stec_type>0) {
                        stec0 += (ci[1]*dlat)+(ci[2]*dlon);
                    }
                    if (stec_type>1) {
                        stec0 += ci[3]*dlat*dlon;
                    }
                    if (stec_type>2) {
                        stec0 += (ci[4]*dlat*dlat)+(ci[5]*dlon*dlon);
                    }
                    atmos->stec[k][s] = stec0+dstec;
                }

                atmos->sat[k][s] = sat[j];
                atmos->nsat[k]++;
#if 0
                add_data_stec(nav->stec+ofst,rtcm->time,ssrg->sat[k][s],
                    0,iono,0.0,0,quality);
#endif
                trace(4,"decode_cssr_atmos: network=%d, tow=%.1f, sys=%d, prn=%d, quality=%.2f, grid=%d, stec=%.4f\n",
                    inet,time2gpst(rtcm->time, NULL),sys,prn,atmos->stec_quality[s],k,atmos->stec[k][s]);
            }
            s++;
        }
        for (k=0;k<atmos->ng;k++) {
            atmos->nsat[k] = s;
            trace(4,"decode_cssr_atmos: grid=%d, nsv=%d\n",k+1,atmos->nsat[k]);
        }
    }
    cssr->nbit = i;
    return sync ? 0: 10;
}

/* check if the buffer length is sufficient to decode the atmospheric correction message */
static int check_bit_width_atmos(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
    int i=i0+4,flg_trop,flg_stec,trop_type,stec_type,ngp,sz_idx,sz,j,nsat;
    uint64_t net_svmask;
    const int dstec_sz_t[4] = {4,4,5,7};
    const int trop_sz_t[3] = {9,23,30};
	const int stec_sz_t[4] = {14,38,48,64};
    
    nsat = svmask2sat(cssr->svmask,NULL);
    if (i+36>rtcm->nbit) return -1;
    i+=21;
    flg_trop = getbitu(rtcm->buff,i,2); i+=2;
    flg_stec = getbitu(rtcm->buff,i,2); i+=2;
    i+=5;
    ngp = getbitu(rtcm->buff,i,6); i+=6;
    if (flg_trop) {
        if (i+8>rtcm->nbit) return -1;
        i+=6;
        trop_type = getbitu(rtcm->buff,i,2); i+=2;
        sz = trop_sz_t[trop_type];
        if (i+sz+5>rtcm->nbit) return -1;
        i+=sz;
        sz_idx = getbitu(rtcm->buff,i,1); i+=1;
        i+=4;
        sz = (sz_idx==0)?6:8;
        if (i+sz*ngp>rtcm->nbit) return -1;
        i+=sz*ngp;
    }
    
    if (flg_stec) {
        if (i+nsat>rtcm->nbit) return -1;
        net_svmask = getbitu(rtcm->buff,i,nsat); i+=nsat;
        for (j=0;j<nsat;j++) {
            if (!((net_svmask>>(nsat-1-j))&1)) continue;
            if (i+8>rtcm->nbit) return -1;
            i+=6;
            stec_type = getbitu(rtcm->buff,i,2); i+=2;
            sz = stec_sz_t[stec_type];
            if (i+sz+2>rtcm->nbit) return -1;
            i+=sz;
            sz_idx = getbitu(rtcm->buff,i,2); i+=2;
            i+=ngp*dstec_sz_t[sz_idx];
            if (i>rtcm->nbit) return -1;
        }
    }
    trace(4,"check_bit_width_atmos(): i0=%d, nbit=%d\n",i,rtcm->nbit);
    return i-i0;
}

/*
 * decode service information message
 */
static int decode_cssr_si(rtcm_t *rtcm, cssr_t *cssr, int i0, int header)
{
	int i=i0+4,j,sync,j0;

	sync = getbitu(rtcm->buff,i,1); i+=1; /* multiple message indicator */
	cssr->si_cnt = getbitu(rtcm->buff,i,3); i+=3;  /* information message counter */
	cssr->si_sz = getbitu(rtcm->buff,i,2)+1; i+=2; /* data size */

	j0=cssr->si_cnt*cssr->si_sz*5;
	for (j=0;j<cssr->si_sz;j++) {
		cssr->si_data[j0+j] = (uint64_t)getbitu(rtcm->buff,i,8)<<32; i+=8;
		cssr->si_data[j0+j] |= getbitu(rtcm->buff,i,32); i+=32;
	}

	if (sync==0 && cssr->flg_cssr_si == (1<<cssr->si_cnt)-1) {
		/* decode SI */
		cssr->flg_cssr_si = 0;
	}

    return sync ? 0:10;
}

/* check if the buffer length is sufficient to decode the service information message */
static int check_bit_width_si(rtcm_t *rtcm, cssr_t *cssr, int i0)
{
	int i=i0+4,data_sz=0;
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
    int i=i0+12,ret=0,subtype;
    cssr_t *cssr = &_cssr;

    i += (head) ? 24:0;
    subtype = getbitu(rtcm->buff,i,4);
    trace(4,"decode_cssr subtype=%d\n",subtype);

    switch (subtype) {
		case CSSR_TYPE_MASK:
			ret=decode_cssr_mask(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_OC:
			ret=decode_cssr_oc(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_CC:
			ret=decode_cssr_cc(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_CB:
			ret=decode_cssr_cb(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_PB:
			ret=decode_cssr_pb(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_BIAS:
			ret=decode_cssr_bias(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_URA:
			ret=decode_cssr_ura(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_STEC:
			ret=decode_cssr_stec(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_GRID:
			ret=decode_cssr_grid(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_OCC:
			ret=decode_cssr_combo(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_ATMOS:
			ret = decode_cssr_atmos(rtcm, cssr, i, head);
			break;
		case CSSR_TYPE_SI:
			ret = decode_cssr_si(rtcm, cssr, i, head);
			break;
		default: break;
    }
    return ret;
}

extern int cssr_check_bitlen(rtcm_t *rtcm,int i0)
{
	int i=i0+12,subtype,nbit=0;
    cssr_t *cssr = &_cssr;
    subtype = getbitu(rtcm->buff,i,4);

	switch (subtype) {
		case CSSR_TYPE_MASK:
			if ((nbit=check_bit_width_mask(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_OC:
			if ((nbit=check_bit_width_oc(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_CC:
			if ((nbit=check_bit_width_cc(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_CB:
			if ((nbit=check_bit_width_cb(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_PB:
			if ((nbit=check_bit_width_pb(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_BIAS:
			if ((nbit=check_bit_width_bias(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_URA:
			if ((nbit=check_bit_width_ura(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_STEC:
			if ((nbit=check_bit_width_stec(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_GRID:
			if ((nbit=check_bit_width_grid(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_OCC:
			if ((nbit=check_bit_width_combo(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_ATMOS:
			if ((nbit=check_bit_width_atmos(rtcm,cssr,i))<0) return 0;
			break;
		case CSSR_TYPE_SI:
			if ((nbit=check_bit_width_si(rtcm,cssr,i))<0) return 0;
			break;
		default:
			trace(2,"invalid subtype:%d nbit=%d\n",subtype,rtcm->nbit);
			return 0;
	}
	return nbit;
}

extern int decode_cssr_msg(rtcm_t *rtcm, int head, uint8_t *frame)
{
    cssr_t *cssr = &_cssr;
    int startbit,week,type,subtype;
    int i, ret = 0;
    double tow;

    if (*frame==0x00 || rtcm->nbit==-1) {
        return 0;
    }

    i = startbit = cssr->nbit;
    if (i+16>rtcm->nbit) return 0;
    type = getbitu(rtcm->buff,i,12);
    if (type != 4073) {
        trace(4, "cssr: decode terminate: frame=%02x, nbit=%d, nbit=%d\n",
        		*frame,rtcm->nbit,cssr->nbit);
        cssr->nbit = 0;
        *frame = 0;
        return 0;
    }
    subtype = getbitu(rtcm->buff,i+12,4);
    if (cssr_check_bitlen(rtcm,i)<0) return 0;
    i+=16;

    tow = time2gpst(timeget(), &week);
    trace(4, "cssr: frame=%02x, type=%d, subtype=%d, week=%d, tow=%.2f\n",
    		*frame,type,subtype, week, tow);

    ret=decode_cssr(rtcm,cssr->nbit,0);

    if (ret==-1) {
    	cssr->nbit=0;
    	cssr->iod=-1;
    	*frame=0;
    	return 0;
    }

    return ret;
}


