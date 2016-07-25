#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T08:58:44
#
#-------------------------------------------------

QT       -= core gui

TARGET = RTKLib
TEMPLATE = lib
CONFIG += staticlib

include(../RTKLib.pri)

QMAKE_CFLAGS += -Wall -ansi -pedantic -Wno-unused-but-set-variable  -DTRACE -g
DEFINES -= UNICODE

SOURCES += rtkcmn.c \
    convkml.c \
    convrnx.c \
    convgpx.c \
    datum.c \
    download.c \
    ephemeris.c \
    geoid.c \
    gis.c \
    ionex.c \
    lambda.c \
    options.c \
    pntpos.c \
    postpos.c \
    ppp.c \
    ppp_ar.c \
    ppp_corr.c \
    preceph.c \
    qzslex.c \
    rcvraw.c \
    rinex.c \
    rtcm.c \
    rtcm2.c \
    rtcm3.c \
    rtcm3e.c \
    rtkpos.c \
    rtksvr.c \
    sbas.c \
    solution.c \
    stream.c \
    streamsvr.c \
    tides.c \
    tle.c \
    rcv/binex.c \
    rcv/crescent.c \
    rcv/gw10.c \
    rcv/javad.c \
    rcv/novatel.c \
    rcv/nvs.c \
    rcv/rcvlex.c \
    rcv/rt17.c \
    rcv/septentrio.c \
    rcv/skytraq.c \
    rcv/ss2.c \
    rcv/ublox.c \
    rcv/cmr.c

HEADERS += rtklib.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
