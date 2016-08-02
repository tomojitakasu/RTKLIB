#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T19:09:57
#
#-------------------------------------------------

QT       += core gui widgets

include(../../RTKLib.pri)

qtHaveModule(webenginewidgets) {
    QT+= webenginewidgets
    DEFINES+=QWEBENGINE
} else {
    qtHaveModule(webkitwidgets) {
        QT+= webkitwidgets
        DEFINES+= QWEBKIT
    }
}

INCLUDEPATH += ../../src/ ../appcmn_qt

win32-g++ {
    CONFIG(debug) {
        RTKLIB = ../../src/debug/libRTKLib.a
    } else {
        RTKLIB =../../src/release/libRTKLib.a
    }

    LIBS += $${RTKLIB} -lWs2_32 -lwinmm
} else {
    RTKLIB =../../src/libRTKLib.a
    LIBS += -lpng $${RTKLIB}
}

PRE_TARGETDEPS = $${RTKLIB}

TARGET = srctblbrows_qt
TEMPLATE = app


SOURCES += \ 
    browsmain.cpp \
    srctblbrows.cpp \
    staoptdlg.cpp \
    ../appcmn_qt/aboutdlg.cpp \
    ../appcmn_qt/gmview.cpp

HEADERS  += \ 
    browsmain.h \
    staoptdlg.h \
    ../appcmn_qt/aboutdlg.h \
    ../appcmn_qt/gmview.h

FORMS    += \ 
    browsmain.ui \
    staoptdlg.ui \
    ../appcmn_qt/aboutdlg.ui \
    ../appcmn_qt/gmview.ui

RESOURCES +=  \
    srctblbrows_qt.qrc

RC_FILE = srctblbrows_qt.rc
