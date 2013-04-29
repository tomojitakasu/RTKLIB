#!/bin/bash
#
# genqc.sh: multipath and cycle-slip count for log data
#
#  genqc.sh yyyy/mm/dd [hh]
#
DATE=$1
HOUR=00
if [ "$2" ] ; then HOUR=$2; fi

LOGDIR=log
OUTDIR=rep
CONV=convbin
TEQC=teqc
CONF=strmon.conf

OUT=${OUTDIR}/strqc`echo $DATE | sed 's@/@@g'`${HOUR}.txt

echo "# STREAM QC" > $OUT

cat $CONF |
while read STA FMT PATH1 PATH2
do
    if [ "$STA" = "" -o "$STA" = "#" ] ; then continue; fi
    
    if   [ "$FMT" = BINEX ] ; then OPT="-v 2.10 -r binex"
    elif [ "$FMT" = JAVAD ] ; then OPT="-v 2.10 -r javad"
    elif [ "$FMT" = RTCM3 ] ; then OPT="-v 2.10 -r rtcm3"
    else continue
    fi
    OPT="$OPT -tr $DATE $HOUR"
    
    LOG=${LOGDIR}/${STA}`echo $DATE | sed 's@/@@g'`${HOUR}.log
    
    OBS=${STA}.obs
    NAV1=${STA}.nav
    NAV2=${STA}.gnav
    
    if [ ! -r "$LOG" ] ; then continue; fi
    
    # convert log to rinex
    $CONV $OPT -o $OBS -n $NAV1 -g $NAV2 $LOG 2> /dev/null
    
    if [ ! -r "$OBS" -o ! -r "$NAV1" ] ; then continue; fi
    
    # multhpath and cycle-slips by teqc
    $TEQC +qc +relax -rep -plot -nav $NAV1,$NAV2 $OBS 2> /dev/null |
    awk -v sta=$STA \
        '/Moving average MP1/{mp1=$5;}    \
         /Moving average MP2/{mp2=$5;}    \
         /IOD slips/         {slip+=$NF;} \
         /IOD or MP slips/   {slip+=$NF;} \
         END{printf("%-6s %.4f  %.4f  %5d\n",sta,mp1,mp2,slip)}' >> $OUT
    
    rm -f $OBS $NAV1 $NAV2
done

echo "write: $OUT"

