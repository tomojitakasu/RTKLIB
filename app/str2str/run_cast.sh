#!/bin/sh
#
# run_cast.sh: script to run NTRIP caster by STR2STR
#
#          Copyright (C) 2007-2016 by T.TAKASU, All rights reserved.
#
# version : $Revision:$ $Date:$
# history : 2016/09/06  1.0  new
#

# NTRIP caster program
#cast=str2str
cast=./gcc/str2str

# NTRIP caster options
#svr_port=80             # server port
svr_port=2102            # server port
cli_port=2101            # client port
svr_pwd=test             # server password
cli_usr=guest            # client user
cli_pwd=test             # client password
tbl=./srctbl.txt         # source table
logdir=./log             # log directory
level=3                  # trace level (0: no trace)

# NTRIP caster logs
log1=$logdir/cast_trac_`date -u +%Y%m%d_%H%M%S`.log
log2=$logdir/cast_stat_`date -u +%Y%m%d_%H%M%S`.log

# start NTRIP caster
start_cast()
{
    mkdir -p $logdir
    inp=ntripc_s://:${svr_pwd}@:${svr_port}
    out=ntripc_c://${cli_usr}:${cli_pwd}@:${cli_port}
    opt="-ft $tbl -fl $log1 -t $level"
    ${cast} -in $inp -out $out $opt > $log2 2>&1 < /dev/null &
    if [ $? != 0 ] ; then echo "ntrip caster start error"; exit 1; fi
    echo "ntrip caster started: $!"
    echo "ntrip caster log to $log1"
    echo "ntrip caster log to $log2"
}

# stop NTRIP caster
stop_cast()
{
    echo -n "ntrip caster stopped:"
    ps -A | grep str2str | while read pid s; do kill $pid; echo -n " $pid"; done
    echo
}

# reload source table
reload_cast()
{
    echo -n "ntrip caster reload:"
    ps -A | grep str2str | while read pid s; do kill -s USR2 $pid; echo -n " $pid"; done
    echo
}

# get status of NRTIP caster
if ps -A | grep str2str > /dev/null; then stat=running; else stat=stopped; fi

# handle commands
case "$1" in
    start) # start NTRIP caster
        if [ $stat = running ] ; then echo "ntrip caster $stat"; exit 1; fi
        start_cast
        ;;
    stop) # stop NTRIP caster
        if [ $stat = stopped ] ; then echo "ntrip caster $stat"; exit 1; fi
        stop_cast
        ;;
    restart) # restart NTRIP caster
        stop_cast; sleep 1; start_cast
        ;;
    reload) # reload source table
        reload_cast
        ;;
    status) # get status of NTRIP caster
        echo "ntrip caster $stat"
        ;;
    *)
        echo "usage: run_cast.sh [start|stop|restart|reload|status]"
        ;;
esac

