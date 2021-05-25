#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui xml serialport

qtHaveModule(webenginewidgets) {
    QT += webenginewidgets
    DEFINES +=QWEBENGINE
} else {
    qtHaveModule(webkitwidgets) {
        QT += webkitwidgets
        DEFINES += QWEBKIT
    }
}
include(../qtapp.pri)

TARGET = rtkplot_qt
TEMPLATE = app

INCLUDEPATH += ../../../src/ ../appcmn_qt

SOURCES += \
    ../appcmn_qt/aboutdlg.cpp \
    conndlg.cpp \
    mapoptdlg.cpp \
    plotcmn.cpp \
    plotdata.cpp \
    plotdraw.cpp \
    plotinfo.cpp \
    plotmain.cpp \
    plotopt.cpp \
    pntdlg.cpp \
    rtkplot.cpp \
    skydlg.cpp \
    ../appcmn_qt/refdlg.cpp \
    ../appcmn_qt/freqdlg.cpp \
    ../appcmn_qt/viewer.cpp \
    ../appcmn_qt/vieweropt.cpp \
    ../appcmn_qt/cmdoptdlg.cpp \
    ../appcmn_qt/fileoptdlg.cpp \
    ../appcmn_qt/serioptdlg.cpp \
    ../appcmn_qt/tcpoptdlg.cpp \
    ../appcmn_qt/mntpoptdlg.cpp \
    ../appcmn_qt/keydlg.cpp \
    ../appcmn_qt/graph.cpp \
    ../appcmn_qt/console.cpp \
    ../appcmn_qt/tspandlg.cpp \
    fileseldlg.cpp \
    mapview.cpp \
    mapviewopt.cpp \
    vmapdlg.cpp

HEADERS  += \
    ../appcmn_qt/aboutdlg.h \
    conndlg.h \
    mapoptdlg.h \
    plotmain.h \
    plotopt.h \
    pntdlg.h \
    skydlg.h \
    ../appcmn_qt/refdlg.h \
    ../appcmn_qt/freqdlg.h \
    ../appcmn_qt/viewer.h \
    ../appcmn_qt/vieweropt.h \
    ../appcmn_qt/cmdoptdlg.h \
    ../appcmn_qt/fileoptdlg.h \
    ../appcmn_qt/serioptdlg.h \
    ../appcmn_qt/tcpoptdlg.h \
    ../appcmn_qt/mntpoptdlg.h \
    ../appcmn_qt/keydlg.h \
    ../appcmn_qt/graph.h \
    ../appcmn_qt/console.h \
    ../appcmn_qt/tspandlg.h \
    fileseldlg.h \
    mapview.h \
    mapviewopt.h \
    vmapdlg.h

FORMS    += \
    ../appcmn_qt/aboutdlg.ui \
    conndlg.ui \
    mapoptdlg.ui \
    plotmain.ui \
    plotopt.ui \
    pntdlg.ui \
    skydlg.ui \
    ../appcmn_qt/refdlg.ui \
    ../appcmn_qt/freqdlg.ui \
    ../appcmn_qt/viewer.ui \
    ../appcmn_qt/vieweropt.ui \
    ../appcmn_qt/cmdoptdlg.ui \
    ../appcmn_qt/fileoptdlg.ui \
    ../appcmn_qt/serioptdlg.ui \
    ../appcmn_qt/tcpoptdlg.ui \
    ../appcmn_qt/mntpoptdlg.ui \
    ../appcmn_qt/keydlg.ui \
    ../appcmn_qt/console.ui \
    ../appcmn_qt/tspandlg.ui \
    fileseldlg.ui \
    mapview.ui \
    mapviewopt.ui \
    vmapdlg.ui

RESOURCES += \
    rtkplot_qt.qrc

RC_FILE = rtkplot_qt.rc
