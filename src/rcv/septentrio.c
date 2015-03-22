/*------------------------------------------------------------------------------
* septentrio.c : Septentrio Binary Format decoder (All Septentrio receivers)
*
*          Copyright (C) 2013 by Fabrizio Tappero.
*          Copyright (C) 2015 by Jens Reimann
*
* reference :
*     [1] Septentrio, SBF Reference Guide, Version 130722r38600, 07/2013
*
* version : $Revision: 1.1 $ $Date: 2013/07/17 15:05:00 $
*
* history : 2013/07/17  1.0  begin writing
*           2013/10/24  1.1  GPS L1 working
*           2013/11/02  1.2  modified by TTAKASU
*           2015/01/26  1.3  fix some problems by Jens Reimann
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id: Septentrio SBF,v 1.1 2013/10/23 FT $";

#define SBF_SYNC1   0x24        /* SBF message header sync field 1 (correspond to $) */
#define SBF_SYNC2   0x40        /* SBF message header sync field 2 (correspont to @)*/

#define ID_MEASEPOCH       4027 /* SBF message id: range measurememts */
#define ID_MEASEPOCHEXTRA  4000 /* SBF message id: range measurememts extra info */
#define ID_MEASEPOCH_END   5922 /* SBF message id: end of SBF range measurememts */

#define ID_GPSRAWCA     4017    /* SBF message id: GPS raw navigation page or frame */
#define ID_GPSRAWL2C    4018    /* SBF message id: GPS raw navigation page or frame */
#define ID_GPSRAWL5     4019    /* SBF message id: GPS raw navigation page or frame */
#define ID_GLORAWCA     4026    /* SBF message id: GLONASS raw navigation page or frame */
#define ID_GALRAWFNAV   4022    /* SBF message id: Galileo raw navigation page or frame */
#define ID_GALRAWINAV   4023    /* SBF message id: Galileo raw navigation page or frame */
#define ID_GEORAWL1     4020    /* SBF message id: SBAS raw navigation page or frame */
#define ID_GEORAWL5     4021    /* SBF message id: SBAS raw navigation page or frame */
#define ID_COMPRAW      4047    /* SBF message id: Compass raw navigation page or frame */
#define ID_QZSSL1CA     4066    /* SBF message id: QZSS raw navigation page or frame */
#define ID_QZSSL2C      4067    /* SBF message id: QZSS raw navigation page or frame */
#define ID_QZSSL5       4068    /* SBF message id: QZSS raw navigation page or frame */

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

#define ID_GALGSTGPS  4032      /* SBF message id: Galileo GPS time offset */

#define ID_GEOMTOO    5925      /* SBF message id: empty SBAS message */
#define ID_GEONAV     5896      /* SBF message id: SBAS navigation data */

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
static int getSigFreq(int _signType);
static int compTwoConv(unsigned int byte);

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}
static signed int     I4(unsigned char *p) {signed int     u; memcpy(&u,p,4); return u;}
static short          I2(unsigned char *p) {short          i; memcpy(&i,p,2); return i;}

/* set fields (little-endian) ------------------------------------------------*/
static void setU1(unsigned char *p, unsigned char  u) {*p=u;}
static void setU2(unsigned char *p, unsigned short u) {memcpy(p,&u,2);}
static void setU4(unsigned char *p, unsigned int   u) {memcpy(p,&u,4);}
static void setI1(unsigned char *p, char           i) {*p=(unsigned char)i;}
static void setI2(unsigned char *p, short          i) {memcpy(p,&i,2);}
static void setI4(unsigned char *p, int            i) {memcpy(p,&i,4);}
static void setR4(unsigned char *p, float          r) {memcpy(p,&r,4);}
static void setR8(unsigned char *p, double         r) {memcpy(p,&r,8);}

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
static unsigned short checksum(unsigned char *buff, int len)
{
    int i;
    unsigned short crc = 0;
    for (i=0; i<len; i++) {
        crc = (crc << 8) ^ CRC_16CCIT_LookUp[ (crc >> 8) ^ buff[i] ];
    }
    return crc;
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

/* check code priority and return obs position -------------------------------*/
static int checkpri(const char *opt, int sys, int code, int freq)
{
    int nex=NEXOBS; /* number of extended obs data */

    if (sys==SYS_GPS) {
        if (strstr(opt,"-GL1P")) return code==CODE_L1P?0:-1;
        if (strstr(opt,"-GL2X")) return code==CODE_L2X?1:-1;
        if (code==CODE_L1P) return nex<1?-1:NFREQ;
        if (code==CODE_L2X) return nex<2?-1:NFREQ+1;
    }
    else if (sys==SYS_GLO) {
        if (strstr(opt,"-RL2C")) return code==CODE_L2C?1:-1;
        if (code==CODE_L2C) return nex<1?-1:NFREQ;
    }
    else if (sys==SYS_GAL) {
        if (strstr(opt,"-EL1B")) return code==CODE_L1B?0:-1;
        if (code==CODE_L1B) return nex<1?-1:NFREQ;
        if (code==CODE_L7Q) return nex<2?-1:NFREQ+1;
        if (code==CODE_L8Q) return nex<3?-1:NFREQ+2;
    }
    return freq<NFREQ?freq:-1;
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
    double tow,tt,psr, adr, tadj=0.0,toff=0.0,tn,CurrentTime, dopplerType1;
    double SNR_DBHZ, SNR2_DBHZ, freqType1, freqType2, alpha;
    int i,ii,j,prn,sat,n=0,nsat,week, lli, code, freq, pos, index, dTowInt;
    unsigned char *p=(raw->buff)+8;                   /* jump to TOW location */
    char *q,tstr[32];
    int SB1length,SB2length,CommonFlags,ReservedBits1,ReservedBits2;
    unsigned char signType1, antID, signType2, antID2;
    unsigned int codeLSB, SB2Num, SB1Num, BlockLength, sys, h;
    unsigned char codeMSB, rxch;
    static double lastTow=0./0.;

    /* signals for type2 sub-block */
    int CodeOffsetMSB, DopplerOffsetMSB;
    unsigned int CarrierMSB, ObsInfo2, CodeOffsetLSB, CarrierLSB;
    unsigned int DopplerOffsetLSB;
    double PRtype2, Ltype2, dopplerType2;

    unsigned int LockTime, LockTime2;
    unsigned char ObsInfo;
    double SB1_WaveLength, SB1_Code = 0.0;
    int SB1_FreqNr;

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

    /* Type1 sub-block length */
    BlockLength = U2(p-2);

    /* number of type1 sub-blocks also equal to number of satellites */
    SB1Num = U1(p+6);
    nsat   = U1(p+6);

    /* additional block information */
    SB1length=U1(p+7);                              /* Type1 sub-block length */
    SB2length=U1(p+8);                              /* Type2 sub-block length */
    CommonFlags=U1(p+9);
    ReservedBits1=U1(p+10);
    ReservedBits2=U1(p+11);

    /* set the pointer from TOW to the beginning of type1 sub-block */
    p = p + 12;

    for (i=0;i<nsat&&i<MAXOBS;i++) {

        /* decode type1 sub-block */
        rxch =U1(p);                              /* used receiver channel    */
        signType1 = U1(p+1) & 0x1f;               /* type of signal, bit[0-4] */
        antID = (U1(p+1) >> 5) & 0x07  ;          /* antenna ID. bit[5-7]     */
        prn = U1(p+2);                            /* satellite number         */
        raw->obs.data[n].time  = time;            /* not sure what ref. it is */
        codeMSB = (U1(p+3) & 0x0f);               /* code phase MSB. bit[0-3] */
        codeLSB = U4(p+4);                        /* code phase LSB           */

        /* code pseudorange in m */
        psr = (codeMSB*4294967296.296+codeLSB)*0.001;

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
        if ( (signType1 > 7) && (signType1 < 16) ) {
          SB1_FreqNr = ((ObsInfo >> 3) & 0x1f) - 8;
        } else {
          SB1_FreqNr = ((ObsInfo >> 3) & 0x1f);
        }

        /* Compute wavelength according to the used frequency */
        if ( (signType1 == 0) ||                                   /* GPSL1CA */
             (signType1 == 1) ||                                   /* GPSL1PY */
             (signType1 == 16) ||                                  /* GALL1A  */
             (signType1 == 17) ||                                  /* GALL1BC */
             (signType1 == 24) ) {                                 /* GEOL1   */
          SB1_WaveLength = 299792458 / 1575.42e6;
        } else if ( (signType1 == 8) ||                            /* GLOL1CA */
                    (signType1 == 9) ) {                           /* GLOL1P  */
          SB1_WaveLength=(299792458/(1602.00e6+(562.50e3*SB1_FreqNr)));
        } else if ( (signType1 == 2) ||                            /* GPSL2PY */
                    (signType1 == 3) ) {                           /* GPSL2C  */
          SB1_WaveLength = 299792458 / 1227.60e6;
        } else if ( (signType1 == 10) ||                           /* GLOL2P  */
                    (signType1 == 11) ) {                          /* GLOL2CA */
          SB1_WaveLength=
                  (9.0/7.0)*(299792458/(1602.00e6+(562.50e3*SB1_FreqNr)));
        } else if ( (signType1 == 4) ||                            /* GPSL5   */
                    (signType1 == 20) ||                           /* GALE5a  */
                    (signType1 == 25) ) {                          /* GEOL5   */
          SB1_WaveLength = 299792458 / 1176.45e6;
        } else if ( (signType1 == 21) ) {                          /* GALE5b  */
          SB1_WaveLength = 299792458 / 1207.14e6;
        } else if ( (signType1 == 22) ) {                          /* GALE5   */
          SB1_WaveLength = 299792458 / 1191.795e6;
        } else if ( (signType1 == 18) ||                           /* GALE6A  */
                    (signType1 == 19) ) {                          /* GALE6BC */
          SB1_WaveLength = 299792458 / 1278.75e6;
        } else {
          SB1_WaveLength = 0.0;
        }

        /* final carrier phase calculation */
        adr = (SB1_Code/SB1_WaveLength)+(I1(p+14)*65.536)+(U2(p+12)*0.001);
        if ((I2(p+14)==-128)&&(U2(p+12)==0)) {
            adr=0;
        }
        /* debug
        trace(1,"signal type = %2d, \n",signType1);
        */

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
        else if ((prn>=71)&&(prn<=102)){
            sys = SYS_GAL;                      /* navigation system: Galileo */
            sat = prn - 70;}
        else if ((prn>=120)&&(prn<=140)){
            sys = SYS_SBS;                      /* navigation system: SBAS    */
            sat = prn;}
        else if ((prn>=141)&&(prn<=172)){
            sys = SYS_CMP;                      /* navigation system: BeiDou  */
            sat = prn - 140;}
        else if ((prn>=181)&&(prn<=187)){
            sys = SYS_QZS;                      /* navigation system: QZSS    */
            sat = prn - 180;}
        else{
            sys = SYS_NONE;                     /* navigation system: none    */
            sat = 0;}

        /* store satellite number */
        sat = satno(sys, sat);
        raw->obs.data[n].sat=sat;

        /* lock to signal indication */
        tt=LockTime;
        if (raw->obs.data[n].LLI[0]&1) raw->lockt[sat-1][0]=0.0;
        else if (tt<0.0||10.0<tt) raw->lockt[sat-1][0]=0.0;
        else raw->lockt[sat-1][0]+=tt;

        /* Lock time */
        raw->lockt[sat-1][0]= LockTime;

        /* start new observation period */
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }

        /* store type1 signal information is RTKLIB signal structure
         * RTKLIB signal structure is:
         *  obs.P[i], L[i], D[i], SNR[i], code[i], LLI[i]
         *
         *  i=0: L1
         *  i=1: L2
         *  i=2: L5/L3
         *  i=3: L6
         *  i=4: L7
         *  i=5: L8
         *  i=NFREQ: additional signal L1
         *  i=NFREQ+1: additional signal L2
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
        freqType1 = getSigFreq(signType1);

        if      (freqType1 == FREQ1) h = 0;
        else if (freqType1 == FREQ2) h = 1;
        else if (freqType1 == FREQ5) h = 2;
        else if (freqType1 == FREQ6) h = 3;
        else if (freqType1 == FREQ7) h = 4;
        else if (freqType1 == FREQ8) h = 5;
        else                         h = 0;

        /* store signal info */
        if ((h<=NFREQ+NEXOBS)&&(adr!=0)&&(psr!=0)){
            raw->obs.data[n].L[h]    = adr;
            raw->obs.data[n].P[h]    = psr;
            raw->obs.data[n].D[h]    = (float)dopplerType1; /* NEGATIVE??*/
            raw->obs.data[n].SNR[h]  = (unsigned char)(SNR_DBHZ*4.0+0.5);
            raw->obs.data[n].LLI[h]  = (unsigned char)0;
            raw->obs.data[n].code[h] = code;
        }

        /* decode all Type2 sub-blocks (if there is any) */
        p = p + SB1length;                 /* get to the begin of Type2 block */

        for (ii=0;ii<(int)SB2Num;ii++)
        {
            signType2 = U1(p) & 0x1f;             /* type of signal, bit[0-4] */
            antID2 = (U1(p) >> 5) & 0x07;         /* antenna ID. bit[5-7]     */

            /* Duration of continuous carrier phase */
            LockTime2 = U1(p+1);

            /* Signal to noise ratio in dbHz */
            if ((signType2==1) || (signType2==2)){
                SNR2_DBHZ=((double)U1(p+2))*0.25;
            }
            else SNR2_DBHZ=(((double)U1(p+2))*0.25)+10;

            CodeOffsetMSB    = compTwoConv(U1(p+3)  & 0x003);     /* bit[0-2] */
            DopplerOffsetMSB = compTwoConv(U1(p+3) >> 3);         /* bit[3-7] */
            CarrierMSB = I1(p+4);

            ObsInfo2 = U1(p+5);                         /* minor informations */

            /* pseudrange in meters */
            CodeOffsetLSB = U2(p+6);
            PRtype2 = psr + (CodeOffsetMSB*65536+CodeOffsetLSB)*0.001;
            if ((CodeOffsetMSB==-4)&&(CodeOffsetLSB==0)) {
                PRtype2=0;
            }

            /* carrier phase in cycles */
            CarrierLSB = U2(p+8);
            Ltype2=(psr/SB1_WaveLength)+(CarrierMSB*65536+CarrierLSB)*0.001;
            if ((CarrierMSB==-128)&&(CarrierLSB==0)) {
                Ltype2=0;
            }

            /* Doppler in Hz */
            DopplerOffsetLSB = U2(p+10);
            freqType1 = getSigFreq(signType1);
            freqType2 = getSigFreq(signType2);
            alpha = pow((freqType1/freqType2),2);
            dopplerType2 = dopplerType1*alpha +\
                    (DopplerOffsetMSB*65536+DopplerOffsetLSB)*1E-4;

            /* store Type2 signal info in rtklib structure */

            if      (freqType2 == FREQ1) h = 0;
            else if (freqType2 == FREQ2) h = 1;
            else if (freqType2 == FREQ5) h = 2;
            else if (freqType2 == FREQ6) h = 3;
            else if (freqType2 == FREQ7) h = 4;
            else if (freqType2 == FREQ8) h = 5;
            else                         h = 0;

            /* store signal info */
            if (h<=NFREQ+NEXOBS && h>0 && Ltype2!=0 && PRtype2!=0){ /* DO NOT REWRITE CHANNEL h=0 */

                raw->obs.data[n].L[h]    = Ltype2;
                raw->obs.data[n].P[h]    = PRtype2;
                raw->obs.data[n].D[h]    = (float)dopplerType2; /* NEGATIVE??*/
                raw->obs.data[n].SNR[h]  = (unsigned char)(SNR2_DBHZ*4.0+0.5);
                raw->obs.data[n].LLI[h]  = (unsigned char)0;
                raw->obs.data[n].code[h] = getSignalCode(signType2);
                raw->lockt[sat][h]       = (unsigned char)LockTime2;

                /* debug */
                printf("sigType=%2d, sat= %2d \n",
                       getSignalCode(signType2),raw->obs.data[n].sat );
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

/* complement two converter --------------------------------------------------*/
static int compTwoConv(unsigned int byte) {
    byte &= 255;
    if (byte>127) return byte-256;
    else          return byte;
}

/* return frequency value in Hz from signal type name ------------------------*/
static int getSigFreq(int _signType){

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
    case 8:                                                        /* GLOL1CA */
        return FREQ1;
    case 9:                                                        /* GLOL1P  */
        return FREQ1;
    case 10:                                                       /* GLOL2P  */
        return FREQ2;
    case 11:                                                       /* GLOL2CA */
        return FREQ2;
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
        return FREQ5;
    case 22:                                                       /* GALE5   */
        return FREQ5;
    case 24:                                                       /* GEOL1   */
        return FREQ1;
    }
    return FREQ1;
}

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

/* return the Septentrio signal type -----------------------------------------*/
static int getSignalCode(int signType){
    int _freq, _code;

    switch (signType)
    {
    case 0:                                                        /* GPSL1CA */
        _code=CODE_L1C;
        _freq=0;
        break;
    case 1:                                                        /* GPSL1PY */
        _code=CODE_L1W;
        _freq=0;
        break;
    case 2:                                                        /* GPSL2PY */
        _code=CODE_L2W;
        _freq=1;
        break;
    case 3:                                                        /* GPSL2C  */
        _code=CODE_L2L;
        _freq=1;
        break;
    case 4:                                                        /* GPSL5   */
        _code=CODE_L5Q;
        _freq=2;
        break;
    case 8:                                                        /* GLOL1CA */
        _code=CODE_L1C;
        _freq=0;
        break;
    case 9:                                                        /* GLOL1P  */
        _code=CODE_L1P;
        _freq=0;
        break;
    case 10:                                                       /* GLOL2P  */
        _code=CODE_L2P;
        _freq=1;
        break;
    case 11:                                                       /* GLOL2CA */
        _code=CODE_L2C;
        _freq=1;
        break;
    case 16:                                                       /* GALE1A  */
        _code=CODE_L1A;
        _freq=0;
        break;
    case 17:                                                       /* GALE1BC */
        _code=CODE_L1C;
        _freq=0;
        break;
    case 18:                                                       /* GALE6A  */
        _code=CODE_L6A;
        _freq=3;
        break;
    case 19:                                                       /* GALE6BC */
        _code=CODE_L6X;
        _freq=3;
        break;
    case 20:                                                       /* GALE5a  */
        _code=CODE_L5Q;
        _freq=4;
        break;
    case 21:                                                       /* GALE5b  */
        _code=CODE_L7Q;
        _freq=4;
        break;
    case 22:                                                       /* GALE5   */
        _code=CODE_L8Q;
        _freq=4;
        break;
    case 24:                                                       /* GEOL1   */
        _code=CODE_L1C;
        _freq=0;
        break;
    default:                                                       /* GPSL1CA */
        _code=CODE_L1C;
        _freq=0;
        break;
    }
    return _code;
}

/* decode SBF nav message for GPS (navigation data) --------------------------*/
static int decode_gpsnav(raw_t *raw){

    unsigned char *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    unsigned short prn, sat;
    int week;

    trace(4,"SBF decode_gpsnav: len=%d\n",raw->len);

    if ((raw->len)<120) {
        trace(2,"SBF decode_gpsnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8);
    sat = prn;

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
    eph.iode   = U1(puiTmp +  18); /* U1(puiTmp +19); strange... */
    eph.code   = U1(puiTmp +  12);
    eph.flag   = U1(puiTmp +  15);
    week       = U2(puiTmp +  10); /* WN */
    eph.fit    = 0;

    if (week>=4096) {
        trace(2,"SBF gps ephemeris week error: sat=%2d week=%d\n",sat,week);
        return -1;
    }

    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;

/*  if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    } */

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode SBF nav message for Galileo (navigation data) --------------------------*/
static int decode_galnav(raw_t *raw){

    unsigned char *puiTmp = (raw->buff)+6;
    eph_t eph={0};
    double toc;
    unsigned short prn, sat;
    int week;

    trace(4,"SBF decode_galnav: len=%d\n",raw->len);

    if ((raw->len)<152) {
        trace(2,"SBF decode_galnav frame length error: len=%d\n",raw->len);
        return -1;
    }

    prn = U1(puiTmp+8)-70;
    sat = satno(SYS_GAL,prn);

    if (!((prn>=1)&&(prn<=36))){
        trace(2,"SBF decode_galnav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.crs    = R4(puiTmp +  82);
    eph.deln   = R4(puiTmp +  66) * PI;
    eph.M0     = R8(puiTmp +  18) * PI;
    eph.cuc    = R4(puiTmp +  70);
    eph.e      = R8(puiTmp +  26);
    eph.cus    = R4(puiTmp +  74);
    eph.A      = pow(R8(puiTmp +  10), 2);
    eph.toes   = U4(puiTmp +  94);
    eph.cic    = R4(puiTmp +  86);
    eph.OMG0   = R8(puiTmp +  50) * PI;
    eph.cis    = R4(puiTmp +  90);
    eph.i0     = R8(puiTmp +  34) * PI;
    eph.crc    = R4(puiTmp +  78);
    eph.omg    = R8(puiTmp +  42) * PI;
    eph.OMGd   = R4(puiTmp +  58) * PI;
    eph.idot   = R4(puiTmp +  62) * PI;
    eph.tgd[0] = R4(puiTmp + 122);
    eph.tgd[1] = R4(puiTmp + 126);
    toc        = U4(puiTmp +  98);
    eph.f2     = R4(puiTmp + 102);
    eph.f1     = R4(puiTmp + 104);
    eph.f0     = R4(puiTmp + 110);
    eph.sva    = U1(puiTmp +  13); /* URA */
    eph.code   = U1(puiTmp +  9);
    eph.flag   = U1(puiTmp +  15);
    week       = U2(puiTmp + 118); /* WN */
    eph.fit    = 0;

    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=raw->time;

/*  if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    }    */

    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

/* decode SBF nav message for glonass (navigation data) ----------------------*/
static int decode_glonav(raw_t *raw){

    unsigned char *puiTmp = (raw->buff)+6;
    geph_t eph={0};
    double toc;
    unsigned short prn, sat;
    int week;

    trace(4,"SBF decode_glonav: len=%d\n",raw->len);

    if ((raw->len)<96) {
        trace(2,"SBF decode_glonav frame length error: len=%d\n",raw->len);
        return -1;
    }
    prn = U1(puiTmp+8)-37;
    sat = satno(SYS_GLO,prn);

    if (!((prn>=1)&&(prn<=24))){
        trace(2,"SBF decode_glonav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.frq   = U1(puiTmp +  9) - 7;
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
    week       = U2(puiTmp +  74); /* WN_toe */

    eph.toe    = gpst2time(week, U2(puiTmp +  70));
    eph.tof    = gpst2time(week, U2(puiTmp +  80)*60);
    eph.iode   = U2(puiTmp +  80) & 0x3f;
    eph.svh    = U1(puiTmp +  83); /* is that right? */
    eph.sva    = U2(puiTmp +  88); /* is that right? */
    eph.age    = U2(puiTmp +  86); /* is that right? */

/*  if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
    }*/

    eph.sat=sat;
    raw->nav.geph[prn-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode SBF nav message for sbas (navigation data) ----------------------*/
static int decode_sbasnav(raw_t *raw){

    unsigned char *puiTmp = (raw->buff)+6;
    seph_t eph={0};
    double toc;
    unsigned short prn, sat;
    int week;

    trace(4,"SBF decode_glonav: len=%d\n",raw->len);

    if ((raw->len)<104) {
        trace(2,"SBF decode_glonav frame length error: len=%d\n",raw->len);
        return -1;
    }
    prn = U1(puiTmp+8);
    sat = satno(SYS_SBS,prn);

    if (!((prn>=120)&&(prn<=140))){
        trace(2,"SBF decode_sbasnav prn error: sat=%d\n",prn);
        return -1;
    }

    eph.svh = 0;
    week       = U2(puiTmp +   6);
    eph.sva    = U2(puiTmp +  12);
    eph.t0     = gpst2time(week, U4(puiTmp +  14));
    eph.tof    = adjweek(eph.t0,U4(puiTmp +  2)*1000);
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

    /* debug
    trace(1,"sat=%2d, week=%d, tow=%f\n",sat,eph.week,eph.toes);
    */

/*  if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(eph.t0,raw->nav.seph[prn-MINPRNSBS].t0))<1.0&&
            eph.sva==raw->nav.seph[prn-MINPRNSBS].sva) return 0;
    }*/

    eph.sat=sat;
    raw->nav.seph[prn-MINPRNSBS]=eph;
    raw->ephsat=eph.sat;
    return 2;
}

/* decode SBF raw nav message (raw navigation data) --------------------------*/
static int decode_gpsrawcanav(raw_t *raw){

    /* NOTE. This function works quite well but it somestimes fails in line:
     * if (resp>5 || resp<=0){
     * To debug the problem an understanding of the whole RTK code is needed
     */

    unsigned char *p=(raw->buff)+6;
    eph_t eph={0};
    int prn,sat;
    unsigned char _buf[30]={0};
    unsigned char *pt=_buf;
    int i=0,ii=0,id,resp;

    trace(3,"SBF decode_gpsrawcanav: len=%d\n",raw->len);

    if (raw->len<58) {
        trace(2,"SBF decode_gpsrawcanav block length error: len=%d\n",raw->len);
        return -1;
    }

    /* get GPS satellite number */
    sat=U1(p+8);
    eph=raw->nav.eph[sat-1];

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
     id=getbitu(pt,43,3); /* get subframe id */

     resp=decode_frame(pt,&eph,raw->nav.alm,raw->nav.ion_gps,
                       raw->nav.utc_gps,&raw->nav.leaps);

     if (resp>5 || resp<=0){
        /* trace(2,"SBF decode_gpsrawcanav subframe error: sat=%d\n",sat); */
        /* this function sometimes exists here, maybe not such a big problem. */
         return -1;
     }

     eph.sat=sat;
     raw->nav.eph[sat-1]=eph;
     raw->ephsat=sat;

     if (!strstr(raw->opt,"-EPHALL")) {
         if (eph.iode==raw->nav.eph[sat-1].iode) return 0;
     }

     trace(4,"SBF, decode_gpsrawcanav: sat=%2d\n",sat);
     return 2;
}
/* decode SBF raw nav message (raw navigation data) --------------------------*/
static int decode_georaw(raw_t *raw){

    unsigned char *p=(raw->buff)+6;
    int prn,sat;
    unsigned char _buf[30]={0};
    unsigned char *pt=_buf;
    int i=0,ii=0,id,resp;

    trace(3,"SBF decode_georaw: len=%d\n",raw->len);

    if (raw->len<52) {
        trace(2,"SBF decode_georaw block length error: len=%d\n",raw->len);
        return -1;
    }

    /* get GPS satellite number */
    prn=U1(p+8);

    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=U4(p+2);
    raw->sbsmsg.week=U2(p+6);
    raw->time=gpst2time(raw->sbsmsg.week,raw->sbsmsg.tow);

    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=*(p+14+i);
    raw->sbsmsg.msg[28]&=0xC0;
    return 3;
}
/* decode SBF raw nav message (raw navigation data) --------------------------*/
static int decode_glorawcanav(raw_t *raw){

    unsigned char *p=(raw->buff)+6,*r,*fid;
    unsigned char buff[12];
    geph_t geph={0};
    int prn,sat,m;
    int i,j,k;

    r=p+14;
    k=0;
    for (j=0;j<3;j++) {
        unsigned int d = U4(r+j*4);
        buff[k++]=(d>>24)&0xff;
        buff[k++]=(d>>16)&0xff;
        buff[k++]=(d>> 8)&0xff;
        buff[k++]=(d>> 0)&0xff;
    }
    /* get Glonass satellite number */
    prn=U1(p+8)-37;
    sat=satno(SYS_GLO,prn);

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
    geph.tof=gpst2time(U2(p+6),U4(p+2)*1000);
    if (!decode_glostr(raw->subfrm[sat-1],&geph)||geph.sat!=sat) return 0;
    geph.frq=U1(p+12)-7;

    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;

    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode) return 0;
    }
    return 2;
}

/* decode SBF gpsion --------------------------------------------------------*/
static int decode_gpsion(raw_t *raw){
    unsigned char prn, *p=(raw->buff)+8;            /* points at TOW location */

    trace(4,"SBF decode_gpsion: len=%d\n", raw->len);
    prn = U1(p + 6);
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
    unsigned char prn, *p=(raw->buff)+6;            /* points at TOW location */

    trace(4,"SBF decode_galion: len=%d\n", raw->len);
    prn = U1(p + 8);
    raw->nav.ion_gal[0] = R4(p + 10);
    raw->nav.ion_gal[1] = R4(p + 14);
    raw->nav.ion_gal[2] = R4(p + 18);
    raw->nav.ion_gal[3] = 0;

    return 9;
}

/* decode SBF gpsutc --------------------------------------------------------*/
static int decode_gpsutc(raw_t *raw)
{
    unsigned char *p=(raw->buff)+8;                 /* points at TOW location */

    trace(4,"SBF decode_gpsutc: len=%d\n", raw->len);

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
    unsigned char *p=(raw->buff)+8;                 /* points at TOW location */
    alm_t alm;

    trace(4,"SBF decode_gpsalm: len=%d\n", raw->len);

    alm.sat =   satno(SYS_GPS,U1(p + 6));
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
    alm.svconf= U1(p + 49);
    alm.svh   = U1(p + 50);

    raw->nav.alm[alm.sat-1]=alm;

    return 9;
}
/* decode SBF galutc --------------------------------------------------------*/
static int decode_galutc(raw_t *raw)
{
    unsigned char *p=(raw->buff)+8;                 /* points at TOW location */

    trace(4,"SBF decode_galutc: len=%d\n", raw->len);

    /* GPS delta-UTC parameters */
    raw->nav.utc_gal[1] = R4(p + 8);                                  /*   A1 */
    raw->nav.utc_gal[0] = R8(p + 12);                                 /*   A0 */
    raw->nav.utc_gal[2] = U4(p + 20);                                 /*  tot */
    raw->nav.utc_gps[3] = adjgpsweek(U2(p + 4));                      /*   WN */
    raw->nav.leaps      = I1(p + 22);                                 /* Dtls */

    /*NOTE. it is kind of strange that I have to use U1(p+4) and not U1(p+24)
            in fact if I take U1(p+24) I do not seem to ge the correct W in
            the header of RINEX nav file, line DELTA-UTC: A0,A1,T,W
    */
    return 9;
}
/* decode SBF galalm --------------------------------------------------------*/
static int decode_galalm(raw_t *raw)
{
    unsigned char *p=(raw->buff)+8;                 /* points at TOW location */
    alm_t alm;

    trace(4,"SBF decode_galalm: len=%d\n", raw->len);

    alm.sat =   satno(SYS_GAL,U1(p + 49));
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

    raw->nav.alm[alm.sat-1]=alm;

    return 9;
}

/* decode SBF raw message --------------------------------------------------*/
static int decode_sbf(raw_t *raw)
{
    unsigned short crc;

    /* read the SBF block ID and revision */
    int type = U2(raw->buff+4) & 0x1fff << 0;
    int revision = U2(raw->buff+4) >> 13;

    trace(3,"decode_sbf: type=%04x len=%d\n",type,raw->len);

    /* read the SBF block CRC */
    crc = U2(raw->buff+2);

    /* checksum skipping first 4 bytes */
    if (checksum(raw->buff+4, raw->len-4) !=  crc){
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
        case ID_GPSRAWCA:       return decode_gpsrawcanav(raw);

        case ID_GLONAV:         return decode_glonav(raw);
        case ID_GLORAWCA:       return decode_glorawcanav(raw);

        case ID_GALNAV:         return decode_galnav(raw);
        case ID_GALION:         return decode_galion(raw);
        case ID_GALUTC:         return decode_galutc(raw);
        case ID_GALALM:         return decode_galalm(raw);

        case ID_GEONAV:         return decode_sbasnav(raw);
        case ID_GEORAWL1:
        case ID_GEORAWL5:
                                return decode_georaw(raw);
#if 0
        case ID_GPSRAWL2C:
        case ID_GPSRAWL5:
        case ID_GALRAWFNAV:
        case ID_GALRAWINAV:
        case ID_GEORAWL1:
        case ID_GEORAWL5:
        case ID_GALRAWFNAV:     return decode_galrawfnav(raw);
        case ID_GEORAW:
        case ID_GLOALM:         return decode_gpsalm(raw);
        case ID_GLOTime:        return decode_gpsalm(raw);

        case ID_MEASEPOCHEXTRA: return decode_measepochextra(raw);
        case ID_PVTGEOD:        return decode_pvtgeod(raw);
        case ID_RXSETUP:        return decode_rxsetup(raw);
        case ID_COMMENT:        return decode_comment(raw);
#endif
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

    /* decode le length of the block and store it in len*/
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

