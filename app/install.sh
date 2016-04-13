#!/bin/sh
#
# make and install all cui applications by gcc to /usr/local/bin
#

echo; echo % pos2kml/gcc
cd pos2kml/gcc
make clean $1 && \
make $1 && \
sudo make install $1
cd ../..

echo; echo % str2str/gcc
cd str2str/gcc
make clean $1 && \
make $1 && \
sudo make install $1
cd ../..

echo; echo % rnx2rtkp/gcc
cd rnx2rtkp/gcc
make clean $1 && \
make $1 && \
sudo make install $1
cd ../..

echo; echo % convbin/gcc
cd convbin/gcc
make clean $1 && \
make $1 && \
sudo make install $1
cd ../..

echo; echo % rtkrcv/gcc
cd rtkrcv/gcc
make clean $1 && \
make $1 && \
sudo make install $1
cd ../..
