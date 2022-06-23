#!/bin/bash
# 2flameg.sh
# INVOKED from flame_grapher.sh
# Generate a CPU FlameGraph !
#
# CPU FlameGraph wrapper script.
# Simple wrapper over Brendan Gregg's superb FlameGraphs!
# Part 2 of 2.
#
# CREDITS: Brendan Gregg, for the original & simply superb FlameGraph
#
# Kaiwan N Billimoria
# (c) 2016,2022 kaiwanTECH
# License: MIT

# 2 params passed:
# $1 : result folder
# $2 : SVG filename
name=$(basename $0)
FLMGR=~/FlameGraph  # location of code

[ $# -ne 2 ] && {
 echo "${name}: should be invoked by the flame_grapher.sh script, not directly!"
 echo "Usage: ${name} result-folder SVG-filename"
 exit 1
}

INFILE=perf.data
OUTFILE=${2}

TOPDIR=${PWD}
cd ${1} || exit 1
#pwd

if [ $# -eq 1 ]; then
  INFILE=perf.data
  OUTFILE=${2}
elif [ $# -eq 3 ]; then
  INFILE=${2}
  OUTFILE=${3}
fi
[ ! -f ${INFILE} ] && {
  echo "${name} : ${INFILE} invalid? Aborting..."
  cd ${TOPDIR}
  exit 1
}

echo
echo "${name}: Working ... generating SVG file \"${1}/${OUTFILE}\"..."
sudo perf script --input ${INFILE} | ${FLMGR}/stackcollapse-perf.pl | ${FLMGR}/flamegraph.pl > ${OUTFILE} || {
  echo "${name}: failed."
  exit 1
}
USERNM=$(echo ${LOGNAME})
sudo chown -R ${USERNM}:${USERNM} ${TOPDIR}/${1}/ 2>/dev/null
cd ${TOPDIR}
ls -lh ${1}/${OUTFILE} #${TOPDIR}/${1}/${OUTFILE}
echo
echo "View the above SVG file in your web browser to see and zoom into the CPU FlameGraph."
exit 0
