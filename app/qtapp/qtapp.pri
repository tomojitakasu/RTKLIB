include(../../RTKLib.pri)

# save root directory
ROOT_DIRECTORY = $${PWD}/../../
OUTPUT_DIRECTORY = $${OUT_PWD}

LIBS += -L$${ROOT_DIRECTORY}/lib/ -lRTKLib

IERS_MODEL {
    LIBS += -liers -lgfortran
}

win* {
    LIBS += -lws2_32 -lwinmm
}

QMAKE_RPATHDIR *= $${ROOT_DIRECTORY}/lib

PRE_TARGETDEPS = $${ROOT_DIRECTORY}/src/rtklib.h

CONFIG += c++11 debug
