#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets serialport
    DEFINES += QT5
}

lessThan(QT_MAJOR_VERSION, 5) {
    LIBS += -lqextserialport-1.2
    DEFINES += QEXTSERIALPORT
}

include(../../RTKLib.pri)

TARGET = rtknavi_qt
TEMPLATE = app

INCLUDEPATH += ../../src/ ../appcmn_qt ../rtkplot_qt

linux{
    RTKLIB =../../src/libRTKLib.a
    LIBS += -lpng $${RTKLIB}
}
win32 {
    CONFIG(debug) {
        RTKLIB = ../../src/debug/libRTKLib.a
    } else {
        RTKLIB =../../src/release/libRTKLib.a
    }

    LIBS+= $${RTKLIB} -lWs2_32 -lwinmm
}

PRE_TARGETDEPS = $${RTKLIB}
SOURCES += \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/cmdoptdlg.cpp \
    ../appcmn_qt/fileoptdlg.cpp \
    ../appcmn_qt/ftpoptdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    ../appcmn_qt/maskoptdlg.cpp \
    ../appcmn_qt/refdlg.cpp \
    ../appcmn_qt/serioptdlg.cpp \
    ../appcmn_qt/tcpoptdlg.cpp \
    ../appcmn_qt/viewer.cpp \
    ../appcmn_qt/vieweropt.cpp \
    instrdlg.cpp \
    logstrdlg.cpp \
    main.cpp \
    mondlg.cpp \
    navimain.cpp \
    naviopt.cpp \
    outstrdlg.cpp \
    rcvoptdlg.cpp \
    markdlg.cpp \
    ../appcmn_qt/graph.cpp

HEADERS  += \
    instrdlg.h \
    rcvoptdlg.h \
    logstrdlg.h \
    mondlg.h \
    navimain.h \
    naviopt.h \
    outstrdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/cmdoptdlg.h \
    ../appcmn_qt/fileoptdlg.h \
    ../appcmn_qt/ftpoptdlg.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/maskoptdlg.h \
    ../appcmn_qt/refdlg.h \
    ../appcmn_qt/serioptdlg.h \
    ../appcmn_qt/tcpoptdlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h \
    markdlg.h \
    ../appcmn_qt/graph.h

FORMS    += \
    instrdlg.ui \
    logstrdlg.ui \
    navimain.ui \
    naviopt.ui \
    mondlg.ui \
    outstrdlg.ui \
    rcvoptdlg.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/cmdoptdlg.ui \
    ../appcmn_qt/fileoptdlg.ui \
    ../appcmn_qt/ftpoptdlg.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/maskoptdlg.ui \
    ../appcmn_qt/refdlg.ui \
    ../appcmn_qt/serioptdlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui \
    ../appcmn_qt/tcpoptdlg.ui \
    markdlg.ui

RESOURCES += \
    rtknavi_qt.qrc

RC_FILE = rtknavi_qt.rc
