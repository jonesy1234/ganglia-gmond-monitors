#!/usr/bin/ksh
#########################################
# /usr/local/bin/sys_ganglia_query.ksh
# Used to wrap the python script so it
# works with Oracle Enterprise Manager
# so you can query metrics from gmetad
#########################################

QUERYSCRIPT="/usr/local/bin/sys_ganglia_query.ksh"
TARGETHOST=$1
METRIC=$2
PORT=$3

export LIBPATH="/opt/freeware/lib:/opt/freeware/lib64:$LIBPATH"
export PATH="/opt/freeware/bin:$PATH"

$QUERYSCRIPT -h$TARGETHOST -m$METRIC -p$PORT
