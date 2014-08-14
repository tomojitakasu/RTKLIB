#compiler
CC=gcc-4.9
CFLAGS=-std=c89 -pedantic -g -c -Wall
SOURCES=$(wildcard *.c ) ../../src/rtkcmn.c
OBJECTS=raf09.o rtkcmn.o
LIBS=

EXECUTABLE=raf09

all: $(EXECUTABLE)

%.o: %.c
	$(CC) -g -Wall -o $@ -c $< $(CFLAGS)
	
$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -g -o $@ $(LIBS)
rtkcmn.o:
	$(CC) -g -Wall -o rtkcmn.o -c ../../src/rtkcmn.c
clean:
	rm $(EXECUTABLE) $(OBJECTS)
