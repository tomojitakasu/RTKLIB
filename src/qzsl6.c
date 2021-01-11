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

#define L6FRMLEN        2000            /* L6 frame length (bits) */
#define LL6HDRLEN       49              /* L6 header length (bits) */
#define L6RSLEN         256             /* L6 reed solomon length (bits) */
#define L6FRMPREAMB     0x1ACFFC1Du     /* L6 frame preamble */
#define RTCM3PREAMB     0xD3            /* rtcm ver.3 frame preamble */

#define L6MSG_SF_MSG     5
#define L6MSG_LEN         250
#define L6MSG_SF_LEN     (L6MSG_SF_MSG*L6MSG_LEN)
#define VENDOR_ID_CLAS    5
#define BLEN_MSG 218

#define CSSR_TYPE_MASK  1

/* preamble,subframe-start marker */
static int sync_l6msg(uint8_t *buff,uint8_t data){
    int i,n=4;
    uint8_t b;
    for (i=0;i<n-1;i++) buff[i]=buff[i+1];
    buff[n-1]=data;
    for (i=0;i<4;i++) { /* preamble check */
        b=(L6FRMPREAMB>>(24-8*i))&0xff;
        if (buff[i]!=b) return 0;
    }
    return 1;
}
/* read L6 message from stream */
static int input_l6dataf(uint8_t *buff,FILE *fp)
{
    int i,data;
    for (i=0;;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if (sync_l6msg(buff,(uint8_t)data)) break;
        if (i>4096) return 0;
    }
    if (fread(buff+4,1,L6MSG_LEN-4,fp)<L6MSG_LEN-4) return -2;
    return 0;
}

/* decode L6 messages in file stream
 * - decoded per sub-frame if sf==1
 */
extern int input_l6msgsf(rtcm_t *rtcm,int sf,FILE *fp)
{
    uint8_t type;
    int facility,i=rtcm->nbit,j;
    static int start=0,facility0=-1,n=0,ns=0;
    static uint8_t buff[250];
    cssr_t *cssr=&rtcm->cssr;
    while (1) {
        if(input_l6dataf(buff,fp)<0) return -1;
        type=buff[5];
        facility=(type>>3)&0x3;
        if (type&0x1) {
            start=1;
            facility0=facility;
            i=n=ns=0;
            rtcm->nbit=0;
            cssr->nbit=0;
        }
        if (!start) continue;
        if (facility!=facility0) {
            start=0; continue;
        }
        setbitu(rtcm->buff,i,7,buff[6]&0x7f);i+=7;
        for (j=0;j<211;j++) {
            setbitu(rtcm->buff,i,8,buff[7+j]);i+=8;
        }
        ns|=(1<<n);
        n++;
        rtcm->nbit+=1695;
        if (sf==0) break;
        if (ns==0x1f) break; /* subframe obtained */
    }
    rtcm->nbyte=(rtcm->nbit+7)/8;
    return 10;
}

/* convert L6 messages into RTCM 3 messages */
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

