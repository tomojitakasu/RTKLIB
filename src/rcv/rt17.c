/*------------------------------------------------------------------------------
* rt17.c : Trimble RT-17 dependent functions
*
*          Copyright (C) 2016 Daniel A. Cook, All rights reserved.
*
* references:
*     [1] https://github.com/astrodanco/RTKLIB/tree/cmr/src/rcv/rt17.c
*     [2] Trimble, Trimble OEM BD9xx GNSS Receiver Family IDC, version 4.82
*         Revision A, December, 2013
*
* version : $Revision:$ $Date:$
* history : 2014/08/26 1.0  imported from GitHub (ref [1])
*                           modified to get initial week number
*                           modified obs types for raw obs data
*                           function added to output message type
*           2014/09/06 1.1  Remove prehistorical revision history
*                           Remove dead code
*                           Fix len vs. plen typo
*                           Check week/time valid flag in GSOF 16 message
*                           Set time when reading GSOF messages, where possible.
*           2016/06/16 1.2  Refactored
*           2016/07/16 1.3  modified by T.T
*                           raw->strfmt -> raw->format
*                           int free_rt17() -> void free_rt17()
*           2016/07/29 1.4  suppress warning
*           2017/04/11 1.5  (char *) -> (signed char *)
*-----------------------------------------------------------------------------*/

/*
| Trimble real-time binary data stream and file handler functions.
|
| Written in July 2014 by Daniel A. Cook, for inclusion into the RTKLIB library.
| Copyright (C) 2014, 2016 by Daniel A. Cook. All Rights Reserved.
|
| Here we implement four public functions, one for reading Trimble real-time
| binary data streams and another for reading log files containng data in the
| same format, a third to initialize RT17 related memory storage and a fourth
| to free up RT17 related memory storage. This real-time streaming data format,
| sometimes called RT-17, was designed by Trimble for use by Trimble receivers.
| Trimble receivers with the "Real-Time Survey Data" or "Binary Outputs" options
| can output this data. The RT-17 moniker refers to the GPS observables data
| record (record type 0) itself. There is also a GNSS observables data record
| (record type 6), which is referred to as RT-27. We also wish to handle RT-27
| data records, but lack sufficient documentation to do so.
|
| Notes:
|
| To specify receiver dependent options, set raw->opt to the following case
| sensitive option strings separated by spaces.
|
| Receiver dependent options:
|
| -EPHALL : Input all ephemerides
| -WEEK=n : Explicitly set starting GPS week number
|
| Neither the Trimble RT-17 observables packets nor the ION / UTC data packets
| contain the GPS week number. By default the current computer "now" time is
| used to determine the GPS week number. This works well in real time, but
| can be problematic when later converting and/or post processing recorded
| data. When recording data, also enable either GSOF Position & Time (1) or
| GSOF Current Time (16) messages on the same serial port along with the
| RT-17 Real-Time Survey Data output. For best results enable the GSOF
| message(s) and get them streaming prior to enabling the RT-17 messages.
|
| If desired the -WEEK=n option can be specified when converting or post
| processing recorded data to explicitly set the starting GPS week number.
| This option overrides anything and everything, including the current
| computer "now" time and any GSOF or other messages read from the raw
| data stream or recorded file. Note that the GPS week number explicitly
| specified with the -WEEK=n option GPS is automatically incremented when
| and if a subsequent GPS week number rollover occurs in the raw data.
|
| In addition to enabling RT-17 Real-Time Survey Data output, it is very
| helpful to also enable GSOF Position & Time (1) or GSOF Current Time (16)
| messages on the same serial port. This allows the GPS week number to be
| determined without using the current computer "now" time or the -WEEK=n
| option. Although not as important for real-time streaming data use where
| the current computer "now" time can be used to determine the current GPS
| week number, it becomes more important when recording files for later
| conversion and/or post processing.  For best results enable the GSOF
| message(s) and get them streaming prior to enabling the RT-17 messages.
|
| Support is provided for the following Trimble RT-17 packet Types:
|
|         Raw Observation   Satellite        ION/UTC
| Format       Data         Ephemerides      Parameters       GSOF
| ------- ---------------   ---------------- ---------------- ------------
| Trimble 0x57 (RAWDATA)    0x55 (RETSVDATA) 0x55 (RETSVDATA) 1, 16,
| RT-17   Recordtype 0 & 7  Subtype  1       Subtype 3        26, 41                   
|
| When the -WEEK=n option is NOT used, the GPS week number is set from any
| RAWDATA record type 7 or GENOUT (GSOF) 1, 16, 26, 41 records encountered
| in the raw data stream. These messages are only used to obtain the GPS
| WEEK number and are not used for any other purpose.
|
| Support is not provided for the GPS L2C or L5 signals. Those would likely
| require Trimble RT-27 protocol support. Support for that and much more
| could easily be added by anyone with the required RAWDATA record type
| 6 documentation.
|
| For Trimble GPS receivers which are capable of RT-17 binary output, the
| receiver and/or receiver configuration software generally provide several
| RT-17 binary output options:
|
| 1. Compact format aka Concise format
|    (RECOMMENDED)
|
| This option causes the raw satellite data to be streamed in a more compact
| format. The compact format does not include L2 DOPPLER observables.
|
| 2. Expanded format
|
| This is usually the default format if compact format is not enabled. The
| only advantage of this format over compact format is that L2 DOPPLER
| observables are output when used in combination with the Real Time
| Enhancements option. Otherwise this format just consumes more bandwidth
| and/or file space than compact format while offering no other advantages.
|
| 3. Real Time Enhancements, aka Real-time Enhanced format, aka R-T FLAGS
|
| This option adds extra data to the raw satellite data output by the
| receiver. When used in combination with expanded format, L2 DOPPLER
| observables are output. L2 DOPPLER can be used by RTKLIB.
|
| 3. Measurements
|    (REQUIRED)
|
| If your configuration has a measurements option, enable it. Measurements
| are the raw satellite data. If you don't see this option then it is
| implied and enabled by default.
|
| 4. Ephermeris AKA Stream Ephemeris
|    (HIGHLY RECOMMENDED)
|
| This option causes satellite ephemerides and UTC / ION data to be streamed
| along with the raw satellite data. Streamed ephemerides and UTC / ION data
| consume very little extra bandwidth in the stream and/or space in a file.
| In most situations with most applications you will need them as well.
|
| 5. Positions AKA Stream Positions
|    (NOT RECOMMENDED)
|
| Streamed postions are of no use to RTKLIB. They will be ignored. RTKLIB
| computes positions from the raw satellite data. It has no use for the
| receiver's position solutions. Streamed positions also consume
| considerable bandwidth in the stream and/or space in a file.
|
| 6. Positions Only
|    (HIGHLY NOT RECOMMENDED)
|
| Enabling the positions only option causes only positions and nothing else
| to be output, including no raw satellite data and no ephemerides and no
| ION / UTC data.
|
| Design notes:
|
| This source code handles GPS L1/L2 only. RT-17 is GPS. RT27 is GNSS.
| If you have RT27 (RAWDATA 57h record type 6) documentation, please
| forward it to the author.
|
| An RT-17 real-time survey data message is a series of RAWDATA (57h,
| Real-time survey data report) and RETSVDATA (55h, Satellite information
| report) packets.
|
| Each assembled RAWDATA message in an RT-17 packet stream may contain
| any of the following: Compact Format raw satellite measurements, Expanded
| Format raw satellite measurements, a receiver computed position or an
| event mark. Receiver computed positions and event marks are of no
| interest to RTKLIB, therefore we ignore them.
|
| Each RETSVDATA message in an RT-17 packet stream may contain any one
| of the following: SV flags indicating tracking, a GPS Ephemeris, a GPS
| Almanac, ION / UTC Data or an Extended GPS Almanac. Of these only
| the GPS Ephemeris and the ION / UTC Data are of interest to RTKLIB.
| In practice only GPS Ephemeris and ION / UTC Data are transmitted.
| Some receivers can be set to transmit them at regular intervals
| rather than only when they change.
|
| Certain simplifying assumptions are made concerning the way in which
| RAWDATA and GENOUT packets are transmitted in the stream or stored
| into the file.
|
| Therefore it is assumed that:
| 
| 1. RAWDATA and GENOUT packets are never interleaved or interspersed
|    with packets of other types.
|
| 2. The sequence of page frames in a RAWDATA message are transmitted in the
|    stream or stored into a file as packets in order from first to last.
|    RAWDATA page numbers are one based. That is, 1 of n, 2 of n, 3 of n,
|    ..., to 15 of 15 for a total of 15 possible pages. We check for this
|    ordering. RAWDATA messages can therefore reach almost 4K in total
|    length. We check for potential buffer overflows in the input_rt17()
|    function.
|
| 3. The Record Interpretation Flags (RIF) field is repeated within the
|    page frame of every page making up a single RAWDATA message. It is
|    assumed that this is redundant and that the actual value of the record
|    interpretation flags does not change from one page to the next within
|    a single RAWDATA message. We check for this too.
|
| 4. The sequence of pages in a GENOUT message are transmitted in the
|    stream or stored into a file as packets in order from first to last.
|    GENOUT page numbers are zero based. That is, 0 of n, 1 of n, 2 of n,
|    ..., to 255 of 255 for a total of 256 possible pages. We check for
|    this ordering. GENOUT messages can therefore reach almost 64K in
|    total length. Such a large GENOUT message could exceed our maximum
|    buffer size. We check for potential buffer overflows in the
|    input_rt17() function.
|
| This code was tested using RT-17 data output from the following receivers:
|
| 1. Trimble 4000SSI, firmware version 7.32
| 2. Trimble 5700, firmware version 2.32
| 3. Spectra Precision Epoch 25 Base, firmware version 2.32
|
| By convention functions within this source file appear in alphabetical
| order. Public functions appear as a set first (there are only two of
| them), followed by private functions as a set. Because of this, forward
| definitions are required for the private functions. Please keep that
| in mind when making changes to this source file.
|
| References:
|
| 1. Trimble Serial Reference Specification, Version 4.82, Revision A,
|    December 2013. Though not being in any way specific to the BD9xx
|    family of receivers, a handy downloadable copy of this document
|    is contained in the "Trimble OEM BD9xx GNSS Receiver Family ICD"
|    document located at <http://www.trimble.com/OEM_ReceiverHelp/
|    v4.85/en/BinaryInterfaceControlDoc.pdf>
|
| 2. Trimble General Serial Output Format (GSOF)
|    <http://www.trimble.com/OEM_ReceiverHelp/v4.85/en/GSOFmessages_GSOF.html>
|
| 3. ICD-GPS-200C, Interface Control Document, Revision C, 10 October 1993
|    <http://www.gps.gov/technical/icwg/ICD-GPS-200C.pdf>
|
| 4. IS-GPS-200H, Interface Specification, 24 September 2013
|    <http://www.gps.gov/technical/icwg/IS-GPS-200H.pdf>
|
| 5. RTKLIB Version 2.4.2 Manual, April 29 2013
|    <http://www.rtklib.com/prog/manual_2.4.2.pdf>
*/

/* Included files: */
#include "rtklib.h"

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

/* Constant definitions: */
#define STX           2         /* Start of packet character */
#define ETX           3         /* End of packet character */
#define GENOUT        0x40      /* General Serial Output Format (GSOF) */
#define RETSVDATA     0x55      /* Satellite information reports */
#define RAWDATA       0x57      /* Position or real-time survey data report */
#define MBUFF_LENGTH  8192      /* Message buffer length */
#define PBUFF_LENGTH  (4+255+2) /* Packet buffer length */

/* Record Interpretation Flags bit masks: */
#define M_CONCISE     M_BIT0    /* Concise format */
#define M_ENHANCED    M_BIT1    /* Enhanced record with real-time flags and IODE information */

/* rt17.flag bit definitions: */
#define M_WEEK_OPTION M_BIT0    /* GPS week number set by WEEK=n option */
#define M_WEEK_EPH    M_BIT1    /* GPS week number set by ephemeris week */
#define M_WEEK_TIME   M_BIT2    /* GPS week set by computer time */
#define M_WEEK_SCAN   M_BIT3    /* WEEK=n option already looked for, no need to do it again */

/* Data conversion macros: */
#define I1(p) (*((signed char*)(p)))    /* One byte signed integer */
#define U1(p) (*((unsigned char*)(p)))  /* One byte unsigned integer */
#define I2(p) ReadI2(p)                 /* Two byte signed integer */
#define U2(p) ReadU2(p)                 /* Two byte unsigned integer */
#define I4(p) ReadI4(p)                 /* Four byte signed integer */
#define U4(p) ReadU4(p)                 /* Four byte unsigned integer */
#define R4(p) ReadR4(p)                 /* IEEE S_FLOAT floating point number */
#define R8(p) ReadR8(p)                 /* IEEE T_FLOAT floating point number */

/* Internal structure definitions. */
typedef union {unsigned short u2; unsigned char c[2];} ENDIAN_TEST; 

/* GENOUT 0x40 message types: */
static const char *GSOFTable[] = {
    /* 00 */ NULL,
    /* 01 */ "Position Time",
    /* 02 */ "Latitude Longitude Height",
    /* 03 */ "ECEF Position",
    /* 04 */ "Local Datum LLH Position",
    /* 05 */ "Local Zone ENU Position",
    /* 06 */ "ECEF Delta",
    /* 07 */ "Tangent Plane Delta",
    /* 08 */ "Velocity Data",
    /* 09 */ "DOP Information",
    /* 10 */ "Clock Information",
    /* 11 */ "Position VCV Information",
    /* 12 */ "Position Sigma Information",
    /* 13 */ "SV Brief Information",
    /* 14 */ "SV Detailed Information",
    /* 15 */ "Receiver Serial Number",
    /* 16 */ "Current Time UTC",
    /* 17 */ NULL,
    /* 18 */ NULL,
    /* 19 */ NULL,
    /* 20 */ NULL,
    /* 21 */ NULL,
    /* 22 */ NULL,
    /* 23 */ NULL,
    /* 24 */ NULL,
    /* 25 */ NULL,
    /* 26 */ "Position Time UTC",
    /* 27 */ "Attitude Information",
    /* 28 */ NULL,
    /* 29 */ NULL,
    /* 30 */ NULL,
    /* 31 */ NULL,
    /* 32 */ NULL,
    /* 33 */ "All SV Brief Information",
    /* 34 */ "All SV Detailed Information",
    /* 35 */ "Received Base Information",
    /* 36 */ NULL,
    /* 37 */ "Battery and Memory Information",
    /* 38 */ NULL,
    /* 39 */ NULL,
    /* 40 */ "L-Band Status Information",
    /* 41 */ "Base Position and Quality Indicator"
};

/* RAWDATA 0x57 message types: */
static const char *RawdataTable[] = {
    /* 00 */ "Real-time GPS Survey Data, type 17",
    /* 01 */ "Position Record, type 11",
    /* 02 */ "Event Mark",
    /* 03 */ NULL,
    /* 04 */ NULL,
    /* 05 */ NULL,
    /* 06 */ "Real-time GNSS Survey Data, type 27",
    /* 07 */ "Enhanced Position Record, type 29"
};

/* RETSVDATA 0x55 message types: */
static const char *RetsvdataTable[] = {
    /* 00 */ "SV Flags",
    /* 01 */ "GPS Ephemeris",
    /* 02 */ "GPS Almanac",
    /* 03 */ "ION / UTC Data",
    /* 04 */ "Disable Satellite, depreciated",
    /* 05 */ "Enable Satellite, depreciated",
    /* 06 */ NULL,
    /* 07 */ "Extended GPS Almanac",
    /* 08 */ "GLONASS Almanac",
    /* 09 */ "GLONASS Ephemeris",
    /* 10 */ NULL,
    /* 11 */ "Galileo Ephemeris",
    /* 12 */ "Galileo Almanac",
    /* 13 */ NULL,
    /* 14 */ "QZSS Ephemeris",
    /* 15 */ NULL,
    /* 16 */ "QZSS Almanac",
    /* 17 */ NULL,
    /* 18 */ NULL,
    /* 19 */ NULL,
    /* 20 */ "SV Flags",
    /* 21 */ "BeiDou Ephemeris",
    /* 22 */ "BeiDou Almanac"
}; 

/*
| Typedefs.
*/

typedef struct {                    /* RT17 information struct type */
    unsigned char *MessageBuffer;   /* Message buffer */
    unsigned char *PacketBuffer;    /* Packet buffer */
    double        Tow;              /* Receive time of week */
    unsigned int  Flags;            /* Miscellaneous internal flag bits */
    unsigned int  MessageBytes;     /* Number of bytes in message buffer */ 
    unsigned int  MessageLength;    /* Message length (bytes) */
    unsigned int  PacketBytes;      /* How many packet bytes have been read so far */
    unsigned int  PacketLength;     /* Total size of packet to be read */
    unsigned int  Page;             /* Last page number */
    unsigned int  Reply;            /* Current reply number */
    int           Week;             /* GPS week number */
} rt17_t;

/*
| Internal private function forward declarations (in alphabetical order):
*/
static int CheckPacketChecksum(unsigned char *PacketBuffer);
static void ClearMessageBuffer(rt17_t *rt17);
static void ClearPacketBuffer(rt17_t *rt17);
static int DecodeBeidouEphemeris(raw_t *Raw);
static int DecodeGalileoEphemeris(raw_t *Raw);
static int DecodeGLONASSEphemeris(raw_t *Raw);
static int DecodeGPSEphemeris(raw_t *Raw);
static int DecodeGSOF(raw_t *Raw);
static int DecodeGSOF1(raw_t *Raw, unsigned char *p);
static int DecodeGSOF3(raw_t *Raw, unsigned char *p);
static int DecodeGSOF15(raw_t *Raw, unsigned char *p);
static int DecodeGSOF16(raw_t *Raw, unsigned char *p);
static int DecodeGSOF26(raw_t *Raw, unsigned char *p);
static int DecodeGSOF41(raw_t *Raw, unsigned char *p);
static int DecodeIONAndUTCData(raw_t *Raw);
static int DecodeQZSSEphemeris(raw_t *Raw);
static int DecodeRawdata(raw_t *Raw);
static int DecodeRetsvdata(raw_t *Raw);
static int DecodeType17(raw_t *Raw, unsigned int rif);
static int DecodeType29(raw_t *Raw);
static int GetWeek(raw_t *Raw, double tow);
static short ReadI2(unsigned char *p);
static int ReadI4(unsigned char *p);
static float ReadR4(unsigned char *p);
static double ReadR8(unsigned char *p);
static unsigned short ReadU2(unsigned char *p);
static unsigned int ReadU4(unsigned char *p);
static void SetWeek(raw_t *Raw, int Week, double tow);
static int SyncPacket(rt17_t *rt17, unsigned char Data);
static void UnwrapRawdata(rt17_t *rt17, unsigned int *rif);
static void UnwrapGenout(rt17_t *rt17);

/* Public functions (in alphabetical order): */

/* free_rt17 - Free up RT17 dependent private storage */
EXPORT void free_rt17(raw_t *Raw)
{
    rt17_t *rt17 = NULL;
    
    if (Raw->format != STRFMT_RT17)
        return;
 
    if ((rt17 = (rt17_t*) Raw->rcv_data))
    {
        if (rt17->MessageBuffer)
        {
            memset(rt17->MessageBuffer, 0, MBUFF_LENGTH);
            free(rt17->MessageBuffer);
            rt17->MessageBuffer = NULL;
        }

        if (rt17->PacketBuffer)
        {
            memset(rt17->PacketBuffer, 0, PBUFF_LENGTH);
            free(rt17->PacketBuffer);
            rt17->PacketBuffer = NULL;
        }

        memset(rt17, 0, sizeof(rt17_t));
        free(rt17);
        Raw->rcv_data = NULL;
    }
}

/* init_rt17 = Initialize RT17 dependent private storage */
EXPORT int init_rt17(raw_t *Raw)
{
	rt17_t *rt17 = NULL;
    unsigned char *MessageBuffer = NULL, *PacketBuffer = NULL;

    if (Raw->format != STRFMT_RT17)
       return 0;
 
    if (!(rt17 = (rt17_t*) calloc(1, sizeof(rt17_t))))
    {
        tracet(0, "RT17: unable to allocate RT17 dependent private data structure.\n");
        return 0;
    }
    Raw->rcv_data = (void*) rt17;
    
    if (!(MessageBuffer = (unsigned char*) calloc(MBUFF_LENGTH, sizeof(unsigned char))))
    {
        tracet(0, "RT17: unable to allocate RT17 message buffer.\n");
        free_rt17(Raw);
        return 0;
    }
    rt17->MessageBuffer = MessageBuffer;

    if (!(PacketBuffer = (unsigned char*) calloc(PBUFF_LENGTH, sizeof(unsigned char))))
    {
        tracet(0, "RT17: unable to allocate RT17 packet buffer.\n");
        free_rt17(Raw);
        return 0;
    }
    rt17->PacketBuffer = PacketBuffer;

    return 1;
}

/*
| input_rt17 - Read an RT-17 mesasge from a raw data stream 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|  2: input ephemeris
|  9: input ion/utc parameter
|
| Each message begins with a 4-byte header, followed by the bytes of data in the packet,
| and the packet ends with a 2-byte trailer. Byte 3 is set to 0 (00h) when the packet
| contains no data.
*/
EXPORT int input_rt17(raw_t *Raw, unsigned char Data)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *MessageBuffer = rt17->MessageBuffer;
    unsigned char *PacketBuffer = rt17->PacketBuffer;
    unsigned int Page, Pages, Reply;
    int Ret = 0;

    /* If no current packet */
    if (rt17->PacketBytes == 0)
    {   
        /* Find something that looks like a packet. */
        if (SyncPacket(rt17, Data))
        {
            /* Found one. */
            rt17->PacketLength = 4 + PacketBuffer[3] + 2; /* 4 (header) + length + 2 (trailer) */
            rt17->PacketBytes = 4; /* We now have four bytes in the packet buffer */
        }
                
        /* Continue reading the rest of the packet from the stream */
        return 0;
    }

    /* Store the next byte of the packet */
    PacketBuffer[rt17->PacketBytes++] = Data;

    /*
    | Keep storing bytes into the current packet
    | until we have what we think are all of them.
    */
    if (rt17->PacketBytes < rt17->PacketLength)
        return 0;

    /*
    | At this point we think have an entire packet.
    | The prospective packet must end with an ETX.
    */
    if (rt17->PacketBuffer[rt17->PacketLength-1] != ETX)
    {
        tracet(2, "RT17: Prospective packet did not end with an ETX character. Some data lost.\n");
        ClearPacketBuffer(rt17);
        return 0;
    }

    /*
    | We do indeed have an entire packet.
    | Check the packet checksum.
    */
    if (!CheckPacketChecksum(PacketBuffer))
    {
        tracet(2, "RT17: Packet checksum failure. Packet discarded.\n");
        ClearPacketBuffer(rt17);
        return 0;
    }

    if (Raw->outtype)
        sprintf(Raw->msgtype, "RT17 0x%02X (%4d)", PacketBuffer[2], rt17->PacketLength);

    /* If this is a SVDATA packet, then process it immediately */
    if (PacketBuffer[2] == RETSVDATA)
    {
        Ret = DecodeRetsvdata(Raw);
        ClearPacketBuffer(rt17);
        return Ret;
    }
        
    /* Accumulate a sequence of RAWDATA packets (pages) */
    if (PacketBuffer[2] == RAWDATA)
    {
        Page  = PacketBuffer[5] >> 4;
        Pages = PacketBuffer[5] & 15;
        Reply = PacketBuffer[6];

        /*
        | If this is the first RAWDATA packet in a sequence of RAWDATA packets,
        | then make sure it's page one and not a packet somewhere in the middle.
        | If not page one, then skip it and continue reading from the stream
        | until we find one that starts at page one. Otherwise make sure it is
        | a part of the same requence of packets as the last one, that it's
        | page number is in sequence.
        */
        if (rt17->MessageBytes == 0)
        {       
            if (Page != 1)
            {
                tracet(2, "RT17: First RAWDATA packet is not page #1. Packet discarded.\n");
                ClearPacketBuffer(rt17);
                return 0;
            }

            rt17->Reply = PacketBuffer[6];
        }
        else if ((Reply != rt17->Reply) || (Page != (rt17->Page + 1)))
        {
            tracet(2, "RT17: RAWDATA packet sequence number mismatch or page out of order. %u RAWDATA packets discarded.\n", Page);
            ClearMessageBuffer(rt17);
            ClearPacketBuffer(rt17);
            return 0;
        }
        
        /* Check for message buffer overflow */
        if ((rt17->MessageBytes + rt17->PacketBytes) > MBUFF_LENGTH)
        {
            tracet(2, "RT17: Buffer would overflow. %u RAWDATA packets discarded.\n", Page);
            ClearMessageBuffer(rt17);
            ClearPacketBuffer(rt17);
            return 0; 
        }

        memcpy(MessageBuffer + rt17->MessageBytes, PacketBuffer, rt17->PacketBytes);
        rt17->MessageBytes += rt17->PacketBytes;
        rt17->MessageLength += rt17->PacketLength;
        ClearPacketBuffer(rt17);

        if (Page == Pages)
        {
            Ret = DecodeRawdata(Raw);
            ClearMessageBuffer(rt17);
            return Ret;
        }

        rt17->Page = Page;

        return 0;
    }

    /* Accumulate a sequence of GENOUT (GSOF) packets (pages) */
    if (PacketBuffer[2] == GENOUT)
    {
        Reply = PacketBuffer[4];
        Page  = PacketBuffer[5];
        Pages = PacketBuffer[6];

        /*
        | If this is the first GENOUT packet in a sequence of GENOUT packets,
        | then make sure it's page zero and not a packet somewhere in the middle.
        | If not page zero, then skip it and continue reading from the stream
        | until we find one that starts at page zero. Otherwise make sure it is
        | a part of the same requence of packets as the last one, that it's
        | page number is in sequence.
        */
        if (rt17->MessageBytes == 0)
        {       
            if (Page != 0)
            {
                tracet(3, "RT17: First GENOUT packet is not page #0. Packet discarded.\n");
                ClearPacketBuffer(rt17);
                return 0;
            }

            rt17->Reply = PacketBuffer[4];
        }
        else if ((Reply != rt17->Reply) || (Page != (rt17->Page + 1)))
        {
            tracet(2, "RT17: GENOUT packet sequence number mismatch or page out of order. %u GENOUT packets discarded.\n", Page);
            ClearMessageBuffer(rt17);
            ClearPacketBuffer(rt17);
            return 0;
        }
        
        /* Check for message buffer overflow. */
        if ((rt17->MessageBytes + rt17->PacketBytes) > MBUFF_LENGTH)
        {
            tracet(2, "RT17: Buffer would overflow. %u GENOUT packets discarded.\n", Page);
            ClearMessageBuffer(rt17);
            ClearPacketBuffer(rt17);
            return 0; 
        }

        memcpy(MessageBuffer + rt17->MessageBytes, PacketBuffer, rt17->PacketBytes);
        rt17->MessageBytes += rt17->PacketBytes;
        rt17->MessageLength += rt17->PacketLength;
        ClearPacketBuffer(rt17);

        if (Page == Pages)
        {
            Ret = DecodeGSOF(Raw);
            ClearMessageBuffer(rt17);
            return Ret;
        }

        rt17->Page = Page;

        return 0;
    }

    /*
    | If we fall through to here, then the packet is not one that we support
    | (and hence we can't really even get here). Dump the packet on the floor
    | and continue reading from the stream.
    */
    tracet(2, "RT17: Packet is not GENOUT, RAWDATA or RETSVDATA. Packet discarded.\n"); 
    ClearPacketBuffer(rt17);
    return 0;
}

/*
| input_rt17f - Read an RT-17 mesasge from a file 
|
| Returns:
|
| -2: End of file (EOF)
| -1: error message
|  0: no message
|  1: input observation data
|  2: input ephemeris
|  9: input ion/utc parameter
*/
EXPORT int input_rt17f(raw_t *Raw, FILE *fp)
{
    int i, Data, Ret;
    
    for (i = 0; i < 4096; i++)
    {
	if ((Data = fgetc(fp)) == EOF) return -2;
	    if ((Ret = input_rt17(Raw, (unsigned char) Data))) return Ret;
    }

    return 0; /* return at every 4k bytes */
}

/*
| Private functions (in alphabetical order):
*/

/*
| CheckPacketChecksum - Check the packet checksum
|
| The checksum is computed as the modulo 256 (unsigned 8-bit integer) sum
| of the packet contents starting with the status byte, including the
| packet type byte, length byte, data bytes and ending with the last byte
| of the data bytes. It does not include the STX leader, the ETX trailer
| nor the checksum byte.
*/
static int CheckPacketChecksum(unsigned char *PacketBuffer)
{
    unsigned char Checksum = 0;
    unsigned char *p = &PacketBuffer[1];        /* Starting with status */
    unsigned int Length = PacketBuffer[3] + 3; /* status, type, length, data */
  
    /* Compute the packet checksum */
    while (Length > 0)
    {
        Checksum += *p++;
        Length--;
    }

    /*
    | Make sure our computed checksum matches the one at the end of the packet.
    | (Note that the above loop by design very conveniently left *p pointing
    |  to the checksum byte at the end of the packet.)
    */ 
    return (Checksum == *p);
}

/* ClearMessageBuffer - Clear the raw data stream buffer */
static void ClearMessageBuffer(rt17_t *rt17)
{
   unsigned char *MessageBuffer = rt17->MessageBuffer;
   int i;
   
   for (i = 0; i < 4; i++)
       MessageBuffer[i] = 0;

    rt17->MessageLength = rt17->MessageBytes = 0;
    rt17->Reply = 0;
}

/* ClearPacketBuffer - Clear the packet buffer */
static void ClearPacketBuffer(rt17_t *rt17)
{
    unsigned char *PacketBuffer = rt17->PacketBuffer;
    int i;

    for (i = 0; i < 4; i++)
        PacketBuffer[i] = 0;

    rt17->PacketLength = rt17->PacketBytes = 0;
}

/*
| DecodeBeidouEphemeris - Decode a Beidou Ephemeris record
|
| Returns:
|
| -1: error message
|  2: input ephemeris
|
| See reference #1 above for documentation of the RETSVDATA Beidou Ephemeris.
*/
static int DecodeBeidouEphemeris(raw_t *Raw)
{
    tracet(3, "DecodeBeidouEphemeris(); not yet implemented.\n");
    return 0;

#if 0
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->PacketBuffer;
    int prn, sat, toc, tow;
    unsigned int Flags, toe;
    double sqrtA;
    eph_t eph={0};
    tracet(3, "RT17: DecodeBeidouEphemeris(); Length=%d\n", rt17->PacketLength);
    if (rt17->PacketLength < 182)
    {
        tracet(2, "RT17: RETSVDATA packet length %d < 182 bytes. GPS ephemeris packet discarded.\n", rt17->PacketLength);
        return -1;
    }
    prn = U1(p+5);
    if (!(sat=satno(SYS_CMP, prn)))
    {
        tracet(2, "RT17: Beidou ephemeris satellite number error, PRN=%d.\n", prn);
        return -1;
    }
 
    eph.week  = U2(p+6);    /* 006-007: Ephemeris Week number (weeks) */
    eph.iodc  = U2(p+8);    /* 008-009: IODC */ 
    /* Reserved byte */     /* 010-010: RESERVED */
    eph.iode  = U1(p+11);   /* 011-011: IODE */
    tow       = I4(p+12);   /* 012-015: TOW */
    toc       = I4(p+16);   /* 016-019: TOC (seconds) */
    toe       = U4(p+20);   /* 020-023: TOE (seconds) */                                   
    eph.tgd[0]= R8(p+24);   /* 024-031: TGD (seconds) */
    eph.f2    = R8(p+32);   /* 032-029: AF2 (seconds/seconds^2) */
    eph.f1    = R8(p+40);   /* 040-047: AF1 (seconds/seconds) */
    eph.f0    = R8(p+48);   /* 048-055: AF0 (seconds) */
    eph.crs   = R8(p+56);   /* 056-063: CRS (meters) */
    eph.deln  = R8(p+64);   /* 064-071: DELTA N (semi-circles/second) */
    eph.M0    = R8(p+72);   /* 072-079: M SUB 0 (semi-circles) */
    eph.cuc   = R8(p+80);   /* 080-087: CUC (semi-circles) */
    eph.e     = R8(p+88);   /* 088-095: ECCENTRICITY (dimensionless) */
    eph.cus   = R8(p+96);   /* 096-103: CUS (semi-circles) */
    sqrtA     = R8(p+104);  /* 104-111: SQRT A (meters ^ 0.5) */
    eph.cic   = R8(p+112);  /* 112-119: CIC (semi-circles) */
    eph.OMG0  = R8(p+120);  /* 120-127: OMEGA SUB 0 (semi-circles) */
    eph.cis   = R8(p+128);  /* 128-135: CIS (semi-circlces) */
    eph.i0    = R8(p+136);  /* 136-143: I SUB 0 (semi-circles) */
    eph.crc   = R8(p+144);  /* 144-151: CRC (meters) */
    eph.omg   = R8(p+152);  /* 152-159: OMEGA (semi-circles?) */
    eph.OMGd  = R8(p+160);  /* 160-167: OMEGA DOT (semi-circles/second) */
    eph.idot  = R8(p+168);  /* 168-175: I DOT (semi-circles/second) */
    Flags     = U4(p+176);  /* 176-179: FLAGS */
  
    /*
    | Multiply these by PI to make semi-circle units into radian units for RTKLIB.
    */
    eph.deln *= SC2RAD;
    eph.i0   *= SC2RAD;
    eph.idot *= SC2RAD;
    eph.M0   *= SC2RAD;
    eph.omg  *= SC2RAD;
    eph.OMG0 *= SC2RAD;
    eph.OMGd *= SC2RAD;
    /*
    | As specifically directed to do so by Reference #1, multiply these by PI.
    | to make semi-circle units into radian units, which is what RTKLIB needs.
    */
    eph.cic *= SC2RAD;
    eph.cis *= SC2RAD;
    eph.cuc *= SC2RAD;
    eph.cus *= SC2RAD;
 
   /*
    | Select the correct curve fit interval as per ICD-GPS-200 sections
    | 20.3.3.4.3.1 and 20.3.4.4 using IODC, fit flag and Table 20-XII.
    */
    if (Flags & M_BIT10)  /* Subframe 2, word 10, bit 17 (fit flag) */
    {
        if ((eph.iodc >= 240) && (eph.iodc <= 247))
            eph.fit = 8;
        else if (((eph.iodc >= 248) && (eph.iodc <= 255)) || (eph.iodc == 496))
            eph.fit = 14;
        else if ((eph.iodc >= 497) && (eph.iodc <= 503))
            eph.fit = 26;
        else if ((eph.iodc >= 504) && (eph.iodc <= 510))
            eph.fit = 50;
        else if ((eph.iodc == 511) || ((eph.iodc >= 752) && (eph.iodc <= 756)))
            eph.fit = 74;
        else if ((eph.iodc >= 757) && (eph.iodc <= 763))
            eph.fit = 98;
        else if (((eph.iodc >= 764) && (eph.iodc <= 767)) || ((eph.iodc >= 1008) && (eph.iodc <= 1010)))
            eph.fit = 122;
        else if ((eph.iodc >= 1011) && (eph.iodc <= 1020))
            eph.fit = 146;
        else
            eph.fit = 6;
    }
    else
        eph.fit = 4;
 
    eph.flag  = (Flags & M_BIT0);   /* Subframe 1, word 4, bit 1, Data flag for L2 P-code */
    eph.code  = (Flags >> 1) & 3;   /* Subframe 1, word 3, bits 11-12, Codes on L2 channel */
    eph.svh   = (Flags >> 4) & 127; /* Subframe 1, word 3, bits 17-22, SV health from ephemeris */
    eph.sva   = (Flags >> 11) & 15; /* Subframe 1, word 3, bits 13-16, User Range Accuracy index */     
    eph.A     = sqrtA * sqrtA;
    eph.toes  = toe;
    eph.toc   = bdt2gpst(bdt2time(eph.week, toc));
    eph.toe   = bdt2gpst(bdt2time(eph.week, toe));
    eph.ttr   = bdt2gpst(bdt2time(eph.week, tow));
    tracet(3, "RT17: DecodeBeidouEphemeris(); SAT=%d, IODC=%d, IODE=%d, WEEK=%d.\n", sat, eph.iodc, eph.iode, eph.week);
    if (!strstr(Raw->opt,"-EPHALL"))
    {
        if (eph.iode == Raw->nav.eph[sat-1].iode)
            return 0; /* unchanged */
    }
    eph.sat = sat;
    Raw->nav.eph[sat-1] = eph;
    Raw->ephsat = sat;
    return 2;
#endif
}

/*
| DecodeGalileoEphemeris - Decode a Galileo Ephemeris record
|
| Returns:
|
| -1: error message
|  2: input ephemeris
|
| See reference #1 above for documentation of the RETSVDATA Galileo Ephemeris.
*/
static int DecodeGalileoEphemeris(raw_t *Raw)
{
    tracet(3, "DecodeGalileoEphemeris(); not yet implemented.\n");
    return 0;

#if 0
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->PacketBuffer;
    int prn, sat, toc, tow;
    unsigned int toe;
    double sqrtA;
    eph_t eph={0};
    unsigned char SISA, MODEL1, MODEL2;
    unsigned short IODnav, HSDVS;
    double BDG1, BDG2;
    tracet(3, "RT17: DecodeGalileoEphemeris(); Length=%d\n", rt17->PacketLength);
    if (rt17->PacketLength < 190)
    {
        tracet(2, "RT17: RETSVDATA packet length %d < 190 bytes. Galileo ephemeris packet discarded.\n", rt17->PacketLength);
        return -1;
    }
    prn = U1(p+5);
    if (!(sat=satno(SYS_GAL, prn)))
    {
        tracet(2, "RT17: Galileo ephemeris satellite number error, PRN=%d.\n", prn);
        return -1;
    }
 
    eph.code  = U1(p+6);    /* 006-006: Data source 0:E1B 1:E5B 2:E5A */
    eph.week  = U2(p+7);    /* 007-008: Ephemeris Week number (weeks) */
    tow       = I4(p+9);    /* 008-012: TOW */
    IODnav    = U2(p+13);   /* 013-014: Ephemeris and clock correction issue of data */
    toe       = U4(p+15);   /* 015-018: TOE (seconds) */ 
    eph.crs   = R8(p+19);   /* 019-026: CRS (meters) */
    eph.deln  = R8(p+27);   /* 027-034: DELTA N (semi-circles/second) */
    eph.M0    = R8(p+35);   /* 035-042: M SUB 0 (semi-circles) */
    eph.cuc   = R8(p+43);   /* 043-050: CUC (semi-circles) */
    eph.e     = R8(p+51);   /* 051-058: ECCENTRICITY (dimensionless) */
    eph.cus   = R8(p+59);   /* 059-066: CUS (semi-circles) */
    sqrtA     = R8(p+67);   /* 067-074: SQRT A (meters ^ 0.5) */
    eph.cic   = R8(p+75);   /* 075-082: CIC (semi-circles) */
    eph.OMG0  = R8(p+83);   /* 083-090: OMEGA SUB 0 (semi-circles) */
    eph.cis   = R8(p+91);   /* 091-098: CIS (semi-circlces) */
    eph.i0    = R8(p+99);   /* 099-106: I SUB 0 (semi-circles) */
    eph.crc   = R8(p+107);  /* 107-114: CRC (meters) */
    eph.omg   = R8(p+115);  /* 115-122: OMEGA (semi-circles?) */
    eph.OMGd  = R8(p+123);  /* 123-130: OMEGA DOT (semi-circles/second) */
    eph.idot  = R8(p+131);  /* 131-138: I DOT (semi-circles/second) */
    SISA      = U1(p+149);  /* 149-149: ? */
    HSDVS     = U2(p+150);  /* 150-151: Signal Health Flag */
    toc       = I4(p+142);  /* 142-145: TOC (seconds) */
    eph.f0    = R8(p+146);  /* 146-153: AF0 (seconds) */
    eph.f1    = R8(p+154);  /* 154-161: AF1 (seconds/seconds) */
    eph.f2    = R8(p+162);  /* 162-169: AF2 (seconds/seconds^2) */
    BDG1      = R8(p+170);  /* 170-177: Seconds */
    MODEL1    = U1(p+178);  /* 178-178: Clock model for TOC/AF0?2/BGD1 */
    BDG2      = R8(p+179);  /* 179-186: Seconds */
    MODEL2    = U1(p+187);  /* 187-187: Clock model for BGD2 */
    /*
    | Multiply these by PI to make semi-circle units into radian units for RTKLIB.
    */
    eph.deln *= SC2RAD;
    eph.i0   *= SC2RAD;
    eph.idot *= SC2RAD;
    eph.M0   *= SC2RAD;
    eph.omg  *= SC2RAD;
    eph.OMG0 *= SC2RAD;
    eph.OMGd *= SC2RAD;
    /*
    | As specifically directed to do so by Reference #1, multiply these by PI.
    | to make semi-circle units into radian units, which is what RTKLIB needs.
    */
    eph.cic *= SC2RAD;
    eph.cis *= SC2RAD;
    eph.cuc *= SC2RAD;
    eph.cus *= SC2RAD;
 
    eph.A     = sqrtA * sqrtA;
    eph.toes  = toe;
    eph.toc   = gst2time(eph.week, toc);
    eph.toe   = gst2time(eph.week, toe);
    eph.ttr   = gst2time(eph.week, tow);
    tracet(3, "RT17: DecodeGalileoEphemeris(); SAT=%d, IODC=%d, IODE=%d, WEEK=%d.\n", sat, eph.iodc, eph.iode, eph.week);
    if (!strstr(Raw->opt,"-EPHALL"))
    {
        if (eph.iode == Raw->nav.eph[sat-1].iode)
            return 0; /* unchanged */
    }
    eph.sat = sat;
    Raw->nav.eph[sat-1] = eph;
    Raw->ephsat = sat;
    return 2;
#endif
}

/*
| DecodeGLONASSEphemeris - Decode a GLONASS Ephemeris record
|
| Returns:
|
| -1: error message
|  2: input ephemeris
|
| See reference #1 above for documentation of the RETSVDATA GLONASS Ephemeris.
*/
static int DecodeGLONASSEphemeris(raw_t *Raw)
{
    tracet(3, "DecodeGLONASSEphemeris(); not yet implemented.\n");
    return 0;
}
   
/*
| DecodeGPSEphemeris - Decode a GPS Ephemeris record
|
| Returns:
|
| -1: error message
|  2: input ephemeris
|
| See ICD-GPS-200C.PDF for documentation of the GPS satellite ephemeris.
| See reference #1 above for documentation of the RETSVDATA GPS Ephemeris.
*/
static int DecodeGPSEphemeris(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->PacketBuffer;
    int prn, sat, toc, tow;
    unsigned int Flags, toe;
    double sqrtA;
    eph_t eph={0};

    tracet(3, "RT17: DecodeGPSEphemeris(); Length=%d\n", rt17->PacketLength);

    if (rt17->PacketLength < 182)
    {
        tracet(2, "RT17: RETSVDATA packet length %d < 182 bytes. GPS ephemeris packet discarded.\n", rt17->PacketLength);
        return -1;
    }

    prn = U1(p+5);

    if (!(sat=satno(SYS_GPS, prn)))
    {
        tracet(2, "RT17: GPS ephemeris satellite number error, PRN=%d.\n", prn);
        return -1;
    }
 
    eph.week  = U2(p+6);    /* 006-007: Ephemeris Week number (weeks) */
    eph.iodc  = U2(p+8);    /* 008-009: IODC */ 
    /* Reserved byte */     /* 010-010: RESERVED */
    eph.iode  = U1(p+11);   /* 011-011: IODE */
    tow       = I4(p+12);   /* 012-015: TOW */
    toc       = I4(p+16);   /* 016-019: TOC (seconds) */
    toe       = U4(p+20);   /* 020-023: TOE (seconds) */                                   
    eph.tgd[0]= R8(p+24);   /* 024-031: TGD (seconds) */
    eph.f2    = R8(p+32);   /* 032-029: AF2 (seconds/seconds^2) */
    eph.f1    = R8(p+40);   /* 040-047: AF1 (seconds/seconds) */
    eph.f0    = R8(p+48);   /* 048-055: AF0 (seconds) */
    eph.crs   = R8(p+56);   /* 056-063: CRS (meters) */
    eph.deln  = R8(p+64);   /* 064-071: DELTA N (semi-circles/second) */
    eph.M0    = R8(p+72);   /* 072-079: M SUB 0 (semi-circles) */
    eph.cuc   = R8(p+80);   /* 080-087: CUC (semi-circles) */
    eph.e     = R8(p+88);   /* 088-095: ECCENTRICITY (dimensionless) */
    eph.cus   = R8(p+96);   /* 096-103: CUS (semi-circles) */
    sqrtA     = R8(p+104);  /* 104-111: SQRT A (meters ^ 0.5) */
    eph.cic   = R8(p+112);  /* 112-119: CIC (semi-circles) */
    eph.OMG0  = R8(p+120);  /* 120-127: OMEGA SUB 0 (semi-circles) */
    eph.cis   = R8(p+128);  /* 128-135: CIS (semi-circlces) */
    eph.i0    = R8(p+136);  /* 136-143: I SUB 0 (semi-circles) */
    eph.crc   = R8(p+144);  /* 144-151: CRC (meters) */
    eph.omg   = R8(p+152);  /* 152-159: OMEGA (semi-circles?) */
    eph.OMGd  = R8(p+160);  /* 160-167: OMEGA DOT (semi-circles/second) */
    eph.idot  = R8(p+168);  /* 168-175: I DOT (semi-circles/second) */
    Flags     = U4(p+176);  /* 176-179: FLAGS */
  
    /*
    | Multiply these by PI to make ICD specified semi-circle units into radian
    | units for RTKLIB.
    */
    eph.deln *= SC2RAD;
    eph.i0   *= SC2RAD;
    eph.idot *= SC2RAD;
    eph.M0   *= SC2RAD;
    eph.omg  *= SC2RAD;
    eph.OMG0 *= SC2RAD;
    eph.OMGd *= SC2RAD;

    /*
    | As specifically directed to do so by Reference #1, multiply these by PI.
    | to make semi-circle units into radian units, which is what ICD-GPS-200C
    | calls for and also what RTKLIB needs.
    */
    eph.cic *= SC2RAD;
    eph.cis *= SC2RAD;
    eph.cuc *= SC2RAD;
    eph.cus *= SC2RAD;
 
   /*
    | Select the correct curve fit interval as per ICD-GPS-200 sections
    | 20.3.3.4.3.1 and 20.3.4.4 using IODC, fit flag and Table 20-XII.
    */
    if (Flags & M_BIT10)  /* Subframe 2, word 10, bit 17 (fit flag) */
    {
        if ((eph.iodc >= 240) && (eph.iodc <= 247))
            eph.fit = 8;
        else if (((eph.iodc >= 248) && (eph.iodc <= 255)) || (eph.iodc == 496))
            eph.fit = 14;
        else if ((eph.iodc >= 497) && (eph.iodc <= 503))
            eph.fit = 26;
        else if ((eph.iodc >= 504) && (eph.iodc <= 510))
            eph.fit = 50;
        else if ((eph.iodc == 511) || ((eph.iodc >= 752) && (eph.iodc <= 756)))
            eph.fit = 74;
        else if ((eph.iodc >= 757) && (eph.iodc <= 763))
            eph.fit = 98;
        else if (((eph.iodc >= 764) && (eph.iodc <= 767)) || ((eph.iodc >= 1008) && (eph.iodc <= 1010)))
            eph.fit = 122;
        else if ((eph.iodc >= 1011) && (eph.iodc <= 1020))
            eph.fit = 146;
        else
            eph.fit = 6;
    }
    else
        eph.fit = 4;
 
    eph.flag  = (Flags & M_BIT0);   /* Subframe 1, word 4, bit 1, Data flag for L2 P-code */
    eph.code  = (Flags >> 1) & 3;   /* Subframe 1, word 3, bits 11-12, Codes on L2 channel */
    eph.svh   = (Flags >> 4) & 127; /* Subframe 1, word 3, bits 17-22, SV health from ephemeris */
    eph.sva   = (Flags >> 11) & 15; /* Subframe 1, word 3, bits 13-16, User Range Accuracy index */     

    eph.A     = sqrtA * sqrtA;

    eph.toes  = toe;
    eph.toc   = gpst2time(eph.week, toc);
    eph.toe   = gpst2time(eph.week, toe);
    eph.ttr   = gpst2time(eph.week, tow);

    tracet(3, "RT17: DecodeGPSEphemeris(); SAT=%d, IODC=%d, IODE=%d, WEEK=%d.\n", sat, eph.iodc, eph.iode, eph.week);
    
    if (rt17->Week && (rt17->Week != eph.week))
    {
        tracet(2, "RT17: Currently set or assumed GPS week does not match received ephemeris week.\n");
        tracet(2, "RT17: Set or assumed GPS week: %d  Received ephemeris week: %d\n", rt17->Week, eph.week);
    }

    if (!(rt17->Flags & M_WEEK_OPTION))
    {
        if (!rt17->Week || (rt17->Flags & M_WEEK_TIME) || (eph.week > rt17->Week))
        {
            if (!rt17->Week)
                tracet(2, "RT17: Initial GPS WEEK number unknown; WEEK number %d assumed for now.\n", eph.week);
            else
                tracet(2, "RT17: Changing assumed week number from %d to %d.\n", rt17->Week, eph.week);
            rt17->Flags &= ~M_WEEK_TIME;
            rt17->Flags |= M_WEEK_EPH;
            rt17->Week = eph.week;
        }
    }
 
    if (!strstr(Raw->opt,"-EPHALL"))
    {
        if (eph.iode == Raw->nav.eph[sat-1].iode)
            return 0; /* unchanged */
    }

    eph.sat = sat;
    Raw->nav.eph[sat-1] = eph;
    Raw->ephsat = sat;

    return 2;
}

/* DecodeGSOF - Decode a General Serial Output Format (GSOF) message */
static int DecodeGSOF(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    int InputLength, Ret = 0;
    unsigned char RecordLength, RecordType, *p;
    char *RecordType_s = NULL;
 
   /*
    | Reassemble origional message by removing packet headers,
    | trailers and page framing.
    */
    UnwrapGenout(rt17);

    p = rt17->MessageBuffer;
    InputLength = rt17->MessageLength;

    while (InputLength)
    {
        RecordType = p[0];
        RecordLength = p[1];

        if (RecordType < (sizeof(GSOFTable) / sizeof(char*)))
            RecordType_s = (char*) GSOFTable[RecordType];

        if (!RecordType_s)
            RecordType_s = "Unknown";

        tracet(3, "RT17: Trimble packet type=0x40 (GENOUT), GSOF record type=%d (%s), Length=%d.\n", RecordType, RecordType_s, RecordLength);
      
        /* Process (or possibly ignore) the message */
        switch (RecordType)
        {
        case 1:
            Ret = DecodeGSOF1(Raw, p);
            break;
        case 3:
            Ret = DecodeGSOF3(Raw, p);
            break;
        case 15:
            Ret = DecodeGSOF15(Raw, p);
            break;
        case 16:
            Ret = DecodeGSOF16(Raw, p);
            break;
        case 26:
            Ret = DecodeGSOF26(Raw, p);
            break;
        case 41:
            Ret = DecodeGSOF41(Raw, p);
            break;
        default:
            tracet(3, "RT17: GSOF message not processed.\n");    
        }

        RecordLength += 2;
        p += RecordLength;
        InputLength -= RecordLength;
    }

    return Ret;
}

/* DecodeGSOF1 - Decode a Position Time GSOF message */
static int DecodeGSOF1(raw_t *Raw, unsigned char *p)
{

    if (p[1] < 6)
        tracet(2, "RT17: GSOF Position Time message record length %d < 6 bytes. Record discarded.\n", p[1]);
    else
        SetWeek(Raw, I2(p+6), ((double) I4(p+2)) * 0.001);

    return 0;
}
 
/* DecodeGSOF3 - Decode an ECEF Position GSOF message */
static int DecodeGSOF3(raw_t *Raw, unsigned char *p)
{
    sta_t *sta = &Raw->sta;
 
    if (p[1] < 24)
        tracet( 2, "RT17: GSOF ECEF Position record length %d < 24 bytes. Record discarded.\n", p[1] );
    else
    {
        sta->pos[0] = R8(p+2);
        sta->pos[1] = R8(p+10);
        sta->pos[2] = R8(p+18);
        sta->del[0] = 0.0;
        sta->del[1] = 0.0;
        sta->del[2] = 0.0;
        sta->hgt    = 0.0;
        sta->deltype = 0;  /* e/n/u */
    }

    return 5;
}

/* DecodeGSOF15 - Decode a Receiver Serial Number GSOF message  */
static int DecodeGSOF15(raw_t *Raw, unsigned char *p)
{
    if (p[1] < 15)
        tracet(2, "RT17: GSOF Receiver Serial Number record length %d < 15 bytes. Record discarded.\n", p[1]);
    else
        sprintf(Raw->sta.recsno, "%u", U4(p+2));

    return 0;
}

/* DecodeGSOF16 - Decode a Current Time GSOF message */
static int DecodeGSOF16(raw_t *Raw, unsigned char *p)
{
    if (p[1] < 9)
        tracet( 2, "RT17: GSOF Current Time message record length %d < 9 bytes. Record discarded.\n", p[1] );
    else if (U1(p+10) & M_BIT0) /* If week and milliseconds of week are valid */
        SetWeek(Raw, I2(p+6), ((double) I4(p+2)) * 0.001);

    return 0;
}

/* DecodeGSOF26 - Decode a Position Time UTC GSOF message */
static int DecodeGSOF26(raw_t *Raw, unsigned char *p)
{
    if (p[1] < 6)
        tracet(2, "RT17: GSOF Position Time UTC message record length %d < 6 bytes. Record discarded.\n", p[1]);
    else
        SetWeek(Raw, I2(p+6), ((double) I4(p+2)) * 0.001);

    return 0;
}

/* DecodeGSOF41 - Decode a Base Position and Quality Indicator GSOF message */
static int DecodeGSOF41(raw_t *raw, unsigned char *p)
{
    if (p[1] < 6)
        tracet(2, "RT17: GSOF Base Position and Quality Indicator message record length %d < 6 bytes. Record discarded.\n", p[1]);
    else 
        SetWeek(raw, I2(p+6), ((double) I4(p+2)) * 0.001);

    return 0;
}

/*
| DecodeIONAndUTCData - Decode an ION / UTC data record
|
| Returns:
|
| -1: error message
|  9: input ion/utc parameter|
|
| See ICD-GPS-200C.PDF for documetation of GPS ION / UTC data.
| See reference #1 above for documentation of RETSVDATA and ION / UTC data.
*/
static int DecodeIONAndUTCData(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    int week;
    unsigned char *p = rt17->PacketBuffer;
    nav_t *nav = &Raw->nav;
    double *ion_gps = nav->ion_gps;
    double *utc_gps = nav->utc_gps;

    tracet(3, "RT17: DecodeIONAndUTCData, Length=%d.\n", rt17->PacketLength);

    if (rt17->PacketLength < 129)
    {
        tracet(2, "RT17: RETSVDATA packet length %d < 129 bytes. GPS ION / UTC data packet discarded.\n", rt17->PacketLength);
        return -1;
    }

    /* ION / UTC data does not have the current GPS week number. Punt! */
    week = GetWeek(Raw, 0.0);
 
    ion_gps[0] = R8(p+6);  /* 006-013: ALPHA 0 (seconds) */
    ion_gps[1] = R8(p+14); /* 014-021: ALPHA 1 (seconds/semi-circle) */
    ion_gps[2] = R8(p+22); /* 022-029: ALPHA 2 (seconds/semi-circle)^2 */ 
    ion_gps[3] = R8(p+30); /* 030-037: ALPHA 3 (seconds/semi-circle)^3 */
    ion_gps[4] = R8(p+38); /* 038-045: BETA 0  (seconds) */
    ion_gps[5] = R8(p+46); /* 046-053: BETA 1  (seconds/semi-circle) */
    ion_gps[6] = R8(p+54); /* 054-061: BETA 2  (seconds/semi-circle)^2 */
    ion_gps[7] = R8(p+62); /* 062-069: BETA 3  (seconds/semi-circle)^3 */
    utc_gps[0] = R8(p+70); /* 070-077: ASUB0   (seconds)*/ 
    utc_gps[1] = R8(p+78); /* 078-085: ASUB1   (seconds/seconds) */     
    utc_gps[2] = R8(p+86); /* 086-093: TSUB0T */ 
    utc_gps[3] = week;
    nav->leaps =(int) R8(p+94); /* 094-101: DELTATLS (seconds) */
    /* Unused by RTKLIB R8 */   /* 102-109: DELTATLSF */
    /* Unused by RTKLIB R8 */   /* 110-117: IONTIME */
    /* Unused by RTKLIB U1 */   /* 118-118: WNSUBT */
    /* Unused by RTKLIB U1 */   /* 119-119: WNSUBLSF */
    /* Unused by RTKLIB U1 */   /* 120-120: DN */
    /* Reserved six bytes */    /* 121-126: RESERVED */
   
   return 9;
}

/*
| DecodeQZSSEphemeris - Decode a QZSS Ephemeris record
|
| Returns:
|
| -1: error message
|  2: input ephemeris
|
| See reference #1 above for documentation of the RETSVDATA QZSS Ephemeris.
*/
static int DecodeQZSSEphemeris(raw_t *Raw)
{
    tracet(3, "DecodeQZSSEphemeris(); not yet implemented.\n");
    return 0;

#if 0
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->PacketBuffer;
    int prn, sat, toc, tow;
    unsigned int Flags, toe;
    double sqrtA;
    eph_t eph={0};
    tracet(3, "RT17: DecodeQZSSEphemeris(); Length=%d\n", rt17->PacketLength);
    if (rt17->PacketLength < 184)
    {
        tracet(2, "RT17: RETSVDATA packet length %d < 184 bytes. QZSS ephemeris packet discarded.\n", rt17->PacketLength);
        return -1;
    }
    prn = U1(p+5);
    if (!(sat=satno(SYS_GPS, prn)))
    {
        tracet(2, "RT17: QZSS ephemeris satellite number error, PRN=%d.\n", prn);
        return -1;
    }
 
    /* Not used by RTKLIB      006-006: Source: 0:L1CA 1:L1C 2:L2C 3:L5 */
    eph.week  = U2(p+8);    /* 008-009: Ephemeris Week number (weeks) */
    eph.iodc  = U2(p+10);   /* 010-011: IODC */ 
    /* Reserved byte           012-012: RESERVED */
    eph.iode  = U1(p+13);   /* 013-013: IODE */
    tow       = I4(p+14);   /* 014-017: TOW */
    toc       = I4(p+18);   /* 018-021: TOC (seconds) */
    toe       = U4(p+22);   /* 022-025: TOE (seconds) */                                   
    eph.tgd[0]= R8(p+26);   /* 026-033: TGD (seconds) */
    eph.f2    = R8(p+34);   /* 034-041: AF2 (seconds/seconds^2) */
    eph.f1    = R8(p+42);   /* 042-049: AF1 (seconds/seconds) */
    eph.f0    = R8(p+50);   /* 050-057: AF0 (seconds) */
    eph.crs   = R8(p+58);   /* 058-065: CRS (meters) */
    eph.deln  = R8(p+66);   /* 066-073: DELTA N (semi-circles/second) */
    eph.M0    = R8(p+74);   /* 074-081: M SUB 0 (semi-circles) */
    eph.cuc   = R8(p+82);   /* 082-089: CUC (semi-circles) */
    eph.e     = R8(p+90);   /* 090-097: ECCENTRICITY (dimensionless) */
    eph.cus   = R8(p+98);   /* 098-105: CUS (semi-circles) */
    sqrtA     = R8(p+106);  /* 106-113: SQRT A (meters ^ 0.5) */
    eph.cic   = R8(p+114);  /* 114-121: CIC (semi-circles) */
    eph.OMG0  = R8(p+122);  /* 122-129: OMEGA SUB 0 (semi-circles) */
    eph.cis   = R8(p+130);  /* 130-137: CIS (semi-circlces) */
    eph.i0    = R8(p+138);  /* 138-145: I SUB 0 (semi-circles) */
    eph.crc   = R8(p+146);  /* 146-153: CRC (meters) */
    eph.omg   = R8(p+154);  /* 154-161: OMEGA (semi-circles?) */
    eph.OMGd  = R8(p+162);  /* 162-169: OMEGA DOT (semi-circles/second) */
    eph.idot  = R8(p+170);  /* 170-177: I DOT (semi-circles/second) */
    Flags     = U4(p+178);  /* 178-181: FLAGS */
  
    /*
    | Multiply these by PI to make ICD specified semi-circle units into radian
    | units for RTKLIB.
    */
    eph.deln *= SC2RAD;
    eph.i0   *= SC2RAD;
    eph.idot *= SC2RAD;
    eph.M0   *= SC2RAD;
    eph.omg  *= SC2RAD;
    eph.OMG0 *= SC2RAD;
    eph.OMGd *= SC2RAD;
    /*
    | As specifically directed to do so by Reference #1, multiply these by PI.
    | to make semi-circle units into radian units, which is what ICD-GPS-200C
    | calls for and also what RTKLIB needs.
    */
    eph.cic *= SC2RAD;
    eph.cis *= SC2RAD;
    eph.cuc *= SC2RAD;
    eph.cus *= SC2RAD;
 
   /*
    | Select the correct curve fit interval as per ICD-GPS-200 sections
    | 20.3.3.4.3.1 and 20.3.4.4 using IODC, fit flag and Table 20-XII.
    */
    if (Flags & M_BIT10)  /* Subframe 2, word 10, bit 17 (fit flag) */
    {
        if ((eph.iodc >= 240) && (eph.iodc <= 247))
            eph.fit = 8;
        else if (((eph.iodc >= 248) && (eph.iodc <= 255)) || (eph.iodc == 496))
            eph.fit = 14;
        else if ((eph.iodc >= 497) && (eph.iodc <= 503))
            eph.fit = 26;
        else if ((eph.iodc >= 504) && (eph.iodc <= 510))
            eph.fit = 50;
        else if ((eph.iodc == 511) || ((eph.iodc >= 752) && (eph.iodc <= 756)))
            eph.fit = 74;
        else if ((eph.iodc >= 757) && (eph.iodc <= 763))
            eph.fit = 98;
        else if (((eph.iodc >= 764) && (eph.iodc <= 767)) || ((eph.iodc >= 1008) && (eph.iodc <= 1010)))
            eph.fit = 122;
        else if ((eph.iodc >= 1011) && (eph.iodc <= 1020))
            eph.fit = 146;
        else
            eph.fit = 6;
    }
    else
        eph.fit = 4;
 
    eph.flag  = (Flags & M_BIT0);   /* Subframe 1, word 4, bit 1, Data flag for L2 P-code */
    eph.code  = (Flags >> 1) & 3;   /* Subframe 1, word 3, bits 11-12, Codes on L2 channel */
    eph.svh   = (Flags >> 4) & 127; /* Subframe 1, word 3, bits 17-22, SV health from ephemeris */
    eph.sva   = (Flags >> 11) & 15; /* Subframe 1, word 3, bits 13-16, User Range Accuracy index */     
    eph.A     = sqrtA * sqrtA;
    eph.toes  = toe;
    eph.toc   = gpst2time(eph.week, toc);
    eph.toe   = gpst2time(eph.week, toe);
    eph.ttr   = gpst2time(eph.week, tow);
    tracet(3, "RT17: DecodeQZSSEphemeris(); SAT=%d, IODC=%d, IODE=%d, WEEK=%d.\n", sat, eph.iodc, eph.iode, eph.week);
    if (!strstr(Raw->opt,"-EPHALL"))
    {
        if (eph.iode == Raw->nav.eph[sat-1].iode)
            return 0; /* unchanged */
    }
    eph.sat = sat;
    Raw->nav.eph[sat-1] = eph;
    Raw->ephsat = sat;
    return 2;
#endif
}

/*
| DecodeRawdata - Decode an RAWDATA packet sequence 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
*/
static int DecodeRawdata(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *MessageBuffer = rt17->MessageBuffer;
    int Ret = 0;
    unsigned int rif;
    char *RecordType_s = NULL;
    unsigned char RecordType = MessageBuffer[4];
 
    if (RecordType < (sizeof(RawdataTable) / sizeof(char*)))
        RecordType_s = (char*) RawdataTable[RecordType];

    if (!RecordType_s)
        RecordType_s = "Unknown";
  
    tracet(3, "RT17: Trimble packet type=0x57 (RAWDATA), Recordtype=%d (%s), Length=%d.\n", RecordType, RecordType_s, rt17->MessageLength);
      
    /*
    | Reassemble origional message by removing packet headers,
    | trailers and page framing.
    */
    UnwrapRawdata(rt17, &rif);

    /* Process (or possibly ignore) the message */
    switch (RecordType)
    {
    case 0:
        Ret = DecodeType17(Raw, rif);
        break;
    case 7:
        Ret = DecodeType29(Raw);
        break;
    default:
        tracet(3, "RT17: Packet not processed.\n");      
    }

    return Ret;
}

/*
| DecodeRetsvdata - Decode an SVDATA packet 
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  2: input ephemeris
|  9: input ion/utc parameter
*/
static int DecodeRetsvdata(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *PacketBuffer = rt17->PacketBuffer;
    int Ret = 0;
    char *Subtype_s = NULL;
    unsigned char Subtype = PacketBuffer[4];

    if (Subtype < (sizeof(RetsvdataTable) / sizeof(char*)))
        Subtype_s = (char*) RetsvdataTable[Subtype];
  
    if (!Subtype_s)
        Subtype_s = "Unknown";
 
    tracet(3, "RT17: Trimble packet type=0x55 (RETSVDATA), Subtype=%d (%s), Length=%d.\n", Subtype, Subtype_s, rt17->PacketLength);
      
    /* Process (or possibly ignore) the message */
    switch (Subtype)
    {
    case 1:
        Ret = DecodeGPSEphemeris(Raw);
        break;
    case 3:
        Ret = DecodeIONAndUTCData(Raw);
        break;
    case 9:
        Ret = DecodeGLONASSEphemeris(Raw);
        break;
    case 11:
        Ret = DecodeGalileoEphemeris(Raw);
        break;
    case 14:
        Ret = DecodeQZSSEphemeris(Raw);
        break;
    case 21:
        Ret = DecodeBeidouEphemeris(Raw);
        break;
    default:
        tracet(3, "RT17: Packet not processed.\n");      
    }

    return Ret;
}

/*
| DecodeType17 - Decode Real-Time survey data (record type 17)
|
| Returns:
|
| -1: error message
|  0: no message (tells caller to please read more data from the stream)
|  1: input observation data
|
| Handles expanded and concise formats with and without enhanced record data.
*/
static int DecodeType17(raw_t *Raw, unsigned int rif)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->MessageBuffer;
    double ClockOffset, tow;
    int Flags1, Flags2, FlagStatus, i, n, nsat, prn, Week;
    gtime_t Time;
    obsd_t *obs;

    tow = R8(p) * 0.001; p += 8;         /* Receive time within the current GPS week. */
    ClockOffset = R8(p) * 0.001; p += 8; /* Clock offset value. 0.0 = not known */ 

#if 0
    tow += ClockOffset;
#endif
 
    /* The observation data does not have the current GPS week number. Punt! */
    Week = GetWeek(Raw, tow);
    Time = gpst2time(Week, tow);

    nsat = U1(p); p++; /* Number of SV data blocks in the record */

    for (i = n = 0; (i < nsat) && (i < MAXOBS); i++)
    {
        obs = &Raw->obs.data[n];
        memset(obs, 0, sizeof(obsd_t));
        obs->time = Time;

        if (rif & M_CONCISE)
        {
            /* Satellite number (1-32). */
            prn = U1(p);
            p++;

            /* These indicate what data is loaded, is valid, etc */
            Flags1 = U1(p);
            p++; 
            Flags2 = U1(p);
            p++;

            /* These are not needed by RTKLIB */
            p++;    /* I1 Satellite Elevation Angle (degrees) */
            p += 2; /* I2 Satellite Azimuth (degrees) */

            if (Flags1 & M_BIT6) /* L1 data valid */
            {
                /* Measure of L1 signal strength (dB * 4) */
                obs->SNR[0] = U1(p);
                p++;
                
                /* Full L1 C/A code or P-code pseudorange (meters) */
                obs->P[0] = R8(p);
                p += 8;
        
                /*  L1 Continuous Phase (cycles) */
                if (Flags1 & M_BIT4) /* L1 phase valid */
                    obs->L[0] = -R8(p);
                p += 8;

                /* L1 Doppler (Hz) */
                obs->D[0] = R4(p);
                p += 4; 
            }

            if (Flags1 & M_BIT0)  /* L2 data loaded */
            {
                /* Measure of L2 signal strength (dB * 4) */
                obs->SNR[1] = U1(p);
                p++;
                
                /* L2 Continuous Phase (cycles) */
                if (Flags1 & M_BIT5)
                    obs->L[1] = -R8(p);
                p += 8; 

                /* L2 P-Code or L2 Encrypted Code */              
                if (Flags1 & M_BIT5) /* L2 range valid */
                    obs->P[1] = obs->P[0] + R4(p);
                p += 4;
            }
        
            /*
            | We can't use the IODE flags in this context.
            | We already have slip flags and don't need slip counters.
            */
            if (rif & M_ENHANCED)
            {
                p++; /* U1 IODE, Issue of Data Ephemeris */
                p++; /* U1 L1 cycle slip roll-over counter */ 
                p++; /* U1 L2 cycle slip roll-over counter */ 
            }           
        }
        else /* Expanded Format */
        {
            /* Satellite number (1-32) */
            prn = U1(p);
            p++;

            /* These indicate what data is loaded, is valid, etc */
            Flags1 = U1(p);
            p++;
            Flags2 = U1(p);
            p++;

            /* Indicates whether FLAGS1 bit 6 and FLAGS2 are valid */
            FlagStatus = U1(p);
            p++;

            /* These are not needed by RTKLIB */
            p += 2; /* I2 Satellite Elevation Angle (degrees) */
            p += 2; /* I2 Satellite Azimuth (degrees) */

            /*
            | FLAG STATUS bit 0 set   = Bit 6 of FLAGS1 and bit 0-7 of FLAGS2 are valid.
            | FLAG STATUS bit 0 clear = Bit 6 of FLAGS1 and bit 0-7 of FLAGS2 are UNDEFINED.
            |
            | According to reference #1 above, this bit should ALWAYS be set
            | for RAWDATA. If this bit is not set, then we're lost and cannot
            | process this message any further.
            */
            if (!(FlagStatus & M_BIT0)) /* Flags invalid */
                return 0;
          
            if (Flags1 & M_BIT6) /* L1 data valid */
            {           
                /* Measure of satellite signal strength (dB) */
                obs->SNR[0] = R8(p) * 4.0;
                p += 8;

                /* Full L1 C/A code or P-code pseudorange (meters) */
                obs->P[0] = R8(p);
                p += 8;

                /* L1 Continuous Phase (cycles) */
                if (Flags1 & M_BIT4) /* L1 phase valid */
                    obs->L[0] = -R8(p);
                p += 8;

                /* L1 Doppler (Hz) */
                obs->D[0] = R8(p);
                p += 8;

                /* Reserved 8 bytes */
                p += 8;
            }

            if (Flags1 & M_BIT0) /* L2 data loaded */
            {
                /* Measure of L2 signal strength (dB) */
                obs->SNR[1] = R8(p) * 4.0;
                p += 8;

                /* L2 Continuous Phase (cycles) */                
                if (Flags1 & M_BIT5) /* L2 phase valid */
                    obs->L[1] = -R8(p);
                p += 8;

                /* L2 P-Code or L2 Encrypted Code */              
                if (Flags1 & M_BIT5) /* L2 pseudorange valid */
                    obs->P[1] = obs->P[0] + R8(p);
                p += 8;
            }   
                
            if (rif & M_ENHANCED)
            {
                /*
                | We can't use the IODE flags in this context.
                | We already have slip flags and don't need slip counters.
                */
                p++; /* U1 IODE, Issue of Data Ephemeris */
                p++; /* U1 L1 cycle slip roll-over counter */ 
                p++; /* U1 L2 cycle slip roll-over counter */ 
                p++; /* U1 Reserved byte */

                /* L2 Doppler (Hz) */
                obs->D[1] = R8(p);
                p += 8;
            }
        }

        obs->code[0] = (obs->P[0] == 0.0) ? CODE_NONE : (Flags2 & M_BIT0) ? CODE_L1P : CODE_L1C;
        obs->code[1] = (obs->P[1] == 0.0) ? CODE_NONE : (Flags2 & M_BIT2) ? CODE_L2W : (Flags2 & M_BIT1) ? CODE_L2P : CODE_L2C;

        if (Flags1 & M_BIT1)
            obs->LLI[0] |= 1;  /* L1 cycle slip */

        if (Flags1 & M_BIT2)
            obs->LLI[1] |= 1;  /* L2 cycle slip */
        if ((Flags2 & M_BIT2) && (obs->P[1] != 0.0))
            obs->LLI[1] |= 4; /* Tracking encrypted code */

        if (!(obs->sat = satno(SYS_GPS, prn)))
        {
            tracet(2, "RT17: Satellite number error, PRN=%d.\n", prn);
            continue;
        }

#if 0
        /* Apply clock offset to observables */
        if (ClockOffset != 0.0)
        {
            obs->P[0] += ClockOffset * (CLIGHT/FREQ1);
            obs->P[1] += ClockOffset * (CLIGHT/FREQ2);
            obs->L[0] += ClockOffset * FREQ1;
            obs->L[1] += ClockOffset * FREQ2;
        }
#endif
         n++;
    }

    Raw->time = Time;
    Raw->obs.n = n;
    
    if (n > 0)
    {
        tracet(2, "RT17: Observations output:\n");
        traceobs(2, Raw->obs.data, Raw->obs.n);
    }

    return (n > 0);
}

/* DecodeType29 - Decode Enhanced position (record type 29) */
static int DecodeType29(raw_t *Raw)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    unsigned char *p = rt17->MessageBuffer;

    if (*p < 7)
        tracet(2, "RT17: Enhanced Position record block #1 length %d < 7 bytes. Record discarded.\n", *p);
    else
        SetWeek(Raw, I2(p+1), ((double) I4(p+3)) * 0.001);

    return 0;
}

/*
| GetWeek - Get GPS week number
|
| Returns: GPS week number
|
| The -WEEK=n initial week option overrides everything else.
| Week rollover and increment from the initial week and
| subsequent weeks is handled.
*/
static int GetWeek(raw_t *Raw, double Tow)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;
    int Week = 0;

    if (rt17->Flags & M_WEEK_OPTION)
    {
        if ((Tow && rt17->Tow) && (Tow < rt17->Tow))
        {
            tracet(2, "RT17: GPS WEEK rolled over from %d to %d.\n", rt17->Week, rt17->Week + 1);
            rt17->Week++;
        }

        if (Tow != 0.0)
            rt17->Tow = Tow;
    }
    else if (!(rt17->Flags & M_WEEK_SCAN))
    {
        char *opt = strstr(Raw->opt, "-WEEK=");

        rt17->Flags |= M_WEEK_SCAN;

        if (opt)
        {
            if (!sscanf(opt+6, "%d", &Week) || (Week <= 0))
                tracet(0, "RT17: Invalid -WEEK=n receiver option value.\n");
            else
            {
                rt17->Week = Week;
                rt17->Flags |= M_WEEK_OPTION;
                tracet(2, "RT17: Initial GPS WEEK explicitly set to %d by user.\n", Week, Week);
            }
        }
    }

    Week = rt17->Week;

    if (!Week && !(rt17->Flags & (M_WEEK_OPTION|M_WEEK_EPH)))
    {
        if ((Raw->time.time == 0) && (Raw->time.sec == 0.0))
            Raw->time = timeget();
        
        time2gpst(Raw->time, &Week);

        if (Tow != 0.0)
            Raw->time = gpst2time(Week, Tow);

        rt17->Week = Week;
        rt17->Flags |= M_WEEK_TIME; 
        tracet(2, "RT17: Initial GPS WEEK number unknown; WEEK number %d assumed for now.\n", Week);       
    }
 
    return Week;
}

/* ReadI2 - Fetch & convert a signed two byte integer (short) */
static short ReadI2(unsigned char *p) 
{
    union I2 {short i2; unsigned char c[2];} u;
    ENDIAN_TEST et;

    memcpy(&u.i2, p, sizeof(u.i2));

    et.u2 = 0; et.c[0] = 1;  
    if (et.u2 == 1)
    {
        unsigned char t;
        t = u.c[0]; u.c[0] = u.c[1]; u.c[1] = t;
    }
    return u.i2;
}

/* ReadI4 - Fetch & convert a four byte signed integer (int) */
static int ReadI4(unsigned char *p)
{
    union i4 {int i4; unsigned char c[4];} u;
    ENDIAN_TEST et;

    memcpy(&u.i4, p, sizeof(u.i4));

    et.u2 = 0; et.c[0] = 1;  
    if (et.u2 == 1)
    {
        unsigned char t;
        t = u.c[0]; u.c[0] = u.c[3]; u.c[3] = t;
        t = u.c[1]; u.c[1] = u.c[2]; u.c[2] = t;
    }   
    return u.i4;
}

/* ReadR4 - Fetch & convert an IEEE S_FLOAT (float) */
static float ReadR4(unsigned char *p)
{
    union R4 {float f; unsigned int u4;} u; 
    u.u4 = U4(p);
    return u.f;
}

/* ReadR8 - Fetch & convert an IEEE T_FLOAT (double) */
static double ReadR8(unsigned char *p)
{
    ENDIAN_TEST et;
    union R8 {double d; unsigned char c[8];} u;

    memcpy(&u.d, p, sizeof(u.d));
 
    et.u2 = 0; et.c[0] = 1;  
    if (et.u2 == 1)
    {
        unsigned char t;
        t = u.c[0]; u.c[0] = u.c[7]; u.c[7] = t;
        t = u.c[1]; u.c[1] = u.c[6]; u.c[6] = t;
        t = u.c[2]; u.c[2] = u.c[5]; u.c[5] = t;
        t = u.c[3]; u.c[3] = u.c[4]; u.c[4] = t;  
    }
    return u.d;
}

/* ReadU2 - Fetch & convert an unsigned twe byte integer (unsigned short) */
static unsigned short ReadU2(unsigned char *p)
{
    ENDIAN_TEST et;
    union U2 {unsigned short u2; unsigned char c[2];} u;

    memcpy(&u.u2, p, sizeof(u.u2)); 
 
    et.u2 = 0; et.c[0] = 1;  
    if (et.u2 == 1)
    {
        unsigned char t;
        t = u.c[0]; u.c[0] = u.c[1]; u.c[1] = t;
    }
    return u.u2;
}

/* ReadU4 - Fetch & convert a four byte unsigned integer (unsigned int) */
static unsigned int ReadU4(unsigned char *p)
{
    ENDIAN_TEST et;
    union U4 {unsigned int u4; unsigned char c[4];} u;

    memcpy(&u.u4, p, sizeof(u.u4));
 
    et.u2 = 0; et.c[0] = 1;  
    if (et.u2 == 1)
    {
        unsigned char t;
        t = u.c[0]; u.c[0] = u.c[3]; u.c[3] = t;
        t = u.c[1]; u.c[1] = u.c[2]; u.c[2] = t;
    }   
    return u.u4;
}

/*
| SetWeek - Set GPS week number
|
| The -WEEK=n initial week option overrides us.
*/
static void SetWeek(raw_t *Raw, int Week, double Tow)
{
    rt17_t *rt17 = (rt17_t*) Raw->rcv_data;

    if (!(rt17->Flags & M_WEEK_OPTION))
    {
        if (rt17->Week)
        {
            if (Week != rt17->Week)
            {
                if (Week == (rt17->Week + 1))
                    tracet(2, "RT17: GPS WEEK rolled over from %d to %d.\n", rt17->Week, Week);
                else
                    tracet(2, "RT17: GPS WEEK changed from %d to %d.\n", rt17->Week, Week);
            }
        }
        else
            tracet(2, "RT17: GPS WEEK initially set to %d.\n", Week);

        rt17->Week = Week;
    }

    /* Also update the time if we can */
    if (Week && (Tow != 0.0))
        Raw->time = gpst2time(Week, Tow);
}

/* SyncPacket - Synchronize the raw data stream to the start of a series of RT-17 packets */
static int SyncPacket(rt17_t *rt17, unsigned char Data)
{
    unsigned char Type, *PacketBuffer = rt17->PacketBuffer;

    PacketBuffer[0] = PacketBuffer[1];
    PacketBuffer[1] = PacketBuffer[2];
    PacketBuffer[2] = PacketBuffer[3];
    PacketBuffer[3] = Data;

    Type = PacketBuffer[2];

    /*
    | Byte 0 must be an STX character.
    | Byte 1 = status byte which we always ignore (for now).
    | Byte 2 = packet type which must be GENOUT (0x40) RAWDATA (0x57) or RETSVDATA (0x55) (for now).
    | Byte 3 = data length which must be non-zero for any packet we're interested in.
    */
    return ((PacketBuffer[0] == STX) && (Data != 0) && ((Type == GENOUT) || (Type == RAWDATA) || (Type == RETSVDATA)));
}

/*
| UnwrapGenout - Reassemble GENOUT message by removing packet headers, trailers and page framing
|
| The GENOUT message is broken up on _arbitrary byte boundries_ into
| pages of no more than 246 bytes each, then wrapped with page frames,
| packet headers and packet trailers. We reassemble the original message
| so that it is uninterrupted by removing the extraneous packet headers,
| trailers and page framing.
*/
static void UnwrapGenout(rt17_t *rt17)
{
    unsigned char *p_in = rt17->MessageBuffer;
    unsigned char *p_out = p_in;
    unsigned int InputLength, InputLengthTotal = rt17->MessageLength;
    unsigned int OutputLength, OutputLengthTotal = 0;

    while (InputLengthTotal > 0)
    {
        InputLength = p_in[3] + 6;
        OutputLength = p_in[3] - 3;
        memmove(p_out, p_in + 7, OutputLength);
        p_in += InputLength;
        p_out += OutputLength;
        OutputLengthTotal += OutputLength;
        InputLengthTotal -= InputLength;  
    }
    rt17->MessageBytes = rt17->MessageLength = OutputLengthTotal;
}

/*
| UnwrapRawdata - Reassemble message by removing packet headers, trailers and page framing
|
| The RAWDATA message is broken up on _arbitrary byte boundries_ into
| pages of no more than 244 bytes each, then wrapped with page frames,
| packet headers and packet trailers. We reassemble the original message
| so that it is uninterrupted by removing the extraneous packet headers,
| trailers and page framing.
|
| While we're at it we also check to make sure the Record Interpretation
| Flags are consistent. They should be the same in every page frame.
*/
static void UnwrapRawdata(rt17_t *rt17, unsigned int *rif)
{
    unsigned char *p_in = rt17->MessageBuffer;
    unsigned char *p_out = p_in;
    unsigned int InputLength, InputLengthTotal = rt17->MessageLength;
    unsigned int OutputLength, OutputLengthTotal = 0;

    *rif = p_in[7];

    while (InputLengthTotal > 0)
    {
        if ((unsigned int)p_in[7] != *rif)
           tracet(2, "RT17: Inconsistent Record Interpretation Flags within a single RAWDATA message.\n");

        InputLength = p_in[3] + 6;
        OutputLength = p_in[3] - 4;
        memmove(p_out, p_in + 8, OutputLength);
        p_in += InputLength;
        p_out += OutputLength;
        OutputLengthTotal += OutputLength;
        InputLengthTotal -= InputLength;
    }
    rt17->MessageBytes = rt17->MessageLength = OutputLengthTotal;
}
