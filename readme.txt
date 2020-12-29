#
#  RTKLIB 2.4.3 Betas
#

DESCRIPTION

The development branch for RTKLIB 2.4.3.

UPDATE HISTORY

2014/09/07  2.4.3 b1  add 3-panel and veritical-panel modes for RTKNAVI
                      add sky image overlay to skyplot for RTKPLOT
                      fix invalid identification of obs type "C2" (#113)
                      fix invalid format of saved image by RTKPLOT (#114)
2014/10/21  2.4.3 b2  add pos2-bdsarmode for beidou amb-res option
                      add beidou amb-res option for rtknavi and rtkpost
                      support stdin/stdout if -in/-out omitted in str2str
                      strtok() -> strtok_r() in expath() for thread-safe
                      fix problem on week rollover in rtcm 2 type 14
                      fix problem on reading "C2" in rinex 2.12
                      fix bug on clock error variance in peph2pos()
                      fix bug on P0(a[3]) computation in tide_oload()
                      fix bug on m2 computation in tide_pole()
                      fix bug on receiver option -GL*,-RL*,-JL* for javad
                      fix bug on receiver option -GL*,-RL*,-EL* for novatel
2014/10/24  2.4.3 b3  fix bug on beidou amb-res with pos2-bdsarmode=0
                      fix bug on return of var_uraeph() if ura<0||15<ura
2014/11/08  2.4.3 b4  fix getconfig error (87) with bluetooth device
                      fix bug on ar-degradation by unhealthy satellites
                      support qzss navigation subframes by decode_frame()
                      add receiver option -RT_INP for rtcm3
                      support message RXM-RAWX and RXM-SFRBX for u-blox
                      add option -a, -i and -o for str2str
                      merge updates of src/rcv/rt17.c (pull-req #45)
2014/11/09  2.4.3 b5  support glonass, qzss and beidou for skytraq
2016/01/23  2.4.3 b6  merge 2.4.2 p11 bug fixes
                      update ppp/ppp-ar features
                      support septentrio
2016/01/26  2.4.3 b7  fix bugs #126,#127,#128,#129,#130,#131
2016/01/28  2.4.3 b8  support galileo navigation data in u-blox RXM-SFRBX
2016/05/10  2.4.3 b9  unsupport ppp-ar features
2016/05/25  2.4.3 b10 fix several bugs
                      update data/TLE_GNSS_SATNO.txt
                      repository for RTKLIB/bin is moved to new RTKLIB_bin to
                      reduce repository size.
                      septentrio.c is updated by Apr 6 ver by JensReimann.
2016/06/12  2.4.3 b11 fix several bugs
                      support solution ground track pane in rtknavi
                      support gpx conversion function to rtkpost and pos2kml
                      support shapefile in rtkplot
                      add data/ubx_m8p_ref_glo/bds_1hz.cmd
2016/06/21  2.4.3 b12 support GPX format as waypoints of RTKPLOT
                      add menu Map Layer of RTKPLOT
                      support solution status format as input of RTKPLOT
                      support solution status format as output of RTKNAVI
2016/07/19  2.4.3 b13 support IRNSS
                      support RINEX 3.03
                      add src/rcv/cmr.c
                      add half-cycle vaild check for ublox ubx-trk-meas
                      add leap second before 2017/1/1 00:00:00
                      support averaging single pos as base position
2016/07/22  2.4.3 b14 fix a fatal bug for baseline analysis
2016/07/29  2.4.3 b15 merge Qt port by JensReimann
                      add output of received stream to tcp port for serial
                      nename api compress() -> rtk_uncompress()
                      nename api crc16()    -> rtk_crc16()
                      nename api crc24q()   -> rtk_crc24q()
                      nename api crc32()    -> rtk_crc32()
2016/07/31  2.4.3 b16 fix several bugs
                      improve draw speed of gis data in rtkplot
                      add tcp output option of input stream from serial output
                      add command of serial output
                      add out-outsingle, out-maxsolstd options
                      add commands for u-blox M8P
2016/08/20  2.4.3 b17 fix several bugs
                      merge pull request #180
                      merge pull request #199
                      merge pull request #206
                      binary ap compiled by C++ builder 10.1 Berlin
                      add ap rtkpost_win64, rtknavi_win64, rnx2rtkp_win64
                      add ap rtkpost_win64, rtknavi_win64, rnx2rtkp_win64
2016/08/20  2.4.3 b18 fix #134 (loss-of-lock not recognized in RTCM MSM 6 or 7)
2016/08/29  2.4.3 b19 add option -STD_SLIP for u-blox receiver driver
                      fix on half-cyc valid for sbas in u-blox trkmeas
                      update kelper terminate condition for alm2pos(),eph2pos()
                      fix bug on week overflow in time2gpst(),gpst2time()
                      fix bug on starting serial thread for windows
                      fix bug on mark and path in Google Earth view of RTKPLOT
                      add protocol analysis in input stream monitor of STRSVR
                      add window size extension of RTKPOST and RTKCONV
2016/09/03  2.4.3 b20 add ntrip caster functions
2016/09/05  2.4.3 b21 fix several bugs
2016/09/06  2.4.3 b22 fix several bugs
                      add app/str2str/run_cast.sh for ntrip caster control 
2016/09/06  2.4.3 b23 fix several bugs
                      add -sys option for rnx2rtkp
                      add 4- and 5-panel modes for rtknavi
2016/09/19  2.4.3 b24 fix several bugs
                      change api rtksvrstart()
                      add minimized panel mode of rtklaunch
                      add relay back message of output streams to strsvr
                      add option -b to str2str
                      support multiple remote console connections by rtkrcv
                      add option -w to rtkrcv
2016/09/25  2.4.3 b25 fix several bugs
                      add ap rtkvideo
                      add ap rtkvplayer
                      change build environment to C++ builder 10.1 Berlin
2016/10/01  2.4.3 b26 fix several bugs
                      change api rtksvrstart(), strsvrstart()
                      support udp server and client for stream types
                      support periodic receiver commands by rtknavi, strsvr
2017/04/25  2.4.3 b27 fix several bugs
2017/05/26  2.4.3 b28 fix bug on decoding skytraq extended raw message
                      add rcv/tersus.c to support tersus BX306
2017/09/02  2.4.3 b29 fix bugs
2018/10/10  2.4.3 b30 support u-blox zed-f9p
                      support api-key for gmview of rtkplot
                      improve galileo sisa, i/nav and f/nav hadling
                      fix many bugs
2018/11/05  2.4.3 b31 update rtcm mt for beidou ephemeirs (1047->1042)
                      fix bug on default stream playback speed (= 0)
                      fix bug on stream file playback as slave mode
                      fix bug on timeset() in gpst instead of utc
                      fix problem on invalid time in message monitor for rtcm 3
                      fix problem on number of cell-mask overflow for rtcm msm (#143)
                      fix problem on missing QZSS L2C signal for u-blox rxm-rawx
2019/05/10  2.4.3 b32 support beidou C36-37 (#145)
                      fix bug on dropping message on tcp stream (#144)
                      save galileo E5b data to obs index 2 in struct obs_t
                      disable ambiguity resolution of gps-qzss for rel-positioning
                      add test of i/nav word type 5 on reading galileo ephemeris
                      support u-blox zed-f9p rxm-rawx, rxm-sfrbx
2019/08/19  2.4.3 b33 support galileo sisa index for reading rinex nav data
                      support binex upgraded galileo ephemeris (0x01-14)
                      support 460800 and 921600 bps for serial stream
                      fix bug on return value of resamb_LAMBDA() error
2020/12/30  2.4.3 b34
    GENERAL:
        NavIC (IRNSS) completely supported.
        RINEX 3.04 supported. BDS-3 and QZSS new signals added.
        RTCM 3.3 amendment-1 supported. MT1041/1131-7 (NavIC ephemeris/MSM) added.
        RTCM3 MT1230 (GLONASS code-phase biases) supported.
        RTCM3 MT4076 (IGS SSR) supported.
        GNSS singal ID changed: L1,L2,L5/3,L6,L7,L8,L9 -> L1,L2,L3,L4,L5.
        Only Windows 64bit APs supported. 32bit APs deleted. 
        Windows scaled DPI APs supported for different DPI screens.
        Directories RTKLIB/app and RTKLIB/data reorganized.
        License clarified. See RTKLIB/LICENSE.txt.
        Bugs and problems fixed including GitHub Issues:
          #461,#477,#480,#514,#540,#547,#555,#560.
    LIBRARY API:
        The following APIs added:
          code2freq(),sat2freq(),code2idx(),timereset(),setseleph(),getseleph(),
          decode_gal_fnav(),decode_irn_nav()
        The following APIs modified:
          obs2code(),code2obs(),setcodepri(),getcodepri(),tickget(),traceb(),
          getbitu(),getbits(),setbitu(),getbits(),rtk_crc32(),rtk_crc24q(),
          rtk_crc16(),docode_word(),decode_frame(),test_glostr(),decode_glostr(),
          decode_bds_d1(),decode_bds_d2(),decode_gal_inav(),input_raw(),input_oem4(),
          input_oem3(),input_ubx(),input_ss2(),input_cres(),input_stq(),gen_ubx(),
          gen_stq(),gen_nvs(),input_rtcm2(),input_rtcm3(),gen_rtcm3(),inputsol(),
          outprcopts(),outsolheads(),outsols(),outsolexs(),outnmea_rmc(),
          out_nmea_gga(),outnmea_gsa(),outnmea_gsv(),sbsdecodemsg(),strread(),
          strwrite(),strsvrstart(),strsvrstat(),showmsg()
        The following APIs deleted:
          lam_carr[],satwavelen(),csmooth(),satseleph(),input_gw10(),input_cmr(),
          input_tersus(),input_lexr(),input_gw10f(),input_cmrf(),input_tersusf(),
          input_lexrf(),gen_lexr(),strgetsel(),strsetsel(),strsetsrctbl(),
          pppcorr_read(),pppcorr_free(),pppcorr_trop().pppcorr_stec(),
          stsvrsetsrctbl(),init_imu(),input_imu(),lexupdatecorr(),lexreadmsg(),
          lexoutmsg(),lexconvbin(),lexeph2pos(),lexioncorr()
        The following types modified:
          obsd_t,eph_t,nav_t,rtcm_t,rnxctr_t,prcopt_t,rnxopt_t,ssat_t,raw_t,strsvr_t
        The following types deleted:
          lexmsg_t,lex_t,lexeph_t,lexion_t,stec_t,trop_t,pppcorr_t,exterr_t,
          half_cyc_t,imud_t,imu_t
    RECEIVER SUPPORTS:
        BINEX NavIC/IRNSS in raw observation data (0x7f-05) supported.
        BINEX IRNSS decoded ephemeris (0x01-07) supported.
        BINEX station info in site metadata (0x00) supported.
        NovAtel OEM7 supported including the following messages:
          RANGEB(43),RANGECMPB(140),RAWEPHEM(41),IONUTCB(8),RAWWAASFRAMEB(287),
          RAWSBASFRAMEB(973),GLOEPHEMERISB(723),GALEPHEMERISB(1122),GALIONOB(1127),
          GALCLOCKB(1121),QZSSRAWEPHEMB(1331),QZSSRAWSUBFRAMEB(1330),QZSSIONUTCB(1347),
          BDSEPHEMERISB(1696),NAVICEPHEMERISB(2123)
        Codes for Septentrio SBF re-written to support Mosaic-X5.
        Septentrio SBF supported including the following messages:
          MEAESPOCH(4027),GPSRAWCA(4017),GLORAWCA(4026),GALRAWFNAV(4022),
          GALRAWINAV(4023),GEORAWL1(4020),BDSRAW(4047),QZSRAWL1CA(4066),
          NAVICRAW(4093)
        u-blox UBX-CFG-VALDEL,VALGET,VALSET messages supported.
        u-blox UBX-RXM-RAWX half-cycle phase shift for BDS GEO satellites added.
        u-blox UBX-RXM-RAWX QZSS L1S supported.
    RTKPLOT:
        Leaflet and standard map tiles (OpenSteetMap etc.) supported in Map View.
        Initial loaded shape files supported.
        Residuals to elevation (Resid-EL) Plot added to solution plots.
        Positions files (.pos) supported by menu Open Waypoint.
        Sliderbar for solution animation widely expanded.
        Line and Fill colors supported in Map Layer dialog.
        TEQC no longer supported for observation data QC. Menu Obs Data QC deleted.
        GE (Google Earth) view, menus and button deleted.
    RTKCONV:
        RINEX 3.04 output supported.
        RINEX NAV BDSA/B and IRNA/B IONOS CORR output supported.
        RINEX NAV GLUT, GAUT, QZUT, BDUT and IRUT TIME SYS CORR output supported.
        RINEX NAV DT_LSF, WN_LSF and DN for LEAP SEC output supported.
        RINEX version check added to exclude unsupported systems and signals.
        Always two-pass processing. Option Scan Obs Types deleted.
        Option Phase Shift added to align carrier phases to refernece signals.
        Option GLONASS FCN added for receiver logs without FCN info like RTCM3 MSM4.
        Default receiver log time obtained from the time-tag file if it exists.
        Recursive new directory generation supported.
        High resolution (16bit) C/N0 suppored.
        Switch of reference stations supported in a RTCM3 log file.
        Format GW10, CMR/CMR+ and TERSUS for receiver logs no longer supported. 
        RINEX 2.12 QZS extension no longer supported.
        LEX log output no longer supported.
    STRSVR:
        Maximum 6 output streams supported.
        Input Log and Output Return Log supported.
        MT1131-7,1041 and 1230 supported for RTCM3 conversion.
        Galileo I/NAV and F/NAV are separately handled in RTCM3 conversion.
        Protocol HTTP/1.1 accepted by NTRIP caster mode.
        Button Get Mountp added in NTRIP Client Options dialog.
        Button Mountp Options added in NTRIP Caster Options dialog.
    RTKPOST:
        TGD and BGD handling improved for Galileo and BDS.
        Always L1-L2 (E1-E5b for Galileo, B1-B2 for BDS) used for Iono-Free LC. 
        Always I/NAV used for Galileo orbits and clocks.
        SP3-d format for precise ephemerides supported.
        CPU usage much improved in SD to DD conversion for ambiguity resolution.
        OpenBLAS linked instead of Intel MKL for fast-matrix computation.
        Option QZSS LEX and Input STEC for ionos-correction no longer supported.
        Option Input ZTD for troposphere correction no longer supported.
        AP RTKPOST_WIN64 and RTKPOST_MKL deleted.
    SRCTBLBROWS:
        Leaflet and OpenStreetMap used instead of Google Maps in Stream Map.
    RTKNAVI:
        NMEA talker ID GQ and GI (NMEA 0183 4.11) for QZSS and NavIC supported.
        NMEA GQ/GB/GI-GSA/GSV sentences supported.
        Option Panel Font added.
        Menus reorganized in RTK Monitor.
        Menu Station Info added to RTK Monitor.
        Satellite positions in Skyplot by TLE data no longer supported.
        Menu LEX and Iono Correction deleted from RTK Monitor.
        AP RTKNAVI_WIN64 deleted.
    RTKGET:
        Data source URL https://... and ftps://... supported.
        Wild-card (*) in file path of data source URL supported.
        Compressed RINEX files with extension .crx or .CRX supported.
    CONVBIN:
        Option -scan forced. Option -noscan deleted.
        Most of new features for RTKCONV involved but not completed.
    RNX2RTKP:
        Most of new features for RTKPOST involved but not completed.
    RTKRCV:
        Most of new features for RTKNAVI involved but not completed.
    STR2STR:
        Most of new features for STRSVR involved but not completed.
    RTKVIDEO and RTKVPLALER:
        APs deleted.
    DATA:
        TLE_GNSS_SATNO.txt, URL_LIST.txt and ant/igs14.atx updated.
    
    LIMITATIONS:
        QT ported APs are just moved to RTKLIB/app/qtapp but not maintained.
        Documents in RTKLIB/doc are not updated.

