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

# 3 params passed:
# $1 : result folder
# $2 : SVG filename
# $3 : style to draw graph in
#        0 => regular
#        1 => icicle (downward-growing!)
name=$(basename $0)
FLMGR=~/FlameGraph  # location of code
echo "#p = $# p= $*"
[ $# -lt 3 ] && {
 echo "${name}: should be invoked by the flame_grapher.sh script, not directly!"
 echo "Usage: ${name} result-folder SVG-filename style-to-display(1 for icicle)"
 exit 1
}

INFILE=perf.data
OUTFILE=${2}

TOPDIR=${PWD}
cd ${1} || exit 1
#pwd

INFILE=perf.data
OUTFILE=${2}
[ ! -f ${INFILE} ] && {
  echo "${name} : perf data file ${INFILE} invalid? Aborting..."
  cd ${TOPDIR}
  exit 1
}
[ $3 -ne 0 && $3 -ne 1 ] && {
	echo "${name}: third parameter graph-style must be 0 or 1"
	exit 1
}

echo
echo "${name}: Working ... generating SVG file \"${1}/${OUTFILE}\"..."

if [ ${3} -eq 0 ] ; then 
  sudo perf script --input ${INFILE} | ${FLMGR}/stackcollapse-perf.pl | ${FLMGR}/flamegraph.pl > ${OUTFILE} || {
  echo "${name}: failed."
  exit 1
  }
else
  sudo perf script --input ${INFILE} | ${FLMGR}/stackcollapse-perf.pl | ${FLMGR}/flamegraph.pl --inverted > ${OUTFILE} || {
  echo "${name}: failed."
  exit 1
  }
fi

USERNM=$(echo ${LOGNAME})
sudo chown -R ${USERNM}:${USERNM} ${TOPDIR}/${1}/ 2>/dev/null
cd ${TOPDIR}
ls -lh ${1}/${OUTFILE}
echo
echo "View the above SVG file in your web browser to see and zoom into the CPU FlameGraph."
exit 0
