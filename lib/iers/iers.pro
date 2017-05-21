#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T08:58:44
#
#-------------------------------------------------

QT       -= core gui

TARGET = iers
TEMPLATE = lib

include(../../RTKLib.pri)

FORTRAN_FLAGS += -O3 -ffixed-line-length-132

CONFIG += staticlib

gfortran.commands = gfortran $${FORTRAN_FLAGS} ${QMAKE_FILE_NAME} -c -o ${QMAKE_FILE_OUT}
gfortran.input = FORTRAN_SOURCE
gfortran.output = /${QMAKE_FILE_BASE}.o
gfortran.CONFIG = target_predeps

# the only change required I guess
gfortran.variable_out = FORTRAN_OBJ

QMAKE_EXTRA_COMPILERS += gfortran 

archive.commands = ar -qsc ${QMAKE_FILE_OUT} $${FORTRAN_OBJ}
archive.input = FORTRAN_OBJ
# I suggest to use $$OUT_PWD here
archive.output = libiers.a
archive.CONFIG = combine target_predeps
QMAKE_EXTRA_COMPILERS += archive

SOURCES += src/cal2jd.f\
	   src/dat.f \
	   src/dehanttideinel.f \
	   src/gmf.f \
	   src/gpt.f \
	   src/norm8.f \ 
	   src/sprod.f \
	   src/st1idiu.f \
           src/st1isem.f \
	   src/st1l1.f \
	   src/step2diu.f \
	   src/step2lon.f \
	   src/vmf1.f \
	   src/vmf1_ht.f \
	   src/zero_vec8.f

