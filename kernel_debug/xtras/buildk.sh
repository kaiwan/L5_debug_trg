#!/bin/bash
# buildk.sh
numcores=$(getconf -a|grep _NPROCESSORS_ONLN|awk '{print $2}')
[ -z "${numcores}" ] && numcores=1
jobs=$((${numcores}*2))

time make -j${jobs} && 
  echo ; sudo make -j4 modules_install && 
   echo ; sudo make -j4 install && 
    echo ; echo Done! || echo Fail.
