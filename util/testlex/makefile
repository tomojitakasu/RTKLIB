# makefile for testlex
CC=gcc
SRC    = ../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENALEX
CFLAGS = -Wall -g -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENAGAL
LDFLAGS= -lm -lrt

SRCS = rtkcmn.c preceph.c rtcm.c rtcm2.c rtcm3.c rtcm3e.c cssr.c qzsl6.c
OBJS = $(SRCS:.c=.o)

all        : dumpssr convlex dumpqzsl6

dumpssr    : dumpssr.o $(OBJS)

dumpqzsl6    : dumpqzsl6.o $(OBJS)

convlex    : convlex.o $(OBJS)

%.o : $(SRC)/%.c
	$(CC) -c $(CFLAGS)  -o $@ $<

install:
	cp -f dumpssr.exe /usr/local/bin

clean:
	rm -f *.o *.stackdump *.trace *.out *.exe

DIR1=../lexdata
DIR2=../lexerrdata

test1:
	./convlex 2020366A.l6 -o 2020366A.txt
