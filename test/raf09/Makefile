#compiler
CC=gcc
#CC=gcc-4.9

#test with fgetln
CFLAGS=-D_HAVE_FGETLN_ -std=c89 -pedantic -g -c -Wall
#test with fgetc
#CFLAGS=-std=c89 -pedantic -g -c -Wall

SOURCES=$(wildcard *.c ) ../../src/rtkcmn.c
OBJECTS=raf09.o rtkcmn.o

#system dependant some needs -lm others no 
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

RAF09.mnt:
	curl http://geodesie.ign.fr/contenu/fichiers/documentation/grilles/metropole/RAF09.mnt > RAF09.mnt

egm2008_se.geoid:
	curl http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/Small_Endian/Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree_SE.gz | gunzip -c > egm2008_se.geoid
 
test: $(EXECUTABLE) RAF09.mnt
	./raf09

test-full: $(EXECUTABLE) RAF09.mnt egm2008_se.geoid
	./raf09
