/*------------------------------------------------------------------------------
* cmr.c : CMR dependent functions
*
*          Copyright (C) 2016 Daniel A. Cook, All rights reserved.
*
* references:
*     [1] https://github.com/astrodanco/RTKLIB/tree/cmr/src/cmr.c
*
* version : $Revision:$ $Date:$
* history : 2016/07/15 1.0  imported from GitHub (ref [1])
*           2016/07/16 1.3  modified by T.T
*                           raw->strfmt -> raw->format
*                           int free_cmr() -> void free_cmr()
*           2016/07/29 1.4  fix typo
*                           suppress warning
*           2016/08/20 1.5  truncated antenna and receiver tables
*                           added message functions for rtk monitor
*           2016/09/07 1.6  Permit version 4 for Leica type 3 messages (GLONASS)
*-----------------------------------------------------------------------------*/

/*
| CMR protocol stream and file handler functions.
|
| Written in June 2016 by Daniel A. Cook, for inclusion into the RTKLIB library.
|
| The Compact Measurement Record Format (CMR) is a de facto industry standard
| reference station data transmission protocol for RTK positioning. Despite
| the availability of practical alternatives such RTCM v3.1, CMR and CMR+
| still remain de facto industry standards, especially in the US market.
|
| Here we implement five public functions, one for reading CMR format streams
| and another for reading CMR format files, a third to supply rover observations
| to the CMR base observations referencing engine and a fourth to initialize
| CMR related memory storage and a fifth to free up CMR related memory storage.
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
| Receiver dependent options:
|
| -STA=nn - Set the base station ID to receive (0-31 for CMR).
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
| implemented.
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
| 1. CBT (Actually just once in the messsage header and it's modulo 4000 instead of 240000.)
| 2. CL1 (But it's a delta against the prior CMR type 0 BL1 for this satellte.)
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
| References:
|
| 1. Talbot, N.C., (1996), Compact Data Transmission Standard for
|    High-Precision GPS, in: Proc. of the 9th International Technical
|    Meeting of the Satellite Division of The Institute of Navigation,
|    Kansas City, Missouri, USA, September, pp. 861-871.
|
| 2. Talbot, N.C., (1997), Improvements in the Compact Measurement
|    Record Format, Trimble User?s Conference, San Jose, California,
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
#define CMR                  0x93   /* Trimble CMR  message number */
#define CMRPLUS              0x94   /* Trimble CMR+ message number */
#define STX                  0x02   /* Start of message character */
#define ETX                  0x03   /* End of message character */
#define BUFFER_LENGTH         512   /* CMR+ message buffer size */
#define MESSAGEBUFFER_LENGTH 2048   /* Message buffer size */

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
#define M_STATUS_LOW_MEMORY     M_BIT0  /* 0 = Memory  OK, 1 = Memory  low */
#define M_STATUS_LOW_BATTERY    M_BIT1  /* 0 = Battery OK, 1 = Battery low */
#define M_STATUS_RETURNTOPOINT  M_BIT2
#define M_STATUS_ROVING         M_BIT3  /* 0 = Static base, 1 = Moving base */
#define M_STATUS_CONTKIN        M_BIT4
#define M_STATUS_NEWBASE        M_BIT5
#define M_STATUS_SYNCED         M_BIT6
#define M_STATUS_RTKINITED      M_BIT7

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
#define M_L2_PHASE_FULL         M_BIT0  /* (E) */
#define M_L2_PHASE_VALID        M_BIT1  /* (D) */
#define M_L2_CODE_VALID         M_BIT2  /* (C) */
#define M_L2_WCODE              M_BIT3  /* (B) GPS     0 = L2P, 1 = L2W  */
#define M_L2_PCODE              M_BIT3  /* (B) GLONASS 0 = L2P, 1 = L2C  */
#define M_L2_CODE_AVAILABLE     M_BIT4  /* (A) */

/* Special message flags bits: */
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

/*
| Typedefs:
*/

typedef struct {                    /* Antenna number to name table record */
    unsigned short Number;          /* Antenna number */
    char           *Name;           /* Antenna name */
} ant_t;

typedef struct {                    /* Receiver number to name table record */
    unsigned short Number;          /* Receiver number */
    char           *Name;           /* Receiver name */
} rcv_t;

typedef struct {                    /* Rover observations cache data record */
    gtime_t       Time;             /* Rover observation time */
    double        P;                /* Rover L1 pseudorange (meters) */
    unsigned char Valid;            /* TRUE = Valid, FALSE = Invalid */
} obsr_t;

typedef struct {                    /* Base observables data record */
    double        P[2];             /* L1/L2 pseudoranges (meters) */
    double        L[2];             /* L1/L2 carrier-phases (cycles) */
    unsigned int  Slot;             /* Slot number */ 
    unsigned char Sat;              /* Satellite number */
    unsigned char Code[2];          /* L1/L2 code indicators (CODE_???) */
    unsigned char SNR[2];           /* L1/L2 signal strengths */
    unsigned char Slip[2];          /* L1/L2 slip counts */
    unsigned char LLI[2];           /* L1/L2 loss of lock indicators */
} obsbd_t;

typedef struct {                    /* Base observables header record */
    gtime_t       Time;             /* Base observables time */
    int           n;                /* Number of observables */
    unsigned char Type;             /* Observables type (0, 3, 4) */
    obsbd_t       Data[MAXOBS];     /* Base observables data records */
} obsb_t;

typedef struct {                    /* CMR information struct type */
    unsigned char *Buffer;          /* Buffer for building full CMR+ message from little parts */
    unsigned char *MessageBuffer;   /* Message buffer */
    obsr_t        *RoverObservables;/* Rover observables table */
    rtksvr_t      *Svr;             /* Pointer to RTK server structure (when running in that environment otherwise NULL) */
    obsbd_t       *T4Data;          /* Type 3 reference data for type 4 observables */
    unsigned int  Nmsg1[8];         /* CMR  message counts */
    unsigned int  Nmsg2[8];         /* CMR+ message counts */
    unsigned int  Flags;            /* Miscellaneous internal flag bits */
    unsigned int  CurrentMessages;  /* Current  base messages active */
    unsigned int  PreviousMessages; /* Previous base messages active */
    unsigned int  BufferBytes;      /* Number of bytes of data in CMR+ message buffer */
    unsigned int  MessageBytes;     /* Number of bytes in message buffer */
    unsigned int  MessageLength;    /* Message Length */
    int           Page;             /* Previous page number added to CMR+ message mini-buffer */
    unsigned char SlipC[MAXSAT][2]; /* Slip counts */
    unsigned char SlipV[MAXSAT][2]; /* Slip counts valid indicator */
    char          Msg[128];         /* Message from the base */
} cmr_t;

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
| CMR+ protocol defined antenna number to name lookup table.
|
| Note that this is not a table like the ones in igs08.atx or ngs_abs.pcv and
| cannot be replaced with nor derrived from the content of those files.
|
| Adapted from "c:\program files (x86)\common files\trimble\config\antenna.ini"
| after installing the latest Trimble Office Configuration File Update Utility
| found at <http://www.trimble.com/infrastructure/trimbleconfiguration_ts.aspx>
| For each antenna definition block in antenna.ini the "Type" keyword provides
| the number and the "IGSAntenna" keyword provides the antenna name. If no
| "IGSAntenna" keyword is present then the "Name" keyword provides the name.
| The comment is taken from the "Manufacturer" keyword followed by the "Name"
| keyword unless the name already contains the manufacturer name in which case
| the manufacturer name is not repeated.
*/
static const ant_t AntennasTable[] = {
{  0,"UNKNOWN_EXT     NONE"},       /* Unknown External */
{  1,"TRM4000ST_INT   NONE"},       /* Trimble 4000ST Internal */
{  2,"TRM14156.00-GP  NONE"},       /* Trimble 4000ST Kinematic Ext. */
{  3,"TRM16741.00     NONE"},       /* Trimble Compact Dome */
{  4,"TRM14177.00     NONE"},       /* Trimble 4000ST L1 Geodetic */
{  5,"TRM14532.00     NONE"},       /* Trimble 4000SST/SSE L1/L2 Geodetic */
{  6,"TRM12562.00+SGP NONE"},       /* Trimble 4000SLD L1/L2 Square */
{  7,"TRM10877.10_H   NONE"},       /* Trimble 4000SX Helical */
{  8,"TRM11877.10+SGP NONE"},       /* Trimble 4000SX Micro Square */
{  9,"TRM12333.00+RGP NONE"},       /* Trimble 4000SL Micro Round */
{ 10,"TRM17200.00     NONE"},       /* Trimble 4000SE Attachable */
{ 11,"TRM14532.10     NONE"},       /* Trimble 4000SSE Kin L1/L2 */
{ 12,"TRM22020.00+GP  NONE"},       /* Trimble Compact L1/L2 w/Ground Plane */
{ 13,"TRM22020.00-GP  NONE"},       /* Trimble Compact L1/L2 */
{ 14,"TRM16741.00_RTK NONE"},       /* Trimble Compact Dome w/Init */
{ 15,"TRM14532.10_RTK NONE"},       /* Trimble L1/L2 Kinematic w/Init */
{ 16,"TRM22020.00_RTK NONE"},       /* Trimble Compact L1/L2 w/Init */
{ 17,"TRM23965.00_RTK NONE"},       /* Trimble Compact L1 w/Init */
{ 18,"TRM23965.00     NONE"},       /* Trimble Compact L1 w/Ground Plane */
{ 19,"TRM23965.00-GP  NONE"},       /* Trimble Compact L1 */
{ 20,"TRM23903.00     NONE"},       /* Trimble Permanent L1/L2 */
{ 21,"TRM4600LS       NONE"},       /* Trimble 4600LS Internal */
{ 22,"TRM12562.10+RGP NONE"},       /* Trimble 4000SLD L1/L2 Round */
{ 23,"AOAD/M_T        NONE"},       /* Dorne Margolin Model T */
{ 24,"ASH700228A      NONE"},       /* Ashtech L1/L2 (A-B) */
{ 25,"ASH700936A_M    NONE"},       /* Ashtech 700936A_M */
{ 26,"LEISRX99+GP     NONE"},       /* Leica SR299/SR399 External w/Ground Plane */
{ 27,"TRM29659.00     NONE"},       /* Trimble Choke Ring */
{ 28,"JPLD/M_R        NONE"},       /* Dorne Margolin Model R */
{ 29,"ASH700829.2     SNOW"},       /* Ashtech Geodetic III USCG w/Radome */
{ 30,"TRM29653.00     NONE"},       /* Trimble Integrated GPS/Beacon */
{ 31,"Mobile GPS Antenna"},         /* Trimble Mobile GPS Antenna */
{ 32,"GeoExplorer Internal"},       /* Trimble GeoExplorer Internal */
{ 33,"TOP72110        NONE"},       /* Topcon Turbo-SII */
{ 34,"TRM22020.00+GP  TCWD"},       /* Trimble Compact L1/L2 w/Ground Plane+Radome */
{ 35,"TRM23903.00     TCWD"},       /* Trimble Permanent L1/L2 w/Radome */
{ 36,"LEISRX99-GP     NONE"},       /* Leica SR299/SR399 External */
{ 37,"AOAD/M_B        NONE"},       /* Dorne Margolin Model B */
{ 38,"TRM4800         NONE"},       /* Trimble 4800 Internal */
{ 39,"TRM33429.00-GP  NONE"},       /* Trimble Micro-centered L1/L2 */
{ 40,"TRM33429.00+GP  NONE"},       /* Trimble Micro-centered L1/L2 w/Ground Plane */
{ 41,"TRM33580.50     NONE"},       /* Trimble Integrated GPS/Beacon/Sat DGPS */
{ 42,"TRM35143.00     NONE"},       /* Trimble AeroAntenna */
{ 43,"TOP700779A      NONE"},       /* Topcon Geodetic 3 Rev.D */
{ 44,"ASH700718A      NONE"},       /* Ashtech Geodetic III L1/L2 */
{ 45,"ASH700936A_M    SNOW"},       /* Ashtech Choke w/Radome */
{ 46,"TOP700337       NONE"},       /* Topcon Geodetic 1 Rev.B */
{ 47,"TRM36569.00+GP  NONE"},       /* Trimble Rugged Micro-centered w/13Inch GP */
{ 48,"GeoExplorer 3"},              /* Trimble GeoExplorer 3 */
{ 49,"TRM33429.20+GP  NONE"},       /* Trimble Micro-centered L1/L2 Permanent */
{ 50,"TRM33429.20+GP  TCWD"},       /* Trimble Micro-centered L1/L2 Permanent w/Radome */
{ 51,"TRM27947.00-GP  NONE"},       /* Trimble Rugged L1/L2 */
{ 52,"TRM27947.00+GP  NONE"},       /* Trimble Rugged L1/L2 w/Ground Plane */
{ 53,"ASH701008.01B   NONE"},       /* Ashtech Geodetic IIIA */
{ 54,"ASH700228C      NONE"},       /* Ashtech L1/L2, no Level (C) */
{ 55,"ASH700228E      NONE"},       /* Ashtech L1/L2, Rev. B (D-E) */
{ 56,"ASH700700.A     NONE"},       /* Ashtech Marine L1/L2 (A) */
{ 57,"ASH700700.B     NONE"},       /* Ashtech Marine L1/L2 (B) */
{ 58,"JPSLEGANT_E     NONE"},       /* Javad Positioning Systems JPS Legant w/Flat Groundplane */
{ 59,"JPSREGANT_SD_E  NONE"},       /* Javad Positioning Systems JPS Regant w/Single Depth Choke, Ext */
{ 60,"JPSREGANT_DD_E  NONE"},       /* Javad Positioning Systems JPS Regant w/Dual Depth Choke, Ext */
{ 61,"LEIAT202-GP     NONE"},       /* Leica AT202 */
{ 62,"LEIAT302-GP     NONE"},       /* Leica AT302 */
{ 63,"LEIAT303        LEIC"},       /* Leica AT303 w/Choke Ring+Radome */
{ 64,"LEIAT303        NONE"},       /* Leica AT303 w/Choke Ring */
{ 65,"LEIAT502        NONE"},       /* Leica AT502 */
{ 66,"LEIAT504        NONE"},       /* Leica AT504 w/Choke Ring */
{ 67,"MAC4647942      MMAC"},       /* Macrometer Crossed Dipoles */
{ 68,"NOV501          NONE"},       /* NovAtel GPS-501 L1 */
{ 69,"NOV501+CR       NONE"},       /* NovAtel GPS-501 L1 w/Choke */
{ 70,"NOV502          NONE"},       /* NovAtel GPS-502 L1/L2 */
{ 71,"NOV502+CR       NONE"},       /* NovAtel GPS-502 L1/L2 w/Choke */
{ 72,"NOV531          NONE"},       /* NovAtel GPS-531 L1 */
{ 73,"NOV531+CR       NONE"},       /* NovAtel GPS-531 L1 w/Choke */
{ 74,"SEN67157514     NONE"},       /* Sensor Systems L1/L2, Passive */
{ 75,"SEN67157514+CR  NONE"},       /* Sensor Systems L1/L2 w/Choke, Passive */
{ 76,"SEN67157549     NONE"},       /* Sensor Systems L1 */
{ 77,"SEN67157549+CR  NONE"},       /* Sensor Systems L1 w/Choke Ring */
{ 78,"SEN67157596     NONE"},       /* Sensor Systems L1/L2, Active */
{ 79,"SEN67157596+CR  NONE"},       /* Sensor Systems L1/L2 w/Choke, Active */
{ 80,"NGSD/M+GP60     NONE"},       /* NGS D/M+gp60 */
{ 81,"ASH701945.02B   NONE"},       /* Ashtech D/M Choke, Rev B */
{ 82,"ASH701946.2     NONE"},       /* Ashtech D/M Choke, Rev B, GPS-Glonass */
{ 83,"SPP571908273    NONE"},       /* Spectra Precision Choke */
{ 84,"SPP571908273    SPKE"},       /* Spectra Precision Choke w/Radome */
{ 85,"TRM39105.00     NONE"},       /* Trimble Zephyr */
{ 86,"TRM41249.00     NONE"},       /* Trimble Zephyr Geodetic */
{ 87,"PF Power Internal"},          /* Trimble PF Power Internal */
{ 88,"SPP571212430    NONE"},       /* Spectra Precision Compact L2 */
{ 89,"SPP571212238+GP NONE"},       /* Spectra Precision Geod. w/GP L2 */
{ 90,"SPP571908941    NONE"},       /* Spectra Precision Mini-Geodetic L1/L2 */
{ 91,"SPPGEOTRACER2000NONE"},       /* Spectra Precision SPP Geotracer 2000/2100 L1 Internal */
{ 92,"SPP571212240    NONE"},       /* Spectra Precision Compact L1 */
{ 93,"SPP571212236    NONE"},       /* Spectra Precision Geod. w/GP L1 */
{ 94,"SPP571212774    NONE"},       /* Spectra Precision Mini-Geodetic L1 */
{ 95,"SPP571212790    NONE"},       /* Spectra Precision GG/GPS Pro L1 */
{ 96,"ZEIMINI_GEOD    NONE"},       /* Zeiss Mini-Geodetic L1/L2 */
{ 97,"TRM5800         NONE"},       /* Trimble R8/5800/SPS78x Internal */
{ 98,"NOV503+CR       NONE"},       /* NovAtel GPS-503 L1/L2 w/Choke */
{ 99,"NOV503+CR       SPKE"},       /* NovAtel GPS-503 L1/L2 w/Choke+Radome */
{100,"ASH103661       NONE"},       /* Ashtech Marine IV L1 */
{101,"ASH104847       NONE"},       /* Ashtech Marine IV GG L1 */
{102,"ASH700489       NONE"},       /* Ashtech Dimension L1 */
{103,"ASH700699A      NONE"},       /* Ashtech Marine III A L1 */
{104,"ASH700699B      NONE"},       /* Ashtech Marine III B L1 */
{105,"DSNP DGU001     NONE"},       /* DSNP DGU001 */
{106,"DSNP DGU002     NONE"},       /* DSNP DGU002 */
{107,"DSNP NAP001     NONE"},       /* DSNP NAP001 */
{108,"DSNP NAP002     NONE"},       /* DSNP NAP002 */
{109,"TRM53406.00     NONE"},       /* Trimble A3 */
{110,"ASH701933C_M    NONE"},       /* Ashtech 701933 */
{111,"ASH701933C_M    SNOW"},       /* Ashtech 701933 w/Radome */
{112,"ASH701941.1     NONE"},       /* Ashtech 701941.021 */
{113,"ASH701975.01A   NONE"},       /* Ashtech Geodetic IV */
{114,"ASH701975.01AGP NONE"},       /* Ashtech Geodetic IV w/Ground Plane */
{115,"NOV600          NONE"},       /* NovAtel GPS-600 */
{116,"SOKA110         NONE"},       /* Sokkia A110 */
{117,"SOKA120         NONE"},       /* Sokkia A120 */
{118,"LEISR299_INT    NONE"},       /* Leica SR299/SR399A Internal */
{119,"TRM26738.00     NONE"},       /* Trimble Permanent L1 */
{120,"GeoXT Internal"},             /* Trimble GeoXT Internal */
{121,"TRM47950.00     NONE"},       /* Trimble MS980 Internal */
{122,"GeoXM Internal"},             /* Trimble GeoXM Internal */
{123,"TRM41249.00     TZGD"},       /* Trimble Zephyr Geodetic w/Radome */
{124,"External Mini"},              /* Trimble External Mini */
{125,"Hurricane"},                  /* Trimble Hurricane */
{126,"LEISR399_INT    NONE"},       /* Leica SR399 Internal */
{127,"AERAT2775_42    NONE"},       /* AeroAntenna AT2775-42 */
{128,"AERAT2775_43    NONE"},       /* AeroAntenna AT2775-43 */
{129,"AERAT2775_43    SPKE"},       /* AeroAntenna AT2775-43+rd */
{130,"AERAT2775_62    NONE"},       /* AeroAntenna AT2775-62 */
{131,"AERAT2775_160   NONE"},       /* AeroAntenna AT2775-160 */
{132,"ASH110454       NONE"},       /* Ashtech ProAntenna L1 */
{133,"JNSMARANT_GGD   NONE"},       /* Javad Positioning Systems Marant GGD */
{134,"MPL1230         NONE"},       /* Micro Pulse MPL1230 */
{135,"MPL1370W        NONE"},       /* Micro Pulse MPL1370W */
{136,"MPL_WAAS_2224NW NONE"},       /* Micro Pulse MPLWAAS */
{137,"SOK502          NONE"},       /* Sokkia 502 */
{138,"SOK600          NONE"},       /* Sokkia 600 */
{139,"SOK_RADIAN_IS   NONE"},       /* Sokkia Radian IS */
{140,"TPSCR3_GGD      NONE"},       /* Topcon CR3 GGD */
{141,"TPSCR3_GGD      CONE"},       /* Topcon CR3 GGD w/Radome */
{142,"TPSHIPER_GD     NONE"},       /* Topcon HiPer GD */
{143,"TPSLEGANT_G     NONE"},       /* Topcon Legant G */
{144,"TPSLEGANT3_UHF  NONE"},       /* Topcon Legant 3 UHF */
{145,"TRM29659.00     UNAV"},       /* Trimble Choke Ring w/Dome */
{146,"TRMR10          NONE"},       /* Trimble R10 Internal */
{147,"TRMSPS985       NONE"},       /* Trimble SPS985 Internal */
{148,"TRM59900.00     NONE"},       /* Trimble GNSS-Ti Choke Ring */
{149,"NOV_WAAS_600    NONE"},       /* NovAtel WAAS-600 */
{150,"Ag252 Internal Phat"},        /* Trimble Ag252 Internal Phat */
{151,"TPSLEGANT2      NONE"},       /* Topcon Legant 2 */
{152,"TPSCR4          NONE"},       /* Topcon CR4 */
{153,"TPSCR4          CONE"},       /* Topcon CR4 w/Radome */
{154,"TPSODYSSEY_I    NONE"},       /* Topcon Odyssey */
{155,"TPSPG_A1        NONE"},       /* Topcon PG-A1 */
{156,"TPSHIPER_LITE   NONE"},       /* Topcon HiPer Lite */
{157,"TPSHIPER_PLUS   NONE"},       /* Topcon HiPer Plus */
{158,"TRM33429.20+GP  UNAV"},       /* Trimble Micro-centered L1/L2 Perm. w/UNAV Dome */
{159,"ASH701945.02B   SNOW"},       /* Ashtech 701945 D/M Choke w/Snow Dome */
{160,"ASH701945.02B   SCIS"},       /* Ashtech 701945 D/M Choke w/SCIS Dome */
{161,"ASH701945.02B   SCIT"},       /* Ashtech 701945 D/M Choke w/SCIT Dome */
{162,"ASH700936D_M    SCIS"},       /* Ashtech 700936 D/M Choke w/SCIS Dome */
{163,"LEIAT504        LEIS"},       /* Leica AT504 w/LEIS Dome */
{164,"AERAT2775_41    NONE"},       /* AeroAntenna AT2775-41 */
{165,"NOV702          NONE"},       /* NovAtel 702 Rev 2.02 */
{166,"SOKSTRATUS      NONE"},       /* Sokkia Stratus L1 */
{167,"AOARASCAL       NONE"},       /* Alan Osborne Associates Rascal */
{168,"NAVAN2004T      NONE"},       /* NavCom AN2004T */
{169,"NAVAN2008T      NONE"},       /* NavCom AN2008T */
{170,"NAVSF2040G      NONE"},       /* NavCom SF2040G */
{171,"NAVRT3010S      NONE"},       /* NavCom RT3010S */
{172,"LEIAX1202       NONE"},       /* Leica AX1202 */
{173,"THAZMAX         NONE"},       /* Thales ZMax */
{174,"GeoXH Internal"},             /* Trimble GeoXH Internal */
{175,"Pathfinder XB Internal"},     /* Trimble Pathfinder XB Internal */
{176,"ProXT Internal"},             /* Trimble ProXT Internal */
{177,"ProXH Internal"},             /* Trimble ProXH Internal */
{178,"NOV702_3.00     NONE"},       /* NovAtel 702 Rev 3.00 */
{179,"SOK702_3.00     NONE"},       /* Sokkia 702 Rev 3.00 */
{180,"TPSPG_A1+GP     NONE"},       /* Topcon PG-A1 w/GP */
{181,"TPSPG_A5        NONE"},       /* Topcon PG-A5 */
{182,"TRM59900.00     SCIS"},       /* Trimble GNSS-Ti Choke w/SCIS Dome */
{183,"Recon GPS CF Card Internal"}, /* Trimble Recon GPS CF Card Internal */
{184,"TRM55970.00     NONE"},       /* Trimble Zephyr - Model 2 */
{185,"TRM55971.00     NONE"},       /* Trimble Zephyr Geodetic 2 */
{186,"TRMR8_GNSS      NONE"},       /* Trimble R8 GNSS/SPS88x Internal */
{187,"TRM67770.00     NONE"},       /* Trimble MS99x Internal */
{188,"LEIAT503        LEIC"},       /* Leica AT503 w/Choke Ring+Radome */
{189,"LEIAT503        NONE"},       /* Leica AT503 w/Choke Ring */
{190,"SPP53406.90     NONE"},       /* Spectra Precision EPOCH L1 */
{191,"TRM55971.00     TZGD"},       /* Trimble Zephyr Geodetic 2 w/Dome */
{192,"SPP39105.90     NONE"},       /* Spectra Precision EPOCH L1/L2 */
{193,"TRM57200.00     NONE"},       /* Trimble Z Plus */
{194,"TRM55550.00     NONE"},       /* Trimble GA510 */
{195,"TRM29659.00     SCIS"},       /* Trimble Choke Ring w/SCIS Dome */
{196,"TRM29659.00     SCIT"},       /* Trimble Choke Ring w/SCIT Dome */
{197,"TRM59800.00     SCIT"},       /* Trimble GNSS Choke w/SCIT Dome */
{198,"TRMR4-2         NONE"},       /* Trimble R4-2 Internal */
{199,"TRMR6           NONE"},       /* Trimble R6 Internal */
{200,"Pathfinder XC Internal"},     /* Trimble Pathfinder XC Internal */
{201,"AOAD/M_T        AUST"},       /* Dorne Margolin D/M Model T w/AUST Dome */
{202,"AOAD/M_T        JPLA"},       /* Dorne Margolin D/M Model T w/JPLA Dome */
{203,"TRM55971.00     SCIT"},       /* Trimble Zephyr Geodetic 2 w/SCIT */
{204,"Juno Internal"},              /* Trimble Juno Internal */
{205,"MPL_WAAS_2225NW NONE"},       /* Micro Pulse MPLWAAS+L5 */
{206,"TRM41249.00     SCIT"},       /* Trimble Zephyr Geodetic w/SCIT Dome */
{207,"TRM41249USCG    SCIT"},       /* Trimble Zephyr Geodetic w/USCG SCIT Dome */
{208,"LEIAX1202A      NONE"},       /* Leica AX1202A */
{209,"TRM59800.00     NONE"},       /* Trimble GNSS Choke */
{210,"TRM59800.00     SCIS"},       /* Trimble GNSS Choke w/SCIS Dome */
{211,"LEIAX1202GG     NONE"},       /* Leica AX1202GG */
{212,"TPSCR.G3        NONE"},       /* Topcon TPS CR.G3 */
{213,"TPSCR.G3        TPSH"},       /* Topcon TPS CR.G3 w/TPSH */
{214,"TPSG3_A1        NONE"},       /* Topcon TPS G3_A1 */
{215,"TPSG3_A1        TPSD"},       /* Topcon TPS G3_A1 w/TPSD */
{216,"TPSGR3          NONE"},       /* Topcon TPS GR3 */
{217,"TPSMAPANT_B     NONE"},       /* Topcon TPSMAPANT_B */
{218,"TPSMG_A2        NONE"},       /* Topcon TPSMG_A2 */
{219,"TPSPG_A1        TPSD"},       /* Topcon TPSPG_A1 w/GP+TPSD */
{220,"TPSPG_A2        NONE"},       /* Topcon TPS PG_A2 */
{221,"TPS_CR.3        SCIS"},       /* Topcon TPSCR.3 w/SCIS */
{222,"TPS_CR4         SCIS"},       /* Topcon TPS CR4 w/SCIS */
{223,"TPS_MC.A5       NONE"},       /* Topcon TPS MC.A5 */
{224,"LEIAT504GG      NONE"},       /* Leica LEI AT504GG */
{225,"LEIAT504GG      LEIS"},       /* Leica LEI AT504GG w/LEIS */
{226,"LEIAT504GG      SCIS"},       /* Leica LEI AT504GG w/SCIS */
{227,"LEIAT504GG      SCIT"},       /* Leica LEI AT504GG w/SCIT */
{228,"LEIATX1230      NONE"},       /* Leica LEI ATX1230 */
{229,"LEIATX1230GG    NONE"},       /* Leica LEI ATX1230GG */
{230,"THA800961+REC   NONE"},       /* Thales THA 800961+REC */
{231,"THA800961+RTK   NONE"},       /* Thales THA 800961+RTK */
{232,"THA800961RECUHF NONE"},       /* Thales THA 800961RECUHF */
{233,"THA800961RTKUHF NONE"},       /* Thales THA 800961RTKUHF */
{234,"THANAP002       NONE"},       /* Thales THA NAP002 */
{235,"NOV533          RADM"},       /* NovAtel NOV 533 w/RADM */
{236,"NOV702L_1.01    NONE"},       /* NovAtel NOV 702L_1.01 */
{237,"SOK702          NONE"},       /* Sokkia SOK 702 */
{238,"SOK_GSR2700IS   NONE"},       /* Sokkia SOK GSR2700IS */
{239,"ASH701975.01BGP NONE"},       /* Ashtech ASH 701975.01BGP */
{240,"AERAT2775_42+CR NONE"},       /* AeroAntenna AER AT2775-42 w/Chokering */
{241,"AERAT2775_150   NONE"},       /* AeroAntenna AER AT2775-150 */
{242,"AERAT2775_159   NONE"},       /* AeroAntenna AER AT2775-159 */
{243,"AERAT2775_159   SPKE"},       /* AeroAntenna AER AT2775-159 w/SPKE */
{244,"AERAT2775_270   NONE"},       /* AeroAntenna AER AT2775-270 */
{245,"Nomad Internal"},             /* Trimble Nomad Internal */
{246,"GeoXH 2008 Internal"},        /* Trimble GeoXH 2008 Internal */
{247,"ASH701933C_M    SCIS"},       /* Ashtech 701933 w/SCIS Dome */
{248,"ASH701933C_M    SCIT"},       /* Ashtech 701933 w/SCIT Dome */
{249,"TRM65212.00     NONE"},       /* Trimble Zephyr - Model 2 Rugged */
{250,"TRM44530.00     NONE"},       /* Trimble GA530 */
{251,"SPP77410.00     NONE"},       /* Spectra Precision EPOCH 35 Internal */
{252,"TRMR4           NONE"},       /* Trimble R4 Internal */
{253,"TRMR6-2         NONE"},       /* Trimble R6-2 Internal */
{254,"RELNULLANTENNA  NONE"},       /* Relative Null Antenna */
{255,"GPPNULLANTENNA  NONE"}        /* AdV Null Antenna */
};

/*
| CMR+ protocol defined receiver number to name lookup table.
|
| Adapted from "c:\program files (x86)\common files\trimble\config\receiver.ini"
| after installing the latest Trimble Office Configuration File Update Utility
| found at <http://www.trimble.com/infrastructure/trimbleconfiguration_ts.aspx>
| For each receiver definition block in receiver.ini the "RxId" keyword provides
| the receiver number and the "IGSReceiver" keyword provides the receiver name.
| A handful of receivers also have an "OldRxId" entry which provides an additional
| number with the same name.
*/
static const rcv_t ReceiversTable[] = {
{1,"TRIMBLE 4000A"},
{2,"TRIMBLE 4000ST"},
{3,"TRIMBLE 4000SE"},
{4,"TRIMBLE 4000SE"},
{5,"TRIMBLE 4000SI"},
{6,"TRIMBLE 4400"},
{7,"TRIMBLE 4600"},
{8,"TRIMBLE MSGR"},
{9,"TRIMBLE 4800"},
{10,"TRIMBLE 4000A"},
{11,"TRIMBLE 4000SX"},
{12,"TRIMBLE 4000SLD"},
{13,"TRIMBLE 4000ST"},
{14,"TRIMBLE 4000SST"},
{15,"TRIMBLE 4000SE"},
{16,"TRIMBLE 4000SE"},
{17,"TRIMBLE 4000SSE"},
{18,"TRIMBLE 4000SI"},
{19,"TRIMBLE 4000SSI"},
{20,"TRIMBLE 4400"},
{21,"TRIMBLE 4600"},
{22,"TRIMBLE MSGR"},
{23,"TRIMBLE 4800"},
{24,"TRIMBLE 4700"},
{25,"TRIMBLE MS750"},
{26,"TRIMBLE 5700"},
{27,"TRIMBLE 5800"},
{28,"TRIMBLE MS980"},
{29,"TRIMBLE NETRS"},
{30,"TRIMBLE BD950"},
{31,"TRIMBLE R7"},
{32,"TRIMBLE R8"},
{33,"TRIMBLE R3"},
{34,"TRIMBLE SPS750"},
{35,"TRIMBLE NETR9 GEO"},
{36,"TRIMBLE SPS780"},
{37,"TRIMBLE SPS770"},
{38,"TRIMBLE SPS850"},
{39,"TRIMBLE SPS880"},
{40,"TRIMBLE EPOCH10"},
{41,"TRIMBLE SPS651"},
{42,"TRIMBLE SPS550"},
{43,"TRIMBLE MS990"},
{44,"TRIMBLE R8 GNSS"},
{45,"TRIMBLE NETR5"},
{46,"TRIMBLE SPS550H"},
{47,"TRIMBLE EPOCH25"},
{48,"TRIMBLE R4-2"},
{49,"TRIMBLE AGRTKBASE"},
{50,"TRIMBLE R7 GNSS"},
{51,"TRIMBLE R6"},
{52,"TRIMBLE R8-4"},
{53,"TRIMBLE R6-2"},
{54,"TRIMBLE SPS781"},
{55,"TRIMBLE SPS881"},
{56,"TRIMBLE SPS551"},
{57,"TRIMBLE SPS551H"},
{58,"TRIMBLE SPS751"},
{59,"TRIMBLE SPS851"},
{60,"TRIMBLE AGGPS432"},
{61,"TRIMBLE AGGPS442"},
{62,"TRIMBLE BD960"},
{63,"TRIMBLE NETR8"},
{64,"TRIMBLE 5800II"},
{65,"TRIMBLE SPS351"},
{66,"TRIMBLE SPS361"},
{67,"TRIMBLE PROXRT"},
{68,"TRIMBLE NETR3"},
{69,"TRIMBLE 5700II"},
{70,"TRIMBLE SPS461"},
{71,"TRIMBLE R8 GNSS3"},
{72,"TRIMBLE SPS882"},
{73,"TRIMBLE 9200-G2"},
{74,"TRIMBLE MS992"},
{75,"TRIMBLE BD970"},
{76,"TRIMBLE NETR9"},
{77,"TRIMBLE BD982"},
{78,"TRIMBLE 9205 GNSS"},
{79,"TRIMBLE GEOXR 6000"},
{80,"TRIMBLE R6-3"},
{81,"TRIMBLE GEOXH2008"},
{82,"TRIMBLE JUNOST"},
{83,"TRIMBLE PFXC"},
{84,"TRIMBLE RECONGPSCF"},
{85,"TRIMBLE PROXH"},
{86,"TRIMBLE PROXT"},
{87,"TRIMBLE PFXB"},
{88,"TRIMBLE GEOXH"},
{89,"TRIMBLE GEOXM"},
{90,"TRIMBLE GEOXT"},
{91,"TRIMBLE PROXRT-3"},
{92,"TRIMBLE PFPOWER"},
{93,"TRIMBLE AGL2"},
{94,"TRIMBLE GEOEXPLORER3"},
{95,"TRIMBLE PROXRS"},
{96,"TRIMBLE PROXR"},
{97,"TRIMBLE PROXL"},
{98,"TRIMBLE GEOEXPLORER"},
{99,"TRIMBLE PFBASIC"},
{100,"TRIMBLE R10"},
{101,"TRIMBLE SPS985"},
{102,"TRIMBLE MS952"},
{103,"RNG FASA+"},
{105,"TRIMBLE SPS552H"},
{106,"TRIMBLE MS972"},
{107,"TRIMBLE SPS852"},
{108,"TRIMBLE PROXRT-2"},
{109,"TRIMBLE BD910"},
{110,"TRIMBLE BD920"},
{111,"TRIMBLE BD930"},
{112,"TRIMBLE AGGPS542"},
{113,"TRIMBLE EPOCH35GNSS"},
{114,"TRIMBLE R4"},
{115,"TRIMBLE R5"},
{116,"TRIMBLE EPOCH50"},
{117,"TRIMBLE MS352"},
{118,"TRIMBLE SPS855"},
{119,"TRIMBLE SPS555H"},
{120,"LEICA L1"},
{121,"LEICA L2"},
{122,"WILD WM101"},
{123,"WILD WM102"},
{124,"LEICA SR261"},
{125,"LEICA SR299"},
{126,"LEICA SR399"},
{127,"LEICA SR9400"},
{128,"LEICA SR9500"},
{129,"LEICA CRS1000"},
{130,"LEICA SR510"},
{131,"LEICA SR520"},
{132,"LEICA SR530"},
{133,"LEICA MC500"},
{134,"LEICA GPS1200"},
{135,"LEICA RS500"},
{136,"TRIMBLE R8S"},
{137,"TRIMBLE AG-342"},
{138,"TRIMBLE SPS356"},
{139,"TRIMBLE DELTA7"},
{140,"ASHTECH L1"},
{141,"ASHTECH L2"},
{142,"ASHTECH DIMENSION"},
{143,"ASHTECH L-XII"},
{144,"ASHTECH M-XII"},
{145,"ASHTECH P-XII3"},
{146,"ASHTECH Z-XII3"},
{147,"ASHTECH GG24C"},
{148,"ASHTECH UZ-12"},
{149,"ASHTECH ICGRS"},
{150,"TRIMBLE GEO 5T"},
{151,"TRIMBLE R6-4"},
{152,"TRIMBLE R4-3"},
{153,"SPP PROMARK700"},
{154,"TRIMBLE NETR9 TI-M"},
{155,"TRIMBLE M7-PPS"},
{156,"TRIMBLE APX-15"},
{159,"TRIMBLE MX100"},
{160,"TOPCON GP-R1"},
{161,"TOPCON GP-R1D"},
{166,"TRIMBLE APX-15V2"},
{168,"TRIMBLE R9S"},
{170,"DELNORTE 3009"},
{171,"SOK RADIAN"},
{172,"SOK GSR2600"},
{180,"GEOD GEOTRACER2000"},
{181,"SPP GPSMODULEL1"},
{182,"SPP GEOTRACERL1L2"},
{185,"NOV MILLEN-STD"},
{186,"NIKON LOGPAK"},
{187,"NIKON LOGPAKII"},
{195,"ZEISS EXPERIENCE"},
{196,"LEICA GRX1200GGPRO"},
{197,"TPS NETG3"},
{198,"LEICA GRX1200+GNSS"},
{199,"TPS NET-G3A"},
{200,"LITTON MINIMAC"},
{220,"TOPCON GP-SX1"},
{221,"TOPCON GP-DX1"},
{222,"NGS NETSURV1000"},
{223,"NGS NETSURV1000L"},
{224,"NGS NETSURV2000"},
{230,"JPS LEGACY"},
{231,"JPS REGENCY"},
{232,"JPS ODYSSEY"},
{233,"TPS HIPER_GD"},
{234,"JPS PREGO"},
{235,"JPS E_GGD"},
{236,"JAVAD TRE_G3TH DELTA"},
{239,"SPP MOBILEMAPPER300"},
{240,"TRIMBLE BD935"},
{242,"TRIMBLE SPS985L"},
{243,"TRIMBLE CPS205"},
{244,"TRIMBLE M7-SPS"},
{250,"TRIMBLE SPS585"},
{251,"TRIMBLE BRAVO7"},
{252,"TRIMBLE GEOXT6000"},
{253,"TRIMBLE GEOXH6000"},
{254,"VRS"},
{255,"UNKNOWN"}
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
| Internal private function forward declarations (in alphabetical order):
*/
static const char *AntennaNumberToName(unsigned short Number);
static void CheckCmrFlags(cmr_t *Cmr, unsigned char *p);
static int CheckMessageChecksum(unsigned char *MessageBuffer);
static int CheckMessageFlags(raw_t *Raw);
static int CheckStation(raw_t *Raw, int StationID);
static gtime_t CmrTimeToGtime(unsigned int CmrTime);
static gtime_t DoubleToGtime(double Time);
static int DecodeCmr(raw_t *Raw);
static int DecodeCmrPlus(raw_t *Raw);
static int DecodeBuffer(raw_t *Raw);
static int DecodeCmrType0(raw_t *Raw);
static int DecodeCmrType1(raw_t *Raw);
static int DecodeCmrType2(raw_t *Raw);
static int DecodeCmrType3(raw_t *Raw);
static int DecodeCmrType4(raw_t *Raw);
static double GtimeToDouble(gtime_t Time);
static int OutputCmrObs(raw_t *Raw, obsb_t *Obs);
static const char *ReceiverNumberToName(unsigned short Number);
static int ReferenceCmrObs(raw_t *Raw, gtime_t Time, unsigned char Type, double P0, obsbd_t *Obs);
static gtime_t ReferenceCmrTime(gtime_t CmrTime, gtime_t RoverTime, double WindowSize);
static int sbitn(const unsigned char *Address, int BitPosition, int BitLength);
static void SetStationCoordinates(raw_t *Raw, unsigned char *p);
static void SetStationDescription(raw_t *Raw, unsigned char *p, size_t Length);
static void SetStationInfo(raw_t *Raw, unsigned char *p);
static void StatusReport(raw_t *Raw);
static int SyncMessage(cmr_t *Cmr, unsigned char Data);
static size_t TrimCopy(char *Destination, size_t DestinationLength, char *Source, size_t SourceLength);
static unsigned int ubitn(const unsigned char *Address, int BitPosition, int BitLength);

/*
| Public functions (in alphabetical order):
*/

/* free_cmr - Free up CMR dependent private storage */
EXPORT void free_cmr(raw_t *Raw)
{
    cmr_t *Cmr = NULL;
    
    if (Raw->format != STRFMT_CMR)
       return;
   
    if ((Cmr = (cmr_t*) Raw->rcv_data))
    {
        if (Cmr->Buffer)
        {
            memset(Cmr->Buffer, 0, BUFFER_LENGTH);
            free(Cmr->Buffer);
            Cmr->Buffer = NULL;
        }
       
        if (Cmr->MessageBuffer)
        {
            memset(Cmr->MessageBuffer, 0, MESSAGEBUFFER_LENGTH);
            free(Cmr->MessageBuffer);
            Cmr->MessageBuffer = NULL;
        }

        if (Cmr->RoverObservables)
        {
            memset(Cmr->RoverObservables, 0, MAXSAT * sizeof(obsr_t));
            free(Cmr->RoverObservables);
            Cmr->RoverObservables = NULL;
        }

        if (Cmr->T4Data)
        {
            memset(Cmr->T4Data, 0, MAXOBS * sizeof(obsbd_t));
            free(Cmr->T4Data);
            Cmr->T4Data = NULL;
        }

        memset(Cmr, 0, sizeof(cmr_t));
        free(Cmr);
        Raw->rcv_data = NULL;
    }
}

/* init_cmr = Initialize CMR dependent private storage */
EXPORT int init_cmr(raw_t *Raw)
{
    cmr_t *Cmr = NULL;
    obsr_t *RoverObservables = NULL;
    obsbd_t *T4Data = NULL;
    unsigned char *MessageBuffer = NULL, *Buffer = NULL;

    if (Raw->format != STRFMT_CMR)
        return 0;

    if (!(Cmr = (cmr_t*) calloc(1, sizeof(cmr_t))))
    {
        tracet(0, "CMR: init_cmr(); unable to allocate CMR dependent private data structure.\n");
        return 0;
    }
    Raw->rcv_data = (void*) Cmr;
    
    if (!(Buffer = (unsigned char*) calloc(BUFFER_LENGTH, sizeof(unsigned char))))
    {
        tracet(0, "CMR: init_cmr(); unable to allocate CMR+ message buffer.\n");
        free_cmr(Raw);
        return 0;
    }
    Cmr->Buffer = Buffer;

    if (!(MessageBuffer = (unsigned char*) calloc(MESSAGEBUFFER_LENGTH, sizeof(unsigned char))))
    {
        tracet(0, "CMR: init_cmr(); unable to allocate CMR message buffer.\n");
        free_cmr(Raw);
        return 0;
    }
    Cmr->MessageBuffer = MessageBuffer;

    if (!(RoverObservables = (obsr_t*) calloc(MAXSAT, sizeof(obsr_t))))
    {
        tracet(0, "CMR: init_cmr(); unable to allocate rover observables table.\n");
        free_cmr(Raw);
        return 0;
    }
    Cmr->RoverObservables = RoverObservables;

    if (!(T4Data = (obsbd_t*) calloc(MAXOBS, sizeof(obsbd_t))))
    {
        tracet(0, "CMR: init_cmr(); unable to allocate high speed GPS observations reference table.\n");
        free_cmr(Raw);
        return 0;
    }
    Cmr->T4Data = T4Data;

    return 1;
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
EXPORT int input_cmr(raw_t *Raw, unsigned char Data)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *MessageBuffer = Cmr->MessageBuffer;
    int Ret;

    /* If no current message */
    if (!Cmr->MessageBytes)
    {   
        /* Find something that looks like a message */
        if (SyncMessage(Cmr, Data))
        {
            /* Found one */
            Cmr->MessageLength = 4 + (unsigned char) MessageBuffer[3] + 2; /* 4 (header) + length + 2 (trailer) */
            Cmr->MessageBytes = 4; /* We now have four bytes in the stream buffer */
        }

        /* Continue reading the rest of the message from the stream */
        return CheckMessageFlags(Raw);
    }

    /* Store the next byte of the message */
    MessageBuffer[Cmr->MessageBytes++] = Data;

    /*
    | Keep storing bytes into the current message
    | until we have what we think are all of them.
    */
    if (Cmr->MessageBytes < Cmr->MessageLength)
        return CheckMessageFlags(Raw);

    /*
    | At this point we think have an entire message.
    | The prospective message must end with an ETX.
    */
    if (MessageBuffer[Cmr->MessageLength-1] != ETX)
    {
        tracet(2, "CMR: Message did not end with an ETX character. Some data lost.\n");
        Cmr->MessageBytes = 0;
        return CheckMessageFlags(Raw);
    }

    /*
    | We do indeed have an entire message.
    | Check the message checksum.
    */
    if (!CheckMessageChecksum(MessageBuffer))
    {
        tracet(2, "CMR: Message checksum failure. Message discarded.\n");
        Cmr->MessageBytes = 0;
        return CheckMessageFlags(Raw);
    }

    /* For the RTK monitor */
    if (Raw->outtype)
        sprintf(Raw->msgtype, "CMR: 0x%02X (%4d)", MessageBuffer[2], Cmr->MessageLength);

    StatusReport(Raw);

    /* If this is a CMR message, then decode it */
    if (MessageBuffer[2] == CMR)
    {
        Ret = DecodeCmr(Raw);
        Cmr->MessageBytes = 0;
        return Ret;
    }

    /* If this is a CMR+ message, then decode it */
    if (MessageBuffer[2] == CMRPLUS) 
    {
        Ret = DecodeCmrPlus(Raw);
        Cmr->MessageBytes = 0;
        return Ret;
    }

    /*
    | If we fall through to here, then the message is not one that we support
    | (and hence we can't really even get here). Dump the message on the floor
    | and continue reading from the stream.
    */
    tracet(2, "CMR: Message is not CMR or CMR+. Message discarded.\n"); 
    Cmr->MessageBytes = 0;

    return CheckMessageFlags(Raw);
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
EXPORT int input_cmrf(raw_t *Raw, FILE *fp)
{
    int i, Data, Ret;

    for (i = 0; i < 4096; i++)
    {
        if ((Data = fgetc(fp)) == EOF) return -2;
        if ((Ret = input_cmr(Raw, (unsigned char) Data))) return Ret;
    }

    return 0; /* return at every 4k bytes */
}

/* message_cmr - Return message from the base */
EXPORT void message_cmr(raw_t *Raw, size_t MsgSize, char *Msg)
{
    cmr_t *Cmr;

    if (Raw && (Cmr = Raw->rcv_data))
        strncpy(Msg, (const char*) Cmr->Msg, MsgSize-1);
}

/* message_counts_cmr - Return CMR/CMR+ message counts */
EXPORT void message_counts_cmr(raw_t *Raw,
                               unsigned int Array1Size,
                               unsigned int *Array1,
                               unsigned int Array2Size,
                               unsigned int *Array2 )
{
    cmr_t *Cmr;
    unsigned int n;

    if (Raw && (Cmr = Raw->rcv_data))
    {
        /* CMR message counts */
        for (n = 0; n < Array1Size; n++)
            Array1[n] = 0;

        if (Array1Size > (sizeof(Cmr->Nmsg1) / sizeof(unsigned int)))
            Array1Size = sizeof(Cmr->Nmsg1) / sizeof(unsigned int);

        for (n = 0; n < Array1Size; n++)
            Array1[n] = Cmr->Nmsg1[n];

        /* CMR+ message counts */
        for (n = 0; n < Array2Size; n++)
            Array2[n] = 0;

        if (Array2Size > (sizeof(Cmr->Nmsg1) / sizeof(unsigned int)))
            Array2Size = sizeof(Cmr->Nmsg2) / sizeof(unsigned int);

        for (n = 0; n < Array2Size; n++)
            Array2[n] = Cmr->Nmsg2[n];

    }
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
extern int update_cmr(raw_t *Raw, rtksvr_t *Svr, obs_t *obs)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    obsr_t *RoverObsTable = (obsr_t*) Cmr->RoverObservables;
    obsd_t *r; int n; unsigned char Sat;

    Cmr->Svr = Svr;

    for (n = 0; (n < obs->n) && (n < MAXOBS); n++)
    {
        r = &obs->data[n];
        Sat = r->sat;

        if ((Sat < MAXSAT) && (r->rcv != 2) && (timediff(r->time, RoverObsTable[Sat].Time) > 0))
        {
            RoverObsTable[Sat].Time = r->time;
            RoverObsTable[Sat].P = r->P[0];
            RoverObsTable[Sat].Valid = TRUE;
        }
    }

    return 0;
}

/*
| Private functions (in alphabetical order):
*/

/* AntennaNumberToName - Lookup antenna name by antenna number */
static const char *AntennaNumberToName(unsigned short Number)
{
    int i, j, k, n;

    /* Binary search */
    for (i = 0, j = (sizeof(AntennasTable) / sizeof(ant_t)) - 1; i < j;)
    {
        k = (i + j) / 2;
        n = AntennasTable[k].Number;
        if (n == Number)
            return AntennasTable[k].Name;
        else if (n < Number)
            i = k + 1;
        else
            j = k;
    }

    return "UNKNOWN EXT     NONE";
}

/* CheckCmrFlags - Check the CMR type 1 and 2 flags */
static void CheckCmrFlags(cmr_t *Cmr, unsigned char *p)
{
    unsigned int Flags = ubitn(p+1,0,5);

    if (Flags & M_CFLAG_LOW_BATTERY)
        Cmr->CurrentMessages |= M_MFLAG_LOWBATMSG1;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWBATMSG1;

    if (Flags & M_CFLAG_LOW_MEMORY)
        Cmr->CurrentMessages |= M_MFLAG_LOWMEMMSG1;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWMEMMSG1;

    if (!(Flags & M_CFLAG_L2ENABLE))
        Cmr->CurrentMessages |= M_MFLAG_NOL2MSG1;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_NOL2MSG1;
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
static int CheckMessageChecksum(unsigned char *MessageBuffer)
{
    unsigned char Checksum = 0;
    unsigned char *p = &MessageBuffer[1];      /* Starting with status */
    unsigned int Length = MessageBuffer[3] + 3;/* status, type, length, data */

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
static int CheckMessageFlags(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    char Msg[128] = {0};

    if (Cmr->CurrentMessages != Cmr->PreviousMessages)
    {
        if (Cmr->CurrentMessages & (M_MFLAG_LOWBATMSG1|M_MFLAG_LOWBATMSG2|M_MFLAG_LOWBATMSG3))
            strcat(Msg, "Low battery at the base");
        
        if (Cmr->CurrentMessages & (M_MFLAG_LOWMEMMSG1|M_MFLAG_LOWMEMMSG2|M_MFLAG_LOWMEMMSG3))
        {
            if (strlen(Msg)) strcat(Msg,", ");
            strcat(Msg, "Low memory at the base");
        }

        if (Cmr->CurrentMessages & (M_MFLAG_NOL2MSG1|M_MFLAG_NOL2MSG2))
        {
            if (strlen(Msg)) strcat(Msg, ", ");
            strcat(Msg, "L2 disabled at the base");
        }

        if (strlen(Msg)) strcat(Msg, ".");

        strncpy(Cmr->Msg, (const char*) Msg, sizeof(Cmr->Msg) - 1);
        tracet(2, "CMR: %s\n", Msg);
    }

    Cmr->PreviousMessages = Cmr->CurrentMessages;

    return 0;
}

/* CheckStation - Check the Station ID number */
static int CheckStation(raw_t *Raw, int StationID)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    int ID; char *p;

    /* If an explicit Station ID has been specified, then enforce it */
    if ((p = strstr(Raw->opt, "-STA=")) && sscanf(p, "-STA=%d", &ID) == 1)
    {
        if (StationID != ID)
        {
            tracet(2, "CMR: Message with wrong Base Station ID (%d) ignored.\n", StationID);
            return 0;
        }
    }

    /*
    | We're accepting any Station ID.
    | Let them know what it is and if it changes on them.
    */
    if (!Raw->staid)
        tracet(2, "CMR: Base Station ID set to %d.\n", StationID);
    else if (Raw->staid != StationID)
        tracet(2, "CMR: Base Station ID changed from %d to %d.\n", Raw->staid, StationID);

    Raw->staid = StationID;

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
static int DecodeCmr(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    int Ret = 0;
    char *Type_s = NULL;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    unsigned int Type = ubitn(p+1,5,3), Version = ubitn(p,5,3);

    if (GtimeToDouble(Raw->time) == 0.0)
        Raw->time = utc2gpst(timeget());

    if (Type < (sizeof(CMRTable) / sizeof(char*)))
        Type_s = (char*) CMRTable[Type];
   
    if (!Type_s)
        Type_s = "Unknown";
   
    tracet(3, "CMR: Trimble Packet Type=0x93 (CMR), CMR Type=%u (%s), CMR Version=%u, Length=%d.\n", Type, Type_s, Version, Cmr->MessageLength);

    /*
    | We support version 4 and below for all messages.
    */
    if (Version > 4) 
    {
        tracet(2, "CMR: Unsupported CMR type %u message version: %u\n", Type, Version);
        return -1;
    }

    /* Decode (or possibly ignore) the message */
    switch (Type)
    {
    case CMR_TYPE_0:
        Ret = DecodeCmrType0(Raw);
        break;
    case CMR_TYPE_1:
        Ret = DecodeCmrType1(Raw);
        break;
    case CMR_TYPE_2:
        Ret = DecodeCmrType2(Raw);
        break;
    case CMR_TYPE_3:
        Ret = DecodeCmrType3(Raw);
        break;
    case CMR_TYPE_4:
        Ret = DecodeCmrType4(Raw);
        break;
    default:
        tracet(2, "CMR: Unsupported CMR message type %u ignored.\n", Type);      
    }

    if (Type < (sizeof(Cmr->Nmsg1) / sizeof(unsigned int)))
        Cmr->Nmsg1[Type]++;

    return Ret;
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
| We can't process CMR+ messages immediately as they are received
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
static int DecodeCmrPlus(raw_t *Raw)
{
    int Page, Pages, Ret = 0;
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *Buffer = (unsigned char*) Cmr->Buffer;
    int StationID; unsigned int Length = Cmr->MessageBuffer[3] - 3;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];

    if (GtimeToDouble(Raw->time) == 0.0)
        Raw->time = utc2gpst(timeget());

    StationID = *p++;
    Page      = *p++;
    Pages     = *p++;

    tracet(3, "CMR: Trimble Packet Type=0x94 (CMR+), Base Station=%d, Page=%d of %d.\n", StationID, Page, Pages);

    if (!CheckStation(Raw, StationID))
        return 0;

    if (((Page == Pages) && Cmr->BufferBytes) || ((Page == 0) && (Pages == 0)))
    {
        /*
        | It's the last page or the only page. Either way process it.
        | But first check for buffer overflow.
        */
        if ((Page != 0) && ((Cmr->BufferBytes + Length) > BUFFER_LENGTH))
        {
            tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
            memset(Buffer, 0, BUFFER_LENGTH);
            Cmr->BufferBytes = 0;
            Cmr->Page = 0;
            return 0; 
        }

        memcpy(Buffer + Cmr->BufferBytes, p, Length);
        Cmr->BufferBytes += Length;
        Cmr->Page = 0;

        Ret = DecodeBuffer(Raw);
    }
    else if (Page == 0)
    {
        /*
        | Cool it's page zero. Clear the buffer and add it to the buffer.
        | But first check for buffer overflow.
        */
        memset(Buffer, 0, BUFFER_LENGTH);
        if (Length > BUFFER_LENGTH)
        {
            tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
            Cmr->BufferBytes = 0;
            Cmr->Page = 0;
            return 0; 
        }

        memcpy(Buffer, p, Length);
        Cmr->BufferBytes = Length;
        Cmr->Page = 0;
    }
    else if (Cmr->BufferBytes)
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
        if (Page == (Cmr->Page + 1))
        {
            /* But first check for buffer overflow */
            if ((Cmr->BufferBytes + Length) > BUFFER_LENGTH)
            {
                tracet(2, "CMR: Buffer would overflow. %d CMR+ messages discarded.\n", Pages+1);
                memset(Buffer, 0, BUFFER_LENGTH);
                Cmr->BufferBytes = 0;
                Cmr->Page = 0;
                return 0; 
            }

            memcpy(Buffer + Cmr->BufferBytes, p, Length);
            Cmr->BufferBytes += Length;
            Cmr->Page = Page;
        }
        else
        {
            memset(Buffer, 0, BUFFER_LENGTH);
            Cmr->BufferBytes = 0;
            Cmr->Page = 0;
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
        Cmr->Page = 0;
    }

    return Ret;
}

/*
| DecodeBuffer - Decode a set of buffered CMR+ messages 
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
static int DecodeBuffer(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *Buffer = (unsigned char*) Cmr->Buffer;
    char *Type_s = NULL;
    int BufferBytes = Cmr->BufferBytes;
    unsigned int Type;
    size_t Length;

    while (BufferBytes > 0)
    {
        Type = Buffer[0];
        Length = Buffer[1];

        if (Type < (sizeof(CMRplusTable) / sizeof(char*)))
            Type_s = (char*) CMRplusTable[Type];

        if (!Type_s)
            Type_s = "Unknown";

        tracet(3, "CMR: CMR+ Message type=%u (%s), Length=%u.\n", Type, Type_s, Length);

        switch (Type)
        {
        case CMRPLUS_TYPE_1:
            SetStationInfo(Raw, &Buffer[2]);
            break;
        case CMRPLUS_TYPE_2:
            SetStationCoordinates(Raw, &Buffer[2]);
            break;
        case CMRPLUS_TYPE_3:
            SetStationDescription(Raw, &Buffer[2], Length-2);
            break;
        default:
            tracet(2, "CMR: Unsupported CMR+ message type %u ignored.\n", Type);  
        }

        if (Type < (sizeof(Cmr->Nmsg2) / sizeof(unsigned int)))
            Cmr->Nmsg2[Type]++;

        Buffer += Length;
        BufferBytes -= Length;
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
static int DecodeCmrType0(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    unsigned int L1Flags, L2Flags, nsat = ubitn(p+1,0,5), Slot;
    gtime_t CmrTime = CmrTimeToGtime(ubitn(p+4,6,18));
    obsb_t Obs; obsbd_t *b;
    int Prn, Sat, StationID = (int) ubitn(p,0,5);

    if (!CheckStation(Raw, StationID))
        return 0;

    memset(&Obs, 0, sizeof(Obs));
    Obs.Time = CmrTime;
    Obs.Type = CMR_TYPE_0;

    /* Position past the header */
    p += 6;

    for (Slot = 0; (Slot < nsat) && (Slot < MAXOBS); Slot++)
    {
        b = &Obs.Data[Obs.n];
        b->Slot = Slot;

        if (!(Prn = ubitn(p,3,5))) Prn = 32;
        L1Flags = ubitn(p,0,3);
        b->P[0] = (ubitn(p+3,0,24) / 8.0) * L1_WAVELENGTH;
        if (L1Flags & M_L1_PHASE_VALID)
            b->L[0] = sbitn(p+6,4,20) / 256.0;
        b->Code[0] = (L1Flags & M_L1_PCODE) ? CODE_L1P : CODE_L1C;
        b->SNR[0] = 28+(ubitn(p+6,0,4)*2);
        b->Slip[0] = ubitn(p+7,0,8);
 
        /* Position past the L1 observables block */
        p += 8;

        if (L1Flags & M_L1_L2_FOLLOWS)
        {
            L2Flags = ubitn(p,3,5);

            if ((L2Flags & M_L2_CODE_AVAILABLE) && (L2Flags & M_L2_CODE_VALID))
            {
                b->P[1] = sbitn(p+2,0,16) / 100.0;
                b->Code[1] = (L2Flags & M_L2_WCODE) ? CODE_L2W : CODE_L2P;
                if (L2Flags & M_L2_WCODE)
                    b->LLI[1] |= 4; /* Tracking encrypted code */
            }

            if ((L2Flags & M_L2_PHASE_VALID) && (L2Flags & M_L2_PHASE_FULL))
                b->L[1] = sbitn(p+5,4,20) / 256.0;

            b->SNR[1] = 28+(ubitn(p+5,0,4)*2);
            b->Slip[1] = ubitn(p+6,0,8);

            /* Position past the L2 observables block */
            p += 7;
        }

        if (!(b->Sat = satno(SYS_GPS, Prn)))
        {
            tracet(1, "CMR: GPS satellite number error, PRN=%d.\n", Prn);
            continue;
        }

        Sat = b->Sat - 1;
        if (Cmr->SlipV[Sat][0] && (Cmr->SlipC[Sat][0] != b->Slip[0])) b->LLI[0] |= 1;
        if (Cmr->SlipV[Sat][1] && (Cmr->SlipC[Sat][1] != b->Slip[1])) b->LLI[1] |= 1;
        Cmr->SlipC[Sat][0] = b->Slip[0];
        Cmr->SlipC[Sat][1] = b->Slip[1];
        Cmr->SlipV[Sat][0] = TRUE;
        Cmr->SlipV[Sat][1] = TRUE;

        Obs.n++;
    }

    return (Obs.n > 0) ? OutputCmrObs(Raw, &Obs) : 0;
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
static int DecodeCmrType1(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    int StationID = (int) ubitn(p, 0, 5);

    if (!CheckStation(Raw, StationID))
        return 0;

    CheckCmrFlags(Cmr, p);

    /* Position past the header */
    p += 6;

    SetStationCoordinates(Raw, p);
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
static int DecodeCmrType2(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    unsigned int Length; int StationID = (int) ubitn(p, 0, 5);

    if (!CheckStation(Raw, StationID))
        return 0;

    CheckCmrFlags(Cmr, p);

    /*
    | Position past the header.
    | Fetch the length.
    | Position past the length.
    */
    p += 6;
    Length = *p;
    p++;

    SetStationDescription(Raw, p, Length);

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
static int DecodeCmrType3(raw_t *Raw)
{
    double L1WaveLength;
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    rtksvr_t *Svr = Cmr->Svr;
    nav_t *Nav = (Svr) ? &Svr->nav : &Raw->nav;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    gtime_t CmrTime = utc2gpst(CmrTimeToGtime(ubitn(p+4,6,18)));
    unsigned int L1Flags, L2Flags, nsat = ubitn(p+1,0,5), Slot; 
    obsb_t Obs; obsbd_t *b;
    int Prn, Sat, StationID = (int) ubitn(p,0,5);

    /* ### NEEDS ALPHA TESTING BY SOMEONE WITH APPROPRIATE RECEIVERS ### */
    tracet(2, "CMR: WARNING: CMR type 3 (GLONASS) support is untested.\n");      

    if (!CheckStation(Raw, StationID))
        return 0;

    memset(&Obs, 0, sizeof(Obs));
    Obs.Time = CmrTime;
    Obs.Type = CMR_TYPE_3;

    /* Position past the header */
    p += 6;

    for (Slot = 0; (Slot < nsat) && (Slot < MAXOBS); Slot++)
    {
        b = &Obs.Data[Obs.n];
        memset(b, 0, sizeof(obsd_t));
        b->Slot = Slot;

        if (!(Prn = ubitn(p,3,5))) Prn = 32;
        L1Flags = ubitn(p,0,3);
        b->P[0] = ubitn(p+3,0,24) / 8.0;
        if (L1Flags & M_L1_PHASE_VALID)
            b->L[0] = sbitn(p+6,4,20) / 256.0;
        b->Code[0] = (L1Flags & M_L1_PCODE) ? CODE_L1P : CODE_L1C;
        b->SNR[0] = SnrTable[ubitn(p+6,0,4)][0];
        b->Slip[0] = ubitn(p+7,0,8);

        /* Position past the L1 observables block */
        p += 8;

        if (L1Flags & M_L1_L2_FOLLOWS)
        {
            L2Flags = ubitn(p,3,5);

            if ((L2Flags & M_L2_CODE_AVAILABLE) && (L2Flags & M_L2_CODE_VALID))
            {
                b->P[1] = sbitn(p+2,0,16) / 100.0;
                b->Code[1] = (L2Flags & M_L2_PCODE) ? CODE_L2C : CODE_L2P;
            }

            if ((L2Flags & M_L2_PHASE_VALID) && (L2Flags & M_L2_PHASE_FULL))
                b->L[1] = sbitn(p+5,4,20) / 256.0;
  
            b->SNR[1] = SnrTable[ubitn(p+5,0,4)][1];
            b->Slip[1] = ubitn(p+6,0,8);

            /* Position past the L2 observables block */
            p += 7;
        }

        if (!(b->Sat = satno(SYS_GLO, Prn)))
        {
            tracet(1, "CMR: GLONASS satellite number error, PRN=%d.\n", Prn);
            continue;
        }

        if ((L1WaveLength = satwavelen(b->Sat, 0, Nav)) == 0.0)
        {
            tracet(0, "CMR: internal error; satwavelen() failure.\n");
            continue;
        }
        b->P[0] *= L1WaveLength;

        Sat = b->Sat - 1;
        if (Cmr->SlipV[Sat][0] && (Cmr->SlipC[Sat][0] != b->Slip[0])) b->LLI[0] |= 1;
        if (Cmr->SlipV[Sat][1] && (Cmr->SlipC[Sat][1] != b->Slip[1])) b->LLI[1] |= 1;
        Cmr->SlipC[Sat][0] = b->Slip[0];
        Cmr->SlipC[Sat][1] = b->Slip[1];
        Cmr->SlipV[Sat][0] = TRUE;
        Cmr->SlipV[Sat][1] = TRUE;

        Obs.n++;
    }
    
    return (Obs.n > 0) ? OutputCmrObs(Raw, &Obs) : 0;
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
| The PRN numbers are not re-transmitted and are assumed to be the same
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
static int DecodeCmrType4(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    obsbd_t *t4 = (obsbd_t*) Cmr->T4Data;
    unsigned char *p = (unsigned char*) &Cmr->MessageBuffer[4];
    gtime_t CmrTime = CmrTimeToGtime(ubitn(p+4,6,10)<<2);
    unsigned int nsat = ubitn(p+1,0,5), Slot; int StationID = (int) ubitn(p,0,5);
    obsb_t Obs; obsbd_t *b;

    if (!CheckStation(Raw, StationID))
        return 0;

    memset(&Obs, 0, sizeof(Obs));
    Obs.Time = CmrTime;
    Obs.Type = CMR_TYPE_4;

    /* Position past the header */
    p += 6;

    for (Slot = 0; (Slot < nsat) && (Slot < MAXOBS); Slot++)
    {
        b = &Obs.Data[Obs.n];
        memset(b, 0, sizeof(obsd_t));
        b->Slot = Slot;
        b->Sat = t4[b->Slot].Sat;

        b->L[0] = sbitn(p+2,0,24);
        b->L[0] = (b->L[0] == 8388608.0) ? 0.0 : b->L[0] / 256.0;

        /*
        | Position past the L1 observables block. 24 bits for an L1 carrier
        | phase delta plus another 16 bits for something called "IONO".
        */
        p += 5;

        if (!b->Sat || (b->L[0] == 0.0))
            continue;

        Obs.n++;
    }

    return (Obs.n > 0) ? OutputCmrObs(Raw, &Obs) : 0;
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
static int OutputCmrObs(raw_t *Raw, obsb_t *Obs)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    rtksvr_t *Svr = Cmr->Svr;
    obsr_t *r, *RoverObsTable = (obsr_t*) Cmr->RoverObservables;
    obsbd_t *b; int n, Ret = 0; unsigned char Sat;
    double WindowSize = (Obs->Type == CMR_TYPE_4) ? 4.0 : 240.0;
    gtime_t ObsTime;

    Raw->obs.n = 0;

    if (Svr && RoverObsTable)
    {
        for (n = 0; !(Ret < 0) && (n < Obs->n) && (Raw->obs.n < MAXOBS); n++)
        {
            b = &Obs->Data[n];
            Sat = b->Sat;
            r = &RoverObsTable[Sat];

            if (r->Valid)
            {
                ObsTime = ReferenceCmrTime(Obs->Time, r->Time, WindowSize);
                if (fabs(timediff(r->Time, ObsTime)) < MAXTIMEDIFF)
                    Ret = ReferenceCmrObs(Raw, ObsTime, Obs->Type, r->P, b);
            }
        }
    }
    else
    {
        /* Throw RTKCONV and CONVBIN a bone */
        for (n = 0; !(Ret < 0) && (n < Obs->n); n++)
            Ret = ReferenceCmrObs(Raw, Obs->Time, Obs->Type, 0.0, &Obs->Data[n]);
    }

    if (Raw->obs.n > 0)
    {
        tracet(2, "CMR: Base observations referenced and output:\n");
        traceobs(2, Raw->obs.data, Raw->obs.n);
    }

    return (Ret < 0) ? Ret : (Raw->obs.n > 0);
}

/* Reference and output a single CMR base observation */
static int ReferenceCmrObs(raw_t *Raw, gtime_t Time, unsigned char Type, double P0, obsbd_t *b)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    obsbd_t *t4 = (obsbd_t*) Cmr->T4Data;
    rtksvr_t *Svr = Cmr->Svr;
    nav_t *Nav = (Svr) ? &Svr->nav : &Raw->nav;
    obsd_t *obs = &Raw->obs.data[Raw->obs.n];
    double L0, L1WaveLength = L1_WAVELENGTH, L2WaveLength = L2_WAVELENGTH;

    if (Type == CMR_TYPE_3)
    {
        if ((L1WaveLength = satwavelen(b->Sat, 0, Nav) == 0.0) ||
            (L2WaveLength = satwavelen(b->Sat, 1, Nav) == 0.0))
        {
            tracet(0, "CMR: internal error; satwavelen() failure.\n");
            return -1;
        }
    }

    /* Reference the CMR base observables */
    if ((Type != CMR_TYPE_4) && (P0 != 0.0))
    {
        b->P[0] += P0 - fmod(P0, RANGE_MS);
        if (b->L[0] != 0.0)
            b->L[0] += b->P[0] / L1WaveLength;
        if (b->P[1] != 0.0)
            b->P[1] += b->P[0];
        if (b->L[1] != 0.0)
            b->L[1] += b->P[0] / L2WaveLength;
    }

    if (Type == CMR_TYPE_0)
        memcpy(&t4[b->Slot], b, sizeof(obsbd_t));
    
    if (Type == CMR_TYPE_4)
    {
        L0 = (Svr) ? b->L[0] + t4[b->Slot].L[0] : b->L[0];
        memcpy(b, &t4[b->Slot], sizeof(obsbd_t));
        b->L[0] = L0;
#if 0
        b->Code[0] = CODE_L1L;
        b->Code[1] = CODE_NONE;
#endif
    }

    memset(obs, 0, sizeof(obsd_t));

    obs->rcv     = 2; /* Note that we don't accept rcv=2 in update_cmr() as being from a rover */
    obs->time    = Time;
    obs->P[0]    = b->P[0];
    obs->P[1]    = b->P[1];
    obs->L[0]    = b->L[0];
    obs->L[1]    = b->L[1];
    obs->sat     = b->Sat;
    obs->code[0] = b->Code[0];
    obs->code[1] = b->Code[1];
    obs->SNR[0]  = SNRATIO(b->SNR[0]);
    obs->SNR[1]  = SNRATIO(b->SNR[1]);
    obs->LLI[0]  = b->LLI[0];
    obs->LLI[1]  = b->LLI[1];

    Raw->time = Time;
    Raw->obs.n++;

    return 1;
}

/* ReceiverNumberToName - Lookup receiver name by receiver number */
static const char *ReceiverNumberToName(unsigned short Number)
{
    int i, j, k, n;

    /* Binary search */
    for (i = 0, j = (sizeof(ReceiversTable) / sizeof(rcv_t)) - 1; i < j;)
    {
        k = (i + j) / 2;
        n = ReceiversTable[k].Number;
        if (n == Number)
            return ReceiversTable[k].Name;
        else if (n < Number)
            i = k + 1;
        else
            j = k;
    }

    return "";
}

/* ReferenceCmrTime- Reference the CMR base time to the rover time */
static gtime_t ReferenceCmrTime(gtime_t CmrTime, gtime_t RoverTime, double WindowSize)
{
    double modtime = GtimeToDouble(RoverTime);
    return DoubleToGtime((modtime - fmod(modtime, WindowSize)) + GtimeToDouble(CmrTime));
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
static void SetStationCoordinates(raw_t *Raw, unsigned char *p)
{
    sta_t *sta = &Raw->sta;
    sta->pos[0]  = ((sbitn(p+3, 0,32)*4.0)+ubitn(p+4, 6,2))*0.001;
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
static void SetStationDescription(raw_t *Raw, unsigned char *p, size_t Length)
{
    /* Set the station name with any leading or trailing nulls & white space trimmed off */
    sta_t *sta = &Raw->sta;
    if (Length > 8) Length = 8;
    memset(sta->name, 0, sizeof(sta->name));
    TrimCopy(sta->name, sizeof(sta->name) - 1, (char*) p, Length);

    tracet(3, "CMR: Reference station decription received. STATION=\"%s\"\n", sta->name);
}

/* SetStationInfo - Set miscellaneous base station information */
static void SetStationInfo(raw_t *Raw, unsigned char *p)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    sta_t *sta = &Raw->sta;
    unsigned int Flags = ubitn(p+1,0,16);

    memset(sta->rectype, 0, sizeof(sta->rectype));
    strncpy(sta->rectype, ReceiverNumberToName(p[2]), sizeof(sta->rectype)-1);
    memset(sta->antdes, 0, sizeof(sta->antdes));
    strncpy(sta->antdes, AntennaNumberToName(p[3]), sizeof(sta->antdes)-1);

    tracet(3, "CMR: Reference station information received. RECEIVER=\"%s\", ANTENNA=\"%s\"\n", sta->rectype, sta->antdes);

    if (Flags & M_PFLAG_LOW_BATTERY)
        Cmr->CurrentMessages |= M_MFLAG_LOWBATMSG2;    
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWBATMSG2;

    if (Flags & M_PFLAG_LOW_MEMORY)
        Cmr->CurrentMessages |= M_MFLAG_LOWMEMMSG2;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWMEMMSG2;

    if (!(Flags & M_PFLAG_L2ENABLE))
        Cmr->CurrentMessages |= M_MFLAG_NOL2MSG2;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_NOL2MSG2;
}

/* StatusReport - Output once a minute base status */
static void StatusReport(raw_t *Raw)
{
    cmr_t *Cmr = (cmr_t*) Raw->rcv_data;
    unsigned char Status = (unsigned char) Cmr->MessageBuffer[1];

    if (Status & M_STATUS_LOW_BATTERY)
        Cmr->CurrentMessages |= M_MFLAG_LOWBATMSG3;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWBATMSG3;

    if (Status & M_STATUS_LOW_MEMORY)
        Cmr->CurrentMessages |= M_MFLAG_LOWMEMMSG3;
    else
        Cmr->CurrentMessages &= ~M_MFLAG_LOWMEMMSG3;
}

/* SyncMessage - Synchronize the CMR data stream to the start of a series of CMR messages */
static int SyncMessage(cmr_t *Cmr, unsigned char Data)
{
    unsigned char Type, *MessageBuffer = Cmr->MessageBuffer;

    MessageBuffer[0] = MessageBuffer[1];
    MessageBuffer[1] = MessageBuffer[2];
    MessageBuffer[2] = MessageBuffer[3];
    MessageBuffer[3] = Data;

    Type = MessageBuffer[2];

    /*
    | Byte 0 must be an STX character.
    | Byte 1 = status byte which we ignore here.
    | Byte 2 = message type which must be CMR (93h) or CMR+ (94h).
    | Byte 3 = data length which must be non-zero for any message we're interested in.
    */
    return ((MessageBuffer[0] == STX) && (Data != 0) && ((Type == CMR) || (Type == CMRPLUS)));
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
