#!/bin/sh
#
# make all cui applications by gcc
#

echo; echo % pos2kml/gcc
cd pos2kml/gcc
make $1
cp pos2kml ../../../bin/
cd ../..

echo; echo % str2str/gcc
cd str2str/gcc
make $1
cp str2str ../../../bin/
cd ../..

echo; echo % rnx2rtkp/gcc
cd rnx2rtkp/gcc
make $1
cp rnx2rtkp ../../../bin/
cd ../..

echo; echo % convbin/gcc
cd convbin/gcc
make $1
cp convbin ../../../bin/
cd ../..

echo; echo % rtkrcv/gcc
cd rtkrcv/gcc
make $1
cp rtkrcv ../../../bin/
cd ../..

