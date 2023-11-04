#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T19:09:57
#
#-------------------------------------------------

QT       += core gui widgets serialport

include(../qtapp.pri)

INCLUDEPATH += ../../../src/ ../appcmn_qt

TARGET = strsvr_qt
TEMPLATE = app

SOURCES += \  
    convdlg.cpp \
    strsvr.cpp \
    svrmain.cpp \
    svroptdlg.cpp \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/tcpoptdlg.cpp \
    ../appcmn_qt/serioptdlg.cpp \
    ../appcmn_qt/cmdoptdlg.cpp \
    ../appcmn_qt/console.cpp \
    ../appcmn_qt/fileoptdlg.cpp \
    ../appcmn_qt/ftpoptdlg.cpp \
    ../appcmn_qt/refdlg.cpp \
    ../appcmn_qt/mntpoptdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    mondlg.cpp

HEADERS  += \ 
    convdlg.h \
    svrmain.h \
    svroptdlg.h \
    ../appcmn_qt/tcpoptdlg.h \
    ../appcmn_qt/serioptdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/cmdoptdlg.h \
    ../appcmn_qt/console.h \
    ../appcmn_qt/fileoptdlg.h \
    ../appcmn_qt/ftpoptdlg.h \
    ../appcmn_qt/refdlg.h \
    ../appcmn_qt/mntpoptdlg.h \
    ../appcmn_qt/keydlg.h \
    mondlg.h

FORMS    += \ 
    convdlg.ui \
    svrmain.ui \
    svroptdlg.ui \
    ../appcmn_qt/tcpoptdlg.ui \
    ../appcmn_qt/serioptdlg.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/cmdoptdlg.ui \
    ../appcmn_qt/console.ui \
    ../appcmn_qt/fileoptdlg.ui \
    ../appcmn_qt/ftpoptdlg.ui \
    ../appcmn_qt/refdlg.ui \
    ../appcmn_qt/mntpoptdlg.ui \
    ../appcmn_qt/keydlg.ui \
    mondlg.ui

RESOURCES +=  \
    strsvr_qt.qrc

RC_FILE = strsvr_qt.rc
