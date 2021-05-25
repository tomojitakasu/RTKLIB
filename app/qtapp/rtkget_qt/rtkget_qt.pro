#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T19:09:57
#
#-------------------------------------------------

QT       += core gui widgets

include(../qtapp.pri)

INCLUDEPATH += ../../../src/ ../appcmn_qt

TARGET = rtkget_qt
TEMPLATE = app


SOURCES += main.cpp \
    getmain.cpp \
    getoptdlg.cpp \
    staoptdlg.cpp \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    ../appcmn_qt/viewer.cpp \
    ../appcmn_qt/vieweropt.cpp \
    ../appcmn_qt/timedlg.cpp

HEADERS  += \
    getmain.h \
    getoptdlg.h \
    staoptdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h \
    ../appcmn_qt/timedlg.h

FORMS    += \
    getmain.ui \
    getoptdlg.ui \
    staoptdlg.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui \
    ../appcmn_qt/timedlg.ui

RESOURCES += \
    rtkget_qt.qrc


RC_FILE = rtkget_qt.rc
