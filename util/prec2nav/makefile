#
# makefile for prec2nav
#
SRC      = ../../src

BINDIR   = /usr/local/bin

INCLUDE= -I$(SRC)
OPTIONS= -DENAGLO -DENAGAL -DENAQZS -DENACMP

CC      = gcc

CFLAGS = -Wall -O3 -ansi -pedantic $(INCLUDE) $(OPTIONS) -g

LDLIBS = -lm

BIN = prec2nav

all        : $(BIN)
prec2nav   : prec2nav.o rtkcmn.o rinex.o preceph.o

prec2nav.o : prec2nav.c
	$(CC) -c $(CFLAGS) prec2nav.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
rinex.o   : $(SRC)/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rinex.c
preceph.o  : $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c

test1  :
	./prec2nav -td 2012/10/21 -ts 336 -ant ../../data/igs08.atx data/igs%W%D.sp3 -out out/test.nav

test2  :
	./prec2nav -td 2012/10/21 -ver 3.00 -ant ../../data/igs08.atx -out out/peph%n0.%yn data/igs%W%D.sp3

test3  :
	./prec2nav -td 2012/10/21 -ns 50 -sat E11,E12 -ver 3 -ant data/ant_gal_qzs.atx -out out/brdc%n0.%yl data/tum%W%D.sp3

install :
	cp $(BIN) $(BINDIR)

clean :
	rm -f *.o *.out *.exe $(BIN) *.stackdump gmon.out

