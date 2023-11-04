#-------------------------------------------------------------------------------
#
# makefile
#
# Purpose:
#
#   Linux make file for software libraries
#
# Last modified:
#
#   2021/06/10  AHA  Created
#   2022/11/02  AHA  Added clean-up of old Qt makefiles
#   2022/11/22  AHA  Restored compilation of iers lib
#
#-------------------------------------------------------------------------------

# Paths

RTKLIB     = .
RTKLIB_bin = $(RTKLIB)/bin

IERS       = $(RTKLIB)/lib/iers/gcc
CONSAPP    = $(RTKLIB)/app/consapp
QTAPP      = $(RTKLIB)/app/qtapp

GENCRC     = $(RTKLIB)/util/gencrc
GENIONO    = $(RTKLIB)/util/geniono
RNX2RTCM   = $(RTKLIB)/util/rnx2rtcm
SIMOBS     = $(RTKLIB)/util/simobs/gcc
UTEST      = $(RTKLIB)/test/utest

# Get number of parallel build jobs

ifneq ($(shell which nproc 2> /dev/null),)
  NJOBS = $(shell nproc)
else
  NJOBS = 1
endif

# Parallel compilation on Linux and Cygwin

PMAKE = make
OS = $(shell uname -o)
ifeq ($(OS),GNU/Linux)
  PMAKE = make -j $(NJOBS)
endif
ifeq ($(OS),Cygwin)
  PMAKE = make -j $(NJOBS)
endif

# Operating system dependent settings for qmake
#
# Default: use standard qmake
QMAKE = qmake

# Linux: use qmake or qmake6
ifeq ($(OS),GNU/Linux)
  ifeq ($(shell which qmake-qt5  2> /dev/null),)
    QMAKE = qmake
  else
    QMAKE = qmake6  # if qmake is not available (e.g. openSuse)
  endif
endif

# Targets

all: init \
	 iers_ consapp_ qtapp_ \
	 gencrc_ rnx2rtcm_  simobs_ utest_ \
	 install_

# Create directory tree

init:
	if [ ! -d "$(RTKLIB_bin)" ]; then mkdir -p $(RTKLIB_bin); fi

iers_:
	cd $(IERS); $(PMAKE)

consapp_:
	cd $(CONSAPP); $(PMAKE)

qtapp_:
	cd $(QTAPP); $(QMAKE) qtapp.pro; $(PMAKE);

gencrc_:
	cd $(GENCRC); $(PMAKE)

geniono_:
	cd $(GENIONO); $(PMAKE)

rnx2rtcm_:
	cd $(RNX2RTCM); $(PMAKE)
	
simobs_:
	cd $(SIMOBS); $(PMAKE)

utest_:
	cd $(UTEST); $(PMAKE)
	
install_:
	cd $(CONSAPP); make install
	for F in $$(ls -d $(QTAPP)/*/*_qt); do cp $$F $(RTKLIB_bin); done

# Clean up

clean:
	if [ -d "$(RTKLIB_bin)" ]; then cd $(RTKLIB_bin); rm -f *_qt; fi
	cd $(IERS);     make clean
	cd $(CONSAPP);  make clean
	cd $(QTAPP);    make clean
	cd $(UTEST);    make clean
	cd $(GENCRC);   make clean
	cd $(GENIONO);  make clean
	cd $(RNX2RTCM); make clean
	cd $(SIMOBS);   make clean 
	for F in $$(ls -d $(QTAPP)/*_qt); do rm $${F}/$${F##*/}; rm $$F/Makefile; done
