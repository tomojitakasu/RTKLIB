/*------------------------------------------------------------------------------
* cmr.c : CMR dependent functions
*
* references:
*     [1] https://github.com/astrodanco/RTKLIB/tree/cmr/src/cmr.c
*
* version : $Revision:$ $Date:$
* history : 2016/07/?? 1.0  imported from GitHub (ref [1])
*-------------------------------------------------------------------------------
*/

/*
| CMR protocol stream and file handler functions.
|
| Written in June 2016 by Daniel A. Cook, for inclusion into the RTKLIB library.
| Copyright (C) 2016 by Daniel A. Cook. All Rights Reserved.
|
| The Compact Measurement Record Format (CMR) is a de facto industry standard
| reference station data transmission protocol for RTK positioning. Despite
| the availability of practical alternatives such RTCM v3.1, CMR and CMR+
| still remain de facto industry standards, especially in the US market.
|
| Here we implement four public functions, one for reading CMR format streams
| and another for reading CMR format files, a third to supply rover observations
| to the CMR base observations referencing engine and aa fourth to free up CMR
| related memory storage.
|
| Although a Trimble proprietary protocol, CMR was documented in reference #1
| and CMR+ was documented in reference #2. These were then adopted by many
| other manufacurers in addition to Trimble. Leica extended the protocol to
| handle GLONASS observations and other manufacturers adopted Leica's extention,
| but there was apparently a disconnect with Trimble on how Trimble implemented
| the same extension making GLONASS compatibility between Trimble and non-
| Trimble receivers problematic whenever the CMR protocol is used. Note that
| so far as the author knows, Trimble never documented their own GLONASS
| extension to the protocol. It is not implemented here.
|
| RTCM3 should always be used whenever possible in lieu of CMR. Use CMR only
| when there is no other option.
|
| Notes:
|
| CMR message types 0, 1 and 2 are implemented as described by reference #1.
|
| CMR message type 3 (GLONASS) is implemented as described by reference #3,
| but I don't have suitable receivers with which to test and debug it. This
| message is the Leica (or rather non-Trimble since other manufacturers also
| output it) implementation of GLONASS base observations for CMR.
|
| CMR message type 4 is high speed CMR (5, 10, or 20 Hz). Support for this
| message is limited to utilizing the GPS L1 carrier phase observables
| contained therein.
|
| CMR+ message types 1, 2 and 3 are implemented. I have not yet been able
| to obtain reference #2 which presumably describes CMR+ in detail.
|
| Ag scrambled CMR+ (sCMR+) is not implemented.
|
| CMRW (message number 0x098) aka CMR-W is not implemented.
|
| CMRx (Compressed Measurement Record Extended) and sCMRx (scrambled CMRx)
| are patented (see the patents) therefore are absolutely positively not
| implemented!
|
| If you have a copy of reference #2, please be kind and send the author
| a copy. In fact if you have any technical information concerning any of
| the CMR family protocols the author would appreciate if you would share
| it with him.
|
| Both stream and file input of raw binary CMR base data are supported.
|
| Design notes:
|
| The handling of CMR is similar to the handling of RTCM and even uses the
| RTCM structure rather than it's own. We just piggyback a little additional
| cmr_t structure within the rtcm_t structure.
|
| CMR messages  increment rtcm->nmsg2[]
| CMR+ messages increment rtcm->nmsg3[]
|
| Note that because the L1 pseudorange is transmitted modulo one light
| millisecond, the base to rover distance must be less than 300 KM.
| CMR will not work at all beyond that distance.
|
| Note that CMR observables are not standalone. Before being utilized as
| base observations they must first be "referenced" to closely matching
| (in time) rover L1 pseudorange observables. Thus they are of little to
| no practical use in the absence of matching rover observations.
|
| CMR does not stream any almanac or ephemeris data. Only observations
| and reference station parameters. Ephemeris data must be obtained from
| the rover or from other sources.
|
| The time in the CMR header is relative rather than absolute. It is also
| ambiguous. It is GPS time in milliseconds modulo four minutes (240,000
| milliseconds or 240 seconds). In a rover receiver this would be aligned
| based upon the rover's current GPS time. We do the same here. Absolute
| time comes from the rover observations. Note that with CMR message type
| 4 (high speed observations) the time is modulo four seconds (4000 milli-
| seconds). Also note that according to reference #3 the time in CMR type
| 3 messages is UTC time, not GPS or GLONASS time. We convert it to GPS time.
|
| This code was tested using CMR/CMR+ data output by the following receivers:
|
| 1. Trimble 4000SSI, firmware version 7.32
| 2. Trimble 5700, firmware version 2.32
| 3. Spectra Precision Epoch 25 Base, firmware version 2.32
|
| For testing purposes one would normally use the Trimble GPS Configurator utility
| to configure the test receiver, but that utility does not understand all receiver
| features such as CMR 5Hz and 10Hz output. The Trimble WINPAN utility was used
| to configure CMR 5Hz and 10Hz output.
|
| For the purposes of this explaination the following terms mean the following things:
|
| C     299,792,458     One light second in meters.
| CMS   C / 1000        One light millisecond in meters.
| L1F   1,574,420,000   L1 frequency in cycles per second.
| L2F   1,227,600,000   L2 frequency in cycles per second.
| L1W   C / L1F         L1 wavelength in meters.
| L2W   C / L2F         L2 wavelength in meters.
|
| L1P   L1 pseudorange.
| L2P   L2 pseudorange.
| L1L   L1 carrier phase.
| L2L   L2 carrier phase.
|
| BT    Base time in millseconds.
| BTS   Base time in seconds.
| BP1   Base L1 pseudorange in meters.
| BL1   Base L1 carrier phase in L1 cycles per second.
| BP2   Base L2 pseudorange in meters.
| BL2   Base L2 carrier phase in L2 cycles per second.
|
| CBS   Base time in milliseconds modulo 240,000.
| CP1   Base L1 pseudorange modulo CMS in 1/8 L1 cycles.
| CL1   Base L1 carrier phase delta in 1/256 L1 cycles.
| CP2   Base L2 pseudorange delta in centimeters.
| CL2   Base L2 carrier phase delta in 1/256 L2 cycles.
|
| RTS   Rover time in seconds.
| RP1   Rover L1 pseudorange in meters.
|
| UBS   Base time in seconds modulo 240.
| UP1   Base L1 pseudorange in meters modulo CMS.
| UL1   Base L1 carrier phase delta in L1 cycles.
| UP2   Base L2 pseudorange delta in meters.
| UL2   Base L2 carrier phase delta in L2 cycles.
|
| CMR type 0 messaages contain the following GPS observables for each satellite:
|
| 1. CBS    (actually just once in the message header)
| 2. CP1    8   * ((BP1 modulo CMS) / L1W)
| 3. CL1    256 * (BL1 - (BP1 / L1W))
| 4. CP2    100 * (BP2 - BP1)
| 5. CL2    256 * (BL2 - (BP1 / L2W))
|
| We temporarilly store them internally as "unreferenced" observables as follows:
|
| 1. UBS    CBS * 0.001     Milliseconds to seconds.    
| 2. UP1    CP1 / 8 * L1W   1/8 L1 cycles to meters.
| 3. UL1    CL1 / 256       1/256 L1 cycles to cycles.
| 4. UP2    CP2 / 100       Centimeters to meters.
| 5. UL2    CL2 / 256       1/256 L2 cycles to cycles.
|
| These above "unreferenced" observables are output by RTKCONV and CONVBIN
| because they have no rover data to reference.
|
| Before use as base observables the above unreferenced observables must be
| referenced to closely corresponding (in time) rover observables as follows:
|
| 1. BTS    UBT + (RTS modulo 240)
| 2. BP1    UP1 + (RP1 - (RP1 modulo CMS))
| 3. BL1    UL1 + (BP1 / L1W) 
| 4. BP2    UP2 + BP1
| 5. BL2    UL2 + (BP1 / L2W)
|
| Time in CMR type 0 messages is GPS time.
|
| CMR type 3 messages contain all the same information as CMR type 0 messages
| except that the carrier phase frequencies and wavelengths are those as for
| GLONASS. For GLONASS those things need to be looked up for every satellite
| and every signal.
|
| According to reference #3 the time in CMR type 3 messages is UTC time, not
| GPS time and not GLONASS time. We convert it to GPS time.
|
| CMR type 4 messages contain the following GPS observables for each satellite:
|
| 1. CBT  (Actually just once in the messsage header and it's modulo 4000 instead of 240000.)
| 2. CL1L (But it's a delta against the prior CMR type 3 BL1L for this satellte.)
|
| CMR type 4 messages are only received with time represending the intervals
| between the seconds and never on an exact second.
|
| By convention functions within this source file appear in alphabetical order.
| Public functions appear as a set first (there are only three of them), followed
| by private functions as a set. Because of this, forward definitions are required
| for the private functions. Please keep that in mind when making changes to this
| source file.
|
| The companion include file cmr.h contains the antenna number to name and receiver
| number to name lookup tables. They were placed there to keep them from cluttering
| up this source file. Everything else is contained herein.
|
| References:
|
| 1. Talbot, N.C., (1996), Compact Data Transmission Standard for
|    High-Precision GPS, in: Proc. of the 9th International Technical
|    Meeting of the Satellite Division of The Institute of Navigation,
|    Kansas City, Missouri, USA, September, pp. 861-871.
|
| 2. Talbot, N.C., (1997), Improvements in the Compact Measurement
|    Record Format, Trimble User�s Conference, San Jose, California,
|    pp. 322-337
|
| 3. A GLONASS Observation Message Compatible With The Compact
|    Measurement Record Format, Leica Geosystems AG,
|    <http://w3.leica-geosystems.com/downloads123/zz/gps/general
|    /white-tech-paper/GLONASS_Observation_Message_Specification.pdf>
|
| 4. Trimble Serial Reference Specification, Version 4.82, Revision A,
|    December 2013. Though not being in any way specific to the BD9xx
|    family of receivers, a handy downloadable copy of this document
|    is contained in the "Trimble OEM BD9xx GNSS Receiver Family ICD"
|    document located at <http://www.trimble.com/OEM_ReceiverHelp/
|    v4.85/en/BinaryInterfaceControlDoc.pdf>
|
| 5. RTKLIB Version 2.4.2 Manual, April 29 2013
|    <http://www.rtklib.com/prog/manual_2.4.2.pdf>
|
| 6. RTKLIB source code located at <https://github.com/tomojitakasu/RTKLIB>
*/

/* Included files: */
#include "rtklib.h"
#include "cmr.h"

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

/* General purpose flag bits masks: */
#define M_BIT0 (1 << 0)
#define M_BIT1 (1 << 1)
#define M_BIT2 (1 << 2)
#define M_BIT3 (1 << 3)
#define M_BIT4 (1 << 4)
#define M_BIT5 (1 << 5)
#define M_BIT6 (1 << 6)
#define M_BIT7 (1 << 7)
#define M_BIT8 (1 << 8)
#define M_BIT9 (1 << 9)
#define M_BIT10 (1 << 10)
#define M_BIT11 (1 << 11)
#define M_BIT12 (1 << 12)
#define M_BIT13 (1 << 13)
#define M_BIT14 (1 << 14)
#define M_BIT15 (1 << 15)

/* Misc. constant definitions: */
#define CMR	                 0x93   /* Trimble CMR  message number */
#define CMRPLUS	             0x94   /* Trimble CMR+ message number */
#define STX                  0x02   /* Start of message character */
#define ETX	                 0x03   /* End of message character */
#define BUFF_LENGTH           512   /* Size of message buffer */

/* CMR message types: */
#define CMR_TYPE_0              0   /* GPS Observables */
#define CMR_TYPE_1              1   /* ECEF Reference Station Coordinates */
#define CMR_TYPE_2              2   /* Reference Station Description */
#define CMR_TYPE_3              3   /* GLONASS Observables */
#define CMR_TYPE_4              4   /* High Speed Observables */

/* CMR+ message types: */
#define CMRPLUS_TYPE_1          1   /* Reference Station Information */
#define CMRPLUS_TYPE_2          2   /* ECEF Reference Station Coordinates */
#define CMRPLUS_TYPE_3          3   /* Reference Station Description */

/* Clock state constants: */
#define CLOCK_OFFSET_INVALID    0
#define CLOCK_OFFSET_VALID      3

/* Receiver status byte bit masks: */
#define M_STATUS_LOW_MEMORY	    M_BIT0  /* 0 = Memory  OK, 1 = Memory  low */
#define M_STATUS_LOW_BATTERY	M_BIT1  /* 0 = Battery OK, 1 = Battery low */
#define M_STATUS_RETURNTOPOINT	M_BIT2
#define M_STATUS_ROVING         M_BIT3  /* 0 = Static base, 1 = Moving base */
#define M_STATUS_CONTKIN	    M_BIT4
#define M_STATUS_NEWBASE	    M_BIT5
#define M_STATUS_SYNCED		    M_BIT6
#define M_STATUS_RTKINITED	    M_BIT7

/* CMR type 1 and 2 flag bit masks: */
#define M_CFLAG_RESERVED_01     M_BIT0
#define M_CFLAG_L2ENABLE        M_BIT1  /* L2 reception is enabled on the base */
#define M_CFLAG_RESERVED_04     M_BIT2
#define M_CFLAG_LOW_MEMORY      M_BIT3  /* 0 = Memory  OK, 1 = Memory  low */
#define M_CFLAG_LOW_BATTERY     M_BIT4  /* 0 = Battery OK, 1 = Battery low */

/* CMR+ type 1 (Station Information) flag word bit masks: */
#define M_PFLAG_UNKNOWN_0001    M_BIT0
#define M_PFLAG_UNKNOWN_0002    M_BIT1
#define M_PFLAG_UNKNOWN_0004    M_BIT2
#define M_PFLAG_UNKNOWN_0008    M_BIT3
#define M_PFLAG_BASE_MOTION     (M_BIT4|M_BIT5)
#define BASE_MOTION_STATIC      1
#define BASE_MOTION_KINEMATIC   2
#define M_PFLAG_UNKNOWN_0040    M_BIT6
#define M_PFLAG_UNKNOWN_0080    M_BIT7
#define M_PFLAG_UNKNOWN_0100    M_BIT8
#define M_PFLAG_L2ENABLE        M_BIT9  /* L2 reception is enabled on the base */
#define M_PFLAG_MAXWELL         M_BIT10 /* Receiver uses a MAXWELL chipset */
#define M_PFLAG_LOW_MEMORY      M_BIT11 /* 0 = Memory  OK, 1 = Memory  low */
#define M_PFLAG_LOW_BATTERY     M_BIT12 /* 0 = Battery OK, 1 = Battery low */
#define M_PFLAG_UNKNOWN_2000    M_BIT13
#define M_PFLAG_UNKNOWN_4000    M_BIT14
#define M_PFLAG_UNKNOWN_8000    M_BIT15

/* L1 observables flag bit masks: */
#define M_L1_L2_FOLLOWS         M_BIT0  /* L2  1 = L2 FOLLOWS, 0 = NO L2  */
#define M_L1_PHASE_VALID        M_BIT1  /* Ph  1 = VALID, 0 = INVALID     */
#define M_L1_PCODE              M_BIT2  /* P/C 1 = L1P, 0 = L1C           */

/* L2 observables flag bit masks: */
#define M_L2_PHASE_FULL	 	    M_BIT0  /* (E) */
#define M_L2_PHASE_VALID	    M_BIT1  /* (D) */
#define M_L2_CODE_VALID	 	    M_BIT2  /* (C) */
#define M_L2_WCODE              M_BIT3  /* (B) GPS     0 = L2P, 1 = L2W  */
#define M_L2_PCODE              M_BIT3  /* (B) GLONASS 0 = L2P, 1 = L2C  */
#define M_L2_CODE_AVAILABLE 	M_BIT4  /* (A) */

/* RTCM special message flags bits: */
#define M_MFLAG_LOWBATMSG1      M_BIT0
#define M_MFLAG_LOWBATMSG2      M_BIT1
#define M_MFLAG_LOWBATMSG3      M_BIT2
#define M_MFLAG_LOWMEMMSG1      M_BIT3 
#define M_MFLAG_LOWMEMMSG2      M_BIT4 
#define M_MFLAG_LOWMEMMSG3      M_BIT5 
#define M_MFLAG_NOL2MSG1        M_BIT6
#define M_MFLAG_NOL2MSG2        M_BIT7

/* Conversion factors: */
#define L1_WAVELENGTH           (CLIGHT/FREQ1)  /* GPS L1 wavelength (meters) */
#define L2_WAVELENGTH           (CLIGHT/FREQ2)  /* GPS L2 wavelength (meters) */
#define RANGE_MS                (CLIGHT*0.001)  /* Meters per millisecond at light speed */

/*
| MAXTIMEDIFF is the maximum tolerable difference in seconds between the base
| and rover observation times. It must be set low enough so that the change in
| satellite range in meters over the time period in question can never exceed
| RANGE_MS. A quick rough back of the envelope calculation says anything over
| about 73 seconds for GPS or 75 seconds for GLONASS is too long. Incomming
| CMR base observables older than this are dropped.
*/
#define MAXTIMEDIFF             60.0 /* Maximum tolerable time difference in seconds */

/* Utility macros: */
#define SNRATIO(snr) (unsigned char)((snr<=0.0)||(snr>=255.5))?(0.0):((snr*4.0)+0.5)

/* Static global literals: */
static const char rcsid[]="$Id:$";

/* CMR 0x93 message types: */
static const char *CMRTable[] = {
    /* 0 */ "GPS Observables",
    /* 1 */ "ECEF Reference Station Coordinates",
    /* 2 */ "Reference Station Description",
    /* 3 */ "GLONASS Observables",
    /* 4 */ "High Speed GPS Observables"
};

/* CMR+ 0x94 message types: */
static const char *CMRplusTable[] = {
    /* 0 */ NULL,
    /* 1 */ "Reference Station Information",    
    /* 2 */ "ECEF Reference Station Coordinates",
    /* 3 */ "Reference Station Description"
};

/*
| Signal to noise ratio conversion for CMR type 3 (GLONASS) observables.
| See reference #3.
*/
static const unsigned char SnrTable[][2] =
    {{0,0},  {30,4},   {32,8}, {34,12},
    {36,16}, {38,20}, {40,24}, {42,28},
    {44,32}, {46,36}, {48,40}, {50,44},
    {52,48}, {54,52}, {56,56}, {58,60}
};

/*
| Typedefs.
*/

typedef struct {            /* Rover observations cache data record */
    gtime_t time;           /* Rover observation time */
    double P;               /* Rover L1 pseudorange (meters) */
    unsigned char valid;    /* TRUE = Valid, FALSE = Invalid */
} obsr_t;

typedef struct {            /* Base observables data record */
    double        P[2];     /* L1/L2 pseudoranges (meters) */
    double        L[2];     /* L1/L2 carrier-phases (cycles) */
    unsigned int  slot;     /* Slot number */ 
    unsigned char sat;      /* Satellite number */
    unsigned char code[2];  /* L1/L2 code indicators (CODE_???) */
    unsigned char SNR[2];   /* L1/L2 signal strengths */
    unsigned char slip[2];  /* L1/L2 slip counts */
    unsigned char LLI[2];   /* L1/L2 loss of lock indicators */
} obsbd_t;

typedef struct {            /* Base observables header record */
    gtime_t time;           /* Base observables time */
    int n;                  /* Number of observables */
    unsigned char type;     /* Observables type (0, 3, 4) */
    obsbd_t data[MAXSAT];   /* Base observables data records */
} obsb_t;

/*
| Internal private function forward declarations (in alphabetical order):
*/
static const char *AntennaNumberToName(unsigned int Number);
static void CheckCmrFlags(rtcm_t *rtcm, unsigned char *p);
static int CheckMessageChecksum(rtcm_t *rtcm);
static int CheckMessageFlags(rtcm_t *rtcm);
static int CheckStation(rtcm_t *rtcm, int staid);
static gtime_t CmrTimeToGtime(unsigned int CmrTime);
static gtime_t DoubleToGtime(double time);static int DecodeCmr(rtcm_t *rtcm);
static int DecodeCmrPlus(rtcm_t *rtcm);
static int DecodeCmrPlusBuffer(rtcm_t *rtcm);
static int DecodeCmrType0(rtcm_t *rtcm);
static int DecodeCmrType1(rtcm_t *rtcm);
static int DecodeCmrType2(rtcm_t *rtcm);
static int DecodeCmrType3(rtcm_t *rtcm);
static int DecodeCmrType4(rtcm_t *rtcm);
static double GtimeToDouble(gtime_t time);
static int OutputCmrObs(rtcm_t *rtcm, obsb_t *obs);
static const char *ReceiverNumberToName(unsigned int Number);
static int ReferenceCmrObs(rtcm_t *rtcm, gtime_t time, unsigned char type, double P0, obsbd_t *obs);
static gtime_t ReferenceCmrTime(gtime_t CmrTime, gtime_t RoverTime, double WindowSize);
static int sbitn(const unsigned char *Address, int BitPosition, int BitLength);
static void SetStationCoordinates(rtcm_t *rtcm, unsigned char *p);
static void SetStationDescription(rtcm_t *rtcm, unsigned char *p, size_t Length);
static void SetStationInfo(rtcm_t *rtcm, unsigned char *p);
static void StatusReport(rtcm_t *rtcm);
static int SyncMessage(rtcm_t *rtcm, unsigned char Data);
static size_t TrimCopy(char *Destination, size_t DestinationLength, char *Source, size_t SourceLength);
static unsigned int ubitn(const unsigned char *Address, int BitPosition, int BitLength);

/*
| Public functions (in alphabetical order):
*/

/* free_cmr - Free up CMR storage */
extern int free_cmr(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    if (cmr->buff) free(cmr->buff);
    if (cmr->roverobs) free(cmr->roverobs);
    if (cmr->t4data) free(cmr->t4data);
    memset(cmr, 0, sizeof(cmr_t));
    return 0;
}

/*
| input_cmr - Read a CMR data stream 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  5: input station pos/ant parameters
|
| Supported CMR messages: 0, 1, 2, 3, 4; CMR+ messages 1, 2, 3.
*/
extern int input_cmr(rtcm_t *rtcm, unsigned char Data)
{
    int ret;

    /* If no current message */
    if (!rtcm->nbyte)
    {   
        /* Find something that looks like a message */
        if (SyncMessage(rtcm, Data))
        {
            /* Found one */
            rtcm->len = 4 + (unsigned char) rtcm->buff[3] + 2; /* 4 (header) + length + 2 (trailer) */
            rtcm->nbyte = 4; /* We now have four bytes in the stream buffer */
        }

        /* Continue reading the rest of the message from the stream */
        return CheckMessageFlags(rtcm);
    }

    /* Store the next byte of the message */
    rtcm->buff[rtcm->nbyte++] = Data;

    /*
    | Keep storing bytes into the current message
    | until we have what we think are all of them.
    */
    if (rtcm->nbyte < rtcm->len)
        return CheckMessageFlags(rtcm);

    /*
    | At this point we think have an entire message.
    | The prospective message must end with an ETX.
    */
    if (rtcm->buff[rtcm->len-1] != ETX)
    {
        tracet(2, "CMR: Message did not end with an ETX character. Some data lost.\n");
        rtcm->nbyte = 0;
        return CheckMessageFlags(rtcm);
    }

    /*
    | We do indeed have an entire message.
    | Check the message checksum.
    */
    if (!CheckMessageChecksum(rtcm))
    {
        tracet(2, "CMR: Message checksum failure. Message discarded.\n");
        rtcm->nbyte = 0;
        return CheckMessageFlags(rtcm);
    }

    /* For the RTK monitor */
    if (rtcm->outtype)
        sprintf(rtcm->msgtype, "CMR: 0x%02X (%4d)", rtcm->buff[2], rtcm->len);

    StatusReport(rtcm);

    /* If this is a CMR message, then decode it */
    if (rtcm->buff[2] == CMR)
    {
        ret = DecodeCmr(rtcm);
        rtcm->nbyte = 0;
        return ret;
    }

    /* If this is a CMR+ message, then decode it */
    if (rtcm->buff[2] == CMRPLUS) 
    {
        ret = DecodeCmrPlus(rtcm);
        rtcm->nbyte = 0;
        return ret;
    }

    /*
    | If we fall through to here, then the message is not one that we support
    | (and hence we can't really even get here). Dump the message on the floor
    | and continue reading from the stream.
    */
    tracet(2, "CMR: Message is not CMR or CMR+. Message discarded.\n"); 
    rtcm->nbyte = 0;

    return CheckMessageFlags(rtcm);
}

/*
| input_cmrf - Read a CMR mesasge from a file 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  5: input station pos/ant parameters
|
| Supported CMR messages: 0, 1, 2, 3, 4; CMR+ messages 1, 2, 3.
*/
extern int input_cmrf(rtcm_t *rtcm, FILE *fp)
{
    int i, Data, ret;

    for (i = 0; i < 4096; i++)
    {
        if ((Data = fgetc(fp)) == EOF) return -2;
        if ((ret = input_cmr(rtcm, (unsigned char) Data))) return ret;
    }

    return 0; /* return at every 4k bytes */
}

/*
| update_cmr - Update the CMR rover observations table
|
| Returns:
|
| -1: error
|  0: no error
|
| Call this function in the RTK SERVER immediately
| after any rover observations have been received.
*/
extern int update_cmr(rtcm_t *rtcm, obs_t *obs)
{
    cmr_t *cmr = &rtcm->cmr;
    obsr_t *RoverObsTable = (obsr_t*) cmr->roverobs;
    obsd_t *r; int n; unsigned char sat;

    if (!RoverObsTable)
    {
        if (!(RoverObsTable = (obsr_t*) calloc(MAXSAT, sizeof(obsr_t))))
        {
            tracet(0, "CMR: internal error; unable to allocate rover observables table.\n");
            return -1;
        }
        cmr->roverobs = (void*) RoverObsTable;
    }

    for (n = 0; (n < obs->n) && (n < MAXOBS); n++)
    {
        r = &obs->data[n];
        sat = r->sat;

        if ((sat < MAXSAT) && (r->rcv != 2) && (timediff(r->time, RoverObsTable[sat].time) > 0))
        {
            RoverObsTable[sat].time = r->time;
            RoverObsTable[sat].P = r->P[0];
            RoverObsTable[sat].valid = TRUE;
        }
    }

    return 0;
}

/*
| Private functions (in alphabetical order):
*/

/*
| AntennaNumberToName - Lookup antenna name by antenna number 
|
| The CMR antennas table is kept in cmr.h so that it doesn't
| clutter up this source file.
*/
static const char *AntennaNumberToName(unsigned int Number)
{
    char *Name = NULL;

    if (Number < (sizeof(AntennasTable) / sizeof(char*)))
        Name = (char*) AntennasTable[Number];

    /*
    | The table can contain NULL entries.
    | Turn them into unknown antennas.
    */
    if (!Name)
        Name = "UNKNOWN EXT     NONE";

    return Name;
}

/* CheckCmrFlags - Check the CMR type 1 and 2 flags */
static void CheckCmrFlags(rtcm_t *rtcm, unsigned char *p)
{
    cmr_t *cmr = &rtcm->cmr;

    if (CmrFlags & M_CFLAG_LOW_BATTERY)
        cmr->cmsg |= M_MFLAG_LOWBATMSG1;
    else
        cmr->cmsg &= ~M_MFLAG_LOWBATMSG1;

    if (CmrFlags & M_CFLAG_LOW_MEMORY)
        cmr->cmsg |= M_MFLAG_LOWMEMMSG1;
    else
        cmr->cmsg &= ~M_MFLAG_LOWMEMMSG1;

    if (!(CmrFlags & M_CFLAG_L2ENABLE))
        cmr->cmsg |= M_MFLAG_NOL2MSG1;
    else
        cmr->cmsg &= ~M_MFLAG_NOL2MSG1;
}

/*
| CheckMessageChecksum - Check the message checksum
|
| The checksum is computed as the modulo 256 (unsigned 8-bit byte integer)
| sum of the message contents starting with the status byte, including the
| message type byte, length byte, data bytes and ending with the last byte
| of the data bytes. It does not include the STX leader, the ETX trailer
| nor the checksum byte.
*/
static int CheckMessageChecksum(rtcm_t *rtcm)
{
    unsigned char Checksum = 0;
    unsigned char *p = &rtcm->buff[1];      /* Starting with status */
    unsigned int Length = rtcm->buff[3] + 3;/* status, type, length, data */

    /* Compute the message checksum */
    while (Length > 0)
    {
        Checksum += *p++;
        Length--;
    }

    /*
    | Make sure our computed checksum matches the one at the end of the
    | message. (Note that the above loop by design very conveniently left
    | *p pointing to the checksum byte at the end of the message.)
    */ 
    return (Checksum == *p);
}

/* CheckMessageFlags - Check for a message */
static int CheckMessageFlags(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    char msg[128] = {0};

    if (cmr->cmsg != cmr->pmsg)
    {
        if (cmr->cmsg & (M_MFLAG_LOWBATMSG1|M_MFLAG_LOWBATMSG2|M_MFLAG_LOWBATMSG3))
            strcat(msg, "Low battery at the base");
        
        if (cmr->cmsg & (M_MFLAG_LOWMEMMSG1|M_MFLAG_LOWMEMMSG2|M_MFLAG_LOWMEMMSG3))
        {
            if (strlen(msg)) strcat(msg,", ");
            strcat(msg, "Low memory at the base");
        }

        if (cmr->cmsg & (M_MFLAG_NOL2MSG1|M_MFLAG_NOL2MSG2))
        {
            if (strlen(msg)) strcat(msg, ", ");
            strcat(msg, "L2 disabled at the base");
        }
        
        if (strlen(msg)) strcat(msg, ".");

        cmr->pmsg = cmr->cmsg;

        strncpy(rtcm->msg, msg, sizeof(rtcm->msg) - 1);
        tracet(2, "CMR: %s\n", msg);
    }

    return 0;
}

/* CheckStation - Check the Station ID number */
static int CheckStation(rtcm_t *rtcm, int staid)
{
    int id;
    char *p;

    /* If an explicit Station ID has been specified, then enforce it */
    if ((p = strstr(rtcm->opt, "-STA=")) && sscanf(p, "-STA=%d", &id) == 1)
    {
        if (staid != id)
        {
            tracet(2, "CMR: Message with wrong Base Station ID (%d) ignored.\n", staid);
            return 0;
        }
    }

    /*
    | We're accepting any Station ID.
    | Let them know what it is and if it changes on them.
    */
    if (!rtcm->staid)
        tracet(2, "CMR: Base Station ID set to %d.\n", staid);
    else if (rtcm->staid != staid)
        tracet(2, "CMR: Base Station ID changed from %d to %d.\n", rtcm->staid, staid);

    rtcm->staid = staid;

    return 1;
}  

/* Convert CMR time in milliseconds to a gtime_t time */
static gtime_t CmrTimeToGtime(unsigned int CmrTime)
{
    return DoubleToGtime(CmrTime*0.001);
}


/*
| DecodeCmr - Decode a CMR message 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  5: input station pos/ant parameters
|
| The CMR record format is divided into header portions and a data portions.
| See reference #1 and reference #3 for more information.
|
| Supported CMR message types: 0, 1, 2, 3, 4
|
| 0. GPS Observables
|
|    This message contains a GPS observables header followed by multiple
|    GPS observables blocks containing L1 observables. Each L1 observables
|    block can optionally be followed by an L2 observable block. Documented
|    in reference #1.
|
| 1. ECEF Reference Station Coordinates
|
|    This message contains a reference Station Coordinates header followed
|    by an Earth-Centred, Earth-Fixed Coordinates Data Block. Documented in
|    reference #1.
|
| 2. Reference Station Description
|
|    This message contains a Reference Station Description Header followed
|    by a Station Description Data Block. Documented in reference #1.
|
| 3. GLONASS Observables (NOT YET TESTED WITH A REAL RECEIVER)
|
|    This message contains a GLONASS observables header followed by multiple
|    GLONASS observable blocks containing L1 observables. Each L1 observables
|    block can optionally be followed by an L2 observables block. Documented
|    in reference #3.
|
| 4. High Speed Observables
|
|    This message contains a high speed observables header followed by
|    multiple high speed observable blocks containing 24 bits of L1 carrier
|    phase observables and 16 bits of IONO? something or other corresponding
|    to each PRN that was transmitted in the immediately prior CMR type 0
|    message. The transmitted carrier phase values are deltas based on those
|    contained in prior CMR type 0 message.
*/
static int DecodeCmr(rtcm_t *rtcm)
{
    int ret = 0;
    char *Type_s = NULL;
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    unsigned int Type = ubitn(p+1,5,3), Version = ubitn(p,5,3);

    if (GtimeToDouble(rtcm->time) == 0.0)
        rtcm->time = utc2gpst(timeget());

    if (Type < (sizeof(CMRTable) / sizeof(char*)))
        Type_s = (char*) CMRTable[Type];
   
    if (!Type_s)
        Type_s = "Unknown";
   
    tracet(3, "CMR: Trimble Packet Type=0x93 (CMR), CMR Type=%u (%s), CMR Version=%u, Length=%d.\n", Type, Type_s, Version, rtcm->len);

    /*
    | We support version 3 and below for all messages except message type 4
    | for which we support version 4 only.
    */
    if ((Version > 3) && !((Version == 4) && (Type == 4))) 
    {
        tracet(2, "CMR: Unsupported CMR type %u message version: %u\n", Type, Version);
        return -1;
    }

    /* Decode (or possibly ignore) the message */
    switch (Type)
    {
    case CMR_TYPE_0:
        ret = DecodeCmrType0(rtcm);
        break;
    case CMR_TYPE_1:
        ret = DecodeCmrType1(rtcm);
        break;
    case CMR_TYPE_2:
        ret = DecodeCmrType2(rtcm);
        break;
    case CMR_TYPE_3:
        ret = DecodeCmrType3(rtcm);
        break;
    case CMR_TYPE_4:
        ret = DecodeCmrType4(rtcm);
        break;
    default:
        tracet(2, "CMR: Unsupported CMR message type %u ignored.\n", Type);      
    }

    /* ### NOTE: RTK MONITOR UI NEEDS UPDATING TO LABEL THIS AS A CMR MESSAGE ### */
    rtcm->nmsg2[Type]++;

    return ret;
}

/*
| DecodeCmrPlus - Decode a CMR+ message 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  5: input station pos/ant parameters
|
| CMR+ is also known as CMR type 5 which is funny because it comes in
| from the base as a separate message 0x94 instead of 0x93 and contains
| no classic CMR header with version and type fields as described for
| CMR in reference #1.
|
| CMR+ messages can be (and are) interleaved with CMR messages. When
| CMR+ messages are being sent CMR message types 1 and 2, containing
| basically the same information, are normally supressed (otherewise
| there would be no point to CMR+).
|
| We can't process CMR+ messages immediatelly as they are received
| because they are incomplete. They are each just a small incomplete
| portion of an original whole message that has been broken down on
| arbitrary boundaries and tricked out over time. We must buffer all
| the little pieces, wait for them all to come in, assemble the whole
| original message from the little parts, then process it. It takes
| 15 seconds for a full CMR+ message to be received and assembled.
|
| Each small incomplete portion has a header consisting of a station
| number byte, a page number byte, a total number of pages byte, then
| some payload data bytes. We strip off the station number, page number
| and total pages bytes, then concatenate the payload data from each
| page together in page order into a single monolithic message. This
| larger message as assembled then consists of a header containing a
| type byte and a length byte followed by message data, then another
| header consisting of a type byte and a length byte followed by
| message data, etcetera, etcetera until the message ends. The length
| bytes within this message are all high by two because they count the
| header in the length.
*/
static int DecodeCmrPlus(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    unsigned char *buff = (unsigned char*) cmr->buff;
    int Page, Pages, ret = 0;
    unsigned int staid, Length = rtcm->buff[3] - 3;
    unsigned char *p = (unsigned char*) &rtcm->buff[4];

    if (GtimeToDouble(rtcm->time) == 0.0)
        rtcm->time = utc2gpst(timeget());

    staid = *p++;
    Page  = *p++;
    Pages = *p++;

    tracet(3, "CMR: Trimble Packet Type=0x94 (CMR+), Base Station=%u, Page=%d of %d.\n", staid, Page, Pages);

    if (!CheckStation(rtcm, staid))
        return 0;

    if (!buff)
    {
        if (!(buff = (unsigned char*) calloc(BUFF_LENGTH, sizeof(unsigned char))))
        {
            tracet(0, "CMR: internal error; unable to allocate CMR+ message buffer.\n");
            return -1;
        }
        cmr->buff = (void*) buff;
    }

    if (((Page == Pages) && cmr->nbyte) || ((Page == 0) && (Pages == 0)))
    {
        /*
        | It's the last page or the only page. Either way process it.
        | But first check for buffer overflow.
        */
        if ((Page != 0) && ((cmr->nbyte + Length) > BUFF_LENGTH))
        {
            tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
            memset(buff, 0, BUFF_LENGTH);
            cmr->nbyte = 0;
            cmr->page = 0;
            return 0; 
        }

        memcpy(buff + cmr->nbyte, p, Length);
        cmr->nbyte += Length;
        cmr->page = 0;

        ret = DecodeCmrPlusBuffer(rtcm);
    }
    else if (Page == 0)
    {
        /*
        | Cool it's page zero. Clear the buffer and add it to the buffer.
        | But first check for buffer overflow.
        */
        memset(buff, 0, BUFF_LENGTH);
        if (Length > BUFF_LENGTH)
        {
            tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
            cmr->nbyte = 0;
            cmr->page = 0;
            return 0; 
        }

        memcpy(buff, p, Length);
        cmr->nbyte = Length;
        cmr->page = 0;
    }
    else if (cmr->nbyte)
    {
        /*
        | It's not page zero, not the last page, but we have accumulated
        | some prior pages. We must be in the middle of accumulating a set
        | of pages starting from page zero and working our way up to the
        | last page. Make sure this page is the one we're expecting. If it
        | is, then concatenate it to the buffer. If it isn't the one we're
        | expecting, then it's an out of order page so dump the buffer and
        | start over.
        */
        if (Page == (cmr->page + 1))
        {
            /* But first check for buffer overflow */
            if ((cmr->nbyte + Length) > BUFF_LENGTH)
            {
                tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
                memset(buff, 0, BUFF_LENGTH);
                cmr->nbyte = 0;
                cmr->page = 0;
                return 0; 
            }

            memcpy(buff + cmr->nbyte, p, Length);
            cmr->nbyte += Length;
            cmr->page = Page;
        }
        else
        {
            memset(buff, 0, BUFF_LENGTH);
            cmr->nbyte = 0;
            cmr->page = 0;
        }
    }
    else
    {
        /*
        | It's not page zero, not the last page and we haven't already
        | accumulated anything into our page buffer. We must have started
        | receiving the stream already in progress in the middle somewhere.
        | Ignore all further pages until next page zero comes along.
        */
        cmr->page = 0;
    }

    return ret;
}

/*
| DecodeCmrPlusBuffer - Decode a set of buffered CMR+ messages 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  5: input station pos/ant parameters
|
| Supported CMR+ message types: 1, 2, 3
|
| 1. Reference Station Information
|
|    After the type and length byte this message contains 16 bits worth of
|    flag bits, a receiver number byte and an antenna number byte. This is
|    new with CMR+ and has no equivilent in CMR.
|
| 2. ECEF Reference Station Coordinates
|
|    After the type and length byte this message contains the Earth-Centred,
|    Earth-Fixed Coordinates Data Block in the same format as in CMR.
|
| 3. Reference Station Description
|
|    After the type and length byte this message contains the Station
|    Description Data Block in the same format as in CMR.
|
| Fortunately these all require the same function return value of 5.
| That makes them nicely compatible with the RTKLIB architecture.
*/
static int DecodeCmrPlusBuffer(rtcm_t *rtcm)
{
    char *Type_s = NULL;
    unsigned int Type;
    unsigned char *buff = (unsigned char*) rtcm->cmr.buff;
    int nbyte = rtcm->cmr.nbyte;
    size_t Length;

    while (nbyte > 0)
    {
        Type = buff[0];
        Length = buff[1];

        if (Type < (sizeof(CMRplusTable) / sizeof(char*)))
            Type_s = (char*) CMRplusTable[Type];

        if (!Type_s)
            Type_s = "Unknown";

        tracet(3, "CMR: CMR+ Message type=%u (%s), Length=%u.\n", Type, Type_s, Length);

        switch (Type)
        {
        case CMRPLUS_TYPE_1:
            SetStationInfo(rtcm, &buff[2]);
            break;
        case CMRPLUS_TYPE_2:
            SetStationCoordinates(rtcm, &buff[2]);
            break;
        case CMRPLUS_TYPE_3:
            SetStationDescription(rtcm, &buff[2], Length-2);
            break;
        default:
            tracet(2, "CMR: Unsupported CMR+ message type %u ignored.\n", Type);  
        }

        /* ### NOTE: RTK MONITOR UI NEEDS UPDATING TO LABEL THIS AS A CMR+ MESSAGE ### */
        rtcm->nmsg3[Type]++;

        buff += Length;
        nbyte -= Length;
    }

    return 5;
}

/*
| DecodeCmrType0 - Decode CMR GPS Observables
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
*/
static int DecodeCmrType0(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    obsbd_t *t4 = (obsbd_t*) cmr->t4data;
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    unsigned int L1Flags, L2Flags, nsat = ubitn(p+1,0,5), slot, staid = ubitn(p,0,5);
    gtime_t CmrTime = CmrTimeToGtime(ubitn(p+4,6,18));
    obsb_t obs; obsbd_t *b;
    int prn, sat;

    if (!CheckStation(rtcm, staid))
        return 0;

    if (!t4)
    {
        if (!(t4 = (obsbd_t*) calloc(MAXOBS, sizeof(obsbd_t))))
        {
            tracet(0, "CMR: internal error; unable to allocate high speed GPS observations reference table.\n");
            return -1;
        }
        cmr->t4data = (void*) t4;
    }

    memset(&obs, 0, sizeof(obs));
    obs.time = CmrTime;
    obs.type = CMR_TYPE_0;

    /* Position past the header */
    p += 6;

    for (slot = 0; (slot < nsat) && (slot < MAXOBS); slot++)
    {
        b = &obs.data[obs.n];
        b->slot = slot;

        if (!(prn = ubitn(p,3,5))) prn = 32;
        L1Flags = ubitn(p,0,3);
        b->P[0] = (ubitn(p+3,0,24) / 8.0) * L1_WAVELENGTH;
        if (L1Flags & M_L1_PHASE_VALID)
            b->L[0] = sbitn(p+6,4,20) / 256.0;
        b->code[0] = (L1Flags & M_L1_PCODE) ? CODE_L1P : CODE_L1C;
        b->SNR[0] = 28+(ubitn(p+6,0,4)*2);
        b->slip[0] = ubitn(p+7,0,8);
 
        /* Position past the L1 observables block */
        p += 8;

        if (L1Flags & M_L1_L2_FOLLOWS)
        {
            L2Flags = ubitn(p,3,5);

            if ((L2Flags & M_L2_CODE_AVAILABLE) && (L2Flags & M_L2_CODE_VALID))
            {
                b->P[1] = sbitn(p+2,0,16) / 100.0;
                b->code[1] = (L2Flags & M_L2_WCODE) ? CODE_L2W : CODE_L2P;
                if (L2Flags & M_L2_WCODE)
                    b->LLI[1] |= 4; /* Tracking encrypted code */
            }

            if ((L2Flags & M_L2_PHASE_VALID) && (L2Flags & M_L2_PHASE_FULL))
                b->L[1] = sbitn(p+5,4,20) / 256.0;

            b->SNR[1] = 28+(ubitn(p+5,0,4)*2);
            b->slip[1] = ubitn(p+6,0,8);

            /* Position past the L2 observables block */
            p += 7;
        }

        if (!(b->sat = satno(SYS_GPS, prn)))
        {
            tracet(1, "CMR: GPS satellite number error, PRN=%d.\n", prn);
            continue;
        }

        sat = b->sat - 1;
        if (rtcm->lock[sat][0] && rtcm->loss[sat][0] != b->slip[0]) b->LLI[0] |= 1;
        if (rtcm->lock[sat][1] && rtcm->loss[sat][1] != b->slip[1]) b->LLI[1] |= 1;
        rtcm->loss[sat][0] = b->slip[0];
        rtcm->loss[sat][1] = b->slip[1];
        rtcm->lock[sat][0] = TRUE;
        rtcm->lock[sat][1] = TRUE;

        obs.n++;
    }

    return OutputCmrObs(rtcm, &obs);
}

/*
| DecodeCmrType1 - Decode CMR ECEF Reference Station Coordinates
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  5: input station pos/ant parameters
*/
static int DecodeCmrType1(rtcm_t *rtcm)
{
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    unsigned int staid = ubitn(p, 0, 5);

    if (!CheckStation(rtcm, staid))
        return 0;

    CheckCmrFlags(rtcm, p);

    /* Position past the header */
    p += 6;

    SetStationCoordinates(rtcm, p);
    return 5;
}

/*
| DecodeCmrType2 - Decode CMR Reference Station Description
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  5: input station pos/ant parameters
*/
static int DecodeCmrType2(rtcm_t *rtcm)
{
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    unsigned int Length, staid = ubitn(p, 0, 5);

    if (!CheckStation(rtcm, staid))
        return 0;

    CheckCmrFlags(rtcm, p);

    /*
    | Position past the header.
    | Fetch the length.
    | Position past the length.
    */
    p += 6;
    Length = *p;
    p++;

    SetStationDescription(rtcm, p, Length);

    return 5;
}

/*
| DecodeCmrType3 - Decode CMR GLONASS Observables
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|
| According to reference #3 the epoch time in the CMR type 3 observables
| header is similar to the CMR type 1 header except that it is UTC time
| instead of GLONASS or GPS time. We convert it to GPS time.
*/
static int DecodeCmrType3(rtcm_t *rtcm)
{
    double L1WaveLength;
    cmr_t *cmr = &rtcm->cmr;
    rtksvr_t *rtksvr = (rtksvr_t*) cmr->rtksvr;
    nav_t *nav = (rtksvr) ? &rtksvr->nav : &rtcm->nav;
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    gtime_t CmrTime = utc2gpst(CmrTimeToGtime(ubitn(p+4,6,18)));
    unsigned int L1Flags, L2Flags, nsat = ubitn(p+1,0,5), slot, staid = ubitn(p,0,5);
    obsb_t obs; obsbd_t *b;
    int prn, sat;

    /* ### NEEDS TESTING BY SOMEONE WITH APPROPRIATE RECEIVERS ### */
    tracet(2, "CMR: WARNING: CMR type 3 (GLONASS) support is untested.\n");      

    if (!CheckStation(rtcm, staid))
        return 0;

    memset(&obs, 0, sizeof(obs));
    obs.time = CmrTime;
    obs.type = CMR_TYPE_3;

    /* Position past the header */
    p += 6;

    for (slot = 0; (slot < nsat) && (slot < MAXOBS); slot++)
    {
        b = &obs.data[obs.n];
        memset(b, 0, sizeof(obsd_t));
        b->slot = slot;

        if (!(prn = ubitn(p,3,5))) prn = 32;
        L1Flags = ubitn(p,0,3);
        b->P[0] = ubitn(p+3,0,24) / 8.0;
        if (L1Flags & M_L1_PHASE_VALID)
            b->L[0] = sbitn(p+6,4,20) / 256.0;
        b->code[0] = (L1Flags & M_L1_PCODE) ? CODE_L1P : CODE_L1C;
        b->SNR[0] = SnrTable[ubitn(p+6,0,4)][0];
        b->slip[0] = ubitn(p+7,0,8);

        /* Position past the L1 observables block */
        p += 8;

        if (L1Flags & M_L1_L2_FOLLOWS)
        {
            L2Flags = ubitn(p,3,5);

            if ((L2Flags & M_L2_CODE_AVAILABLE) && (L2Flags & M_L2_CODE_VALID))
            {
                b->P[1] = sbitn(p+2,0,16) / 100.0;
                b->code[1] = (L2Flags & M_L2_PCODE) ? CODE_L2C : CODE_L2P;
            }

            if ((L2Flags & M_L2_PHASE_VALID) && (L2Flags & M_L2_PHASE_FULL))
                b->L[1] = sbitn(p+5,4,20) / 256.0;
  
            b->SNR[1] = SnrTable[ubitn(p+5,0,4)][1];
            b->slip[1] = ubitn(p+6,0,8);

            /* Position past the L2 observables block */
            p += 7;
        }

        if (!(b->sat = satno(SYS_GLO, prn)))
        {
            tracet(1, "CMR: GLONASS satellite number error, PRN=%d.\n", prn);
            continue;
        }

        if ((L1WaveLength = satwavelen(b->sat, 0, nav)) == 0.0)
        {
            tracet(0, "CMR: internal error; satwavelen() failure.\n");
            continue;
        }
        b->P[0] *= L1WaveLength;

        sat = b->sat - 1;
        if (rtcm->lock[sat][0] && rtcm->loss[sat][0] != b->slip[0]) b->LLI[0] |= 1;
        if (rtcm->lock[sat][1] && rtcm->loss[sat][1] != b->slip[1]) b->LLI[1] |= 1;
        rtcm->loss[sat][0] = b->slip[0];
        rtcm->loss[sat][1] = b->slip[1];
        rtcm->lock[sat][0] = TRUE;
        rtcm->lock[sat][1] = TRUE;

        obs.n++;
    }
    
    return OutputCmrObs(rtcm, &obs);
}

/*
| DecodeCmrType4 - Decode CMR High Speed Observables
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|
| The CMR type 4 message is for high speed observables (5, 10, or 20 Hz).
| Support for this message is limited to utilizing the GPS L1 carrier phase
| observables contained therein. This message also contains other data, but
| I'm not certain exactly what that data is nor how RTKLIB might utilize it.
| The GPS L1 carrier phase observables are 24 bit twos complement signed
| deltas from those which where transmitted at the last CMR type 0 message.
| The PRN numbers are not re-transmitted and are assumed to be tho same
| ones in the same order transmitted in the last CMR type 0 message.
|
| So far as I know this is a GPS only message and it has not been extended
| to GLONASS. I therefore make the assumption herein that CMR type 3 messages
| are transparent to CMR type 4 messages. But I could be wrong about that.
| I don't have suitable receivers with which to test that assumption. If
| this assumption is incorrect it is trivially easy so correct it - so long
| as one also knows how to tell a GLONASS type 4 message from a GPS type
| 4 message. Presumably there would be something either in the header or
| somewhere else to indicate that.
|
| I suppose the fastest these could possibly be sent would be every four
| milliseconds or 250Hz because the time in the message header is in units
| of four milliseconds.
*/
static int DecodeCmrType4(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    obsbd_t *t4 = (obsbd_t*) cmr->t4data;
    unsigned char *p = (unsigned char*) &rtcm->buff[4];
    gtime_t CmrTime = CmrTimeToGtime(ubitn(p+4,6,10)<<2);
    unsigned int nsat = ubitn(p+1,0,5), slot, staid = ubitn(p,0,5);
    obsb_t obs; obsbd_t *b;

    if (!CheckStation(rtcm, staid))
        return 0;

    if (!t4)
    {
        tracet(3, "CMR: prior regular speed GPS observations not yet seen; naked high speed GPS observation ignored.\n");
        return 0;
    }

    memset(&obs, 0, sizeof(obs));
    obs.time = CmrTime;
    obs.type = CMR_TYPE_4;

    /* Position past the header */
    p += 6;

    for (slot = 0; (slot < nsat) && (slot < MAXOBS); slot++)
    {
        b = &obs.data[obs.n];
        memset(b, 0, sizeof(obsd_t));
        b->slot = slot;
        b->sat = t4[b->slot].sat;

        b->L[0] = sbitn(p+2,0,24);
        b->L[0] = (b->L[0] == 8388608.0) ? 0.0 : b->L[0] / 256.0;

        /*
        | Position past the L1 observables block. 24 bits for an L1 carrier
        | phase delta plus another 16 bits for something called "IONO".
        */
        p += 5;

        if (b->L[0] == 0.0)
            continue;

        obs.n++;
    }

    return OutputCmrObs(rtcm, &obs);
}

/* Convert a double to a gtime_t time */
static gtime_t DoubleToGtime(double Double)
{
    gtime_t Gtime;
    Gtime.time = floor(Double);
    Gtime.sec = Double - Gtime.time;
    return Gtime;
}

/* Convert a gtime_t time to a double */
static double GtimeToDouble(gtime_t Gtime)
{
    return Gtime.sec + Gtime.time;
}

/* Output a set of CMR base observations */
static int OutputCmrObs(rtcm_t *rtcm, obsb_t *obs)
{
    double WindowSize = (obs->type == CMR_TYPE_4) ? 4.0 : 240.0;
    cmr_t *cmr = &rtcm->cmr;
    obsr_t *r, *RoverObsTable = (obsr_t*) cmr->roverobs;
    obsbd_t *b; int n, ret = 0; unsigned char sat;
    gtime_t ObsTime;

    rtcm->obs.n = 0;

    if (RoverObsTable)
    {
        for (n = 0; !(ret < 0) && (n < obs->n) && (rtcm->obs.n < MAXOBS); n++)
        {
            b = &obs->data[n];
            sat = b->sat;
            r = &RoverObsTable[sat];

            if (r->valid)
            {
                ObsTime = ReferenceCmrTime(obs->time, r->time, WindowSize);
                if (fabs(timediff(r->time, ObsTime)) < MAXTIMEDIFF)
                    ret = ReferenceCmrObs(rtcm, ObsTime, obs->type, r->P, b);
            }
        }
    }
    else
    {
        /* Throw RTKCONV and CONVBIN a bone */
        for (n = 0; !(ret < 0) && (n < obs->n); n++)
            ret = ReferenceCmrObs(rtcm, obs->time, obs->type, 0.0, &obs->data[n]);
    }

    if (rtcm->obs.n > 0)
    {
        tracet(2, "CMR: Base observations referenced and output:\n");
        traceobs(2, rtcm->obs.data, rtcm->obs.n);
    }

    return (ret < 0) ? ret : (rtcm->obs.n > 0);
}

/* Reference and output a single CMR base observation */
static int ReferenceCmrObs(rtcm_t *rtcm, gtime_t time, unsigned char type, double P0, obsbd_t *b)
{
    cmr_t *cmr = &rtcm->cmr;
    obsbd_t *t4 = (obsbd_t*) cmr->t4data;
    rtksvr_t *rtksvr = (rtksvr_t*) cmr->rtksvr;
    nav_t *nav = (rtksvr) ? &rtksvr->nav : &rtcm->nav;
    obsd_t *obs = &rtcm->obs.data[rtcm->obs.n];
    double L0, L1WaveLength, L2WaveLength;

    if (((type == CMR_TYPE_0) || (type == CMR_TYPE_4)) && !t4)
    {
        tracet(0, "CMR: internal error; CMR->T4DATA=NULL in ReferenceCmrObs().\n");
        return -1;
    }

    if (type == CMR_TYPE_0)
    {
        L1WaveLength = L1_WAVELENGTH;
        L2WaveLength = L2_WAVELENGTH;
    }
    else if (type == CMR_TYPE_3)
    {
        if ((L1WaveLength = satwavelen(b->sat, 0, nav) == 0.0) ||
            (L2WaveLength = satwavelen(b->sat, 1, nav) == 0.0))
        {
            tracet(0, "CMR: internal error; satwavelen() failure.\n");
            return -1;
        }
    }

    /* Reference the CMR base observables */
    if ((type != CMR_TYPE_4) && (P0 != 0.0))
    {
        b->P[0] += P0 - fmod(P0, RANGE_MS);
        if (b->L[0] != 0.0)
            b->L[0] += b->P[0] / L1WaveLength;
        if (b->P[1] != 0.0)
            b->P[1] += b->P[0];
        if (b->L[1] != 0.0)
            b->L[1] += b->P[0] / L2WaveLength;
    }

    if (type == CMR_TYPE_0)
        memcpy(&t4[b->slot], b, sizeof(obsbd_t));
    
    if (type == CMR_TYPE_4)
    {
        if (rtksvr)
        {
                L0 = b->L[0] + t4[b->slot].L[0];
                memcpy(b, &t4[b->slot], sizeof(obsbd_t));
                b->L[0] = L0;
        }
#if 0
        b->code[0] = CODE_L1L;
        b->code[1] = CODE_NONE;
#endif
    }

    memset(obs, 0, sizeof(obsd_t));

    obs->rcv     = 2; /* And we don't accept rcv=2 in update_cmr() as a rover */
    obs->time    = time;
    obs->P[0]    = b->P[0];
    obs->P[1]    = b->P[1];
    obs->L[0]    = b->L[0];
    obs->L[1]    = b->L[1];
    obs->sat     = b->sat;
    obs->code[0] = b->code[0];
    obs->code[1] = b->code[1];
    obs->SNR[0]  = SNRATIO(b->SNR[0]);
    obs->SNR[1]  = SNRATIO(b->SNR[1]);
    obs->LLI[0]  = b->LLI[0];
    obs->LLI[1]  = b->LLI[1];

    rtcm->time = time;
    rtcm->obs.n++;

    return 1;
}

/*
| ReceiverNumberToName - Lookup receiver name by receiver number
|
| Returns: Pointer to receiver name string
|
| The CMR+ receivers table is kept in cmr.h so that it doesn't
| clutter up this source file.
*/
static const char *ReceiverNumberToName(unsigned int Number)
{
    char *Name = NULL;

    if (Number < (sizeof(ReceiversTable) / sizeof(char*)))
        Name = (char*) ReceiversTable[Number];

    /*
    | The table can contain NULL entries.
    | Turn them into empty strings.
    */
    if (!Name)
        Name = "";

    return Name;
}

/* ReferenceCmrTime- Reference the CMR base time to the rover time */
static gtime_t ReferenceCmrTime(gtime_t CmrTime, gtime_t RoverTime, double WindowSize)
{
    double modtime = GtimeToDouble(RoverTime);
    return DoubleToGtime((modtime - fmod(modtime, WindowSize)) + CmrTime.time + CmrTime.sec);
}

/*
| sbitn - Fetch a bit aligned signed integer value
|
| Returns: Signed integer value
|
| Bits are numbered from least significant to most significant with bit
| zero in a byte being the least significant bit and bit 7 in a byte being
| the most significant bit.
|
| For the purposes of this function the input data is always considered to
| be big-endian. Bits beyond 7 are taken from bytes at increasingly lower
| addresses, not higher addresses. Works the same on big-endian or little
| endian machines.
|
| The minimum bit position is 0, the maximum bit position is 31,
| the minimum length is 1 and the maximum length is 32. Other values
| for position and length cause a zero value to be returned.
|
| Adapted from similar code written for RTKLIB by T.TAKASU.
*/
static int sbitn(const unsigned char *Address, int BitPosition, int BitLength)
{
    unsigned int Number = ubitn(Address, BitPosition, BitLength);
    if ((BitLength == 0) || (32 <= BitLength) || !(Number & (1 << (BitLength - 1))))
        return (int) Number;
    Number |= (~0UL << BitLength); /* extend sign */
    return (int) Number;
}

/* SetStationCoordinates - Set the station coordinates*/
static void SetStationCoordinates(rtcm_t *rtcm, unsigned char *p)
{
    sta_t *sta = &rtcm->sta;
    sta->pos[0]  = ((sbitn(p+3, 0,32)*4.0)+ubitn(p+4,6,2)) *0.001;
    sta->pos[1]  = ((sbitn(p+9, 0,32)*4.0)+ubitn(p+10,6,2))*0.001;
    sta->pos[2]  = ((sbitn(p+15,0,32)*4.0)+ubitn(p+16,6,2))*0.001;
    sta->del[0]  = sbitn(p+11,0,14)*0.001;
    sta->del[1]  = sbitn(p+17,0,14)*0.001;
    sta->del[2]  = 0.0;
    sta->hgt     = sbitn(p+5, 0,14)*0.001;
    sta->deltype = 0;  /* e/n/u */
    tracet(3, "CMR: Reference station coordinates received. X=%f, Y=%f, Z=%f, East offset=%f, North offset=%f, Up offset=0.0, Height=%f\n",
        sta->pos[0], sta->pos[1], sta->pos[2], sta->del[0], sta->del[1], sta->hgt);
}

/* SetStationDescription - Set the station description */
static void SetStationDescription(rtcm_t *rtcm, unsigned char *p, size_t Length)
{
    /* Set the station name with any leading or trailing nulls & white space trimmed off */
    sta_t *sta = &rtcm->sta;
    if (Length > 8) Length = 8;
    memset(sta->name, 0, sizeof(sta->name));
    TrimCopy(sta->name, sizeof(sta->name) - 1, (char*) p, Length);
    tracet(3, "CMR: Reference station decription received. STATION=\"%s\"\n", sta->name);
}

/* SetStationInfo - Set miscellaneous base station information */
static void SetStationInfo(rtcm_t *rtcm, unsigned char *p)
{
    cmr_t *cmr = &rtcm->cmr;
    sta_t *sta = &rtcm->sta;
    unsigned int CmrPlusFlags = ubitn(p+1,0,16);

    memset(sta->rectype, 0, sizeof(sta->rectype));
    strncpy(sta->rectype, ReceiverNumberToName(p[2]), sizeof(sta->rectype)-1);
    memset(sta->antdes, 0, sizeof(sta->antdes));
    strncpy(sta->antdes, AntennaNumberToName(p[3]), sizeof(sta->antdes)-1);
    tracet(3, "CMR: Reference station information received. RECEIVER=\"%s\", ANTENNA=\"%s\"\n", sta->rectype, sta->antdes);

    if (CmrPlusFlags & M_PFLAG_LOW_BATTERY)
        cmr->cmsg |= M_MFLAG_LOWBATMSG2;    
    else
        cmr->cmsg &= ~M_MFLAG_LOWBATMSG2;

    if (CmrPlusFlags & M_PFLAG_LOW_MEMORY)
        cmr->cmsg |= M_MFLAG_LOWMEMMSG2;
    else
        cmr->cmsg &= ~M_MFLAG_LOWMEMMSG2;

    if (!(CmrPlusFlags & M_PFLAG_L2ENABLE))
        cmr->cmsg |= M_MFLAG_NOL2MSG2;
    else
        cmr->cmsg &= ~M_MFLAG_NOL2MSG2;
}

/* StatusReport - Output once a minute base status */
static void StatusReport(rtcm_t *rtcm)
{
    cmr_t *cmr = &rtcm->cmr;
    unsigned char Status = (unsigned char) rtcm->buff[1];

    if (Status & M_STATUS_LOW_BATTERY)
        cmr->cmsg |= M_MFLAG_LOWBATMSG3;
    else
        cmr->cmsg &= ~M_MFLAG_LOWBATMSG3;

    if (Status & M_STATUS_LOW_MEMORY)
        cmr->cmsg |= M_MFLAG_LOWMEMMSG3;
    else
        cmr->cmsg &= ~M_MFLAG_LOWMEMMSG3;
}

/* SyncMessage - Synchronize the CMR data stream to the start of a series of CMR messages */
static int SyncMessage(rtcm_t *rtcm, unsigned char Data)
{
    unsigned char Type, *buff = rtcm->buff;

    buff[0] = buff[1];
    buff[1] = buff[2];
    buff[2] = buff[3];
    buff[3] = Data;

    Type = buff[2];

    /*
    | Byte 0 must be an STX character.
    | Byte 1 = status byte which we always ignore (for now).
    | Byte 2 = message type which must be CMR (93h) or CMR+ (94h).
    | Byte 3 = data length which must be non-zero for any message we're interested in.
    */
    return ((buff[0] == STX) && (Data != 0) && ((Type == CMR) || (Type == CMRPLUS)));
}

/* TrimCopy - Copy source to destination with trim */
static size_t TrimCopy(char *Destination, size_t DestinationLength, char *Source, size_t SourceLength)
{
    char *e;

    /*
    | Trim sequences leading and training spaces & nulls off the source.
    | (Adjusts the starting address and the source length.)
    */
    for (; (SourceLength > 0) && ((*Source == 0) || isspace(*Source)); Source++, SourceLength--);
    for (e = Source + SourceLength - 1; (SourceLength > 0) && ((*e == 0) || isspace(*e)); e--, SourceLength--);

    /* Only copy as many characters as we actually have */
    if (DestinationLength > SourceLength)
        DestinationLength = SourceLength;

    memcpy(Destination, Source, DestinationLength);
    return DestinationLength;
}

/*
| ubitn - Fetch a bit aligned unsigned integer value
|
| Returns: Unsigned integer value.
|
| Bits are numbered from least significant to most significant with bit
| zero in a byte being the least significant bit and bit 7 in a byte being
| the most significant bit.
|
| WARNING: For the purposes of this function The input data is always
| considered to be in network byte order (AKA BIG-ENDIAN or Motorola format).
| Bits beyond 7 are taken from bytes at increasingly lower addresses, not
| higher addresses.
|
| For the purposes of this function our machine endianess is irrelevant.
|
| The minimum bit position is 0, the maximum bit position is 31,
| the minimum length is 1 and the maximum length is 32. Other values
| for position and length cause a zero value to be returned.
|
| Adapted from similar code written for RTKLIB by T.TAKASU.
*/
static unsigned int ubitn(const unsigned char *Address, int BitPosition, int BitLength)
{
    int i;
    unsigned int n;

    if ((BitPosition < 0) || (BitPosition > 31) || (BitLength <= 0) || (BitLength > 32))
    {
        tracet(0, "CMR: internal error; ubitn() bit position:length violation %u:%u.\n", BitPosition, BitLength);
        return 0;
    }

    for (n=0, i= BitPosition + BitLength - 1; i >= BitPosition; i--)
        n = (n << 1) | ((*(Address - (i/8)) >> (i%8)) & 1UL);

    return n;
}
