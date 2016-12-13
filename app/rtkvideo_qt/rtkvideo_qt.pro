#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T18:29:59
#
#-------------------------------------------------

QT       += widgets core gui multimedia multimediawidgets

include(../app.pri)

TARGET = rtkvideo_qt
TEMPLATE = app

INCLUDEPATH += ../../src/ ../appcmn_qt

SOURCES += \
    videoopt.cpp \
    videomain.cpp \ 
    rtkvideo.cpp \
    cameraframegrabber.cpp

HEADERS  += \
    videomain.h \
    videoopt.h \
    cameraframegrabber.h

FORMS    += \
    videomain.ui \
    videoopt.ui

RESOURCES += \
    rtkvideo_qt.qrc

#RC_FILE = rtkvideo_qt.rc
