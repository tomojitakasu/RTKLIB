#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T19:09:57
#
#-------------------------------------------------

QT       += core gui widgets

include(../qtapp.pri)

qtHaveModule(webenginewidgets) {
    QT+= webenginewidgets
    DEFINES+=QWEBENGINE
} else {
    qtHaveModule(webkitwidgets) {
        QT+= webkitwidgets
        DEFINES+= QWEBKIT
    }
}

INCLUDEPATH += ../../../src/ ../appcmn_qt

TEMPLATE = app

SOURCES += \ 
    browsmain.cpp \
    srctblbrows.cpp \
    staoptdlg.cpp \
    ../appcmn_qt/gmview.cpp \
    ../appcmn_qt/aboutdlg.cpp

HEADERS  += \ 
    browsmain.h \
    staoptdlg.h \
    ../appcmn_qt/gmview.h \
    ../appcmn_qt/aboutdlg.h

FORMS    += \ 
    browsmain.ui \
    staoptdlg.ui \
    ../appcmn_qt/gmview.ui \
    ../appcmn_qt/aboutdlg.ui

RESOURCES +=  \
    srctblbrows_qt.qrc

RC_FILE = srctblbrows_qt.rc
