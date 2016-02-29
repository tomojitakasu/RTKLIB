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

LIBS += -lpng ../../src/libRTKLib.a

SOURCES += \ 
    codeopt.cpp \
    convmain.cpp \
    convopt.cpp \
    rtkconv.cpp \
    startdlg.cpp

HEADERS  += \ 
    codeopt.h \
    convmain.h \
    convopt.h \
    startdlg.h

FORMS    += \ 
    codeopt.ui \
    convopt.ui \
    startdlg.ui

RESOURCES += \
    rtkconv_qt.qrc
