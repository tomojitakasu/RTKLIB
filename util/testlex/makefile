# makefile for testlex

SRC    = ../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENALEX
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO
LDFLAGS= -lm -lrt

all        : convlex dumplex dumpssr outlexion

convlex    : rtkcmn.o preceph.o qzslex.o
dumplex    : rtkcmn.o preceph.o qzslex.o
dumpssr    : rtkcmn.o preceph.o rtcm.o rtcm3.o rtcm2.o dumpssr.o
outlexion  : rtkcmn.o preceph.o qzslex.o

rtkcmn.o   : $(SRC)/rtklib.h $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
preceph.o  : $(SRC)/rtklib.h $(SRC)/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/preceph.c
qzslex.o   : $(SRC)/rtklib.h $(SRC)/qzslex.c
	$(CC) -c $(CFLAGS) $(SRC)/qzslex.c
rtcm.o     : $(SRC)/rtklib.h $(SRC)/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm.c
rtcm3.o    : $(SRC)/rtklib.h $(SRC)/rtcm3.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm3.c
rtcm2.o    : $(SRC)/rtklib.h $(SRC)/rtcm2.c
	$(CC) -c $(CFLAGS) $(SRC)/rtcm2.c

install:
	cp -f convlex.exe /usr/local/bin
	cp -f dumplex.exe /usr/local/bin
	cp -f outlexion.exe /usr/local/bin

clean:
	rm -f *.o *.stackdump *.trace *.out *.exe

DIR1=../lexdata
DIR2=../lexerrdata

test1:
	./convlex $(DIR1)/LEX_20100704.bin -o $(DIR1)/LEX_20100704.lex
	./convlex $(DIR1)/LEX_20100725.bin -o $(DIR1)/LEX_20100725.lex
	./convlex $(DIR1)/LEX_20100803.bin -o $(DIR1)/LEX_20100803.lex
	./convlex $(DIR1)/LEX_20101204.bin -o $(DIR1)/LEX_20101204.lex
	./convlex $(DIR1)/LEX_20101205.bin -o $(DIR1)/LEX_20101205.lex

test2:
#	./diffeph -t0 2010/07/04 0:00:00 -ts 24 -ti 30 -1 5 $(DIR1)/LEX_20100704.lex $(DIR2)/igs15910.* $(DIR2)/igs05_1604.atx -x 2 > LEX_20100704.diff
#	./diffeph -t0 2010/07/25 0:00:00 -ts 24 -ti 30 -1 5 $(DIR1)/LEX_20100725.lex $(DIR2)/igs15940.* $(DIR2)/igs05_1604.atx -x 2 > LEX_20100725.diff
#	./diffeph -t0 2010/08/03 0:00:00 -ts 24 -ti 30 -1 5 $(DIR1)/LEX_20100803.lex $(DIR2)/igs15952.* $(DIR2)/igs05_1604.atx -x 2 > LEX_20100803.diff
#	./diffeph -t0 2010/12/04 0:00:00 -ts 24 -ti 30 -1 5 $(DIR1)/LEX_20101204.lex $(DIR2)/igr161*.*  $(DIR2)/igs05_1604.atx -x 2 > LEX_20101204.diff
#	./diffeph -t0 2010/12/05 0:00:00 -ts 24 -ti 30 -1 5 $(DIR1)/LEX_20101205.lex $(DIR2)/igr161*.*  $(DIR2)/igs05_1604.atx -x 2 > LEX_20101205.diff

test3:
#	./diffeph -t0 2010/12/04 0:00:00 -ts 24 -ti 30 -1 3 $(DIR2)/brdc33*0.10n $(DIR2)/clk11_201012040000.rtcm3 $(DIR2)/igr161*.*  $(DIR2)/igs05_1604.atx -x 2 > SSR_20101204.diff
	./diffeph -t0 2010/12/04 0:00:00 -ts 24 -ti 30 -1 3 $(DIR2)/brdc33*0.10n $(DIR2)/clk20_201012040000.rtcm3 $(DIR2)/igr161*.*  $(DIR2)/igs05_1604.atx -x 2 > SSR_20101204.diff

test4:
	./outlexion -t0 2010/12/04 0:00:00 -ts 24 $(DIR1)/LEX_20101204.lex -o LEXION_20101204.m
