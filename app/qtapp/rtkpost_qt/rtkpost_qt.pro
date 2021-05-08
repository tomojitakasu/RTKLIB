#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui

include(../app.pri)

TARGET = rtkpost_qt
TEMPLATE = app

INCLUDEPATH += ../../src/ ../appcmn_qt

SOURCES += \ 
    extopt.cpp \
    kmzconv.cpp \
    postmain.cpp \
    postopt.cpp \
    rtkpost.cpp \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    ../appcmn_qt/maskoptdlg.cpp \
    ../appcmn_qt/refdlg.cpp \
    ../appcmn_qt/viewer.cpp \
    ../appcmn_qt/vieweropt.cpp \
    ../appcmn_qt/timedlg.cpp

HEADERS  += \ 
    extopt.h \
    kmzconv.h \
    postmain.h \
    postopt.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/maskoptdlg.h \
    ../appcmn_qt/refdlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/timedlg.h

FORMS    += \ 
    extopt.ui \
    kmzconv.ui \
    postmain.ui \
    postopt.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/maskoptdlg.ui \
    ../appcmn_qt/refdlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/timedlg.ui

RESOURCES += \
    rtkpost_qt.qrc

RC_FILE = rtkpost_qt.rc
