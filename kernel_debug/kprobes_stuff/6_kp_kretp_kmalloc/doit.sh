#!/bin/bash
# doit.sh
# Simple wrapper over the 'kp_kretp_kmalloc' code
name=$(basename $0)
KMOD=kp_kretp_kmalloc
sudo grep -w register_jprobe /boot/System.map-$(uname -r) || {
	echo "${name}: Jprobes does not seem to be supported on this kernel."
	exit 1
}
sudo rmmod ${KMOD}
sudo dmesg -C
sudo make clean ; sudo make && sudo insmod ${KMOD}.ko
dmesg
