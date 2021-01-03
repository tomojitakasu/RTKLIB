# makefile for testlex
CC=gcc
SRC    = ../../src
#CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENALEX
CFLAGS = -Wall -g -ansi -pedantic -I$(SRC) -DTRACE -DENAQZS -DENAGLO -DENAGAL
LDFLAGS= -lm -lrt

SRCS = rtkcmn.c preceph.c rtcm.c rtcm2.c rtcm3.c rtcm3e.c cssr.c qzsl6.c
OBJS = $(SRCS:.c=.o)

all        : dumpssr dumpqzsl6 l6msg2rtcm

dumpssr    : dumpssr.o $(OBJS)

dumpqzsl6    : dumpqzsl6.o $(OBJS)

l6msg2rtcm    : l6msg2rtcm.o $(OBJS)

#convlex    : convlex.o $(OBJS)

%.o : $(SRC)/%.c
	$(CC) -c $(CFLAGS)  -o $@ $<

install:
	cp -f dumpssr.exe /usr/local/bin

clean:
	rm -f *.o *.stackdump *.trace *.out *.exe

test1:
	./l6msg2rtcm 2020366A.l6 -o 2020366A.rtcm3
	./dumpqzsl6 2020366A.l6

test2:
	./dumpssr -x 4 2020366A.rtcm3


