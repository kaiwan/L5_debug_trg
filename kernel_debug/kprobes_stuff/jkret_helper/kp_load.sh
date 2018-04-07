#!/bin/sh
#------------------------------------------------------------------------------
# kp_load.sh
#
# Helper script for the kp_helper kernel module - that require the address of 
# a function to attach a Kprobes pre-handler to.
# The function must not be marked 'static' in the kernel module.
#
# This script will extract the function's address from /proc/kallsyms and
# pass it as a parameter to the kernel module.
#
# !!UPDATE!!
#  Things have now become a lot simpler:
#  The kprobes infrastructure code just requires the symbolic name of the function to set
#  a probe on; it will figure out the kernel virtual addr. Hence, this script becomes a very
#  simple (silly) wrapper; just pass along the function name as a param to the 'helper_kp'
#  kernel module that invokes the Kprobe API...
#
#
# author: kaiwan billimoria <kaiwan@kaiwantech.com>
# License: GPL
#------------------------------------------------------------------------------

# PROBE_KERNEL=0 : we're going to attempt to kprobe a function in a kernel module
#  In this case (the default), the script's first parameter is the kernel module.
# PROBE_KERNEL=1 : we're going to attempt to kprobe a function in the kernel itself
#  In this case, the script's first parameter will not be a kernel module pathname
#  and we shall accordingly treat the first parameter as the name of the function
#  to kprobe.
PROBE_KERNEL=0

TARGET_MODULE=""
SEP="-------------------------------------------------------------------------------"
name=`basename $0`

if [ `id -u` -ne 0 ]; then
	echo "$name: Sorry, you need to run $0 as superuser."
	exit 1
fi

#echo "$#: $*"
KMOD=helper_jkret.ko
KPATH="/lib/modules/`uname -r`/build"

if [ $# -eq 1 ]; then
	echo "$name: Assume that we're k-probing something in the kernel itself."
	PROBE_KERNEL=1
	FUNCTION=$1
elif [ $# -eq 2 ]; then
	TARGET_MODULE=$1
	FUNCTION=$2
else
	echo "Usage: $name [module-pathname] function-to-probe
 module-pathname: pathname of kernel module that has the
 function-to-probe

 If module-pathname is not passed, then we assume the function to be kprobed
 is in the kernel itself."
	exit 2
fi

if [ -z $FUNCTION ]; then
	echo "$name: function name invalid, aborting now.."
	exit 1
fi

if [ $PROBE_KERNEL -eq 0 ]; then
	if [ ! -f $TARGET_MODULE ]; then
		echo "$name: kernel module name '$TARGET_MODULE' an invalid pathname, aborting now.."
		exit 1
	fi
	echo "Target kernel Module: $TARGET_MODULE"
fi

echo "Function name: $FUNCTION"

#exit 0

# rmmod any old instances
/sbin/rmmod $KMOD 2>/dev/null
if [ $PROBE_KERNEL -eq 0 ]; then
	/sbin/rmmod $TARGET_MODULE 2>/dev/null
fi

# 1. First insert the kernel module (whose function is to be probed)
if [ $PROBE_KERNEL -eq 0 ]; then
	/sbin/insmod $TARGET_MODULE || {
		echo "$name: insmod $TARGET_MODULE unsuccessful, aborting now.."
		echo "dmesg|tail"
		dmesg|tail
		exit 5
	}
	echo "$name: insmod $TARGET_MODULE successful."
	echo "dmesg|tail"
	dmesg|tail
fi
echo $SEP

#exit 0
# 3. Insert the helper_kp kernel module that will set up the kprobe
/sbin/insmod $KMOD funcname=$FUNCTION || {
	echo "$name: insmod $KMOD unsuccessful, aborting now.."
	if [ $PROBE_KERNEL -eq 0 ]; then
		/sbin/rmmod $TARGET_MODULE
	fi
	echo "dmesg|tail"
	dmesg|tail
	exit 7
}
echo "$name: successful."
echo "dmesg|tail"
dmesg|tail
exit 0

