cmake_minimum_required(VERSION 3.10)

# set the project name
project(rtklib)

# add the executable
add_library(rtk
src/convgpx.c
src/convkml.c
src/convrnx.c
src/datum.c
src/download.c
src/ephemeris.c
src/geoid.c
src/gis.c
src/ionex.c
src/lambda.c
src/options.c
src/pntpos.c
src/postpos.c
src/ppp.c
src/ppp_ar.c
src/preceph.c
src/rcvraw.c
src/rinex.c
src/rtcm.c
src/rtcm2.c
src/rtcm3.c
src/rtcm3e.c
src/rtkcmn.c
src/rtkpos.c
src/rtksvr.c
src/sbas.c
src/solution.c
src/stream.c
src/streamsvr.c
src/tides.c
src/tle.c
src/rcv/binex.c
src/rcv/crescent.c
src/rcv/javad.c
src/rcv/novatel.c
src/rcv/nvs.c
src/rcv/rt17.c
src/rcv/septentrio.c
src/rcv/skytraq.c
src/rcv/ss2.c
src/rcv/ublox.c
)


include_directories(rtk
./src/rcv
./src
)