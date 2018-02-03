INSTDIR=/usr/local/bin
DIRS = pos2kml str2str rnx2rtkp convbin rtkrcv

all:
	for i in $(DIRS); do make -C app/$$i/gcc; done
	make -C util/rnx2rtcm

install:
	@echo installing...
	for i in $(DIRS); do cp app/$$i/gcc/$$i $(INSTDIR); done
	cp util/rnx2rtcm/rnx2rtcm $(INSTDIR)

clean:
	@echo cleaning up...
	for i in $(DIRS); do make -C app/$$i/gcc clean; done
	make -C util/rnx2rtcm clean
