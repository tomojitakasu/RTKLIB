#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T19:09:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../../RTKLib.pri)

INCLUDEPATH += ../../src/ ../appcmn_qt

linux{
    LIBS += -lpng ../../src/libRTKLib.a
}
win32 {
    debug:LIBS += ../../src/debug/libRTKLib.a -lWs2_32 -lwinmm
    else:LIBS += ../../src/release/libRTKLib.a -lWs2_32 -lwinmm
}

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

HEADERS  += \
    getmain.h \
    getoptdlg.h \
    staoptdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h \

FORMS    += \
    getmain.ui \
    getoptdlg.ui \
    staoptdlg.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui

RESOURCES += \
    rtkget_qt.qrc
