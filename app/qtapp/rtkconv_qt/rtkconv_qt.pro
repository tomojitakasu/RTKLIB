#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets 
}

include(../../RTKLib.pri)

TARGET = rtkconv_qt
TEMPLATE = app

INCLUDEPATH += ../../src/ ../appcmn_qt 

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
    codeopt.cpp \
    convmain.cpp \
    convopt.cpp \
    rtkconv.cpp \
    startdlg.cpp \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    ../appcmn_qt/timedlg.cpp \
    ../appcmn_qt/viewer.cpp \
    ../appcmn_qt/vieweropt.cpp

HEADERS  += \ 
    codeopt.h \
    convmain.h \
    convopt.h \
    startdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/timedlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h

FORMS    += \ 
    codeopt.ui \
    convopt.ui \
    startdlg.ui \
    convmain.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/timedlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui

RESOURCES += \
    rtkconv_qt.qrc

RC_FILE = rtkconv_qt.rc
