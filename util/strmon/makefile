#
# makefile for strmon
#
SRC      = ../../src

BINDIR   = /usr/local/bin

INCLUDE= -I$(SRC)
OPTIONS= -DENAGLO -DENAGAL -DENAQZS -DENACMP

CC      = gcc

CFLAGS = -Wall -O3 -ansi -pedantic $(INCLUDE) $(OPTIONS) -g

LDLIBS = -lm

BIN = strmon

all        : $(BIN)
strmon     : strmon.o rtkcmn.o stream.o rtcm.o rtcm2.o rtcm3.o rtcm3e.o rcvraw.o
strmon     : crescent.o gw10.o javad.o novatel.o nvs.o rcvlex.o skytraq.o ss2.o ublox.o

strmon.o   : strmon.c
	$(CC) -c $(CFLAGS) strmon.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
stream.o   : $(SRC)/stream.c
	$(CC) -c $(CFLAGS) $(SRC)/stream.c
rtcm.o     : $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
rtcm2.o    : $(SRC)/rtcm2.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm2.c
rtcm3.o    : $(SRC)/rtcm3.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm3.c
rtcm3e.o   : $(SRC)/rtcm3e.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm3e.c
rcvraw.o   : $(SRC)/rcvraw.c
	$(CC) -c $(CFLAGS) $(SRC)/rcvraw.c
crescent.o : $(SRC)/rcv/crescent.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/crescent.c
gw10.o     : $(SRC)/rcv/gw10.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/gw10.c
javad.o    : $(SRC)/rcv/javad.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/javad.c
novatel.o  : $(SRC)/rcv/novatel.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/novatel.c
nvs.o      : $(SRC)/rcv/nvs.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/nvs.c
rcvlex.o   : $(SRC)/rcv/rcvlex.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/rcvlex.c
skytraq.o  : $(SRC)/rcv/skytraq.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/skytraq.c
ss2.o      : $(SRC)/rcv/ss2.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ss2.c
ublox.o    : $(SRC)/rcv/ublox.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ublox.c

test1  :
	./strmon

install :
	cp $(BIN) $(BINDIR)

clean :
	rm -f *.o *.out *.exe $(BIN) *.stackdump gmon.out


test1  :
	./strmon

install :
	cp $(BIN) $(BINDIR)

clean :
	rm -f *.o *.out *.exe $(BIN) *.stackdump gmon.out

