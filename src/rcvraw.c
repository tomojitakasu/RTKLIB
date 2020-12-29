/*------------------------------------------------------------------------------
* rcvraw.c : receiver raw data functions
*
*          Copyright (C) 2009-2020 by T.TAKASU, All rights reserved.
*          Copyright (C) 2014 by T.SUZUKI, All rights reserved.
*
* references :
*     [1] IS-GPS-200K, Navstar GPS Space Segment/Navigation User Interfaces,
*         March 4, 2019
*     [2] Global navigation satellite system GLONASS interface control document
*         navigation radiosignal in bands L1,L2 (version 5.1), 2008
*     [3] BeiDou satellite navigation system signal in space interface control
*         document open service signal B1I (version 3.0), February, 2019
*     [4] Quasi-Zenith Satellite System Interface Specification Satellite
*         Positioning, Navigation and Timing Service (IS-QZSS-PN-003), November
*         5, 2018
*     [5] European GNSS (Galileo) Open Service Signal In Space Interface Control
*         Document, Issue 1.3, December, 2016
*     [6] ISRO-IRNSS-ICD-SPS-1.1, Indian Regional Navigation Satellite System
*         Signal in Space ICD for Standard Positioning Service version 1.1,
*         August, 2017
*
* version : $Revision:$ $Date:$
* history : 2009/04/10 1.0  new
*           2009/06/02 1.1  support glonass
*           2010/07/31 1.2  support eph_t struct change
*           2010/12/06 1.3  add almanac decoding, support of GW10
*                           change api decode_frame()
*           2013/04/11 1.4  fix bug on decode fit interval
*           2014/01/31 1.5  fix bug on decode fit interval
*           2014/06/22 1.6  add api decode_glostr()
*           2014/06/22 1.7  add api decode_bds_d1(), decode_bds_d2()
*           2014/08/14 1.8  add test_glostr()
*                           add support input format rt17
*           2014/08/31 1.9  suppress warning
*           2014/11/07 1.10 support qzss navigation subframes
*           2016/01/23 1.11 enable septentrio
*           2016/01/28 1.12 add decode_gal_inav() for galileo I/NAV
*           2016/07/04 1.13 support CMR/CMR+
*           2017/05/26 1.14 support TERSUS
*           2018/10/10 1.15 update reference [5]
*                           add set of eph->code/flag for galileo and beidou
*           2018/12/05 1.16 add test of galileo i/nav word type 5
*           2020/11/30 1.17 add API decode_gal_fnav() and decode_irn_nav()
*                           allocate double size of raw->nav.eph[] for multiple
*                            ephemeris sets (e.g. Gallieo I/NAV and F/NAV)
*                           no support of STRFMT_LEXR by API input_raw/rawf()
*                           update references [1], [3] and [4]
*                           add reference [6]
*                           use integer types in stdint.h
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#define P2_8        0.00390625            /* 2^-8 */
#define P2_15       3.051757812500000E-05 /* 2^-15 */
#define P2_28       3.725290298461914E-09 /* 2^-28 */
#define P2_34       5.820766091346740E-11 /* 2^-34 */
#define P2_41       4.547473508864641E-13 /* 2^-41 */
#define P2_46       1.421085471520200E-14 /* 2^-46 */
#define P2_51       4.440892098500626E-16 /* 2^-51 */
#define P2_59       1.734723475976810E-18 /* 2^-59 */
#define P2_66       1.355252715606881E-20 /* 2^-66 */
#define P2_68       3.388131789017201E-21 /* 2^-68 */
#define P2P11       2048.0                /* 2^11 */
#define P2P12       4096.0                /* 2^12 */
#define P2P14       16384.0               /* 2^14 */
#define P2P15       32768.0               /* 2^15 */
#define P2P16       65536.0               /* 2^16 */

#define SQR(x)      ((x)*(x))

/* get two component bits ----------------------------------------------------*/
static uint32_t getbitu2(const uint8_t *buff, int p1, int l1, int p2, int l2)
{
    return (getbitu(buff,p1,l1)<<l2)+getbitu(buff,p2,l2);
}
static int32_t getbits2(const uint8_t *buff, int p1, int l1, int p2, int l2)
{
    if (getbitu(buff,p1,1))
        return (int32_t)((getbits(buff,p1,l1)<<l2)+getbitu(buff,p2,l2));
    else
        return (int32_t)getbitu2(buff,p1,l1,p2,l2);
}
/* get three component bits --------------------------------------------------*/
static uint32_t getbitu3(const uint8_t *buff, int p1, int l1, int p2, int l2,
                         int p3, int l3)
{
    return (getbitu(buff,p1,l1)<<(l2+l3))+(getbitu(buff,p2,l2)<<l3)+
            getbitu(buff,p3,l3);
}
static int32_t getbits3(const uint8_t *buff, int p1, int l1, int p2, int l2,
                        int p3, int l3)
{
    if (getbitu(buff,p1,1))
        return (int32_t)((getbits(buff,p1,l1)<<(l2+l3))+
                   (getbitu(buff,p2,l2)<<l3)+getbitu(buff,p3,l3));
    else
        return (int32_t)getbitu3(buff,p1,l1,p2,l2,p3,l3);
}
/* merge two components ------------------------------------------------------*/
static uint32_t merge_two_u(uint32_t a, uint32_t b, int n)
{
    return (a<<n)+b;
}
static int32_t merge_two_s(int32_t a, uint32_t b, int n)
{
    return (int32_t)((a<<n)+b);
}
/* get sign-magnitude bits ---------------------------------------------------*/
static double getbitg(const uint8_t *buff, int pos, int len)
{
    double value=getbitu(buff,pos+1,len-1);
    return getbitu(buff,pos,1)?-value:value;
}
/* decode NavIC/IRNSS ephemeris ----------------------------------------------*/
static int decode_irn_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_irn={0};
    double tow1,tow2,toc,sqrtA;
    int i,id1,id2,week;
    
    trace(4,"decode_irn_eph:\n");
    
    i=8; /* subframe 1 */
    tow1          =getbitu(buff,i,17)*12.0;         i+=17+2;
    id1           =getbitu(buff,i, 2);              i+= 2+1;
    week          =getbitu(buff,i,10);              i+=10;
    eph_irn.f0    =getbits(buff,i,22)*P2_31;        i+=22;
    eph_irn.f1    =getbits(buff,i,16)*P2_43;        i+=16;
    eph_irn.f2    =getbits(buff,i, 8)*P2_55;        i+= 8;
    eph_irn.sva   =getbitu(buff,i, 4);              i+= 4;
    toc           =getbitu(buff,i,16)*16.0;         i+=16;
    eph_irn.tgd[0]=getbits(buff,i, 8)*P2_31;        i+= 8;
    eph_irn.deln  =getbits(buff,i,22)*P2_41*SC2RAD; i+=22;
    eph_irn.iode  =getbitu(buff,i, 8);              i+= 8+10;
    eph_irn.svh   =getbitu(buff,i, 2);              i+= 2;
    eph_irn.cuc   =getbits(buff,i,15)*P2_28;        i+=15;
    eph_irn.cus   =getbits(buff,i,15)*P2_28;        i+=15;
    eph_irn.cic   =getbits(buff,i,15)*P2_28;        i+=15;
    eph_irn.cis   =getbits(buff,i,15)*P2_28;        i+=15;
    eph_irn.crc   =getbits(buff,i,15)*0.0625;       i+=15;
    eph_irn.crs   =getbits(buff,i,15)*0.0625;       i+=15;
    eph_irn.idot  =getbits(buff,i,14)*P2_43*SC2RAD;
    
    i=8*37+8; /* subframe 2 */
    tow2          =getbitu(buff,i,17)*12.0;         i+=17+2;
    id2           =getbitu(buff,i, 2);              i+= 2+1;
    eph_irn.M0    =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_irn.toes  =getbitu(buff,i,16)*16.0;         i+=16;
    eph_irn.e     =getbitu(buff,i,32)*P2_33;        i+=32;
    sqrtA         =getbitu(buff,i,32)*P2_19;        i+=32;
    eph_irn.OMG0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_irn.omg   =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_irn.OMGd  =getbits(buff,i,22)*P2_41*SC2RAD; i+=22;
    eph_irn.i0    =getbits(buff,i,32)*P2_31*SC2RAD;
    
    /* test subframe id, tow and consistency of toe and toc */
    if (id1!=0||id2!=1||tow1+12.0!=tow2||toc!=eph_irn.toes) {
        return 0;
    }
    eph_irn.A=SQR(sqrtA);
    eph_irn.iodc=eph_irn.iode;
    week=adjgpsweek(week);
    eph_irn.week=week; /* week number consistent to toe */
    eph_irn.toe=eph_irn.toc=gpst2time(eph_irn.week,eph_irn.toes);
    if      (tow1<eph_irn.toes-302400.0) week++;
    else if (tow1>eph_irn.toes+302400.0) week--;
    eph_irn.ttr=gpst2time(week,tow1);
    *eph=eph_irn;
    return 1;
}
/* decode NavIC/IRNSS iono parameters ----------------------------------------*/
static int decode_irn_ion(const uint8_t *buff, double *ion)
{
    int i,id3,id4;
    
    trace(4,"decode_irn_ion:\n");
    
    /* subframe 3 and 4 message ids */
    id3=getbitu(buff,8*37*2+30,6);
    id4=getbitu(buff,8*37*3+30,6);

    /* 11: eop and ionosphere coefficients */
    if      (id3==11) i=8*37*2+174;
    else if (id4==11) i=8*37*3+174;
    else return 0;
    
    ion[0]=getbits(buff,i,8)*P2_30; i+=8;
    ion[1]=getbits(buff,i,8)*P2_27; i+=8;
    ion[2]=getbits(buff,i,8)*P2_24; i+=8;
    ion[3]=getbits(buff,i,8)*P2_24; i+=8;
    ion[4]=getbits(buff,i,8)*P2P11; i+=8;
    ion[5]=getbits(buff,i,8)*P2P14; i+=8;
    ion[6]=getbits(buff,i,8)*P2P16; i+=8;
    ion[7]=getbits(buff,i,8)*P2P16;
    return 1;
}
/* decode NavIC/IRNSS UTC parameters -----------------------------------------*/
static int decode_irn_utc(const uint8_t *buff, double *utc)
{
    int i,id3,id4;

    trace(4,"decode_irn_utc:\n");
    
    /* subframe 3 and 4 message ids */
    id3=getbitu(buff,8*37*2+30,6);
    id4=getbitu(buff,8*37*3+30,6);
    
    /* 9 or 26: utc and time sync parameters */
    if      (id3==9||id3==26) i=8*37*2+36;
    else if (id4==9||id3==26) i=8*37*3+36;
    else return 0;

    utc[0]=getbits(buff,i,16)*P2_35; i+=16; /* A0 */
    utc[1]=getbits(buff,i,13)*P2_51; i+=13; /* A1 */
    utc[8]=getbits(buff,i, 7)*P2_68; i+= 7; /* A2 */
    utc[4]=getbits(buff,i, 8);       i+= 8; /* dt_LS */
    utc[2]=getbitu(buff,i,16)*16.0;  i+=16; /* tot */
    utc[3]=getbitu(buff,i,10);       i+=10; /* WNt */
    utc[5]=getbitu(buff,i,10);       i+=10; /* WN_LSF */
    utc[6]=getbitu(buff,i, 4);       i+= 4; /* DN */
    utc[7]=getbits(buff,i, 8);              /* dt_LSF */
    return 1;
}
/* decode NavIC/IRNSS navigation data ------------------------------------------
* decode NavIC/IRNSS navigation data (ref [6] 5.9-6)
* args   : uint8_t *buff    I   NavIC/IRNSS subframe data (CRC checked)
*                                 buff[  0- 36]: subframe 1 (292 bits)
*                                 buff[ 37- 73]: subframe 2
*                                 buff[ 74-110]: subframe 3
*                                 buff[111-147]: subframe 4
*          eph_t *eph       IO  NavIC/IRNSS ephemeris        (NULL: not output)
*          double *ion      IO  NavIC/IRNSS iono parametgers (NULL: not output)
*                                 ion[0-3]: alpha_0,...,alpha_3
*                                 ion[4-7]: beta_0,...,beta_3
*          double *utc      IO  NavIC/IRNSS UTC parametgers  (NULL: not output)
*                                 utc[0-3]: A0,A1,tot,WNt
*                                 utc[4-7]: dt_LS,WN_LSF,DN,dt_LSF
*                                 utc[8]  : A2
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_irn_nav(const uint8_t *buff, eph_t *eph, double *ion,
                          double *utc)
{
    trace(4,"decode_irn_nav:\n");
    
    if (eph&&!decode_irn_eph(buff,eph)) return 0;
    if (ion&&!decode_irn_ion(buff,ion)) return 0;
    if (utc&&!decode_irn_utc(buff,utc)) return 0;
    return 1;
}
/* decode Galileo I/NAV ephemeris --------------------------------------------*/
static int decode_gal_inav_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_gal={0};
    double tow,toc,tt,sqrtA;
    int i,week,svid,e5b_hs,e1b_hs,e5b_dvs,e1b_dvs,type[6],iod_nav[4];
    
    trace(4,"decode_gal_inav_eph:\n");
    
    i=128; /* word type 1 */
    type[0]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[0]  =getbitu(buff,i,10);              i+=10;
    eph_gal.toes=getbitu(buff,i,14)*60.0;         i+=14;
    eph_gal.M0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.e   =getbitu(buff,i,32)*P2_33;        i+=32;
    sqrtA       =getbitu(buff,i,32)*P2_19;
    
    i=128*2; /* word type 2 */
    type[1]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[1]  =getbitu(buff,i,10);              i+=10;
    eph_gal.OMG0=getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.i0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.omg =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.idot=getbits(buff,i,14)*P2_43*SC2RAD;
    
    i=128*3; /* word type 3 */
    type[2]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[2]  =getbitu(buff,i,10);              i+=10;
    eph_gal.OMGd=getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    eph_gal.deln=getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
    eph_gal.cuc =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.cus =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.crc =getbits(buff,i,16)*P2_5;         i+=16;
    eph_gal.crs =getbits(buff,i,16)*P2_5;         i+=16;
    eph_gal.sva =getbitu(buff,i, 8);
    
    i=128*4; /* word type 4 */
    type[3]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[3]  =getbitu(buff,i,10);              i+=10;
    svid        =getbitu(buff,i, 6);              i+= 6;
    eph_gal.cic =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.cis =getbits(buff,i,16)*P2_29;        i+=16;
    toc         =getbitu(buff,i,14)*60.0;         i+=14;
    eph_gal.f0  =getbits(buff,i,31)*P2_34;        i+=31;
    eph_gal.f1  =getbits(buff,i,21)*P2_46;        i+=21;
    eph_gal.f2  =getbits(buff,i, 6)*P2_59;
    
    i=128*5; /* word type 5 */
    type[4]     =getbitu(buff,i, 6);              i+= 6+11+11+14+5;
    eph_gal.tgd[0]=getbits(buff,i,10)*P2_32;      i+=10; /* BGD E5a/E1 */
    eph_gal.tgd[1]=getbits(buff,i,10)*P2_32;      i+=10; /* BGD E5b/E1 */
    e5b_hs      =getbitu(buff,i, 2);              i+= 2;
    e1b_hs      =getbitu(buff,i, 2);              i+= 2;
    e5b_dvs     =getbitu(buff,i, 1);              i+= 1;
    e1b_dvs     =getbitu(buff,i, 1);              i+= 1;
    week        =getbitu(buff,i,12);              i+=12; /* gst-week */
    tow         =getbitu(buff,i,20);
    
    /* test word types */
    if (type[0]!=1||type[1]!=2||type[2]!=3||type[3]!=4||type[4]!=5) {
        trace(3,"decode_gal_inav error: type=%d %d %d %d %d\n",type[0],type[1],
              type[2],type[3],type[4]);
        return 0;
    }
    /* test consistency of iod_nav */
    if (iod_nav[0]!=iod_nav[1]||iod_nav[0]!=iod_nav[2]||iod_nav[0]!=iod_nav[3]) {
        trace(3,"decode_gal_inav error: iod_nav=%d %d %d %d\n",iod_nav[0],
              iod_nav[1],iod_nav[2],iod_nav[3]);
        return 0;
    }
    if (!(eph_gal.sat=satno(SYS_GAL,svid))) {
        trace(2,"decode_gal_inav svid error: svid=%d\n",svid);
        return 0;
    }
    eph_gal.A=sqrtA*sqrtA;
    eph_gal.iode=eph_gal.iodc=iod_nav[0];
    eph_gal.svh=(e5b_hs<<7)|(e5b_dvs<<6)|(e1b_hs<<1)|e1b_dvs;
    eph_gal.ttr=gst2time(week,tow);
    tt=timediff(gst2time(week,eph_gal.toes),eph_gal.ttr);
    if      (tt> 302400.0) week--; /* week consistent with toe */
    else if (tt<-302400.0) week++;
    eph_gal.toe=gst2time(week,eph_gal.toes);
    eph_gal.toc=gst2time(week,toc);
    eph_gal.week=week+1024; /* gal-week = gst-week + 1024 */
    eph_gal.code=(1<<9); /* I/NAV: af0-2,Toc,SISA for E5b-E1 */
    *eph=eph_gal;
    return 1;
}
/* decode Galileo I/NAV iono parameters --------------------------------------*/
static int decode_gal_inav_ion(const uint8_t *buff, double *ion)
{
    int i=128*5; /* word type 5 */

    trace(4,"decode_gal_inav_ion:\n");
    
    if (getbitu(buff,i,6)!=5) return 0;
    i+=6;
    ion[0]=getbitu(buff,i,11)*0.25;  i+=11;
    ion[1]=getbits(buff,i,11)*P2_8;  i+=11;
    ion[2]=getbits(buff,i,14)*P2_15; i+=14;
    ion[3]=getbitu(buff,i, 5);
    return 1;
}
/* decode Galileo I/NAV UTC parameters ---------------------------------------*/
static int decode_gal_inav_utc(const uint8_t *buff, double *utc)
{
    int i=128*6; /* word type 6 */
    
    trace(4,"decode_gal_inav_utc:\n");
    
    if (getbitu(buff,i,6)!=6) return 0;
    i+=6;
    utc[0]=getbits(buff,i,32)*P2_30;  i+=32; /* A0 */
    utc[1]=getbits(buff,i,24)*P2_50;  i+=24; /* A1 */
    utc[4]=getbits(buff,i, 8);        i+= 8; /* dt_LS */
    utc[2]=getbitu(buff,i, 8)*3600.0; i+= 8; /* tot */
    utc[3]=getbitu(buff,i, 8);        i+= 8; /* WNt */
    utc[5]=getbitu(buff,i, 8);        i+= 8; /* WN_LSF */
    utc[6]=getbitu(buff,i, 3);        i+= 3; /* DN */
    utc[7]=getbits(buff,i, 8);               /* dt_LSF */
    return 1;
}
/* decode Galileo I/NAV navigation data ----------------------------------------
* decode Galileo I/NAV navigation data (ref [5] 4.3)
* args   : uint8_t *buff    I   Galileo I/NAV subframe data (CRC checked)
*                                 buff[ 0- 15]: word type 0 (128 bits)
*                                 buff[16- 31]: word type 1
*                                 buff[32- 47]: word type 2
*                                 buff[48- 63]: word type 3
*                                 buff[64- 79]: word type 4
*                                 buff[80- 95]: word type 5
*                                 buff[96-111]: word type 6
*          eph_t    *eph    IO  Galileo I/NAV ephemeris       (NULL: not output)
*          double   *ion    IO  Galileo I/NAV iono parameters (NULL: not output)
*                                 ion[0-3]: a_i0,a_i1,a_i2,flags
*          double   *utc    IO  Galileo I/NAV UTC parameters  (NULL: not output)
*                                 utc[0-3]: A0,A1,tot,WNt
*                                 utc[4-7]: dt_LS,WN_LSF,DN,dt_LSF
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_gal_inav(const uint8_t *buff, eph_t *eph, double *ion,
                           double *utc)
{
    trace(4,"decode_gal_fnav:\n");
    
    if (eph&&!decode_gal_inav_eph(buff,eph)) return 0;
    if (ion&&!decode_gal_inav_ion(buff,ion)) return 0;
    if (utc&&!decode_gal_inav_utc(buff,utc)) return 0;
    return 1;
}
/* decode Galileo F/NAV ephemeris --------------------------------------------*/
static int decode_gal_fnav_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_gal={0};
    double tow[4],toc,tt,sqrtA;
    int i,week[3],svid,e5a_hs,e5a_dvs,type[4],iod_nav[4];
    
    trace(4,"decode_gal_fnav_eph:\n");
    
    i=0; /* page type 1 */
    type[0]     =getbitu(buff,i, 6);              i+= 6;
    svid        =getbitu(buff,i, 6);              i+= 6;
    iod_nav[0]  =getbitu(buff,i,10);              i+=10;
    toc         =getbitu(buff,i,14)*60.0;         i+=14;
    eph_gal.f0  =getbits(buff,i,31)*P2_34;        i+=31;
    eph_gal.f1  =getbits(buff,i,21)*P2_46;        i+=21;
    eph_gal.f2  =getbits(buff,i, 6)*P2_59;        i+= 6;
    eph_gal.sva =getbitu(buff,i, 8);              i+= 8+11+11+14+5;
    eph_gal.tgd[0]=getbits(buff,i,10)*P2_32;      i+=10; /* BGD E5a/E1 */
    e5a_hs      =getbitu(buff,i, 2);              i+= 2;
    week[0]     =getbitu(buff,i,12);              i+=12; /* gst-week */
    tow[0]      =getbitu(buff,i,20);              i+=20;
    e5a_dvs     =getbitu(buff,i, 1);
    
    i=31*8; /* page type 2 */
    type[1]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[1]  =getbitu(buff,i,10);              i+=10;
    eph_gal.M0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.OMGd=getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    eph_gal.e   =getbitu(buff,i,32)*P2_33;        i+=32;
    sqrtA       =getbitu(buff,i,32)*P2_19;        i+=32;
    eph_gal.OMG0=getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.idot=getbits(buff,i,14)*P2_43*SC2RAD; i+=14;
    week[1]     =getbitu(buff,i,12);              i+=12;
    tow[1]      =getbitu(buff,i,20);
    
    i=62*8; /* page type 3 */
    type[2]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[2]  =getbitu(buff,i,10);              i+=10;
    eph_gal.i0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.omg =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_gal.deln=getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
    eph_gal.cuc =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.cus =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.crc =getbits(buff,i,16)*P2_5;         i+=16;
    eph_gal.crs =getbits(buff,i,16)*P2_5;         i+=16;
    eph_gal.toes=getbitu(buff,i,14)*60.0;         i+=14;
    week[2]     =getbitu(buff,i,12);              i+=12;
    tow[2]      =getbitu(buff,i,20);
    
    i=93*8; /* page type 4 */
    type[3]     =getbitu(buff,i, 6);              i+= 6;
    iod_nav[3]  =getbitu(buff,i,10);              i+=10;
    eph_gal.cic =getbits(buff,i,16)*P2_29;        i+=16;
    eph_gal.cis =getbits(buff,i,16)*P2_29;

    /* test page types */
    if (type[0]!=1||type[1]!=2||type[2]!=3||type[3]!=4) {
        trace(3,"decode_gal_fnav error: svid=%d type=%d %d %d %d\n",svid,
              type[0],type[1],type[2],type[3]);
        return 0;
    }
    /* test consistency of iod_nav */
    if (iod_nav[0]!=iod_nav[1]||iod_nav[0]!=iod_nav[2]||iod_nav[0]!=iod_nav[3]) {
        trace(3,"decode_gal_fnav error: svid=%d iod_nav=%d %d %d %d\n",svid,
              iod_nav[0],iod_nav[1],iod_nav[2],iod_nav[3]);
        return 0;
    }
    if (!(eph_gal.sat=satno(SYS_GAL,svid))) {
        trace(2,"decode_gal_fnav svid error: svid=%d\n",svid);
        return 0;
    }
    eph_gal.A=sqrtA*sqrtA;
    eph_gal.tgd[1]=0.0; /* BGD E5b/E1 */
    eph_gal.iode=eph_gal.iodc=iod_nav[0];
    eph_gal.svh=(e5a_hs<<4)|(e5a_dvs<<3);
    eph_gal.ttr=gst2time(week[0],tow[0]);
    tt=timediff(gst2time(week[0],eph_gal.toes),eph_gal.ttr);
    if      (tt> 302400.0) week[0]--; /* week consistent with toe */
    else if (tt<-302400.0) week[0]++;
    eph_gal.toe=gst2time(week[0],eph_gal.toes);
    eph_gal.toc=gst2time(week[0],toc);
    eph_gal.week=week[0]+1024; /* gal-week = gst-week + 1024 */
    eph_gal.code=(1<<8); /* F/NAV: af0-af2,Toc,SISA for E5a,E1 */
    *eph=eph_gal;
    return 1;
}
/* decode Galileo F/NAV iono parameters --------------------------------------*/
static int decode_gal_fnav_ion(const uint8_t *buff, double *ion)
{
    int i=0; /* page type 1 */
    
    trace(4,"decode_gal_fnav_ion:\n");
    
    if (getbitu(buff,i,6)!=1) return 0;
    i+=6+6+10+14+31+21+6+8;
    ion[0]=getbitu(buff,i,11)*0.25;  i+=11;
    ion[1]=getbits(buff,i,11)*P2_8;  i+=11;
    ion[2]=getbits(buff,i,14)*P2_15; i+=14;
    ion[3]=getbitu(buff,i, 5);
    return 1;
}
/* decode Galileo F/NAV UTC parameters ---------------------------------------*/
static int decode_gal_fnav_utc(const uint8_t *buff, double *utc)
{
    int i=93*8; /* page type 4 */
    
    trace(4,"decode_gal_fnav_utc:\n");
    
    if (getbitu(buff,i,6)!=4) return 0;
    i+=6+10+16+16;
    utc[0]=getbits(buff,i,32)*P2_30;  i+=32; /* A0 */
    utc[1]=getbits(buff,i,24)*P2_50;  i+=24; /* A1 */
    utc[4]=getbits(buff,i, 8);        i+= 8; /* dt_LS */
    utc[2]=getbitu(buff,i, 8)*3600.0; i+= 8; /* tot */
    utc[3]=getbitu(buff,i, 8);        i+= 8; /* WN_ot */
    utc[5]=getbitu(buff,i, 8);        i+= 8; /* WN_LSF */
    utc[6]=getbitu(buff,i, 3);        i+= 3; /* DN */
    utc[7]=getbits(buff,i, 8);               /* dt_LSF */
    return 1;
}
/* decode Galileo F/NAV navigation data ----------------------------------------
* decode Galileo F/NAV navigation data (ref [5] 4.2)
* args   : uint8_t *buff    I   Galileo F/NAV subframe data (CRC checked)
*                                 buff[  0- 30]: page type 1 (244 bit)
*                                 buff[ 31- 61]: page type 2
*                                 buff[ 62- 92]: page type 3
*                                 buff[ 93-123]: page type 4
*                                 buff[124-154]: page type 5
*                                 buff[155-185]: page type 6 
*          eph_t    *eph    IO  Galileo F/NAV ephemeris       (NULL: not output)
*          double   *ion    IO  Galileo F/NAV iono parameters (NULL: not output)
*                                 ion[0-3]: a_i0,a_i1,a_i2,flags
*          double   *utc    IO  Galileo F/NAV UTC parameters  (NULL: not output)
*                                 utc[0-3]: A0,A1,tot,WNt
*                                 utc[4-7]: dt_LS,WN_LSF,DN,dt_LSF
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_gal_fnav(const uint8_t *buff, eph_t *eph, double *ion,
                           double *utc)
{
    trace(4,"decode_gal_fnav:\n");

    if (eph&&!decode_gal_fnav_eph(buff,eph)) return 0;    
    if (ion&&!decode_gal_fnav_ion(buff,ion)) return 0;    
    if (utc&&!decode_gal_fnav_utc(buff,utc)) return 0;
    return 1; 
}
/* decode BDS D1 navigation data ---------------------------------------------*/
static int decode_bds_d1_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_bds={0};
    double toc_bds,sqrtA;
    uint32_t toe1,toe2,sow1,sow2,sow3;
    int i,frn1,frn2,frn3;
    
    i=8*38*0; /* subframe 1 */
    frn1          =getbitu (buff,i+ 15, 3);
    sow1          =getbitu2(buff,i+ 18, 8,i+30,12);
    eph_bds.svh   =getbitu (buff,i+ 42, 1); /* SatH1 */
    eph_bds.iodc  =getbitu (buff,i+ 43, 5); /* AODC */
    eph_bds.sva   =getbitu (buff,i+ 48, 4);
    eph_bds.week  =getbitu (buff,i+ 60,13); /* week in BDT */
    toc_bds       =getbitu2(buff,i+ 73, 9,i+ 90, 8)*8.0;
    eph_bds.tgd[0]=getbits (buff,i+ 98,10)*0.1*1E-9;
    eph_bds.tgd[1]=getbits2(buff,i+108, 4,i+120, 6)*0.1*1E-9;
    eph_bds.f2    =getbits (buff,i+214,11)*P2_66;
    eph_bds.f0    =getbits2(buff,i+225, 7,i+240,17)*P2_33;
    eph_bds.f1    =getbits2(buff,i+257, 5,i+270,17)*P2_50;
    eph_bds.iode  =getbitu (buff,i+287, 5); /* AODE */
    
    i=8*38*1; /* subframe 2 */
    frn2          =getbitu (buff,i+ 15, 3);
    sow2          =getbitu2(buff,i+ 18, 8,i+30,12);
    eph_bds.deln  =getbits2(buff,i+ 42,10,i+ 60, 6)*P2_43*SC2RAD;
    eph_bds.cuc   =getbits2(buff,i+ 66,16,i+ 90, 2)*P2_31;
    eph_bds.M0    =getbits2(buff,i+ 92,20,i+120,12)*P2_31*SC2RAD;
    eph_bds.e     =getbitu2(buff,i+132,10,i+150,22)*P2_33;
    eph_bds.cus   =getbits (buff,i+180,18)*P2_31;
    eph_bds.crc   =getbits2(buff,i+198, 4,i+210,14)*P2_6;
    eph_bds.crs   =getbits2(buff,i+224, 8,i+240,10)*P2_6;
    sqrtA         =getbitu2(buff,i+250,12,i+270,20)*P2_19;
    toe1          =getbitu (buff,i+290, 2); /* TOE 2-MSB */
    eph_bds.A     =sqrtA*sqrtA;
    
    i=8*38*2; /* subframe 3 */
    frn3          =getbitu (buff,i+ 15, 3);
    sow3          =getbitu2(buff,i+ 18, 8,i+30,12);
    toe2          =getbitu2(buff,i+ 42,10,i+ 60, 5); /* TOE 5-LSB */
    eph_bds.i0    =getbits2(buff,i+ 65,17,i+ 90,15)*P2_31*SC2RAD;
    eph_bds.cic   =getbits2(buff,i+105, 7,i+120,11)*P2_31;
    eph_bds.OMGd  =getbits2(buff,i+131,11,i+150,13)*P2_43*SC2RAD;
    eph_bds.cis   =getbits2(buff,i+163, 9,i+180, 9)*P2_31;
    eph_bds.idot  =getbits2(buff,i+189,13,i+210, 1)*P2_43*SC2RAD;
    eph_bds.OMG0  =getbits2(buff,i+211,21,i+240,11)*P2_31*SC2RAD;
    eph_bds.omg   =getbits2(buff,i+251,11,i+270,21)*P2_31*SC2RAD;
    eph_bds.toes  =merge_two_u(toe1,toe2,15)*8.0;
    
    /* check consistency of subframe ids, sows and toe/toc */
    if (frn1!=1||frn2!=2||frn3!=3) {
        trace(3,"decode_bds_d1_eph error: frn=%d %d %d\n",frn1,frn2,frn3);
        return 0;
    }
    if (sow2!=sow1+6||sow3!=sow2+6) {
        trace(3,"decode_bds_d1_eph error: sow=%d %d %d\n",sow1,sow2,sow3);
        return 0;
    }
    if (toc_bds!=eph_bds.toes) {
        trace(3,"decode_bds_d1_eph error: toe=%.0f toc=%.0f\n",eph_bds.toes,
              toc_bds);
        return 0;
    }
    eph_bds.ttr=bdt2gpst(bdt2time(eph_bds.week,sow1)); /* bdt -> gpst */
    if      (eph_bds.toes>sow1+302400.0) eph_bds.week++;
    else if (eph_bds.toes<sow1-302400.0) eph_bds.week--;
    eph_bds.toe=bdt2gpst(bdt2time(eph_bds.week,eph_bds.toes));
    eph_bds.toc=bdt2gpst(bdt2time(eph_bds.week,toc_bds));
    eph_bds.code=0; /* data source = unknown */
    eph_bds.flag=1; /* nav type = IGSO/MEO */
    *eph=eph_bds;
    return 1;
}
/* decode BDS D1 iono parameters ---------------------------------------------*/
static int decode_bds_d1_ion(const uint8_t *buff, double *ion)
{
    int i=8*38*0; /* subframe 1 */
    
    trace(4,"decode_bds_d1_ion:\n");
    
    /* subframe 1 */
    if (getbitu(buff,i+15,3)!=1) return 0;
    
    ion[0]=getbits (buff,i+126, 8)*P2_30;
    ion[1]=getbits (buff,i+134, 8)*P2_27;
    ion[2]=getbits (buff,i+150, 8)*P2_24;
    ion[3]=getbits (buff,i+158, 8)*P2_24;
    ion[4]=getbits2(buff,i+166, 6,i+180, 2)*P2P11;
    ion[5]=getbits (buff,i+182, 8)*P2P14;
    ion[6]=getbits (buff,i+190, 8)*P2P16;
    ion[7]=getbits2(buff,i+198, 4,i+210, 4)*P2P16;
    return 1;
}
/* decode BDS D1 UTC parameters ----------------------------------------------*/
static int decode_bds_d1_utc(const uint8_t *buff, double *utc)
{
    int i=8*38*4; /* subframe 5 */
    
    trace(4,"decode_bds_d1_utc:\n");
    
    if (getbitu(buff,15,3)!=1) return 0; /* subframe 1 */
    
    /* subframe 5 page 10 */
    if (getbitu(buff,i+15,3)!=5||getbitu(buff,i+43,7)!=10) return 0;
    
    utc[4]=getbits2(buff,i+ 50, 2,i+ 60, 6); /* dt_LS */
    utc[7]=getbits (buff,i+ 66, 8);          /* dt_LSF */
    utc[5]=getbitu (buff,i+ 74, 8);          /* WN_LSF */
    utc[0]=getbits2(buff,i+ 90,22,i+120,10)*P2_30; /* A0 */
    utc[1]=getbits2(buff,i+130,12,i+150,12)*P2_50; /* A1 */
    utc[6]=getbitu (buff,i+162, 8);          /* DN */
    utc[2]=getbitu2(buff,i+ 18, 8,i+ 30,12); /* SOW */
    utc[3]=getbitu (buff,60,13);             /* WN */
    return 1;
}
/* decode BDS D1 navigation data -----------------------------------------------
* decode BDS D1 navigation data (IGSO/MEO) (ref [3] 5.2)
* args   : uint8_t *buff    I   BDS D1 subframe data (CRC checked with parity)
*                                  buff[  0- 37]: subframe 1 (300 bits)
*                                  buff[ 38- 75]: subframe 2
*                                  buff[ 76-113]: subframe 3
*                                  buff[114-141]: subframe 4
*                                  buff[152-189]: subframe 5
*          eph_t    *eph    IO  BDS D1 ephemeris       (NULL: not output)
*          double   *ion    IO  BDS D1 iono parameters (NULL: not output)
*                                 ion[0-3]: alpha_0,...,alpha_3
*                                 ion[4-7]: beta_0,...,beta_3
*          double   *utc    IO  BDS D1 UTC parameters  (NULL: not output)
*                                 utc[0-2]: A0,A1,tot,WNt
*                                 utc[4-7]: dt_LS,WN_LSF,DN,dt_LSF
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_bds_d1(const uint8_t *buff, eph_t *eph, double *ion,
                         double *utc)
{
    trace(4,"decode_bds_d1:\n");
    
    if (eph&&!decode_bds_d1_eph(buff,eph)) return 0;
    if (ion&&!decode_bds_d1_ion(buff,ion)) return 0;
    if (utc&&!decode_bds_d1_utc(buff,utc)) return 0;
    return 1;
}
/* decode BDS D2 ephemeris ---------------------------------------------------*/
static int decode_bds_d2_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_bds={0};
    double toc_bds,sqrtA;
    uint32_t f1p4,cucp5,ep6,cicp7,i0p8,OMGdp9,omgp10;
    uint32_t sow1,sow3,sow4,sow5,sow6,sow7,sow8,sow9,sow10;
    int i,f1p3,cucp4,ep5,cicp6,i0p7,OMGdp8,omgp9;
    int pgn1,pgn3,pgn4,pgn5,pgn6,pgn7,pgn8,pgn9,pgn10;
    
    trace(4,"decode_bds_d1_eph:\n");
    
    i=8*38*0; /* page 1 */
    pgn1          =getbitu (buff,i+ 42, 4);
    sow1          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    eph_bds.svh   =getbitu (buff,i+ 46, 1); /* SatH1 */
    eph_bds.iodc  =getbitu (buff,i+ 47, 5); /* AODC */
    eph_bds.sva   =getbitu (buff,i+ 60, 4);
    eph_bds.week  =getbitu (buff,i+ 64,13); /* week in BDT */
    toc_bds       =getbitu2(buff,i+ 77, 5,i+ 90,12)*8.0;
    eph_bds.tgd[0]=getbits (buff,i+102,10)*0.1*1E-9;
    eph_bds.tgd[1]=getbits (buff,i+120,10)*0.1*1E-9;
    
    i=8*38*2; /* page 3 */
    pgn3          =getbitu (buff,i+ 42, 4);
    sow3          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    eph_bds.f0    =getbits2(buff,i+100,12,i+120,12)*P2_33;
    f1p3          =getbits (buff,i+132,4);
    
    i=8*38*3; /* page 4 */
    pgn4          =getbitu (buff,i+ 42, 4);
    sow4          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    f1p4          =getbitu2(buff,i+ 46, 6,i+ 60,12);
    eph_bds.f2    =getbits2(buff,i+ 72,10,i+ 90, 1)*P2_66;
    eph_bds.iode  =getbitu (buff,i+ 91, 5); /* AODE */
    eph_bds.deln  =getbits (buff,i+ 96,16)*P2_43*SC2RAD;
    cucp4         =getbits (buff,i+120,14);
    
    i=8*38*4; /* page 5 */
    pgn5          =getbitu (buff,i+ 42, 4);
    sow5          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    cucp5         =getbitu (buff,i+ 46, 4);
    eph_bds.M0    =getbits3(buff,i+ 50, 2,i+ 60,22,i+ 90, 8)*P2_31*SC2RAD;
    eph_bds.cus   =getbits2(buff,i+ 98,14,i+120, 4)*P2_31;
    ep5           =getbits (buff,i+124,10);
    
    i=8*38*5; /* page 6 */
    pgn6          =getbitu (buff,i+ 42, 4);
    sow6          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    ep6           =getbitu2(buff,i+ 46, 6,i+ 60,16);
    sqrtA         =getbitu3(buff,i+ 76, 6,i+ 90,22,i+120,4)*P2_19;
    cicp6         =getbits (buff,i+124,10);
    eph_bds.A     =sqrtA*sqrtA;
    
    i=8*38*6; /* page 7 */
    pgn7          =getbitu (buff,i+ 42, 4);
    sow7          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    cicp7         =getbitu2(buff,i+ 46, 6,i+ 60, 2);
    eph_bds.cis   =getbits (buff,i+ 62,18)*P2_31;
    eph_bds.toes  =getbitu2(buff,i+ 80, 2,i+ 90,15)*8.0;
    i0p7          =getbits2(buff,i+105, 7,i+120,14);
    
    i=8*38*7; /* page 8 */
    pgn8          =getbitu (buff,i+ 42, 4);
    sow8          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    i0p8          =getbitu2(buff,i+ 46, 6,i+ 60, 5);
    eph_bds.crc   =getbits2(buff,i+ 65,17,i+ 90, 1)*P2_6;
    eph_bds.crs   =getbits (buff,i+ 91,18)*P2_6;
    OMGdp8        =getbits2(buff,i+109, 3,i+120,16);
    
    i=8*38*8; /* page 9 */
    pgn9          =getbitu (buff,i+ 42, 4);
    sow9          =getbitu2(buff,i+ 18, 8,i+ 30,12);
    OMGdp9        =getbitu (buff,i+ 46, 5);
    eph_bds.OMG0  =getbits3(buff,i+ 51, 1,i+ 60,22,i+ 90, 9)*P2_31*SC2RAD;
    omgp9         =getbits2(buff,i+ 99,13,i+120,14);
    
    i=8*38*9; /* page 10 */
    pgn10         =getbitu (buff,i+ 42, 4);
    sow10         =getbitu2(buff,i+ 18, 8,i+ 30,12);
    omgp10        =getbitu (buff,i+ 46, 5);
    eph_bds.idot  =getbits2(buff,i+ 51, 1,i+ 60,13)*P2_43*SC2RAD;
    
    /* check consistency of page numbers, sows and toe/toc */
    if (pgn1!=1||pgn3!=3||pgn4!=4||pgn5!=5||pgn6!=6||pgn7!=7||pgn8!=8||pgn9!=9||
        pgn10!=10) {
        trace(3,"decode_bds_d2 error: pgn=%d %d %d %d %d %d %d %d %d\n",
              pgn1,pgn3,pgn4,pgn5,pgn6,pgn7,pgn8,pgn9,pgn10);
        return 0;
    }
    if (sow3!=sow1+6||sow4!=sow3+3||sow5!=sow4+3||sow6!=sow5+3||
        sow7!=sow6+3||sow8!=sow7+3||sow9!=sow8+3||sow10!=sow9+3) {
        trace(3,"decode_bds_d2 error: sow=%d %d %d %d %d %d %d %d %d\n",
              sow1,sow3,sow4,sow5,sow6,sow7,sow8,sow9,sow10);
        return 0;
    }
    if (toc_bds!=eph_bds.toes) {
        trace(3,"decode_bds_d2 error: toe=%.0f toc=%.0f\n",eph_bds.toes,
              toc_bds);
        return 0;
    }
    eph_bds.f1  =merge_two_s(f1p3  ,f1p4  ,18)*P2_50;
    eph_bds.cuc =merge_two_s(cucp4 ,cucp5 , 4)*P2_31;
    eph_bds.e   =merge_two_s(ep5   ,ep6   ,22)*P2_33;
    eph_bds.cic =merge_two_s(cicp6 ,cicp7 , 8)*P2_31;
    eph_bds.i0  =merge_two_s(i0p7  ,i0p8  ,11)*P2_31*SC2RAD;
    eph_bds.OMGd=merge_two_s(OMGdp8,OMGdp9, 5)*P2_43*SC2RAD;
    eph_bds.omg =merge_two_s(omgp9 ,omgp10, 5)*P2_31*SC2RAD;
    
    eph_bds.ttr=bdt2gpst(bdt2time(eph_bds.week,sow1)); /* bdt -> gpst */
    if      (eph_bds.toes>sow1+302400.0) eph_bds.week++;
    else if (eph_bds.toes<sow1-302400.0) eph_bds.week--;
    eph_bds.toe=bdt2gpst(bdt2time(eph_bds.week,eph_bds.toes));
    eph_bds.toc=bdt2gpst(bdt2time(eph_bds.week,toc_bds));
    eph_bds.code=0; /* data source = unknown */
    eph_bds.flag=2; /* nav type = GEO */
    *eph=eph_bds;
    return 1;
}
/* decode BDS D2 UTC parameters ----------------------------------------------*/
static int decode_bds_d2_utc(const uint8_t *buff, double *utc)
{
    int i=8*38*10; /* subframe 5 pase 102 */
    
    trace(4,"decode_bds_d2_utc:\n");
    
    /* subframe 1 page 1 */
    if (getbitu(buff,15,3)!=1||getbitu(buff,42,4)!=1) return 0;
    
    /* subframe 5 page 102 */
    if (getbitu(buff,i+15,3)!=5||getbitu(buff,i+43,7)!=102) return 0;
    
    utc[4]=getbits2(buff,i+ 50, 2,i+ 60, 6); /* dt_LS */
    utc[7]=getbits (buff,i+ 66, 8);          /* dt_LSF */
    utc[5]=getbitu (buff,i+ 74, 8);          /* WN_LSF */
    utc[0]=getbits2(buff,i+ 90,22,i+120,10)*P2_30; /* A0 */
    utc[1]=getbits2(buff,i+130,12,i+150,12)*P2_50; /* A1 */
    utc[6]=getbitu (buff,i+162, 8);          /* DN */
    utc[2]=getbits2(buff,i+ 18, 8,i+ 30,12); /* SOW */
    utc[3]=getbitu (buff,64,13);             /* WN */
    return 1;
}
/* decode BDS D2 navigation data -----------------------------------------------
* decode BDS D2 navigation data (GEO) (ref [3] 5.3)
* args   : uint8_t *buff    I   BDS D2 subframe data (CRC checked with parity)
*                                 buff[  0- 37]: subframe 1 page 1 (300 bits)
*                                 buff[ 38- 75]: subframe 1 page 2
*                                 ...
*                                 buff[342-379]: subframe 1 page 10
*                                 buff[380-417]: subframe 5 page 102
*          eph_t    *eph    IO  BDS D2 ephemeris       (NULL: not output)
*          double   *utc    IO  BDS D2 UTC parameters  (NULL: not output)
*                                 utc[0-2]: A0,A1,tot,WNt
*                                 utc[4-7]: dt_LS,WN_LSF,DN,dt_LSF
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_bds_d2(const uint8_t *buff, eph_t *eph, double *utc)
{
    trace(4,"decode_bds_d2:\n");
    
    if (eph&&!decode_bds_d2_eph(buff,eph)) return 0;
    if (utc&&!decode_bds_d2_utc(buff,utc)) return 0;
    return 1;
}
/* test hamming code of GLONASS navigation string ------------------------------
* test hamming code of GLONASS navigation string (ref [2] 4.7)
* args   : uint8_t *buff    I   GLONASS navigation string with hamming code
*                                 buff[ 0]: string bit 85-78
*                                 buff[ 1]: string bit 77-70
*                                 ...
*                                 buff[10]: string bit  5- 1 (0 padded)
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int test_glostr(const uint8_t *buff)
{
    static const uint8_t xor_8bit[256]={ /* xor of 8 bits */
        0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
        1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
        0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
        0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
        0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
        1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
    };
    static const uint8_t mask_hamming[][12]={ /* mask of hamming codes */
        {0x55,0x55,0x5A,0xAA,0xAA,0xAA,0xB5,0x55,0x6A,0xD8,0x08},
        {0x66,0x66,0x6C,0xCC,0xCC,0xCC,0xD9,0x99,0xB3,0x68,0x10},
        {0x87,0x87,0x8F,0x0F,0x0F,0x0F,0x1E,0x1E,0x3C,0x70,0x20},
        {0x07,0xF8,0x0F,0xF0,0x0F,0xF0,0x1F,0xE0,0x3F,0x80,0x40},
        {0xF8,0x00,0x0F,0xFF,0xF0,0x00,0x1F,0xFF,0xC0,0x00,0x80},
        {0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x01,0x00},
        {0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00},
        {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8}
    };
    uint8_t cs;
    int i,j,n=0;
    
    for (i=0;i<8;i++) {
        for (j=0,cs=0;j<11;j++) {
            cs^=xor_8bit[buff[j]&mask_hamming[i][j]];
        }
        if (cs) n++;
    }
    return n==0||(n==2&&cs);
}
/* decode GLONASS ephemeris --------------------------------------------------*/
static int decode_glostr_eph(const uint8_t *buff, geph_t *geph)
{
    geph_t geph_glo={0};
    double tow,tod,tof,toe;
    int P,P1,P2,P3,P4,tk_h,tk_m,tk_s,tb,ln,NT,slot,M,week;
    int i=1,frn1,frn2,frn3,frn4;
    
    trace(4,"decode_glostr_eph:\n");
    
    /* frame 1 */
    frn1           =getbitu(buff,i, 4);           i+= 4+2;
    P1             =getbitu(buff,i, 2);           i+= 2;
    tk_h           =getbitu(buff,i, 5);           i+= 5;
    tk_m           =getbitu(buff,i, 6);           i+= 6;
    tk_s           =getbitu(buff,i, 1)*30;        i+= 1;
    geph_glo.vel[0]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
    geph_glo.acc[0]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
    geph_glo.pos[0]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;
    
    /* frame 2 */
    frn2           =getbitu(buff,i, 4);           i+= 4;
    geph_glo.svh   =getbitu(buff,i, 1);           i+= 1+2; /* MSB of Bn */
    P2             =getbitu(buff,i, 1);           i+= 1;
    tb             =getbitu(buff,i, 7);           i+= 7+5;
    geph_glo.vel[1]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
    geph_glo.acc[1]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
    geph_glo.pos[1]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;
    
    /* frame 3 */
    frn3           =getbitu(buff,i, 4);           i+= 4;
    P3             =getbitu(buff,i, 1);           i+= 1;
    geph_glo.gamn  =getbitg(buff,i,11)*P2_40;     i+=11+1;
    P              =getbitu(buff,i, 2);           i+= 2;
    ln             =getbitu(buff,i, 1);           i+= 1;
    geph_glo.vel[2]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
    geph_glo.acc[2]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
    geph_glo.pos[2]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;
    
    /* frame 4 */
    frn4           =getbitu(buff,i, 4);           i+= 4;
    geph_glo.taun  =getbitg(buff,i,22)*P2_30;     i+=22;
    geph_glo.dtaun =getbitg(buff,i, 5)*P2_30;     i+= 5;
    geph_glo.age   =getbitu(buff,i, 5);           i+= 5+14;
    P4             =getbitu(buff,i, 1);           i+= 1;
    geph_glo.sva   =getbitu(buff,i, 4);           i+= 4+3;
    NT             =getbitu(buff,i,11);           i+=11;
    slot           =getbitu(buff,i, 5);           i+= 5;
    M              =getbitu(buff,i, 2);
    
    if (frn1!=1||frn2!=2||frn3!=3||frn4!=4) {
        trace(3,"decode_glostr error: frn=%d %d %d %d %d\n",frn1,frn2,frn3,
              frn4);
        return 0;
    }
    if (!(geph_glo.sat=satno(SYS_GLO,slot))) {
        trace(2,"decode_glostr error: slot=%d\n",slot);
        return 0;
    }
    geph_glo.frq=0; /* set default */
    geph_glo.iode=tb;
    tow=time2gpst(gpst2utc(geph->tof),&week);
    tod=fmod(tow,86400.0); tow-=tod;
    tof=tk_h*3600.0+tk_m*60.0+tk_s-10800.0; /* lt->utc */
    if      (tof<tod-43200.0) tof+=86400.0;
    else if (tof>tod+43200.0) tof-=86400.0;
    geph_glo.tof=utc2gpst(gpst2time(week,tow+tof));
    toe=tb*900.0-10800.0; /* lt->utc */
    if      (toe<tod-43200.0) toe+=86400.0;
    else if (toe>tod+43200.0) toe-=86400.0;
    geph_glo.toe=utc2gpst(gpst2time(week,tow+toe)); /* utc->gpst */
    *geph=geph_glo;
    return 1;
}
/* decode GLONASS UTC parameters ---------------------------------------------*/
static int decode_glostr_utc(const uint8_t *buff, double *utc)
{
    int i=1+80*4; /* frame 5 */
    
    trace(4,"decode_glostr_utc:\n");
    
    /* frame 5 */
    if (getbitu(buff,i,4)!=5) return 0;
    i+=4+11;  
    utc[0]=getbits(buff,i,32)*P2_31; i+=32+1+6; /* tau_C */
    utc[1]=getbits(buff,i,22)*P2_30;            /* tau_GPS */
    utc[2]=utc[3]=utc[4]=utc[5]=utc[6]=utc[7]=0.0;
    return 1;
}
/* decode GLONASS navigation data strings --------------------------------------
* decode GLONASS navigation data string (ref [2])
* args   : uint8_t *buff    I   GLONASS navigation data string
*                               (w/o hamming and time mark)
*                                 buff[ 0- 9]: string 1 (77 bits)
*                                 buff[10-19]: string 2
*                                 buff[20-29]: string 3
*                                 buff[30-39]: string 4
*                                 buff[40-49]: string 5
*          geph_t *geph     IO  GLONASS ephemeris      (NULL: not output)
*          double *utc      IO  GLONASS UTC parameters (NULL: not output)
*                                 utc[0]  : A0 (=-tau_C)
*                                 utc[1-7]: reserved
* return : status (1:ok,0:error)
* notes  : geph->tof should be set to frame time within 1/2 day before calling
*          geph->frq is set to 0
*-----------------------------------------------------------------------------*/
extern int decode_glostr(const uint8_t *buff, geph_t *geph, double *utc)
{
    trace(4,"decode_glostr:\n");
    
    if (geph&&!decode_glostr_eph(buff,geph)) return 0;
    if (utc &&!decode_glostr_utc(buff,utc )) return 0;
    return 1;
}
/* decode GPS/QZSS ephemeris -------------------------------------------------*/
static int decode_frame_eph(const uint8_t *buff, eph_t *eph)
{
    eph_t eph_sat={0};
    double tow1,tow2,tow3,toc,sqrtA;
    int i=48,id1,id2,id3,week,iodc0,iodc1,iode,tgd;
    
    trace(4,"decode_frame_eph:\n");
    
    i=240*0+24; /* subframe 1 */
    tow1        =getbitu(buff,i,17)*6.0;          i+=17+2;
    id1         =getbitu(buff,i, 3);              i+=3+2;
    week        =getbitu(buff,i,10);              i+=10;
    eph_sat.code=getbitu(buff,i, 2);              i+= 2;
    eph_sat.sva =getbitu(buff,i, 4);              i+= 4; /* ura index */
    eph_sat.svh =getbitu(buff,i, 6);              i+= 6;
    iodc0       =getbitu(buff,i, 2);              i+= 2;
    eph_sat.flag=getbitu(buff,i, 1);              i+= 1+87;
    tgd         =getbits(buff,i, 8);              i+= 8;
    iodc1       =getbitu(buff,i, 8);              i+= 8;
    toc         =getbitu(buff,i,16)*16.0;         i+=16;
    eph_sat.f2  =getbits(buff,i, 8)*P2_55;        i+= 8;
    eph_sat.f1  =getbits(buff,i,16)*P2_43;        i+=16;
    eph_sat.f0  =getbits(buff,i,22)*P2_31;
    
    i=240*1+24; /* subframe 2 */
    tow2        =getbitu(buff,i,17)*6.0;          i+=17+2;
    id2         =getbitu(buff,i, 3);              i+=3+2;
    eph_sat.iode=getbitu(buff,i, 8);              i+= 8;
    eph_sat.crs =getbits(buff,i,16)*P2_5;         i+=16;
    eph_sat.deln=getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
    eph_sat.M0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_sat.cuc =getbits(buff,i,16)*P2_29;        i+=16;
    eph_sat.e   =getbitu(buff,i,32)*P2_33;        i+=32;
    eph_sat.cus =getbits(buff,i,16)*P2_29;        i+=16;
    sqrtA       =getbitu(buff,i,32)*P2_19;        i+=32;
    eph_sat.toes=getbitu(buff,i,16)*16.0;         i+=16;
    eph_sat.fit =getbitu(buff,i, 1)?0.0:4.0; /* 0:4hr,1:>4hr */
    
    i=240*2+24; /* subframe 3 */
    tow3        =getbitu(buff,i,17)*6.0;          i+=17+2;
    id3         =getbitu(buff,i, 3);              i+=3+2;
    eph_sat.cic =getbits(buff,i,16)*P2_29;        i+=16;
    eph_sat.OMG0=getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_sat.cis =getbits(buff,i,16)*P2_29;        i+=16;
    eph_sat.i0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_sat.crc =getbits(buff,i,16)*P2_5;         i+=16;
    eph_sat.omg =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph_sat.OMGd=getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    iode        =getbitu(buff,i, 8);              i+= 8;
    eph_sat.idot=getbits(buff,i,14)*P2_43*SC2RAD;
    
    eph_sat.A=sqrtA*sqrtA;
    eph_sat.iodc=(iodc0<<8)+iodc1;
    eph_sat.tgd[0]=(tgd==-128)?0.0:tgd*P2_31; /* ref [4] */
    
    /* test subframe ids */
    if (id1!=1||id2!=2||id3!=3) {
        trace(3,"decode_frame_eph error: id=%d %d %d\n",id1,id2,id3);
        return 0;
    }
    /* test iode and iodc consistency */
    if (iode!=eph_sat.iode||iode!=(eph_sat.iodc&0xFF)) {
        trace(3,"decode_frame_eph error: iode=%d %d iodc=%d\n",eph_sat.iode,
              iode,eph_sat.iodc);
        return 0;
    }
    eph_sat.week=adjgpsweek(week);
    eph_sat.ttr=gpst2time(eph_sat.week,tow1);
    if      (eph_sat.toes<tow1-302400.0) eph_sat.week++;
    else if (eph_sat.toes>tow1+302400.0) eph_sat.week--;
    eph_sat.toe=gpst2time(eph_sat.week,eph_sat.toes);
    eph_sat.toc=gpst2time(eph_sat.week,toc);
    *eph=eph_sat;
    return 1;
}
/* decode GPS/QZSS satellite almanac -----------------------------------------*/
static void decode_alm_sat(const uint8_t *buff, int type, alm_t *alm)
{
    gtime_t toa0={0};
    double deltai,sqrtA,i_ref,e_ref;
    int i=50,f0;

    trace(4,"decode_alm_sat:\n");
    
    /* type=0:GPS,1:QZS-QZO,2:QZS-GEO */
    e_ref=(type==0)?0.0:((type==1)?0.06:0.0);
    i_ref=(type==0)?0.3:((type==1)?0.25:0.0);
    
    alm->e   =getbits(buff,i,16)*P2_21+e_ref;  i+=16;
    alm->toas=getbitu(buff,i, 8)*4096.0;       i+= 8;
    deltai   =getbits(buff,i,16)*P2_19;        i+=16;
    alm->OMGd=getbits(buff,i,16)*P2_38*SC2RAD; i+=16;
    alm->svh =getbitu(buff,i, 8);              i+= 8;
    sqrtA    =getbitu(buff,i,24)*P2_11;        i+=24;
    alm->OMG0=getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm->omg =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm->M0  =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    f0       =getbits(buff,i, 8);              i+= 8;
    alm->f1  =getbits(buff,i,11)*P2_38;        i+=11;
    alm->f0  =getbitu(buff,i, 3)*P2_17+f0*P2_20;
    alm->A   =sqrtA*sqrtA;
    alm->i0  =(i_ref+deltai)*SC2RAD;
    alm->week=0;
    alm->toa=toa0;
}
/* decode GPS almanac/health -------------------------------------------------*/
static int decode_alm_gps(const uint8_t *buff, int frm, alm_t *alm)
{
    int i,j,sat,toas,week,svid=getbitu(buff,50,6);
    
    trace(4,"decode_alm_gps:\n");
    
    if ((frm==5&&svid>=1&&svid<=24)||(frm==4&&svid>=25&&svid<=32)) {
        if (!(sat=satno(SYS_GPS,svid))) return 0;
        alm[sat-1].sat=sat;
        decode_alm_sat(buff,0,alm+sat);
        return 1;
    }
    else if (frm==5&&svid==51) { /* subframe 5 page 25 */
        i=56;
        toas=getbitu(buff,i,8)*4096; i+=8;
        week=getbitu(buff,i,8);      i+=8;
        for (j=0;j<24;j++,i+=6) {
            if (!(sat=satno(SYS_GPS,j+1))) continue;
            alm[sat-1].svh=getbitu(buff,i,6);
        }
        for (j=0;j<32;j++) {
            if (!(sat=satno(SYS_GPS,j+1))||alm[sat-1].sat!=sat||
                alm[sat-1].toas!=toas) continue;
            alm[sat-1].week=adjgpsweek(week);
            alm[sat-1].toa=gpst2time(alm[sat-1].week,toas);
        }
        return 1;
    }
    else if (frm==4&&svid==63) { /* subframe 4 page 25 */
        i=186;
        for (j=0;j<8;j++,i+=6) {
            if (!(sat=satno(SYS_GPS,j+25))) continue;
            alm[sat-1].svh=getbitu(buff,i,6);
        }
        return 1;
    }
    return 0;
}
/* decode QZSS almanac/health ------------------------------------------------*/
static int decode_alm_qzs(const uint8_t *buff, alm_t *alm)
{
    int i,j,sat,toas,week,svid=getbitu(buff,50,6);
    
    trace(4,"decode_alm_qzs:\n");
    
    if (svid>=1&&svid<=9) {
        if (!(sat=satno(SYS_QZS,192+svid))) return 0;
        alm[sat-1].sat=sat;
        decode_alm_sat(buff,(svid<=6)?1:2,alm+sat);
        return 1;
    }
    else if (svid==51) {
        i=56;
        toas=getbitu(buff,i,8)*4096; i+=8;
        week=getbitu(buff,i,8);      i+=8;
        for (j=0;j<10;j++,i+=6) {
            if (!(sat=satno(SYS_QZS,193+j))) continue;
            alm[sat-1].svh=getbitu(buff,i,6);
        }
        for (j=0;j<10;j++) {
            if (!(sat=satno(SYS_QZS,193+j))||alm[sat-1].sat!=sat||
                alm[sat-1].toas!=toas) continue;
            alm[sat-1].week=adjgpsweek(week);
            alm[sat-1].toa=gpst2time(alm[sat-1].week,toas);
        }
        return 1;
    }
    return 0;
}
/* decode GPS/QZSS almanac/health --------------------------------------------*/
static int decode_frame_alm(const uint8_t *buff, alm_t *alm)
{
    int frm,dataid,ret=0;
    
    trace(4,"decode_frame_alm:\n");
    
    for (frm=4,buff+=90;frm<=5;frm++,buff+=30) { /* subframe 4/5 */
        if (getbitu(buff,43,3)!=frm) continue;
        dataid=getbitu(buff,48,2);
        
        if (dataid==1) { /* GPS */
            ret|=decode_alm_gps(buff,frm,alm);
        }
        else if (dataid==3) { /* QZSS */
            ret|=decode_alm_qzs(buff,alm);
        }
    }
    return ret;
}
/* decode GPS/QZSS iono parameters -------------------------------------------*/
static int decode_frame_ion(const uint8_t *buff, double *ion)
{
    int i,frm;
    
    trace(4,"decode_frame_ion:\n");
    
    /* subframe 4/5 and svid=56 (page18) (wide area for QZSS) */
    for (frm=4,buff+=90;frm<=5;frm++,buff+=30) {
        if (frm==5&&getbitu(buff,48,2)==1) continue;
        if (getbitu(buff,43,3)!=frm||getbitu(buff,50,6)!=56) continue;
        i=56;
        ion[0]=getbits(buff,i,8)*P2_30; i+=8;
        ion[1]=getbits(buff,i,8)*P2_27; i+=8;
        ion[2]=getbits(buff,i,8)*P2_24; i+=8;
        ion[3]=getbits(buff,i,8)*P2_24; i+=8;
        ion[4]=getbits(buff,i,8)*P2P11; i+=8;
        ion[5]=getbits(buff,i,8)*P2P14; i+=8;
        ion[6]=getbits(buff,i,8)*P2P16; i+=8;
        ion[7]=getbits(buff,i,8)*P2P16;
        return 1;
    }
    return 0;
}
/* decode GPS/QZSS UTC parameters --------------------------------------------*/
static int decode_frame_utc(const uint8_t *buff, double *utc)
{
    int i,frm;
    
    trace(4,"decode_frame_utc:\n");
    
    /* subframe 4/5 and svid=56 (page18) */
    for (frm=4,buff+=90;frm<=5;frm++,buff+=30) {
        if (frm==5&&getbitu(buff,48,2)==1) continue;
        if (getbitu(buff,43,3)!=frm||getbitu(buff,50,6)!=56) continue;
        i=120;
        utc[1]=getbits(buff,i,24)*P2_50; i+=24; /* A1 (s) */
        utc[0]=getbits(buff,i,32)*P2_30; i+=32; /* A0 (s) */
        utc[2]=getbitu(buff,i, 8)*P2P12; i+= 8; /* tot (s) */
        utc[3]=getbitu(buff,i, 8);       i+= 8; /* WNt */
        utc[4]=getbits(buff,i, 8);       i+= 8; /* dt_LS */
        utc[5]=getbitu(buff,i, 8);       i+= 8; /* WN_LSF */
        utc[6]=getbitu(buff,i, 8);       i+= 8; /* DN */
        utc[7]=getbits(buff,i, 8);              /* dt_LSF */
        return 1;
    }
    return 0;
}
/* decode GPS/QZSS navigation data ---------------------------------------------
* decode GPS/QZSS navigation data (ref [1],[4])
* args   : uint8_t *buff    I   GPS/QZSS navigation data (w/o parity bits)
*                                 buff[  0- 29]: subframe 1 (240 bits)
*                                 buff[ 30- 59]: subframe 2
*                                 buff[ 60- 89]: subframe 3
*                                 buff[ 90-119]: subframe 4
*                                 buff[120-149]: subframe 5
*          eph_t *eph       IO  GPS/QZSS ephemeris       (NULL: not output)
*          alm_t *alm       IO  GPS/QZSS almanac/health  (NULL: not output)
*                                 alm[sat-1]: almanac/health (sat=sat no)
*          double *ion      IO  GPS/QZSS iono parameters (NULL: not output)
*                                 ion[0-3]: alpha_0,...,alpha_3
*                                 ion[4-7]: beta_0,...,beta_3
*          double *utc      IO  GPST/QZSS UTC parameters (NULL: not output)
*                                 utc[0-3]: A0,A1,tot,WNt(8bit)
*                                 utc[4-7]: dt_LS,WN_LSF(8bit),DN,dt_LSF
* return : status (1:ok,0:error or no data)
* notes  : use CPU time to resolve modulo 1024 ambiguity of the week number
*          see ref [1]
*-----------------------------------------------------------------------------*/
extern int decode_frame(const uint8_t *buff, eph_t *eph, alm_t *alm,
                        double *ion, double *utc)
{
    trace(4,"decode_frame:\n");
    
    if (eph&&!decode_frame_eph(buff,eph)) return 0;
    if (alm&&!decode_frame_alm(buff,alm)) return 0;
    if (ion&&!decode_frame_ion(buff,ion)) return 0;
    if (utc&&!decode_frame_utc(buff,utc)) return 0;
    return 1;
}
/* initialize receiver raw data control ----------------------------------------
* initialize receiver raw data control struct and reallocate observation and
* epheris buffer
* args   : raw_t *raw       IO  receiver raw data control struct
*          int   format     I   stream format (STRFMT_???)
* return : status (1:ok,0:memory allocation error)
*-----------------------------------------------------------------------------*/
extern int init_raw(raw_t *raw, int format)
{
    gtime_t time0={0};
    obsd_t data0={{0}};
    eph_t  eph0 ={0,-1,-1};
    alm_t  alm0 ={0,-1};
    geph_t geph0={0,-1};
    seph_t seph0={0};
    sbsmsg_t sbsmsg0={0};
    int i,j,ret=1;
    
    trace(3,"init_raw: format=%d\n",format);
    
    raw->time=time0;
    raw->ephset=raw->ephsat=0;
    raw->sbsmsg=sbsmsg0;
    raw->msgtype[0]='\0';
    for (i=0;i<MAXSAT;i++) {
        for (j=0;j<380;j++) raw->subfrm[i][j]=0;
        for (j=0;j<NFREQ+NEXOBS;j++) {
            raw->tobs [i][j]=time0;
            raw->lockt[i][j]=0.0;
            raw->halfc[i][j]=0;
        }
        raw->icpp[i]=raw->off[i]=raw->prCA[i]=raw->dpCA[i]=0.0;
    }
    for (i=0;i<MAXOBS;i++) raw->freqn[i]=0;
    raw->icpc=0.0;
    raw->nbyte=raw->len=0;
    raw->iod=raw->flag=raw->tbase=raw->outtype=0;
    raw->tod=-1;
    for (i=0;i<MAXRAWLEN;i++) raw->buff[i]=0;
    raw->opt[0]='\0';
    raw->format=-1;
    
    raw->obs.data =NULL;
    raw->obuf.data=NULL;
    raw->nav.eph  =NULL;
    raw->nav.alm  =NULL;
    raw->nav.geph =NULL;
    raw->nav.seph =NULL;
    raw->rcv_data =NULL;
    
    if (!(raw->obs.data =(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->obuf.data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->nav.eph  =(eph_t  *)malloc(sizeof(eph_t )*MAXSAT*2))||
        !(raw->nav.alm  =(alm_t  *)malloc(sizeof(alm_t )*MAXSAT))||
        !(raw->nav.geph =(geph_t *)malloc(sizeof(geph_t)*NSATGLO))||
        !(raw->nav.seph =(seph_t *)malloc(sizeof(seph_t)*NSATSBS*2))) {
        free_raw(raw);
        return 0;
    }
    raw->obs.n =0;
    raw->obuf.n=0;
    raw->nav.n =MAXSAT*2;
    raw->nav.na=MAXSAT;
    raw->nav.ng=NSATGLO;
    raw->nav.ns=NSATSBS*2;
    for (i=0;i<MAXOBS   ;i++) raw->obs.data [i]=data0;
    for (i=0;i<MAXOBS   ;i++) raw->obuf.data[i]=data0;
    for (i=0;i<MAXSAT*2 ;i++) raw->nav.eph  [i]=eph0;
    for (i=0;i<MAXSAT   ;i++) raw->nav.alm  [i]=alm0;
    for (i=0;i<NSATGLO  ;i++) raw->nav.geph [i]=geph0;
    for (i=0;i<NSATSBS*2;i++) raw->nav.seph [i]=seph0;
    raw->sta.name[0]=raw->sta.marker[0]='\0';
    raw->sta.antdes[0]=raw->sta.antsno[0]='\0';
    raw->sta.rectype[0]=raw->sta.recver[0]=raw->sta.recsno[0]='\0';
    raw->sta.antsetup=raw->sta.itrf=raw->sta.deltype=0;
    for (i=0;i<3;i++) {
        raw->sta.pos[i]=raw->sta.del[i]=0.0;
    }
    raw->sta.hgt=0.0;
    
    /* initialize receiver dependent data */
    raw->format=format;
    switch (format) {
        case STRFMT_RT17: ret=init_rt17(raw); break;
    }
    if (!ret) {
        free_raw(raw);
        return 0;
    }
    return 1;
}
/* free receiver raw data control ----------------------------------------------
* free observation and ephemeris buffer in receiver raw data control struct
* args   : raw_t  *raw      IO  receiver raw data control struct
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
    
    /* free receiver dependent data */
    switch (raw->format) {
        case STRFMT_RT17: free_rt17(raw); break;
    }
    raw->rcv_data=NULL;
}
/* input receiver raw data from stream -----------------------------------------
* fetch next receiver raw data and input a message from stream
* args   : raw_t  *raw      IO  receiver raw data control struct
*          int    format    I   receiver raw data format (STRFMT_???)
*          uint8_t data     I   stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*-----------------------------------------------------------------------------*/
extern int input_raw(raw_t *raw, int format, uint8_t data)
{
    trace(5,"input_raw: format=%d data=0x%02x\n",format,data);
    
    switch (format) {
        case STRFMT_OEM4  : return input_oem4  (raw,data);
        case STRFMT_OEM3  : return input_oem3  (raw,data);
        case STRFMT_UBX   : return input_ubx   (raw,data);
        case STRFMT_SS2   : return input_ss2   (raw,data);
        case STRFMT_CRES  : return input_cres  (raw,data);
        case STRFMT_STQ   : return input_stq   (raw,data);
        case STRFMT_JAVAD : return input_javad (raw,data);
        case STRFMT_NVS   : return input_nvs   (raw,data);
        case STRFMT_BINEX : return input_bnx   (raw,data);
        case STRFMT_RT17  : return input_rt17  (raw,data);
        case STRFMT_SEPT  : return input_sbf   (raw,data);
    }
    return 0;
}
/* input receiver raw data from file -------------------------------------------
* fetch next receiver raw data and input a message from file
* args   : raw_t  *raw      IO  receiver raw data control struct
*          int    format    I   receiver raw data format (STRFMT_???)
*          FILE   *fp       I   file pointer
* return : status(-2: end of file/format error, -1...31: same as above)
*-----------------------------------------------------------------------------*/
extern int input_rawf(raw_t *raw, int format, FILE *fp)
{
    trace(4,"input_rawf: format=%d\n",format);
    
    switch (format) {
        case STRFMT_OEM4  : return input_oem4f  (raw,fp);
        case STRFMT_OEM3  : return input_oem3f  (raw,fp);
        case STRFMT_UBX   : return input_ubxf   (raw,fp);
        case STRFMT_SS2   : return input_ss2f   (raw,fp);
        case STRFMT_CRES  : return input_cresf  (raw,fp);
        case STRFMT_STQ   : return input_stqf   (raw,fp);
        case STRFMT_JAVAD : return input_javadf (raw,fp);
        case STRFMT_NVS   : return input_nvsf   (raw,fp);
        case STRFMT_BINEX : return input_bnxf   (raw,fp);
        case STRFMT_RT17  : return input_rt17f  (raw,fp);
        case STRFMT_SEPT  : return input_sbff   (raw,fp);
    }
    return -2;
}
