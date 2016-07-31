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
