#!/bin/bash
# stressor.sh
# Stress system
#  with stress(1) : CPU, I/O, VM, disk

# Wrapper over stress(1) and stress-ng(1)

[ $# -ne 1 ] && {
	echo "Usage: $0 duration-in-seconds (for both IO and n/w)"
	exit 1
}

#sudo stress -v --cpu 4 --io 4 --vm 4 --hdd 4 &
# IO, disk
echo "1. sudo stress -v --io 8 --hdd 8 --timeout $1"
sudo stress -v --io 8 --hdd 8 --timeout $1
# net
echo "2. sudo stress-ng -v --all 0 --class network ? --timeout 3"
sudo stress-ng -v --all 0 --class network ? --timeout 3
