#!/bin/bash
#
# $Id: gdbline,v 1.1 2004/08/02 16:27:55 corbet Exp $
#
# gdbline module image
#
# Outputs an add-symbol-file line suitable for pasting into gdb to examine
# a loaded module.
# (Here used in conjunction with the hello_gdb.ko kernel module; gdb demo).
# 
# [minor changes, kaiwan]

if [ $# -ne 2 ]; then
	echo "Usage: $0 module-name image-filename"
	echo "  module-name: name of the (already inserted) kernel module (without the .ko)"
	echo "  image-filename: pathname to the kernel module."
	exit 1
fi
if [ ! -d /sys/module/$1/sections ]; then
	echo "$0: $1 not a valid file"
	exit 1
fi
if [ ! -f $2 ]; then
	echo "$0: $2 not a valid file"
	exit 1
fi

cd /sys/module/$1/sections
echo "Copy-paste the following lines into GDB"
echo "---snip---"

[ -f .text ] && {
   sudo echo -n add-symbol-file $2 `/bin/cat .text`
   sudo echo  " \\"
} || [ -f .init.text ] && {
   sudo echo -n add-symbol-file $2 `/bin/cat .init.text`
}

for section in .[a-z]* *; do
    if [ ${section} != ".text" -o ${section} != ".init.text" ]; then
	    sudo echo  " \\"
	    sudo echo -n "       -s" ${section} `/bin/cat ${section}`
    fi
done
echo "
---snip---"
echo
