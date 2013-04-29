# makefile for gencrc

BINDIR = /usr/local/bin
SRC    = ../../../src
CFLAGS = -Wall -O3 -ansi -pedantic
LDLIBS  = -lm

gencrc     : gencrc.o

install:
	cp gencr $(BINDIR)

clean:
	rm -f gencrc gencrc.exe *.o

test:
	./gencrc -16 > crc16.c
	./gencrc -24 > crc24.c
