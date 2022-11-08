# makefile for simobs

BINDIR = ../../../bin
SRC    = ../../../src
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO -DENAGAL -DENAQZS -DNFREQ=4 -DMAXOBS=128
LDLIBS  = -lm

simobs     : simobs.o ephemeris.o preceph.o rinex.o rtkcmn.o sbas.o


simobs.o   : ../simobs.c
	$(CC) -c $(CFLAGS) ../simobs.c
ephemeris.o   : $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
preceph.o    : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c	
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
sbas.o   : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c

simobs.o    : $(SRC)/rtklib.h
ephemeris.o : $(SRC)/rtklib.h
preceph.o   : $(SRC)/rtklib.h
rtkcmn.o    : $(SRC)/rtklib.h
sbas.o      : $(SRC)/rtklib.h

install:
	cp simobs $(BINDIR)

clean:
	rm -f simobs simobs.exe *.o

OPT0=-ts 2009/4/1 -te 2009/4/2 -ti 10
OPT1=../sim/brdc0910.09n ../sim/brdc0910.09g ../sim/brdc0910.09l
OPT2=-r 36.106114294 140.087190410 70.3010
OPT3=-r 36.103635125 140.086307150 69.7442
OPT4=-r 36.031339503 140.202443500 70.9029
OPT5=-r 36.263268936 140.174264100 94.2843

OPT00=-ts 2022/4/1 00:00:00 -te 2022/4/2 00:00:00 -ti 30
OPT11=$(HOME)/GNSS_NAV/IGS/2022/BRDC00IGS_R_20220910000_01D_MN.rnx -s $(HOME)/GNSS_DAT/COD0IGSRAP/2022/COD0IGSRAP_20220910000_01D_05M_ORB.SP3 -c $(HOME)/GNSS_DAT/COD0IGSRAP/2022/COD0IGSRAP_20220910000_01D_30S_CLK.CLK

sim1:
	./simobs $(OPT00) $(OPT11) $(OPT2) -o ../sim/TEST00TST_S_20220910000_24H_30S_MO.rnx -sys G,R,E

sim:
	./simobs $(OPT0) $(OPT1) $(OPT2) -o ../sim/base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT3) -o ../sim/rov0_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT4) -o ../sim/rov1_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT5) -o ../sim/rov2_20080509.obs
