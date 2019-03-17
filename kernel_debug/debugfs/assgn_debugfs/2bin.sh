#!/bin/sh
name=$(basename $0)
if [ $# -ne 1 ]; then
	echo "Usage: $name number-to-convert-to-binary"
	exit 1
fi
DRV=assgn_debugfs
DBGFILE=/sys/kernel/debug/assgn_debugfs/dec2bin

[ ! -f $DBGFILE ] && {
 echo "Debugfs file $DEBUGFS not found, aborting..."
 exit 1
}
lsmod |grep $DRV >/dev/null || {
	echo "Kernel module $DRV not inserted?"
	exit 1
}
echo $1 > $DBGFILE
cat $DBGFILE

