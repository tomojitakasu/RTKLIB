/*------------------------------------------------------------------------------
* septentrio.c : Septentrio Binary Format decoder (All Septentrio receivers)
*
*          Copyright (C) 2013 by Fabrizio Tappero.
*          Copyright (C) 2015 by Jens Reimann
*
* reference :
*     [1] Septentrio, SBF Reference Guide, Version 130722r38600, 07/2013
*
* note: - QZSS and Compass/Beidou is deactivated. The code is not tested. Use -DTESTING to activate.
*
* version : $Revision: 1.4 $ $Date: 2016/01/29 15:05:00 $
*
* history : 2013/07/17  1.0  begin writing
*           2013/10/24  1.1  GPS L1 working
*           2013/11/02  1.2  modified by TTAKASU
*           2015/01/26  1.3  fix some problems by Jens Reimann
*           2016/02/04  1.4  by Jens Reimann
*                           - added more sanity checks
*                           - added galileon raw decoding
*                           - added usage of decoded SBAS messages for testing
*                           - add QZSS and Compass/Beidou navigation messages
*                           - fixed code and Doppler for 2nd and following frequency
*                           - fixed bug in glonass ephemeris
*                           - fixed decoding of galileo ephemeris
*                           - fixed lost lock indicator
*                           - fixed sbas decoding
*                           - cleanups
*           2016/03/03  1.5 - fixed TOW in SBAS messages
*           2016/03/12  1.6 - respect code priorities
*                           - fixed bug in carrier phase calculation of type2 data
*                           - unify frequency determination
*                           - improve lock handling
*                           - various bug fixes
*           2016/05/25  1.7  rtk_crc24q() -> crc24q() by T.T
*           2016/07/29  1.8  crc24q() -> rtk_crc24q() by T.T
*           2017/04/11  1.9  (char *) -> (signed char *) by T.T
*           2017/09/01  1.10 suppress warnings
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

#include <math.h>
#include <stdint.h>

extern const sbsigpband_t igpband1[][8]; /* SBAS IGP band 0-8 */
extern const sbsigpband_t igpband2[][5]; /* SBAS IGP band 9-10 */

static unsigned char locktime[255][32];

/* SBF definitions Version 2.9.1 */
#define SBF_SYNC1   0x24        /* SBF message header sync field 1 (correspond to $) */
#define SBF_SYNC2   0x40        /* SBF message header sync field 2 (correspont to @)*/

#define ID_MEASEPOCH       4027 /* SBF message id: range measurememts */
#define ID_MEASEPOCHEXTRA  4000 /* SBF message id: range measurememts extra info */
#define ID_MEASEPOCH_END   5922 /* SBF message id: end of SBF range measurememts */

#define ID_GPSRAWCA     4017    /* SBF message id: GPS raw navigation page or frame */
#define ID_GPSRAWL2C    4018    /* SBF message id: GPS raw navigation page or frame */
#define ID_GPSRAWL5     4019    /* SBF message id: GPS raw navigation page or frame */
#define ID_GEORAWL1     4020    /* SBF message id: SBAS raw navigation page or frame */
#define ID_GEORAWL5     4021    /* SBF message id: SBAS raw navigation page or frame */
#define ID_GALRAWFNAV   4022    /* SBF message id: Galileo raw navigation page or frame */
#define ID_GALRAWINAV   4023    /* SBF message id: Galileo raw navigation page or frame */
#define ID_GALRAWCNAV   4024    /* SBF message id: Galileo raw navigation page or frame */
#define ID_GLORAWCA     4026    /* SBF message id: GLONASS raw navigation page or frame */
#define ID_CMPRAW       4047    /* SBF message id: Compass raw navigation page or frame */
#define ID_QZSSL1CA     4066    /* SBF message id: QZSS raw navigation page or frame */
#define ID_QZSSL2C      4067    /* SBF message id: QZSS raw navigation page or frame */
#define ID_QZSSL5       4068    /* SBF message id: QZSS raw navigation page or frame */
#define ID_IRNSSRAW     4093    /* SBF message id: IRNSS raw navigation page or frame */

#define ID_GEONAV                   5896 /* SBF message id:  SBAS navigation message */
#define ID_GEOALM                   5897 /* SBF message id:  SBAS satellite almanac */
#define ID_GEOSERVICELEVEL          5917 /* SBF message id:  SBAS Service Message */
#define ID_GEONETWORKTIME           5918 /* SBF message id:  SBAS Network Time/UTC offset parameters */
#define ID_GEOMT00                  5925 /* SBF message id:  SBAS: Don't use for safety application */
#define ID_GEOPRNMASK               5926 /* SBF message id:  PRN Mask assignments */
#define ID_GEOFASTCORR              5927 /* SBF message id:  Fast Corrections */
#define ID_GEOINTEGRITY             5928 /* SBF message id:  Integrity information */
#define ID_GEOFASTCORRDEGR          5929 /* SBF message id:  fast correction degradation factor */
#define ID_GEODEGRFACTORS           5930 /* SBF message id:  Degration factors */
#define ID_GEOIGPMASK               5931 /* SBF message id:  Ionospheric grid point mask */
#define ID_GEOLONGTERMCOR           5932 /* SBF message id:  Long term satellite error corrections */
#define ID_GEOIONODELAY             5933 /* SBF message id:  Inospheric delay correction */
#define ID_GEOCLOCKEPHCOVMATRIX     5934 /* SBF message id:  Clock-Ephemeris Covariance Matrix l*/


#define ID_GPSNAV   5891        /* SBF message id: GPS navigation data */
#define ID_GPSALM   5892        /* SBF message id: GPS almanac */
#define ID_GPSION   5893        /* SBF message id: GPS ionosphere data, Klobuchar coefficients */
#define ID_GPSUTC   5894        /* SBF message id: GPS UTC data */

#define ID_GLONAV   4004        /* SBF message id: GLONASS navigation data */
#define ID_GLOALM   4005        /* SBF message id: GLONASS almanac */
#define ID_GLOTIME  4036        /* SBF message id: GLONASS time data */

#define ID_GALNAV   4002        /* SBF message id: Galileo navigation data */
#define ID_GALALM   4003        /* SBF message id: Galileo almanac */
#define ID_GALION   4030        /* SBF message id: Galileo ionosphere data, Klobuchar coefficients */
#define ID_GALUTC   4031        /* SBF message id: Galileo UTC data */

#define ID_CMPNAV   4081        /* SBF message id: Compass navigation data */
#define ID_QZSSNAV  4095        /* SBF message id: QZSS navigation data */

#define ID_GALGSTGPS  4032      /* SBF message id: Galileo GPS time offset */

#define ID_PVTCART    4006      /* SBF message id: Rx Position Velocity and Time data in Cartesian coordinates in m */
#define ID_PVTGEOD    4007      /* SBF message id: Rx Position Velocity and Time data in Geodetic coordinates */
#define ID_DOP        4001      /* SBF message id: Dilution of Precision data */
#define ID_PVTSATCART 4008      /* SBF message id: Satellite Position Velocity and Time data */

#define ID_ENDOFPVT     5921    /* SBF message id: End of any PVT block */

#define ID_RXTIME       5914    /* SBF message id: Receiver time data */

#define ID_DIFFCORRIN   5919    /* SBF message id: incoming RTCM2 or RTCM3 or CMR message */

#define ID_BASESTATION  5949    /* SBF message id: Base station position */

#define ID_CHNSTATUS   4013     /* SBF message id: Status of the receiver channels */
#define ID_RXSTATUS    4014     /* SBF message id: Status of the receiver */
#define ID_RXSETUP     5902     /* SBF message id: Status of the receiver */
#define ID_COMMENT     5936     /* SBF message id: Status of the receiver */

#define ID_SATVISIBILITY  4012  /* SBF message id: Ssatellites visibility */
#define ID_BBSMPS         4040  /* SBF message id: series of successive Rx baseband samples */

/* function prototypes -------------------------------------------------------*/
static int getSignalCode(int signType);
static double getSigFreq(int _signType, int freqNo);
static int getFreqNo(int signType);

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((signed char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}
static signed int     I4(unsigned char *p) {signed int     u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}

/* checksum lookup table -----------------------------------------------------*/
static const unsigned int CRC_16CCIT_LookUp[256] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

/* SBF checksum calculation --------------------------------------------------*/
static unsigned short sbf_checksum(unsigned char *buff, int len)
{
    int i;
    unsigned short crc = 0;
    for (i=0; i<len; i++) {
        crc = (crc << 8) ^ CRC_16CCIT_LookUp[ (crc >> 8) ^ buff[i] ];
    }
    return crc;
}

/* 8-bit week -> full week ---------------------------------------------------*/
static void adj_utcweek(gtime_t time, double *utc)
{
    int week;

    if (utc[3]>=256.0) return;
    time2gpst(time,&week);
    utc[3]+=week/256*256;
    if      (utc[3]<week-128) utc[3]+=256.0;
    else if (utc[3]>week+128) utc[3]-=256.0;
}
/* adjust daily rollover of time ---------------------------------------------*/
static gtime_t adjday(gtime_t time, double tod)
{
    double ep[6],tod_p;
    time2epoch(time,ep);
    tod_p=ep[3]*3600.0+ep[4]*60.0+ep[5];
    if      (tod<tod_p-43200.0) tod+=86400.0;
    else if (tod>tod_p+43200.0) tod-=86400.0;
    ep[3]=ep[4]=ep[5]=0.0;
    return timeadd(epoch2time(ep),tod);
}

/* decode SBF measurements message (observables) -----------------------------*/
/*
 * this is the most importan block in the SBF format. It it contains all code
 * pseudoranges and carrier phase measurements of all received satellites. 
 * This block is made of one Type1 sub-block per santellite followed by, if any,
 * a certain number of Type2 sub-blocks. SB2Num defines how many Type2
 * sub-blocks there are inside its Type1 sub-block.
 * Type1 subplock contains code pseudorange and carrier phase range of the first
 * decoded sygnal defined by signType1, this is typically L1 signal. 
 * Any following Type2 sub-block (if there are any) contains signType2 signal
 * information, typically L2 signal. Inside Type2 sub-blocks, information is
 * expressed as difference from the data in signType1 sub-block. This makes the
 * format a little more compact.
 *
*/
static int decode_measepoch(raw_t *raw){
    gtime_t time;
    double tow,psr, adr, dopplerType1;
    double SNR_DBHZ, SNR2_DBHZ, freqType1, freqType2, alpha;
    int16_t i,ii,j,prn,sat,n=0,nsat,week, code, h;
    uint8_t *p=(raw->buff)+8;                   /* jump to TOW location */
    int SB1length,SB2length;
    uint8_t signType1, signType2;
    uint32_t codeLSB, SB2Num, sys;
    uint8_t codeMSB, CommonFlags;
    int pri;

    /* signals for type2 sub-block */
    int32_t CodeOffsetMSB, DopplerOffsetMSB, CarrierMSB;
    uint16_t DopplerOffsetLSB, CodeOffsetLSB,CarrierLSB;
    double PRtype2, Ltype2, dopplerType2;

    uint16_t LockTime, LockTime2;
    uint8_t ObsInfo,ObsInfo2, offsetMSB;
    double SB1_WaveLength, SB1_Code = 0.0;
    short SB1_FreqNr;

    trace(4,"SBF decode_measepoch: len=%d\n",raw->len);

    /* Get time information */
    tow =U4(p);                                            /*       TOW in ms */
    week=U2(p+4);                                          /* number of weeks */

    /* Tweak pseudoranges to allow Rinex to represent the time of measure
       it is taken from nvs.c but it does not seem necessary
    dTowInt  = 10.0*floor((tow/10.0)+0.5);
    time=gpst2time(week, dTowInt*0.001);
    */

    time=gpst2time(week, tow*0.001);

    /* number of type1 sub-blocks also equal to number of satellites */
    nsat   = U1(p+6);

    /* additional block information */
    SB1length=U1(p+7);                              /* Type1 sub-block length */
    SB2length=U1(p+8);                              /* Type2 sub-block length */

    CommonFlags = U1 (p+9);

    if ((CommonFlags & 0x80)==0x80) return 0;         /* data is ccrambled and not valid */

    /* set the pointer from TOW to the beginning of type1 sub-block */
    p = p + 12;

    for (i=0;i<nsat&&i<MAXOBS;i++) {

        /* decode type1 sub-block */
        signType1 = U1(p+1) & 0x1f;               /* type of signal, bit[0-4] */
        prn = U1(p+2);                            /* satellite number         */
        raw->obs.data[n].time  = time;            /* not sure what ref. it is */
        codeMSB = (U1(p+3) & 0x0f);               /* code phase MSB. bit[0-3] */
        codeLSB = U4(p+4);                        /* code phase LSB           */

        /* code pseudorange in m */
        psr = (codeMSB*4294967296.0+codeLSB)*0.001;

        /*  Doppler in Hz */
        dopplerType1  = 0.0001*I4(p+8);

        /* signal to noise ratio in dBHz */
        if ((signType1==1) || (signType1==2)){
            SNR_DBHZ=((double)U1(p+15))*0.25;
        }
        else SNR_DBHZ=(((double)U1(p+15))*0.25)+10;

        /* Compute the carrier phase measurement (a little complicated) */
        LockTime = U2(p+16);             /* Duration of contin. carrier phase */
        ObsInfo = U1(p+18);              /* some extra info                   */
        SB2Num = U1(p+19);               /* number of type2 sub-blocks        */
        SB1_Code = psr;                  /* code phase (from before)          */

        /* FreqNr */
        SB1_FreqNr = ((ObsInfo >> 3) & 0x1f) - 8;

        SB1_WaveLength=CLIGHT/getSigFreq(signType1,SB1_FreqNr);

        /* final carrier phase calculation */
        adr = (SB1_Code/SB1_WaveLength)+(I1(p+14)*65536.0+U2(p+12))*0.001;
        if ((I2(p+14)==-128)&&(U2(p+12)==0)) {
            adr=0;
        }

        /* debug */
        trace(4,"signal type = %2d, \n",signType1);

        /* from the signal tiype get the type of RTKLIB signal code*/
        code = getSignalCode(signType1);

        /* NOT SURE IF THIS IS CORRECT, MAYBE ITS SHOULD APPLY TO L2 AS WELL */
        /* phase polarity flip option (-INVCP) */
        if (strstr(raw->opt,"-INVCP")) {
            adr = - adr;
        }

        /* work out sat number and type of GNSS system */
        if ((prn>=1)&&(prn<=37)){
            sys = SYS_GPS;                      /* navigation system: GPS     */
            sat = prn;}
        else if ((prn>=38)&&(prn<=61)){
            sys = SYS_GLO;                      /* navigation system: GLONASS */
            sat = prn - 37;}
        else if ((prn>=63)&&(prn<=68)){
            sys = SYS_GLO;                      /* navigation system: GLONASS */
            sat = prn - 38;}
        else if ((prn>=71)&&(prn<=102)){
            sys = SYS_GAL;                      /* navigation system: Galileo */
            sat = prn - 70;}
        else if ((prn>=120)&&(prn<=140)){
            sys = SYS_SBS;                      /* navigation system: SBAS    */
            sat = prn;}
        else if ((prn>=141)&&(prn<=177)){
            sys = SYS_CMP;                      /* navigation system: BeiDou  */
            sat = prn - 140;}
        else if ((prn>=181)&&(prn<=187)){
            sys = SYS_QZS;                      /* navigation system: QZSS    */
            sat = prn - 180;}
        else if ((prn>=191)&&(prn<=197)){
            sys = SYS_IRN;                      /* navigation system: IRNSS  */
            sat = prn - 190;}
        else if ((prn>=198)&&(prn<=215)){
            sys = SYS_SBS;                      /* navigation system: SBAS, */
            sat = prn - 157;}
        else{
            sys = SYS_NONE;                     /* navigation system: none    */
            sat = 0;}

        /* store satellite number */
        sat = satno(sys, sat);
        if (sat == 0)
        {
            p = p + SB1length; /* skip data */
            p = p + SB2length*SB2Num;
            continue;
        };

        raw->obs.data[n].sat=sat;

        /* start new observation period */
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }

        /* store type1 signal information is RTKLIB signal structure
         */
        /* Set all channels to 0 */
        for (j=0;j<NFREQ+NEXOBS;j++) {
            raw->obs.data[n].L[j]=0.0;
            raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=(float)0.0;
            raw->obs.data[n].SNR[j]=(unsigned char)0;
            raw->obs.data[n].LLI[j]=(unsigned char)0;
            raw->obs.data[n].code[j]=CODE_NONE;            
        }
        /* detect which signals is stored in Type1 sub-block */
#if 1
        h=getFreqNo(signType1);
#else
        freqType1 = getSigFreq(signType1,8);
        if      (freqType1 == FREQ1) h = 0;
        else if (freqType1 == FREQ2) h = 1;
        else if (freqType1 == FREQ5) h = 2;
        else if (freqType1 == FREQ6) h = 3;
        else if (freqType1 == FREQ7) h = 4;
        else if (freqType1 == FREQ8) h = 5;
        else                         h = 0;
#endif
        /* store signal info */
        if (h<=NFREQ+NEXOBS) {
            raw->obs.data[n].L[h]    = adr;
            raw->obs.data[n].P[h]    = psr;
            raw->obs.data[n].D[h]    = (float)dopplerType1;
            raw->obs.data[n].SNR[h]  = (unsigned char)(SNR_DBHZ*4.0);
            raw->obs.data[n].code[h] = code;

            /* lock to signal indication */
            if ((ObsInfo&0x4)==0x4) raw->obs.data[n].LLI[h]|=0x2; /* half-cycle ambiguity */
            if (LockTime!=65535){
                LockTime = LockTime>254?254:LockTime; /* limit locktime to sizeof(unsigned char) */
                if (locktime[sat][signType1]>LockTime) raw->obs.data[n].LLI[h]|=0x1;
                raw->lockt[sat][h]       = (unsigned char)LockTime;
                locktime[sat][signType1] = LockTime;
            };
        }

        /* decode all Type2 sub-blocks (if there is any) */
        p = p + SB1length;                 /* get to the begin of Type2 block */

        for (ii=0;ii<(int)SB2Num;ii++)
        {
            signType2 = U1(p) & 0x1f;             /* type of signal, bit[0-4] */

            /* Duration of continuous carrier phase */
            LockTime2 = U1(p+1);

            /* Signal to noise ratio in dbHz */
            if ((signType2==1) || (signType2==2)){
                SNR2_DBHZ=((double)U1(p+2))*0.25;
            }
            else SNR2_DBHZ=(((double)U1(p+2))*0.25)+10;

            offsetMSB = U1(p+3);
            CodeOffsetMSB=((offsetMSB&0x04)==0x04)?offsetMSB| ~((int32_t)0x03):offsetMSB&0x03;                 /* bit[0-2] */
            DopplerOffsetMSB=((offsetMSB&0x80)==0x80)?(offsetMSB>>3)| ~((int32_t)0x1f):(offsetMSB>>3)&0x1f;/* bit[3-7] */

            CarrierMSB = I1(p+4);

            ObsInfo2 = U1(p+5);                         /* minor informations */

            freqType1 = getSigFreq(signType1,SB1_FreqNr);
            freqType2 = getSigFreq(signType2,SB1_FreqNr);

            /* pseudrange in meters */
            CodeOffsetLSB = U2(p+6);
            PRtype2 = psr + (CodeOffsetMSB*65536.+CodeOffsetLSB)*0.001;
            if ((CodeOffsetMSB==-4)&&(CodeOffsetLSB==0)) {
                PRtype2=0;
            }

            /* carrier phase in cycles */
            CarrierLSB = U2(p+8);
            Ltype2=(PRtype2/CLIGHT*freqType2)+(CarrierMSB*65536.+CarrierLSB)*0.001;
            if ((CarrierMSB==-128)&&(CarrierLSB==0)) {
                Ltype2=0;
            }

            /* Doppler in Hz */
            DopplerOffsetLSB = U2(p+10);
            alpha = (freqType2/freqType1);
            if ((DopplerOffsetMSB==-16) && (DopplerOffsetLSB==0)) dopplerType2=0;
            else
                dopplerType2 = dopplerType1*alpha +\
                    (DopplerOffsetMSB*65536.+DopplerOffsetLSB)*1E-4;

            /* store Type2 signal info in rtklib structure */
#if 1
            h=getFreqNo(signType2);
#else
            freqType2 = getSigFreq(signType2,8);
            if      (freqType2 == FREQ1) h = 0;
            else if (freqType2 == FREQ2) h = 1;
            else if (freqType2 == FREQ5) h = 2;
            else if (freqType2 == FREQ6) h = 3;
            else if (freqType2 == FREQ7) h = 4;
            else if (freqType2 == FREQ8) h = 5;
            else                         h = 0;
#endif
            pri=getcodepri(sys,getSignalCode(signType2),raw->opt); /* get signal priority */
            /* store signal info */
            if ((h<=NFREQ+NEXOBS)&&
                    (pri>getcodepri(sys,raw->obs.data[n].code[h],raw->opt))) {
                raw->obs.data[n].L[h]    = Ltype2;
                raw->obs.data[n].P[h]    = PRtype2;
                raw->obs.data[n].D[h]    = (float)dopplerType2;
                raw->obs.data[n].SNR[h]  = (unsigned char)(SNR2_DBHZ*4.0);
                raw->obs.data[n].code[h] = getSignalCode(signType2);

                /* lock to signal indication */
                if ((ObsInfo2&0x4)==0x4) raw->obs.data[n].LLI[h]|=0x2; /* half-cycle ambiguity */
                if (LockTime2!=255) {
                    if (locktime[sat][signType2]>LockTime2) raw->obs.data[n].LLI[h]|=0x1;
                    raw->lockt[sat][h]       = (unsigned char)LockTime2;
                    locktime[sat][signType2] = (unsigned char)LockTime2;
                };
            }

            /* get to the beginning of next Type 2 block */
            p = p + SB2length;
        }

        /* Receiver channel goes up */
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}

/* return frequency value in Hz from signal type name ------------------------*/
static double getSigFreq(int _signType, int freqNo){

    switch (_signType)
    {
    case 0:                                                        /* GPSL1CA */
        return FREQ1;
    case 1:                                                        /* GPSL1PY */
        return FREQ1;
    case 2:                                                        /* GPSL2PY */
        return FREQ2;
    case 3:                                                        /* GPSL2C  */
        return FREQ2;
    case 4:                                                        /* GPSL5   */
        return FREQ5;
    case 6:                                                        /* QZSL1C  */
        return FREQ1;
    case 7:                                                        /* QZSL2C  */
        return FREQ2;
    case 8:                                                        /* GLOL1CA */
        return FREQ1_GLO+(freqNo*9./16.)*1e6;
    case 9:                                                        /* GLOL1P  */
        return FREQ1_GLO+(freqNo*9./16.)*1e6;
    case 10:                                                       /* GLOL2P  */
        return FREQ2_GLO+(freqNo*7./16.)*1e6;
    case 11:                                                       /* GLOL2CA */
        return FREQ2_GLO+(freqNo*7./16.)*1e6;
    case 12:                                                       /* GLOL3X  */
        return 1.202025*1e9;
    case 15:                                                       /* IRNSSL5  */
        return FREQ5;
    case 16:                                                       /* GALL1A  */
        return FREQ1;
    case 17:                                                       /* GALL1BC */
        return FREQ1;
    case 18:                                                       /* GALE6A  */
        return FREQ6;
    case 19:                                                       /* GALE6BC */
        return FREQ6;
    case 20:                                                       /* GALE5a  */
        return FREQ5;
    case 21:                                                       /* GALE5b  */
        return FREQ7;
    case 22:                                                       /* GALE5   */
        return FREQ8;
    case 24:                                                       /* GEOL1   */
        return FREQ1;
    case 25:                                                       /* GEOL5   */
        return FREQ5;
    case 26:                                                       /* QZSL5   */
        return FREQ5;
    case 28:                                                       /* CMPL1   */
        return FREQ1_CMP;
    case 29:                                                       /* CMPE5B  */
        return FREQ2_CMP;
    case 30:                                                       /* CMPB3   */
        return FREQ3_CMP;
    }
    return FREQ1;
}

#if 0 /* UNUSED */

/* adjust weekly rollover of gps time ----------------------------------------*/
static gtime_t adjweek(gtime_t time, double tow)
{
    double tow_p;
    int week;
    tow_p=time2gpst(time,&week);
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    return gpst2time(week,tow);
}

#endif /* UNUSED */

/* return the Septentrio signal type -----------------------------------------*/
static int getSignalCode(int signType){
    int _code=-1;

    switch (signType)
    {
    case 0:                                                        /* GPSL1CA */
        _code=CODE_L1C;
        break;
    case 1:                                                        /* GPSL1PY */
        _code=CODE_L1W;
        break;
    case 2:                                                        /* GPSL2PY */
        _code=CODE_L2W;
        break;
    case 3:                                                        /* GPSL2C  */
        _code=CODE_L2L;
        break;
    case 4:                                                        /* GPSL5   */
        _code=CODE_L5Q;
        break;
    case 6:                                                        /* QZSL1   */
        _code=CODE_L1C;
        break;
    case 7:                                                        /* QZSL2   */
        _code=CODE_L2L;
        break;
    case 8:                                                        /* GLOL1CA */
        _code=CODE_L1C;
        break;
    case 9:                                                        /* GLOL1P  */
        _code=CODE_L1P;
        break;
    case 10:                                                       /* GLOL2P  */
        _code=CODE_L2P;
        break;
    case 11:                                                       /* GLOL2CA */
        _code=CODE_L2C;
        break;
    case 12:                                                       /* GLOL3 */
        _code=CODE_L3Q;
        break;
    case 15:                                                       /* IRNSSL5  */
        _code=CODE_L5A;
        break;
    case 16:                                                       /* GALE1A  */
        _code=CODE_L1A;
        break;
    case 17:                                                       /* GALE1BC */
        _code=CODE_L1C;
        break;
    case 18:                                                       /* GALE6A  */
        _code=CODE_L6A;
        break;
    case 19:                                                       /* GALE6BC */
        _code=CODE_L6C;
        break;
    case 20:                                                       /* GALE5a  */
        _code=CODE_L5Q;
        break;
    case 21:                                                       /* GALE5b  */
        _code=CODE_L7Q;
        break;
    case 22:                                                       /* GALE5   */
        _code=CODE_L8Q;
        break;
    case 24:                                                       /* GEOL1   */
        _code=CODE_L1C;
        break;
    case 25:                                                       /* GEOL5   */
        _code=CODE_L5I;
        break;
    case 26:                                                       /* QZSL5   */
        _code=CODE_L5Q;
        break;
    case 28:                                                       /* CMPL1   */
        _code=CODE_L2I;
        break;
    case 29:                                                       /* CMPE5B  */
        _code=CODE_L7I;
        break;
    case 30:                                                       /* CMPB3   */
        _code=CODE_L6I;
        break;
    default:                                                       /* GPSL1CA */
        _code=CODE_L1C;
        break;
    }
    return _code;
}
/* return the signal type -----------------------------------------*/
static int getFreqNo(int signType){
    int _freq;

    switch (signType)
    {
    case 0:                                                        /* GPSL1CA */
        _freq=0;
        break;
    case 1:                                                        /* GPSL1PY */
        _freq=0;
        break;
    case 2:                                                        /* GPSL2PY */
        _freq=1;
        break;
    case 3:                                                        /* GPSL2C  */
        _freq=1;
        break;
    case 4:                                                        /* GPSL5   */
        _freq=2;
        break;
    case 6:                                                        /* QZSL1   */
        _freq=0;
        break;
    case 7:                                                        /* QZSL2   */
        _freq=1;
        break;
    case 8:                                                        /* GLOL1CA */
        _freq=0;
        break;
    case 9:                                                        /* GLOL1P  */
        _freq=0;
        break;
    case 10:                                                       /* GLOL2P  */
        _freq=1;
        break;
    case 11:                                                       /* GLOL2CA */
        _freq=2;
        break;
    case 12:                                                       /* GLOL3 */
        _freq=3;
        break;
    case 15:                                                       /* IRNSSL5  */
        _freq=2;
        break;
    case 16:                                                       /* GALE1A  */
        _freq=0;
        break;
    case 17:                                                       /* GALE1BC */
        _freq=0;
        break;
    case 18:                                                       /* GALE6A  */
        _freq=1;
        break;
    case 19:                                                       /* GALE6BC */
        _freq=1;
        break;
    case 20:                                                       /* GALE5a  */
        _freq=1;
        break;
    case 21:                                                       /* GALE5b  */
        _freq=1;
        break;
    case 22:                                                       /* GALE5   */
        _freq=2;
        break;
    case 24:                                                       /* GEOL1   */
        _freq=0;
        break;
    case 25:                                                       /* GEOL5   */
        _freq=2;
        break;
    case 26:                                                       /* QZSSL5  */
        _freq=2;
        break;
    case 28:                                                       /* CMPL1   */
        _freq=0;
        break;
    case 29:                                                       /* CMPE5B  */
        _freq=2;
        break;
    case 30:                                                       /* CMPB3   */
        _freq=1;
        break;
    default:                                                       /* GPSL1CA */
        _freq=0;
        break;
    }
    return _freq;
}

/* decode SBF nav message for GPS (navigation data) --------------------------*/
static int decode_gpsnav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    uint8_t prn, sat;
    uint16_t week;

    trace(4,"SBF decode_gpsnav: len=%d\n",raw->len);

    if ((raw->len)<120) {
        trace(2,"SBF decode_gpsnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8);
    sat = satno(SYS_GPS,prn);

    if (sat == 0) return -1;

    if (!((prn>=1)&&(prn<=37))){
        trace(2,"SBF decode_gpsnav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.crs    = R4(puiTmp +  42);
    eph.deln   = R4(puiTmp +  46) * PI;
    eph.M0     = R8(puiTmp +  50) * PI;
    eph.cuc    = R4(puiTmp +  58);
    eph.e      = R8(puiTmp +  62);
    eph.cus    = R4(puiTmp +  70);
    eph.A      = pow(R8(puiTmp +  74), 2);
    eph.toes   = U4(puiTmp +  82);
    eph.cic    = R4(puiTmp +  86);
    eph.OMG0   = R8(puiTmp +  90) * PI;
    eph.cis    = R4(puiTmp +  98);
    eph.i0     = R8(puiTmp + 102) * PI;
    eph.crc    = R4(puiTmp + 110);
    eph.omg    = R8(puiTmp + 114) * PI;
    eph.OMGd   = R4(puiTmp + 122) * PI;
    eph.idot   = R4(puiTmp + 126) * PI;
    eph.tgd[0] = R4(puiTmp +  22);
    toc        = U4(puiTmp +  26);
    eph.f2     = R4(puiTmp +  30);
    eph.f1     = R4(puiTmp +  34);
    eph.f0     = R4(puiTmp +  38);
    eph.sva    = U1(puiTmp +  13); /* URA */
    eph.iodc   = U2(puiTmp +  16);
    eph.iode   = U1(puiTmp +  18);
    eph.code   = U1(puiTmp +  12);
    eph.flag   = U1(puiTmp +  15);
    eph.fit    = U1(puiTmp +  20)?0:4;
    week       = U2(puiTmp +  10); /* WN */

    if (week>=4096) {
        trace(2,"SBF gps ephemeris week error: sat=%2d week=%d\n",sat,week);
        return -1;
    }

    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;

    if (!strstr(raw->opt,"-EPHALL")) {
        if ((eph.iode==raw->nav.eph[sat-1].iode) &&
            (eph.iodc==raw->nav.eph[sat-1].iodc)) return 0;
    }

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode SBF nav message for Galileo (navigation data) --------------------------*/
static int decode_galnav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    int prn, sat;
    uint16_t week_oe, week_oc;
    uint32_t tow;

    trace(4,"SBF decode_galnav: len=%d\n",raw->len);

    if ((raw->len)<152) {
        trace(2,"SBF decode_galnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8)-70;
    sat = satno(SYS_GAL,prn);

    if (sat == 0) return -1;

    if (!((prn>=1)&&(prn<=36))){
        trace(2,"SBF decode_galnav prn error: sat=%d\n",prn);
        return -1;
    }

    tow        = U4(puiTmp +  2);
    eph.week   = U2(puiTmp +  6); /* GAL week number */
    eph.code   = U1(puiTmp +  9)==2?0:1; /* 0:INAV,1:FNAV */
    eph.A      = pow(R8(puiTmp +  10), 2);
    eph.M0     = R8(puiTmp +  18) * PI;
    eph.e      = R8(puiTmp +  26);
    eph.i0     = R8(puiTmp +  34) * PI;
    eph.omg    = R8(puiTmp +  42) * PI;
    eph.OMG0   = R8(puiTmp +  50) * PI;
    eph.OMGd   = R4(puiTmp +  58) * PI;
    eph.idot   = R4(puiTmp +  62) * PI;
    eph.deln   = R4(puiTmp +  66) * PI;
    eph.cuc    = R4(puiTmp +  70);
    eph.cus    = R4(puiTmp +  74);
    eph.crc    = R4(puiTmp +  78);
    eph.crs    = R4(puiTmp +  82);
    eph.cic    = R4(puiTmp +  86);
    eph.cis    = R4(puiTmp +  90);
    eph.toes   = U4(puiTmp +  94);
    toc        = U4(puiTmp +  98);
    eph.f2     = R4(puiTmp + 102);
    eph.f1     = R4(puiTmp + 106);
    eph.f0     = R8(puiTmp + 110);
    week_oe    = U2(puiTmp + 118); /* WNt_oc */
    week_oc    = U2(puiTmp + 120);
    eph.iode   =
    eph.iodc   = U2(puiTmp + 122);
    if (eph.code==0) /* INAV */
    {
        eph.sva    = U1(puiTmp + 128);
        eph.svh    = (U2(puiTmp + 124)& 0x00ff)^0x0011;
    } else { /* FNAV */
        eph.sva    = U1(puiTmp + 127);
        eph.svh    = (U2(puiTmp + 124)& 0x0f0f)^0x0101;
    }

    eph.tgd[0] = R4(puiTmp + 130);
    eph.tgd[1] = R4(puiTmp + 134);
    eph.fit    = 0;

    week_oe=adjgpsweek(week_oe);
    week_oc=adjgpsweek(week_oc);
    eph.toe=gpst2time(week_oe,eph.toes);
    eph.toc=gpst2time(week_oc,toc);
    eph.ttr=gpst2time(eph.week,tow/1000);

    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    }

    if (sat == 0) return -1;

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

/* decode SBF nav message for glonass (navigation data) ----------------------*/
static int decode_glonav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    geph_t eph={0};
    int prn, sat;
    uint16_t week;

    trace(4,"SBF decode_glonav: len=%d\n",raw->len);

    if ((raw->len)<96) {
        trace(2,"SBF decode_glonav frame length error: len=%d\n",raw->len);
        return -1;
    }
    prn = U1(puiTmp+8)-37;
    sat = satno(SYS_GLO,prn);

    if (sat == 0) return -1;

    if (!((prn>=1)&&(prn<=24))){
        trace(2,"SBF decode_glonav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.frq    = U1(puiTmp +  9) - 8;
    eph.pos[0] = R8(puiTmp +  10) * 1000;
    eph.pos[1] = R8(puiTmp +  18) * 1000;
    eph.pos[2] = R8(puiTmp +  26) * 1000;
    eph.vel[0] = R4(puiTmp +  34) * 1000;
    eph.vel[1] = R4(puiTmp +  38) * 1000;
    eph.vel[2] = R4(puiTmp +  42) * 1000;
    eph.acc[0] = R4(puiTmp +  46) * 1000;
    eph.acc[1] = R4(puiTmp +  50) * 1000;
    eph.acc[2] = R4(puiTmp +  54) * 1000;
    eph.gamn   = R4(puiTmp +  58);
    eph.taun   = R4(puiTmp +  62);
    eph.dtaun  = R4(puiTmp +  66);
    week       = U2(puiTmp +  74); /* WN_toe modulo 1024 */
    week       = adjgpsweek(week);

    eph.toe    = gpst2time(week, U4(puiTmp +  70));
    eph.tof    = raw->time;
    eph.age    = U1(puiTmp +  78);
    eph.svh    = U1(puiTmp +  79);
    eph.iode   = U2(puiTmp +  80);
    eph.sva    = U2(puiTmp +  88);

    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.geph[prn-1].iode) return 0;
    }

    eph.sat=sat;
    raw->nav.geph[prn-1]=eph;
    raw->ephsat=sat;
    raw->nav.glo_fcn[prn-1] = eph.frq + 8; /* savbe frequency number */

    return 2;
}
/* decode SBF nav message for sbas (navigation data) ----------------------*/
static int decode_sbasnav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    seph_t eph={0};
    int prn, sat;
    uint16_t week;
    uint32_t tod,tow;

    trace(4,"SBF decode_sbasnav: len=%d\n",raw->len);

    if ((raw->len)<104) {
        trace(2,"SBF decode_sbasnav frame length error: len=%d\n",raw->len);
        return -1;
    }
    prn = U1(puiTmp+8);
    sat = satno(SYS_SBS,prn);

    if (!((prn>=120)&&(prn<=140))){
        trace(2,"SBF decode_sbasnav prn error: sat=%d\n",prn);
        return -1;
    }

    if (sat == 0) return -1;

    week       = U2(puiTmp +   6);
    tow        = U4(puiTmp +  2)/1000;
    tod        = U4(puiTmp +  14);
    eph.tof    = gpst2time(adjgpsweek(week),tow);
    eph.t0     = adjday(eph.tof,tod);
    eph.sva    = U2(puiTmp +  12);
    eph.svh    = eph.sva==15?1:0;
    eph.pos[0] = R8(puiTmp +  18);
    eph.pos[1] = R8(puiTmp +  26);
    eph.pos[2] = R8(puiTmp +  34);
    eph.vel[0] = R8(puiTmp +  42);
    eph.vel[1] = R8(puiTmp +  50);
    eph.vel[2] = R8(puiTmp +  58);
    eph.acc[0] = R8(puiTmp +  66);
    eph.acc[1] = R8(puiTmp +  74);
    eph.acc[2] = R8(puiTmp +  82);
    eph.af0    = R4(puiTmp +  90);
    eph.af1    = R4(puiTmp +  94);

    /* debug */
    trace(2,"sat=%2d, week=%d, tow=%f\n",sat,week,U4(puiTmp +  2)/1000);

    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(eph.t0,raw->nav.seph[prn-120].t0))<1.0&&
            eph.sva==raw->nav.seph[prn-120].sva)
            return 0;
    }

    eph.sat=sat;
    raw->nav.seph[prn-120]=eph;
    raw->ephsat=eph.sat;
    return 2;
}

#if 0 /* UNUSED */

/* decode SBF nav message for Compass/Beidou (navigation data) --------------------------*/
static int decode_cmpnav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    int prn, sat;
    uint16_t week_oc,week_oe;

    trace(4,"SBF decode_cmpnav: len=%d\n",raw->len);

    if ((raw->len)<140) {
        trace(2,"SBF decode_cmpnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8)-140;
    sat = satno(SYS_CMP,prn);

    if (sat == 0) return -1;

    if (!((prn>=1)&&(prn<=32))){
        trace(2,"SBF decode_cmpnav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.code   = 0;
    eph.sva    = U1(puiTmp + 12);
    eph.svh    = U1(puiTmp + 13);
    eph.iodc   = U1(puiTmp + 14);
    eph.iode   = U1(puiTmp + 15);
    eph.tgd[0] = R4(puiTmp + 18);
    eph.tgd[1] = R4(puiTmp + 22);
    toc        = U4(puiTmp + 26);
    eph.f2     = R4(puiTmp + 30);
    eph.f1     = R4(puiTmp + 34);
    eph.f0     = R4(puiTmp + 38);
    eph.crs    = R4(puiTmp + 42);
    eph.deln   = R4(puiTmp + 46) * PI;
    eph.M0     = R8(puiTmp + 50) * PI;
    eph.cuc    = R4(puiTmp + 58);
    eph.e      = R8(puiTmp + 62);
    eph.cus    = R4(puiTmp + 70);
    eph.A      = pow(R8(puiTmp +  74), 2);
    eph.toes   = U4(puiTmp + 82);
    eph.cic    = R4(puiTmp + 86);
    eph.OMG0   = R8(puiTmp + 90) * PI;
    eph.cis    = R4(puiTmp + 98);
    eph.i0     = R8(puiTmp +102) * PI;
    eph.crc    = R4(puiTmp +110);
    eph.omg    = R8(puiTmp +114) * PI;
    eph.OMGd   = R4(puiTmp +122) * PI;
    eph.idot   = R4(puiTmp +126) * PI;
    week_oc    = U2(puiTmp +130); /* WNt_oc */
    week_oe    = U2(puiTmp +132); /* WNt_oe l*/
    eph.fit    = 0;

    eph.week=adjgpsweek(week_oc);
    eph.toe=bdt2time(adjgpsweek(week_oe),eph.toes);
    eph.toc=bdt2time(eph.week,toc);
    eph.ttr=raw->time;

    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    }

    if (sat == 0) return -1;

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

/* decode SBF nav message for QZSS (navigation data) --------------------------*/
static int decode_qzssnav(raw_t *raw){

    uint8_t *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    int prn, sat;
    uint16_t week_oc,week_oe;

    trace(4,"SBF decode_qzssnav: len=%d\n",raw->len);

    if ((raw->len)<140) {
        trace(2,"SBF decode_qzssnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8)-180;
    sat = satno(SYS_QZS,prn);

    if (sat == 0) return -1;

    if (!((prn>=1)&&(prn<=7))){
        trace(2,"SBF decode_qzssnav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.code   = U1(puiTmp + 12);
    eph.sva    = U1(puiTmp + 13);
    eph.svh    = U1(puiTmp + 14);
    eph.iodc   = U2(puiTmp + 16);
    eph.iode   = U1(puiTmp + 18);
    eph.fit    = U1(puiTmp + 20);
    eph.tgd[0] = R4(puiTmp + 22);
    toc        = U4(puiTmp + 26);
    eph.f2     = R4(puiTmp + 30);
    eph.f1     = R4(puiTmp + 34);
    eph.f0     = R4(puiTmp + 38);
    eph.crs    = R4(puiTmp + 42);
    eph.deln   = R4(puiTmp + 46) * PI;
    eph.M0     = R8(puiTmp + 50) * PI;
    eph.cuc    = R4(puiTmp + 58);
    eph.e      = R8(puiTmp + 62);
    eph.cus    = R4(puiTmp + 70);
    eph.A      = pow(R8(puiTmp +  74), 2);
    eph.toes   = U4(puiTmp + 82);
    eph.cic    = R4(puiTmp + 86);
    eph.OMG0   = R8(puiTmp + 90) * PI;
    eph.cis    = R4(puiTmp + 98);
    eph.i0     = R8(puiTmp +102) * PI;
    eph.crc    = R4(puiTmp +110);
    eph.omg    = R8(puiTmp +114) * PI;
    eph.OMGd   = R4(puiTmp +122) * PI;
    eph.idot   = R4(puiTmp +126) * PI;
    week_oc    = U2(puiTmp +130); /* WNt_oc */
    week_oe    = U2(puiTmp +132); /* WNt_oe l*/

    eph.week=adjgpsweek(week_oc);
    eph.toe=gpst2time(adjgpsweek(week_oe),eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;

    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    }

    if (sat == 0) return -1;

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

#endif /* UNUSED */

/* decode SBF raw nav message (raw navigation data) --------------------------*/
static int decode_rawnav(raw_t *raw, int sys){

    /* NOTE. This function works quite well but it somestimes fails in line:
     * if (resp>5 || resp<=0){
     * To debug the problem an understanding of the whole RTK code is needed
     */

    uint8_t *p=(raw->buff)+6,id;
    eph_t eph={0};
    int sat,prn;
    uint8_t _buf[30]={0};
    int i=0,ii=0;

    trace(3,"SBF decode_gpsrawcanav: len=%d\n",raw->len);

    if (raw->len<58) {
        trace(2,"SBF decode_gpsrawcanav block length error: len=%d\n",raw->len);
        return -1;
    }

    /* get GPS satellite number */
    prn=U1(p+8);
    if (sys==SYS_QZS) prn-=180;

    sat=satno(sys,prn);
    if (sat == 0) return -1;

    /* clean up subframe from Septentrio. This is a little bit of work because
     * Septentrio Rx add some parity bits to this message.
     * We have to throw away the reserved bits as well as the parity bits.
     */

     /*   | 2bits |         24bits        |  6bits  |       <- SBF 32-bit word
         ------------------------------------------
                  | byte1 | bite2 | byte3 |                 <- sat nav message
     */

     for (i=0;i<40;i+=4){
         _buf[ii]   =((U4(p+14+i)>>22) & 0x000000FF);     /* take first byte  */
         _buf[1+ii] =((U4(p+14+i)>>14) & 0x000000FF);     /* take second byte */
         _buf[2+ii] =((U4(p+14+i)>>6 ) & 0x000000FF);     /* take third byte  */
         ii = ii+3;
     }

     /* Now that we have a classic subframe we call the generic function */
     id=getbitu(_buf,43,3); /* get subframe id */
     if ((id < 1) || (id > 5)) return -1;

     memcpy(raw->subfrm[sat-1]+(id-1)*30,_buf,30);

     if (decode_frame(raw->subfrm[sat-1]   ,&eph,NULL,NULL,NULL,NULL)==1&&
         decode_frame(raw->subfrm[sat-1]+30,&eph,NULL,NULL,NULL,NULL)==2&&
         decode_frame(raw->subfrm[sat-1]+60,&eph,NULL,NULL,NULL,NULL)==3) {

             if (!strstr(raw->opt,"-EPHALL")) {
                 if ((eph.iode==raw->nav.eph[sat-1].iode)&&
                     (eph.iodc==raw->nav.eph[sat-1].iodc)) return 0;
             }
             eph.sat=sat;
             raw->nav.eph[sat-1]=eph;
             raw->ephsat=sat;
             return 2;
     }
     if (id==4) {
         if (sys==SYS_GPS) {
                 decode_frame(raw->subfrm[sat-1]+90,NULL,raw->nav.alm,raw->nav.ion_gps,
                              raw->nav.utc_gps,&raw->nav.leaps);
                 adj_utcweek(raw->time,raw->nav.utc_gps);
             }
             else if (sys==SYS_QZS) {
                 decode_frame(raw->subfrm[sat-1]+90,NULL,raw->nav.alm,raw->nav.ion_qzs,
                              raw->nav.utc_qzs,&raw->nav.leaps);
                 adj_utcweek(raw->time,raw->nav.utc_qzs);
             }
         return 9;
     };
     if (id==5) {
         if (sys==SYS_GPS) {
                 decode_frame(raw->subfrm[sat-1]+120,NULL,raw->nav.alm,NULL,NULL,NULL);
             }
             else if (sys==SYS_QZS) {
                 decode_frame(raw->subfrm[sat-1]+120,NULL,raw->nav.alm,raw->nav.ion_qzs,
                              raw->nav.utc_qzs,&raw->nav.leaps);
                 adj_utcweek(raw->time,raw->nav.utc_qzs);
             }
         return 9;
     };

     trace(4,"SBF, decode_gpsrawcanav: sat=%2d\n",sat);
     return 0;
}
/* decode SBF raw nav message (raw navigation data) --------------------------*/
static int decode_georaw(raw_t *raw){

    uint8_t *p=(raw->buff)+6;
    uint8_t buff[8*4];
    int prn;
    int i=0;
    uint32_t tmp,crc;

    trace(3,"SBF decode_georaw: len=%d\n",raw->len);

    if (raw->len<52) {
        trace(2,"SBF decode_georaw block length error: len=%d\n",raw->len);
        return -1;
    }

    if (U1(p+9)!=1) /* CRC test failed */
    {
        return -1;
    }

    /* get GPS satellite number */
    prn=U1(p+8);

    /* copy data */
    for (i=0;i<8;i++)
    {
        tmp = U4(p+14+i*4);
        buff[4*i]=(tmp>>24) & 0xff;
        buff[4*i+1]=(tmp>>16) & 0xff;
        buff[4*i+2]=(tmp>>8) & 0xff;
        buff[4*i+3]=(tmp>>0) & 0xff;
    }

    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=U4(p+2)/1000;
    raw->sbsmsg.week=U2(p+6);
    raw->time=gpst2time(raw->sbsmsg.week,raw->sbsmsg.tow);

    crc=(buff[31])+(buff[30]<<8)+(buff[29]<<16);
    if (crc!=rtk_crc24q(buff,29)) return 0;

    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=buff[i];
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}

/* decode SBF raw nav message (raw navigation data) for galileo I/NAV---------*/
static int decode_galrawinav(raw_t *raw){
    eph_t eph={0};
    uint8_t buff[8*4],crc_buff[25]={0};
    uint8_t *p;
    uint32_t tmp;
    int sat,prn;
    uint8_t type;
    uint8_t part1,part2,page1,page2;
    int i,j;

    p=(raw->buff)+6;
    prn=U1(p+8)-70;
    sat=satno(SYS_GAL,prn);
    if (sat == 0) return -1;

    if (raw->len<52) {
        trace(2,"SBF decode_galrawinav length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }

    if (U1(p+9)!=1) /* CRC test failed */
    {
        return -1;
    }

    /* copy data */
    for (i=0;i<8;i++)
    {
        tmp = U4(p+14+i*4);
        buff[4*i]=(tmp>>24) & 0xff;
        buff[4*i+1]=(tmp>>16) & 0xff;
        buff[4*i+2]=(tmp>>8) & 0xff;
        buff[4*i+3]=(tmp>>0) & 0xff;
    }

    part1=getbitu(buff,0,1);
    page1=getbitu(buff,1,1);
    part2=getbitu(buff,114,1);
    page2=getbitu(buff,115,1);

    /* skip alert page */
    if (page1==1||page2==1) return 0;

    /* test even-odd parts */
    if (part1!=0||part2!=1) {
        trace(2,"decode_galrawinav gal page even/odd error: sat=%2d\n",sat);
        return -1;
    }
    /* test crc */
    for (i=0,j=  4;i<49;i++,j+=4) setbitu(crc_buff,j,4,getbitu(buff,i*4,4));
    if (rtk_crc24q(crc_buff,25)!=getbitu(buff,196,24)) {
        trace(2,"decode_galrawinav gal page crc error: sat=%2d\n",sat);
        return -1;
    }

    if ((U4(p+14)&0x80)!=0x80) /* E5b-I */
    {
        int pos,i;
        type=getbitu(buff,2,6);

        if (type>6) return 0;

        pos=type*16;

        for (i=0,j=2;i<16;i++,j+=8) raw->subfrm[sat-1][pos++]=getbitu(buff,j,8);
    } else
    { /* E1-B */
        int pos,i;

        type=getbitu(buff,2,6);

        if (type>6) return 0;

        pos=type*16;

        for (i=0,j=2;i<16;i++,j+=8) raw->subfrm[sat-1][pos++]=getbitu(buff,j,8);
    };

    /* decode galileo inav ephemeris */
    if (!decode_gal_inav(raw->subfrm[sat-1],&eph)) {
        return 0; /* incomplete ephemeris */
    }
    /* test svid consistency */
    if (eph.sat!=sat) {
        trace(2,"decode_galrawinav gal svid error: sat=%2d %2d\n",sat,eph.sat);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode&& /* unchanged */
            timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0&&
            timediff(eph.toc,raw->nav.eph[sat-1].toc)==0.0) return 0;
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

/* decode SBF raw nav message (raw navigation data) for glonass---------------*/
static int decode_glorawcanav(raw_t *raw){

    uint8_t *p=(raw->buff)+6,*r,m;
    uint8_t buff[12];
    geph_t geph={0};
    int prn,sat;
    int j,k;

    if (raw->len<12)
    {
        trace(1,"SBF decode_glorawcanav: Block too short\n");
        return -1;
    }

    r=p+14;
    k=0;
    for (j=0;j<3;j++) {
        uint32_t d = U4(r+j*4);
        buff[k++]=(d>>24)&0xff;
        buff[k++]=(d>>16)&0xff;
        buff[k++]=(d>> 8)&0xff;
        buff[k++]=(d>> 0)&0xff;
    }
    /* get Glonass satellite number */
    prn=U1(p+8)-37;
    if (prn == 0) return -1;
    if (prn > 25) return -1;

    sat=satno(SYS_GLO,prn);
    if (sat == 0) return -1;

    if (raw->len<32) {
        trace(2,"SBF decode_gpsrawcanav block length error: len=%d\n",raw->len);
        return -1;
    }

    /* test hamming of glonass string */
    if (!test_glostr(buff)) {
        trace(2,"septentrio glo string hamming error: sat=%2d\n",sat);
        return -1;
    }
    m=getbitu(buff,1,4);
    if (m<1||15<m) {
        trace(2,"septentrio glo string no error: sat=%2d\n",sat);
        return -1;
    }

    memcpy(raw->subfrm[sat-1]+(m-1)*10,buff,10);

    if (m!=4) return 0;

    /* decode glonass ephemeris strings */
    geph.tof=gpst2time(U2(p+6),U4(p+2)/1000);
    if (!decode_glostr(raw->subfrm[sat-1],&geph)||geph.sat!=sat) return 0;
    geph.frq=U1(p+12)-8;

    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode) return 0;
    }

    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    raw->nav.glo_fcn[prn-1] = geph.frq + 8; /* savbe frequency number */

    return 2;
}

#if 0 /* UNUSED */

/* decode SBF raw nav message (raw navigation data) for COMPASS ---------*/
static int decode_cmpraw(raw_t *raw){
    eph_t eph={0};
    unsigned int words[10];
    uint8_t *p;
    int sat,prn;
    int i,id,pgn;

    p=(raw->buff)+6;
    prn=U1(p+8)-140;
    sat=satno(SYS_CMP,prn);
    if (sat == 0) return -1;

    if (raw->len<60) {
        trace(2,"SBF decode_cmprawinav length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }

    if (U1(p+9)!=1) /* CRC test failed */
    {
        return -1;
    }
    for (i=0;i<10;i++) words[i]=U4(p+12+i*4)&0x3FFFFFFF; /* 30 bits */

    satsys(sat,&prn);
    id=(words[0]>>12)&0x07; /* subframe id (3bit) */
    if (id<1||5<id) {
        trace(2,"SBF decode_cmprawinav length error: sat=%2d\n",sat);
        return -1;
    }
    if (prn>=5) { /* IGSO/MEO */

        for (i=0;i<10;i++) {
            setbitu(raw->subfrm[sat-1]+(id-1)*38,i*30,30,words[i]);
        }
        if (id!=3) return 0;

        /* decode beidou D1 ephemeris */
        if (!decode_bds_d1(raw->subfrm[sat-1],&eph)) return 0;
    }
    else { /* GEO */
        if (id!=1) return 0;

        /* subframe 1 */
        pgn=(words[1]>>14)&0x0F; /* page number (4bit) */
        if (pgn<1||10<pgn) {
            trace(2,"ubx rawsfrbx page number error: sat=%2d\n",sat);
            return -1;
        }
        for (i=0;i<10;i++) {
            setbitu(raw->subfrm[sat-1]+(pgn-1)*38,i*30,30,words[i]);
                }
                if (pgn!=10) return 0;

                /* decode beidou D2 ephemeris */
                if (!decode_bds_d2(raw->subfrm[sat-1],&eph)) return 0;
            }
            if (!strstr(raw->opt,"-EPHALL")) {
                if (timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0) return 0; /* unchanged */
            }
            eph.sat=sat;
            raw->nav.eph[sat-1]=eph;
            raw->ephsat=sat;
            return 2;
}

#endif /* UNUSED */

/* decode SBF gloutc --------------------------------------------------------*/
static int decode_gloutc(raw_t *raw)
{
    uint8_t *p=(raw->buff)+8;                 /* points at TOW location */

    trace(4,"SBF decode_gloutc: len=%d\n", raw->len);

    if (raw->len<40)
    {
        trace(1,"SBF decode_gloutc: Block too short\n");
        return -1;
    }

    /* GPS delta-UTC parameters */
    raw->nav.utc_glo[0] = R8(p + 16);                                 /*  tau_c */
    raw->nav.utc_glo[1] = U4(p + 24);                                 /*  B1 */
    raw->nav.utc_glo[2] = R4(p + 28);                                 /*  B2 */
    raw->nav.utc_glo[3] = R4(p + 12);                                 /*  tau_GPS */

    return 9;
}
/* decode SBF gpsion --------------------------------------------------------*/
static int decode_gpsion(raw_t *raw){
    uint8_t *p=(raw->buff)+8;            /* points at TOW location */

    trace(4,"SBF decode_gpsion: len=%d\n", raw->len);

    if (raw->len<48)
    {
        trace(1,"SBF decode_gpsion: Block too short\n");
        return -1;
    }

    raw->nav.ion_gps[0] = R4(p + 8);
    raw->nav.ion_gps[1] = R4(p + 12);
    raw->nav.ion_gps[2] = R4(p + 16);
    raw->nav.ion_gps[3] = R4(p + 20);
    raw->nav.ion_gps[4] = R4(p + 24);
    raw->nav.ion_gps[5] = R4(p + 28);
    raw->nav.ion_gps[6] = R4(p + 32);
    raw->nav.ion_gps[7] = R4(p + 36);

    return 9;
}
/* decode SBF galion --------------------------------------------------------*/
static int decode_galion(raw_t *raw){
    uint8_t *p=(raw->buff)+6;            /* points at TOW location */

    trace(4,"SBF decode_galion: len=%d\n", raw->len);

    if (raw->len<29)
    {
        trace(1,"SBF decode_galion: Block too short\n");
        return -1;
    }

    raw->nav.ion_gal[0] = R4(p + 10);
    raw->nav.ion_gal[1] = R4(p + 14);
    raw->nav.ion_gal[2] = R4(p + 18);
    raw->nav.ion_gal[3] = 0;

    return 9;
}

/* decode SBF gpsutc --------------------------------------------------------*/
static int decode_gpsutc(raw_t *raw)
{
    uint8_t *p=(raw->buff)+8;                 /* points at TOW location */

    trace(4,"SBF decode_gpsutc: len=%d\n", raw->len);

    if (raw->len<37)
    {
        trace(1,"SBF decode_gpsutc: Block too short\n");
        return -1;
    }

    /* GPS delta-UTC parameters */
    raw->nav.utc_gps[1] = R4(p + 8);                                  /*   A1 */
    raw->nav.utc_gps[0] = R8(p + 12);                                 /*   A0 */
    raw->nav.utc_gps[2] = U4(p + 20);                                 /*  tot */
    /* raw->nav.utc_gps[3] = U1(p + 24); */                           /*  WNt */
    raw->nav.utc_gps[3] = adjgpsweek(U2(p + 4));                      /*   WN */
    raw->nav.leaps      = I1(p + 25);                                 /* Dtls */

    /*NOTE. it is kind of strange that I have to use U1(p+4) and not U1(p+24)
            in fact if I take U1(p+24) I do not seem to ge the correct W in
            the header of RINEX nav file, line DELTA-UTC: A0,A1,T,W
    */
    return 9;
}
/* decode SBF gpsalm --------------------------------------------------------*/
static int decode_gpsalm(raw_t *raw)
{
    uint8_t *p=(raw->buff)+8;                 /* points at TOW location */
    alm_t alm;

    trace(4,"SBF decode_gpsalm: len=%d\n", raw->len);

    if (raw->len<60)
    {
        trace(1,"SBF decode_gpsalm: Block too short\n");
        return -1;
    }

    alm.sat =   satno(SYS_GPS,U1(p + 6));
    alm.e     = R4(p + 8);
    alm.toas  = U4(p + 12);
    alm.i0    = R4(p + 16);
    alm.OMGd  = R4(p + 20);
    alm.A     = pow(R4(p + 24),2);
    alm.OMG0  = R4(p + 28);
    alm.omg   = R4(p + 32);
    alm.M0    = R4(p + 36);
    alm.f1    = R4(p + 40);
    alm.f0    = R4(p + 44);
    alm.week  = U1(p + 48);
    alm.toa   = gpst2time(alm.week,alm.toas);
    alm.svconf= U1(p + 49);
    alm.svh   = U1(p + 50);

    if (alm.sat == 0) return -1;

    raw->nav.alm[alm.sat-1]=alm;

    return 9;
}
/* decode SBF galutc --------------------------------------------------------*/
static int decode_galutc(raw_t *raw)
{
    uint8_t *p=(raw->buff)+8;                 /* points at TOW location */

    trace(4,"SBF decode_galutc: len=%d\n", raw->len);

    if (raw->len<36)
    {
        trace(1,"SBF decode_galutc: Block too short\n");
        return -1;
    }

    /* GPS delta-UTC parameters */
    raw->nav.utc_gal[1] = R4(p + 8);                                  /*   A1 */
    raw->nav.utc_gal[0] = R8(p + 12);                                 /*   A0 */
    raw->nav.utc_gal[2] = U4(p + 20);                                 /*  tot */
    raw->nav.utc_gal[3] = adjgpsweek(U2(p + 4));                      /*   WN */
    raw->nav.leaps      = I1(p + 25);                                 /* Dtls */

    /*NOTE. it is kind of strange that I have to use U1(p+4) and not U1(p+24)
            in fact if I take U1(p+24) I do not seem to ge the correct W in
            the header of RINEX nav file, line DELTA-UTC: A0,A1,T,W
    */
    return 9;
}
/* decode SBF galalm --------------------------------------------------------*/
static int decode_galalm(raw_t *raw)
{
    uint8_t *p=(raw->buff)+8;                 /* points at TOW location */
    alm_t alm;

    trace(4,"SBF decode_galalm: len=%d\n", raw->len);

    if (raw->len<62)
    {
        trace(1,"SBF decode_galalm: Block too short\n");
        return -1;
    }

    alm.sat =   satno(SYS_GAL,U1(p + 49)-70);
    alm.e     = R4(p + 8);
    alm.toas  = U4(p + 12);
    alm.i0    = R4(p + 16) + 0.3;
    alm.OMGd  = R4(p + 20);
    alm.A     = pow(R4(p + 24),2);
    alm.OMG0  = R4(p + 28);
    alm.omg   = R4(p + 32);
    alm.M0    = R4(p + 36);
    alm.f1    = R4(p + 40);
    alm.f0    = R4(p + 44);
    alm.week  = U1(p + 48);
    alm.toa   = gpst2time(alm.week,alm.toas);
    alm.svconf= 0;
    alm.svh   = 0;

    if (alm.sat == 0) return -1;
    raw->nav.alm[alm.sat-1]=alm;

    return 9;
}

#if 0 /* UNUSED */

/* type 2-5,0: fast corrections ---------------------------------------*/
static int decode_sbsfast(raw_t *raw)
{
    int i,j;
    int prn,sat;
    uint8_t sbLength,sbCount,iodf,type;
    double prc_old,dt;
    gtime_t t0_old;
    uint32_t tow;
    uint16_t week;
    uint8_t *p=(raw->buff)+6;

    trace(4,"SBF decode_sbsfast: len=%d\n", raw->len);

    if (raw->len<20)
    {
        trace(1,"SBF decode_sbsfast: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 140) return -1;

    sat=satno(SYS_SBS,prn);
    if (sat == 0) return -1;

    tow=U4(p+2)/1000;
    week=U2(p+6);
    week=adjgpsweek(week);

    type=U1(p+9);
    iodf=U1(p+11);
    sbCount=U1(p+12);
    sbLength=U1(p+13);

    if (raw->nav.sbssat.iodp!=U1(p+10)) return 0;
    if (type>5) return -1;

    for (i=0;i<sbCount;i++) {
        if ((j=13*((type==0?2:type)-2)+i)>=raw->nav.sbssat.nsat) break;
        t0_old =raw->nav.sbssat.sat[j].fcorr.t0;
        prc_old =raw->nav.sbssat.sat[j].fcorr.prc;

        raw->nav.sbssat.sat[j].fcorr.t0=gpst2time(week,tow);
        raw->nav.sbssat.sat[j].fcorr.udre=U1(p+14+i*sbLength+1);
        raw->nav.sbssat.sat[j].fcorr.prc=R4(p+14+i*sbLength+4);

        dt=timediff(raw->nav.sbssat.sat[j].fcorr.t0,t0_old);
        if (t0_old.time==0||dt<=0.0||18.0<dt||raw->nav.sbssat.sat[j].fcorr.ai==0) {
            raw->nav.sbssat.sat[j].fcorr.rrc=0.0;
            raw->nav.sbssat.sat[j].fcorr.dt=0.0;
        }
        else {
            raw->nav.sbssat.sat[j].fcorr.rrc=(raw->nav.sbssat.sat[j].fcorr.prc-prc_old)/dt;
            raw->nav.sbssat.sat[j].fcorr.dt=dt;
        }
        raw->nav.sbssat.sat[j].fcorr.iodf=iodf;
    }
    trace(5,"SBF decode_sbsfast: type=%d iodf=%d\n",U1(p+9),iodf);
    return 0;
}

/* decode type 1: prn masks --------------------------------------------------*/
static int decode_sbsprnmask(raw_t *raw)
{
    int i,n,sat,prn;
    uint8_t *p=(raw->buff)+6;


    trace(4,"SBF decode_sbsprnmask:\n");

    if (raw->len<18)
    {
        trace(1,"SBF decode_sbsprnmask: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    raw->nav.sbssat.nsat=U1(p+10);

    for (n=0;n<raw->nav.sbssat.nsat&&n<MAXSAT;n++) {
       i=U1(p+11+n);
       if      (i<= 37) sat=satno(SYS_GPS,i);    /*   0- 37: gps */
       else if (i<= 61) sat=satno(SYS_GLO,i-37); /*  38- 61: glonass */
       else if (i<=119) sat=0;                   /*  62-119: future gnss */
       else if (i<=138) sat=satno(SYS_SBS,i);    /* 120-138: geo/waas */
       else if (i<=182) sat=0;                   /* 139-182: reserved */
       else if (i<=192) sat=satno(SYS_SBS,i+10); /* 183-192: qzss ref [2] */
       else if (i<=202) sat=satno(SYS_QZS,i);    /* 193-202: qzss ref [2] */
       else             sat=0;                   /* 203-   : reserved */
       raw->nav.sbssat.sat[n].sat=sat;
    }
    raw->nav.sbssat.iodp=U1(p+9);

    trace(5,"SBF decode_sbsprnmask: nprn=%d iodp=%d\n",n,raw->nav.sbssat.iodp);
    return 0;
}

/* decode type 6: integrity info ---------------------------------------------*/
static int decode_sbsintegriy(raw_t *raw)
{
    int i,prn;
    uint8_t *p=(raw->buff)+6;
    uint8_t iodf[4],udre;


    trace(4,"decode_sbsintegriy:\n");

    if (raw->len<71)
    {
        trace(1,"SBF decode_sbsintegriy: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    for (i=0;i<4;i++) {
        iodf[i]=U1(p+10+i);
    }
    for (i=0;i<raw->nav.sbssat.nsat&&i<MAXSAT;i++) {
        if (raw->nav.sbssat.sat[i].fcorr.iodf!=iodf[i/13]) continue;
        udre=U1(p+14+i);
        raw->nav.sbssat.sat[i].fcorr.udre=udre;
    }
    trace(5,"SBF decode_sbsintegriy: iodf=%d %d %d %d\n",iodf[0],iodf[1],iodf[2],iodf[3]);
    return 0;
}

/* decode type 7: fast correction degradation factor -------------------------*/
static int decode_sbsfastcorrdegr(raw_t *raw)
{
    int i,prn;
    uint8_t *p=(raw->buff)+6;


    trace(4,"SBF decode_sbsfastcorrdegr:\n");

    if (raw->len<68)
    {
        trace(1,"SBF decode_sbsfastcorrdegr: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    if (raw->nav.sbssat.iodp!=U1(p+9)) return 0;

    raw->nav.sbssat.tlat=U1(p+10);

    for (i=0;i<raw->nav.sbssat.nsat&&i<MAXSAT;i++) {
        raw->nav.sbssat.sat[i].fcorr.ai=U1(p+11+i);
    }
    return 0;
}

/* decode type 26: ionospheric delay corrections -----------------------------*/
static int decode_sbsionodelay(raw_t *raw)
{
    int i,j,give,prn;
    int band;
    uint8_t *p=(raw->buff)+6, sbLength, count;
    uint16_t week;
    uint32_t tow;

    trace(4,"SBF decode_sbsionodelay:\n");

    if (raw->len<20)
    {
        trace(1,"SBF decode_sbsionodelay: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    band=U1(p+9);

    if (band>MAXBAND||raw->nav.sbsion[band].iodi!=U1(p+10)) return 0;

    tow=U4(p+2)/1000;
    week=U2(p+6);
    week=adjgpsweek(week);

    sbLength=U1(p+12);
    count=U1(p+11);

    if (count!=15)
    {
        trace(1,"SBF decode_sbsionodelay: wrong number of IDC blocks: %d\n",count);
        return -1;
    }

    for (i=0;i<count;i++) {
        j=U1(p+14+i*sbLength);
        give=U1(p+14+i*sbLength+1);

        raw->nav.sbsion[band].igp[j].t0=gpst2time(week,tow);
        raw->nav.sbsion[band].igp[j].delay=R4(p+14+i*sbLength+4);
        raw->nav.sbsion[band].igp[j].give=give;

        if (raw->nav.sbsion[band].igp[j].give>=16) {
            raw->nav.sbsion[band].igp[j].give=0;
        }
    }
    trace(5,"decode_sbsionodelay: band=%d\n",band);
    return 0;
}

/* decode type 18: ionospheric grid point masks ------------------------------*/
static int decode_sbsigpmask(raw_t *raw) /* TODO: verify this function */
{
    const sbsigpband_t *b;
    int i,j,n,m,prn;
    uint8_t band;

    uint8_t *p=(raw->buff)+6;

    trace(4,"SBF decode_sbsigpmask:\n");

    if (raw->len<20)
    {
        trace(1,"SBF decode_sbsigpmask: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    band=U1(p+10);

    if      (band<= 8) {b=igpband1[band  ]; m=8;}
    else if (9<=band&&band<=10) {b=igpband2[band-9]; m=5;}
    else return 0;

    raw->nav.sbsion[band].iodi=U1(p+11);
    raw->nav.sbsion[band].nigp=U1(p+12);

    for (n=0;n<raw->nav.sbsion[band].nigp;n++)
    {
        i=U1(p+13+n);
        for (j=0;j<m;j++) {
            if (i<b[j].bits||b[j].bite<i) continue;
            raw->nav.sbsion[band].igp[n].lat=band<=8?b[j].y[i-b[j].bits]:b[j].x;
            raw->nav.sbsion[band].igp[n++].lon=band<=8?b[j].x:b[j].y[i-b[j].bits];
            break;
        }
    }

    trace(5,"decode_sbsigpmask: band=%d nigp=%d\n",band,n);
    return 0;
}
/* decode long term correction ------------------------------------------*/
static int decode_sbslongcorrh(raw_t* raw)
{
    int prn,i;
    uint8_t *p=(raw->buff)+6;
    uint8_t count,sbLength,no;
    uint32_t tow;
    uint16_t week;
    int64_t t;

    trace(4,"SBF decode_sbslongcorrh:\n");

    if (raw->len<20)
    {
        trace(1,"SBF decode_sbslongcorrh: Block too short\n");
        return -1;
    }

    /* get satellite number */
    prn=U1(p+8);
    if (prn < 120) return -1;
    if (prn > 139) return -1;

    tow=U4(p+2)/1000;
    week=U2(p+6);
    week=adjgpsweek(week);

    count=U1(p+9);
    sbLength=U1(p+10);

    if (count >4) return -1;

    for (i=0;i<count;i++)
    {
        no=U1(p+14+i*sbLength+1);
        raw->nav.sbssat.sat[no-1].lcorr.iode=U1(p+14+i*sbLength+3);
        raw->nav.sbssat.sat[no-1].lcorr.dpos[0]=R4(p+14+i*sbLength+ 4);
        raw->nav.sbssat.sat[no-1].lcorr.dpos[1]=R4(p+14+i*sbLength+ 8);
        raw->nav.sbssat.sat[no-1].lcorr.dpos[2]=R4(p+14+i*sbLength+12);
        if (U1(p+14+i*sbLength)==1)
        {
            raw->nav.sbssat.sat[no-1].lcorr.dvel[i]=R4(p+14+i*sbLength+16);
            raw->nav.sbssat.sat[no-1].lcorr.dvel[i]=R4(p+14+i*sbLength+20);
            raw->nav.sbssat.sat[no-1].lcorr.dvel[i]=R4(p+14+i*sbLength+24);

            raw->nav.sbssat.sat[no-1].lcorr.daf1=R4(p+14+i*sbLength+32);
        } else
        {
            raw->nav.sbssat.sat[no-1].lcorr.dvel[0]=raw->nav.sbssat.sat[no-1].lcorr.dvel[1]=raw->nav.sbssat.sat[no-1].lcorr.dvel[2]=0.0;
            raw->nav.sbssat.sat[no-1].lcorr.daf1=0;
        };
        raw->nav.sbssat.sat[no-1].lcorr.daf0=R4(p+14+i*sbLength+28);

        t=(int)U4(p+14+i*sbLength+32)-(int)tow%86400;
        if      (t<=-43200) t+=86400;
        else if (t>  43200) t-=86400;
        raw->nav.sbssat.sat[no-1].lcorr.t0=gpst2time(week,tow+t);
    };

    return 0;
}

#endif /* UNUSED */

/* decode SBF raw message --------------------------------------------------*/
static int decode_sbf(raw_t *raw)
{
    unsigned short crc;

    /* read the SBF block ID and revision */
    int type = U2(raw->buff+4) & 0x1fff << 0;
    int revision = U2(raw->buff+4) >> 13;
    (void)revision;

    trace(3,"decode_sbf: type=%04x len=%d\n",type,raw->len);

    /* read the SBF block CRC */
    crc = U2(raw->buff+2);

    /* checksum skipping first 4 bytes */
    if (sbf_checksum(raw->buff+4, raw->len-4) !=  crc){
        trace(2,"sbf checksum error: type=%04x len=%d\n",type, raw->len);
        return -1;
    }

    if (raw->outtype) {
        sprintf(raw->msgtype,"SBF 0x%04X (%4d):",type, raw->len);
    }

    switch (type) {
        case ID_MEASEPOCH:      return decode_measepoch(raw);

        case ID_GPSNAV:         return decode_gpsnav(raw);
        case ID_GPSION:         return decode_gpsion(raw);
        case ID_GPSUTC:         return decode_gpsutc(raw);
        case ID_GPSALM:         return decode_gpsalm(raw);
        case ID_GPSRAWCA:
        case ID_GPSRAWL2C:
        case ID_GPSRAWL5:       return decode_rawnav(raw,SYS_GPS);

        case ID_GEONAV:         return decode_sbasnav(raw);
        case ID_GEORAWL1:
        case ID_GEORAWL5:       return decode_georaw(raw);

#ifdef ENAGLO
        case ID_GLONAV:         return decode_glonav(raw);
        case ID_GLORAWCA:       return decode_glorawcanav(raw);
        case ID_GLOTIME:        return decode_gloutc(raw);
#endif

#ifdef ENAGAL
        case ID_GALNAV:         return decode_galnav(raw);
        case ID_GALION:         return decode_galion(raw);
        case ID_GALUTC:         return decode_galutc(raw);
        case ID_GALALM:         return decode_galalm(raw);
        case ID_GALRAWINAV:     return decode_galrawinav(raw);
#endif

#ifdef TESTING /* not tested */
#ifdef ENAQZS
        case ID_QZSSL1CA:
        case ID_QZSSL2C:
        case ID_QZSSL5:         return decode_rawnav(raw, SYS_QZS);
        case ID_QZSS_NAV:       return decode_qzssnav(raw);
#endif

#ifdef ENACMP
        case ID_CMPRAW:         return decode_cmpraw(raw);
        case ID_CMPNAV:         return decode_cmpnav(raw);
#endif
#endif

#if 0 /* UNUSED */
        case ID_GEOMT00:        return decode_sbsfast(raw);
        case ID_GEOPRNMASK:     return decode_sbsprnmask(raw);
        case ID_GEOFASTCORR:    return decode_sbsfast(raw);
        case ID_GEOINTEGRITY:   return decode_sbsintegriy(raw);
        case ID_GEOFASTCORRDEGR:return decode_sbsfastcorrdegr(raw);
        case ID_GEOIGPMASK:     return decode_sbsigpmask(raw);
        case ID_GEOLONGTERMCOR: return decode_sbslongcorrh(raw);
        case ID_GEOIONODELAY:   return decode_sbsionodelay(raw);
#endif /* UNUSED */

#if 0 /* UNUSED */
        case ID_GALRAWFNAV:     return decode_galrawfnav(raw); /* not yet supported in RTKLIB */
        case ID_GLOALM:         return decode_glosalm(raw); /* not yet supported in RTKLIB */

        case ID_PVTGEOD:        return decode_pvtgeod(raw);
        case ID_RXSETUP:        return decode_rxsetup(raw);
        case ID_COMMENT:        return decode_comment(raw);
#endif /* UNUSED */
        default:
            trace(3,"decode_sbf: unused frame type=%04x len=%d\n",type,raw->len);
        /* there are many more SBF blocks to be extracted */
    }
    return 0;
}

/* sync to the beginning of a block ------------------------------------------*/
static int sync_sbf(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]== SBF_SYNC1 && buff[1]==SBF_SYNC2;
}
/* input sbf raw data from stream ----------------------------------------------
* get to the next sbf raw block from stream
* args   : raw_t  *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*-----------------------------------------------------------------------------*/
extern int input_sbf(raw_t *raw, unsigned char data)
{
    trace(5,"input_sbf: data=%02x\n",data);

    if (raw->nbyte==0) {
        if (sync_sbf(raw->buff,data)) raw->nbyte=2;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;

    if (raw->nbyte<8) return 0;

    if ((raw->len=U2(raw->buff+6))>MAXRAWLEN) {
        trace(2,"sbf length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<raw->len) return 0;
    raw->nbyte=0;

    return decode_sbf(raw);
}
/* sbf raw block finder --------------------------------------------------------
* get to the next sbf raw block from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_sbff(raw_t *raw, FILE *fp)
{
    int i,data;

    trace(4,"input_sbff:\n");

    /* go to the beginning of the first block */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_sbf(raw->buff,(unsigned char)data)) break;
            if (i>=MAXRAWLEN) return 0;
        }
    }

    /* load block header content (8 bytes) in raw->buff */
    /* since we already read the first two, we just read the next 6 bytes */
    if (fread(raw->buff+2,1,6,fp)<6) return -2;
    raw->nbyte=8;

    /* decode the length of the block and store it in len*/
    if ((raw->len=U2(raw->buff+6))>MAXRAWLEN) {
        trace(2,"sbf length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }

    /* let's store in raw->buff the whole block of length len */
    /* 8 bytes have been already read, we read raw->len-8 more */
    if (fread(raw->buff+8,1,raw->len-8,fp)<(size_t)(raw->len-8)) return -2;
    raw->nbyte=0;           /* this indicates where we point inside raw->buff */

    /* decode SBF block */
    return decode_sbf(raw);
}

