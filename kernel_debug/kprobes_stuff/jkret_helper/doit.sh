#!/bin/bash
# doit.sh
# Simple wrapper over the 'helper_jkret' code
name=$(basename $0)
sudo grep -w register_jprobe /boot/System.map-$(uname -r) || {
	echo "${name}: Jprobes does not seem to be supported on this kernel."
	exit 1
}
# This is hard-coded to the __kmalloc   :-(
sudo rmmod helper_jkret
sudo dmesg -c
sudo make && sudo ./kp_load.sh __kmalloc
dmesg
