#!/bin/sh
#
# count source lines
#

cd ../..

for file in src/*.h src/*.c src/rcv/*.c
do
    cat $file |
    grep -v "^$" | grep -v "^[ \t]*/\*" | grep -v "^[ \t]*\*" | grep -v "^[ \t]*//" |
    awk "END{print \"""$file""\",NR}"
done |
awk '{n+=$2; printf("%-32s %6d\n",$1,$2)} END{printf("%-32s %6d\n\n","src total",n)}'

for file in app/*/*.[hc]*
do
    cat $file |
    grep -v "^$" | grep -v "^[ \t]*/\*" | grep -v "^[ \t]*\*" | grep -v "^[ \t]*//" |
    awk "END{print \"""$file""\",NR}"
done |
awk '{n+=$2; printf("%-32s %6d\n",$1,$2)} END{printf("%-32s %6d\n\n","app total",n)}'

