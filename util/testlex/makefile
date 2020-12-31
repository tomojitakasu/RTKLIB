# makefile for testlex

SRC    = ../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENALEX
CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO
LDFLAGS= -lm -lrt

SRCS = rtkcmn.c preceph.c rtcm.c rtcm2.c rtcm3.c rtcm3e.c cssr.c
OBJS = $(SRCS:.c=.o)

all        : dumpssr

dumpssr    : dumpssr.o $(OBJS)

%.o : $(SRC)/%.c
	$(CC) -c $(CFLAGS)  -o $@ $<

install:
	cp -f dumpssr.exe /usr/local/bin

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
