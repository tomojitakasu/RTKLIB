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
