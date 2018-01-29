# makefile for rtkrcv

BINDIR = /usr/local/bin
SRC    = ../../../src

# for beagleboard
#CTARGET= -mfpu=neon -mfloat-abi=softfp -ffast-math
CTARGET= -DENAGLO -DENAQZS -DENACMP -DENAGAL -DENAIRN -DNFREQ=3 -DSVR_REUSEADDR

CC=gcc
CFLAGS = -Wall -O3 -ansi -pedantic -Wno-unused-but-set-variable -I$(SRC) -I.. -DTRACE $(CTARGET) -g
LDLIBS  = -lm -lrt -lpthread

all        : rtkrcv
rtkrcv     : rtkrcv.o vt.o rtkcmn.o rtksvr.o rtkpos.o geoid.o solution.o lambda.o
rtkrcv     : sbas.o stream.o rcvraw.o rtcm.o preceph.o options.o pntpos.o ppp.o ppp_ar.o
rtkrcv     : novatel.o ublox.o ss2.o crescent.o skytraq.o gw10.o javad.o nvs.o binex.o
rtkrcv     : rt17.o ephemeris.o rinex.o ionex.o rtcm2.o rtcm3.o rtcm3e.o qzslex.o
rtkrcv     : ppp_corr.o tides.o septentrio.o cmr.o tersus.o swiftnav.o

rtkrcv.o   : ../rtkrcv.c
	$(CC) -c $(CFLAGS) ../rtkrcv.c
vt.o       : ../vt.c
	$(CC) -c $(CFLAGS) ../vt.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
rtksvr.o   : $(SRC)/rtksvr.c
	$(CC) -c $(CFLAGS) $(SRC)/rtksvr.c
rtkpos.o   : $(SRC)/rtkpos.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkpos.c
geoid.o    : $(SRC)/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/geoid.c
solution.o : $(SRC)/solution.c
	$(CC) -c $(CFLAGS) $(SRC)/solution.c
lambda.o   : $(SRC)/lambda.c
	$(CC) -c $(CFLAGS) $(SRC)/lambda.c
sbas.o     : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
stream.o   : $(SRC)/stream.c
	$(CC) -c $(CFLAGS) $(SRC)/stream.c
rcvraw.o : $(SRC)/rcvraw.c
	$(CC) -c $(CFLAGS) $(SRC)/rcvraw.c
rtcm.o     : $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
rtcm2.o    : $(SRC)/rtcm2.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm2.c
rtcm3.o    : $(SRC)/rtcm3.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm3.c
rtcm3e.o   : $(SRC)/rtcm3e.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm3e.c
preceph.o  : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
options.o  : $(SRC)/options.c
	$(CC) -c $(CFLAGS) $(SRC)/options.c
pntpos.o   : $(SRC)/pntpos.c
	$(CC) -c $(CFLAGS) $(SRC)/pntpos.c
ppp.o      : $(SRC)/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp.c
ppp_ar.o   : $(SRC)/ppp_ar.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp_ar.c
novatel.o  : $(SRC)/rcv/novatel.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/novatel.c
ublox.o    : $(SRC)/rcv/ublox.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ublox.c
ss2.o      : $(SRC)/rcv/ss2.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ss2.c
crescent.o : $(SRC)/rcv/crescent.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/crescent.c
skytraq.o  : $(SRC)/rcv/skytraq.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/skytraq.c
gw10.o     : $(SRC)/rcv/gw10.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/gw10.c
javad.o    : $(SRC)/rcv/javad.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/javad.c
nvs.o      : $(SRC)/rcv/nvs.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/nvs.c
binex.o    : $(SRC)/rcv/binex.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/binex.c
rt17.o     : $(SRC)/rcv/rt17.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/rt17.c
ephemeris.o: $(SRC)/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/ephemeris.c
rinex.o    : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
ionex.o    : $(SRC)/ionex.c
	$(CC) -c $(CFLAGS) $(SRC)/ionex.c
qzslex.o   : $(SRC)/qzslex.c
	$(CC) -c $(CFLAGS) $(SRC)/qzslex.c
tides.o    : $(SRC)/tides.c
	$(CC) -c $(CFLAGS) $(SRC)/tides.c
ppp_corr.o : $(SRC)/ppp_corr.c
	$(CC) -c $(CFLAGS) $(SRC)/ppp_corr.c
septentrio.o: $(SRC)/rcv/septentrio.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/septentrio.c
cmr.o      : $(SRC)/rcv/cmr.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/cmr.c
tersus.o   : $(SRC)/rcv/tersus.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/tersus.c
swiftnav.o   : $(SRC)/rcv/swiftnav.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/swiftnav.c

rtkrcv.o   : $(SRC)/rtklib.h ../vt.h Makefile
rtkcmn.o   : $(SRC)/rtklib.h Makefile
rtksvr.o   : $(SRC)/rtklib.h Makefile
rtkpos.o   : $(SRC)/rtklib.h Makefile
geoid.o    : $(SRC)/rtklib.h Makefile
solution.o : $(SRC)/rtklib.h Makefile
lambda.o   : $(SRC)/rtklib.h Makefile
sbas.o     : $(SRC)/rtklib.h Makefile
rcvraw.o   : $(SRC)/rtklib.h Makefile
rtcm.o     : $(SRC)/rtklib.h Makefile
rtcm2.o    : $(SRC)/rtklib.h Makefile
rtcm3.o    : $(SRC)/rtklib.h Makefile
rtcm3e.o   : $(SRC)/rtklib.h Makefile
preceph.o  : $(SRC)/rtklib.h Makefile
options.o  : $(SRC)/rtklib.h Makefile
pntpos.o   : $(SRC)/rtklib.h Makefile
ppp.o      : $(SRC)/rtklib.h Makefile
novatel.o  : $(SRC)/rtklib.h Makefile
ublox.o    : $(SRC)/rtklib.h Makefile
ss2.o      : $(SRC)/rtklib.h Makefile
crescent.o : $(SRC)/rtklib.h Makefile
skytraq.o  : $(SRC)/rtklib.h Makefile
gw10.o     : $(SRC)/rtklib.h Makefile
javad.o    : $(SRC)/rtklib.h Makefile
nvs.o      : $(SRC)/rtklib.h Makefile
binex.o    : $(SRC)/rtklib.h Makefile
rt17.o     : $(SRC)/rtklib.h Makefile
septentrio.o: $(SRC)/rtklib.h Makefile
cmr.o      : $(SRC)/rtklib.h Makefile
tersus.o   : $(SRC)/rtklib.h Makefile
swiftnav.o : $(SRC)/rtklib.h Makefile
ephemeris.o: $(SRC)/rtklib.h Makefile
rinex.o    : $(SRC)/rtklib.h Makefile
ionex.o    : $(SRC)/rtklib.h Makefile
qzslex.o   : $(SRC)/rtklib.h Makefile
tides.o    : $(SRC)/rtklib.h Makefile
ppp_corr.o : $(SRC)/rtklib.h Makefile

install:
	cp rtkrcv $(BINDIR)

test1:
	./rtkrcv -t 4 -m 52001 -t 4

test2:
	./rtkrcv -p 2105 -m 52001

test3:
	./rtkrcv -o ../rtk_pb.conf

clean:
	rm -f rtkrcv rtkrcv.exe rtkrcv.nav *.o *.out *.trace

