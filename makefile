INSTDIR=/usr/local/bin
DIRS = pos2kml str2str rnx2rtkp convbin rtkrcv

all:
	for i in $(DIRS); do make -C app/$$i/gcc; done

install:
	@echo installing...
	for i in $(DIRS); do cp app/$$i/gcc/$$i $(INSTDIR); done

clean:
	@echo cleaning up...
	for i in $(DIRS); do make -C app/$$i/gcc clean; done
