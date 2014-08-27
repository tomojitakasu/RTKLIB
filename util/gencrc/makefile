# makefile for gencrc

BINDIR = /usr/local/bin
SRC    = ../../../src
CFLAGS = -Wall -O3 -ansi -pedantic
LDLIBS  = -lm

all        : gencrc genxor genmsk
gencrc     : gencrc.o
genxor     : genxor.o
genmsk     : genmsk.o

clean:
	rm -f gencrc genxor genmsk *.o

test:
	./gencrc -16 > crc16.c
	./gencrc -24 > crc24.c
	./genxor > xor.c
	./genmsk > msk.c
