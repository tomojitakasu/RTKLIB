# makefile for simobs

BINDIR = ../../../bin
SRC    = ../../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO -DENAGAL -DENAQZS -DNFREQ=4 -DMAXOBS=128
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAGLO -DENAQZS -DENAGAL -DENACMP -DENAIRN -DNFREQ=5 -DIERS_MODEL
#LDLIBS  = -lm
LDLIBS  = ../../../lib/iers/gcc/iers.a -lgfortran -lm -lrt

simobs     : simobs.o geoid.o ephemeris.o preceph.o rinex.o rtkcmn.o sbas.o tides.o


simobs.o   : ../simobs.c
	$(CC) -c $(CFLAGS) ../simobs.c
ephemeris.o   : $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
geoid.o    : $(SRC)/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/geoid.c
preceph.o    : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c	
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
sbas.o   : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
tides.o   : $(SRC)/tides.c
	$(CC) -c $(CFLAGS) $(SRC)/tides.c

simobs.o    : $(SRC)/rtklib.h
geoid.o     : $(SRC)/rtklib.h
ephemeris.o : $(SRC)/rtklib.h
preceph.o   : $(SRC)/rtklib.h
rtkcmn.o    : $(SRC)/rtklib.h
sbas.o      : $(SRC)/rtklib.h
tides.o     : $(SRC)/rtklib.h

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

sim:
	./simobs $(OPT0) $(OPT1) $(OPT2) -o ../sim/base_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT3) -o ../sim/rov0_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT4) -o ../sim/rov1_20080509.obs
	./simobs $(OPT0) $(OPT1) $(OPT5) -o ../sim/rov2_20080509.obs
