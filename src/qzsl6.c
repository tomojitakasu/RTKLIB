/*------------------------------------------------------------------------------
* qzsl6.c : qzss L6 functions
*
* references :
*     [1] IS-QZSS-L6-003, Quasi-Zenith Satellite System
*         Interface Specification Centimeter Level Augmentation Service,
*         Cabinet Office, August 20, 2020

* history : 2021/01/05 1.0  new (based on qzslex.c)
*-----------------------------------------------------------------------------*/
#include "rtklib.h"
#include "cssr.h"

#define L6FRMLEN        2000            /* L6 frame length (bits) */
#define LL6HDRLEN       49              /* L6 header length (bits) */
#define L6RSLEN         256             /* L6 reed solomon length (bits) */
#define L6FRMPREAMB     0x1ACFFC1Du     /* L6 frame preamble */
#define RTCM3PREAMB     0xD3            /* rtcm ver.3 frame preamble */

#define L6MSG_SF_MSG 	5
#define L6MSG_LEN 		250
#define L6MSG_SF_LEN 	(L6MSG_SF_MSG*L6MSG_LEN)
#define VENDOR_ID_CLAS	5
#define BLEN_MSG 218

extern cssr_t _cssr;

/* preamble,subframe-start marker */
static int sync_l6msg(uint8_t *buff,int chk_start,uint8_t data){
	int i,n=4;
	uint8_t b;
	if (chk_start) n=6;
	for (i=0;i<n-1;i++) buff[i]=buff[i+1];
	buff[n-1]=data;
	for (i=0;i<4;i++) { /* preamble check */
		b=(L6FRMPREAMB>>(24-8*i))&0xff;
		if (buff[i]!=b) return 0;
	}
	if (chk_start)
		return buff[5]&0x1; /* subframe start marker */
	return 1;
}

/* read and decode QZS L6 stream */
int read_qzs_msg(rtcm_t *rtcm, unsigned char *pbuff, int nframe)
{
    int i=0,j,k,jn,jn0=-1,prn,msgid,alert;
    unsigned char *buff;

    for (j=0;j<5;j++) {
        buff = pbuff+j*BLEN_MSG;

        prn = buff[4];
        msgid = buff[5];
        alert = (buff[6]>>7) & 0x1;

        if (msgid & 0x1) { /* head */
            jn0 = j;
            break;
        }
    }

    if (jn0<0) {
        memset(rtcm->buff, 0x00, sizeof(rtcm->buff));
        return 0;
    }

    trace(4,"read_qzs_msg prn=%d msgid=%d alert=%d\n",prn,msgid,alert);

    for (j=0,jn=jn0;j<5;j++,jn++) {
        if (jn >= 5) {
            jn = 0;
        }
        buff = pbuff+jn*BLEN_MSG;

        setbitu(rtcm->buff,i,7, buff[6] & 0x7f); i+=7;
        for (k=0;k<211;k++) {
            setbitu(rtcm->buff,i,8, buff[7+k]); i+=8;
        }
    }

    return 0;
}

/* decode L6 messages in the QZS L6 subframe */
extern int input_l6msg(rtcm_t *rtcm, uint8_t data, uint8_t *frame)
{
	int i,j,facility;
    uint8_t prn,msgid,alert;
    static int nframe = 0;
    static uint8_t buff[BLEN_MSG];
    static int decode_start=0,facility0=-1;
    cssr_t *cssr=&_cssr;

    /* synchronize frame */
    if (rtcm->nbyte==0) {
    	if(sync_l6msg(buff,0,data)==0) return 0;
        rtcm->nbyte=4;
        return 0;
    }
    buff[rtcm->nbyte++]=data;
    rtcm->len = BLEN_MSG;

    if (rtcm->nbyte<rtcm->len) return 0;
    rtcm->nbyte=0;

    prn = buff[4];
    msgid = buff[5];
    alert = (buff[6]>>7) & 0x1;

    if (msgid&0x1) {
    	rtcm->nbit=0;
    	cssr->nbit=0;
    	decode_start=1;
    	*frame = 0;
    	nframe = 0;
    	facility0=(msgid>>2)&0x3;
    }

    if (decode_start) {
    	facility=(msgid>>2)&0x3;
    	if (facility!=facility0) return 0;
    	i=1695*nframe;
    	setbitu(rtcm->buff,i,7,buff[6]&0x7f); i+=7;
    	for (j=0;j<211;j++) {
    		setbitu(rtcm->buff,i,8,buff[7+j]); i+=8;
    	}
    	rtcm->nbit += 1695;
    	*frame |= (1<<nframe);
    	nframe++;
    }
    trace(4,"input_l6msg: prn=%3d msgid=%3d alert=%d frame=%d rtcm-nbit=%d\n",
    		prn,msgid,alert,*frame,rtcm->nbit);
    return 0;
}

/* decode L6 messages from file stream ---------------------------------------------*/
extern int input_l6msgf(rtcm_t *rtcm,FILE *fp)
{
    int i,data=0,ret;
    static uint8_t frame = 0;

    trace(4,"input_cssrf: data=%02x\n",data);
    for (i=0;i<4096;i++) {
        if ((ret=decode_cssr_msg(rtcm,0,&frame))) return ret;
        if ((data=fgetc(fp))==EOF) return -2;
        input_l6msg(rtcm, (uint8_t)data, &frame);
    }
    return 0; /* return at every 4k bytes */
}

/* input L6 message subframe from file stream
 * ---------------------------------------------*/
extern int input_l6msgsf(rtcm_t *rtcm,FILE *fp)
{
	int i,j,k,ii,sz=0,data,facility,facility_p=-1,type,
		vendor_id;
	static uint8_t buff[250*5];

	/* sync sub-frame */
	for (i=0;;i++) {
		if ((data=fgetc(fp))==EOF) return -2;
		if (sync_l6msg(buff,1,(uint8_t)data)) break;
		if (i>4096) return 0;
	}

	if (fread(buff+6,1,L6MSG_SF_LEN-6,fp)<L6MSG_SF_LEN-6) return -2;

	/* check vendor-id, facility-id */
	for (k=0;k<5;k++) {
		type=buff[L6MSG_LEN*k+5];
		vendor_id =(type>>5)&0x7;
		if (vendor_id!=VENDOR_ID_CLAS) return 0;
		facility =(type>>3)&0x3;
		if (k>0 && facility!=facility_p) return 0;
		facility_p=facility;
	}
	/* copy buffer */
	for (k=0,j=0;k<5;k++) {
		i=k*2000+49;
		for (ii=0;ii<212;ii++,i+=sz,j+=sz) {
			sz=(ii==211)?7:8;
			setbitu(rtcm->buff,j,sz,getbitu(buff,i,sz));
		}
	}

	rtcm->nbit=1695*5;
	rtcm->nbyte=(rtcm->nbit+7)/8;
    return 10;
}

extern int l6msg2rtcm(rtcm_t *rtcm,int i0,uint8_t *buff)
{
	int i=i0,j,n=0,type,subtype,ret,nbit=0;
	uint32_t crc;

	type=getbitu(rtcm->buff,i,12);
	if (type!=4073) return 0;

	subtype=getbitu(rtcm->buff,i+12,4);
	if (subtype==CSSR_TYPE_MASK) {
		if((ret=decode_cssr(rtcm,i,0))<0) return 0;
	}
	nbit=cssr_check_bitlen(rtcm,i);
	if (i+nbit>rtcm->nbit) return 0;
	n=(nbit+7)/8;
	setbitu(buff, 0, 8,RTCM3PREAMB);
	setbitu(buff, 8, 6,0);
	setbitu(buff,14,10,n);
	for(j=0;j<n;j++) {
		buff[j+3]=getbitu(rtcm->buff,i+j*8,8);
	}
	crc=rtk_crc24q(buff,3+n);
	setbitu(buff,24+n*8,24,crc);

	printf("subtype=%2d nbit=%4d cssr->nbit=%4d\n",subtype,nbit,i);
	return nbit;
}

