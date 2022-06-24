#!/bin/bash
# Generate CPU FlameGraph wrapper script.
# Simple wrapper over Brendan Gregg's superb CPU FlameGraphs!
# Part 1 of 2.
# This script records the perf data file, and then invokes the
# second script 2flameg.sh which renders the SVG file.
# All "result" files are under the 'result/' folder.
#
# CREDITS: Brendan Gregg, for the original & simply superb FlameGraph
#
# Kaiwan N Billimoria
# (c) 2016,2022 kaiwanTECH
# License: MIT
name=$(basename $0)
FLMGR=~/FlameGraph  # location of code
STYLE_INVERTED_ICICLE=1

usage()
{
  echo "Usage: ${name} [process-PID-to-sample] svg-out-filename(without .svg)
 No PID implies the entire system is sampled..."
 echo "
Also note that the default style here is to display the stack frames"
 [ ${STYLE_INVERTED_ICICLE} -eq 0 ] && echo " normally (i.e. reaching/growing 'upward'"
 [ ${STYLE_INVERTED_ICICLE} -eq 1 ] && echo " 'icicle'-style (i.e. downward-growing (technically correct!))"

 echo "
${name} --help , to show this help screen."
}

### "main" here

which perf >/dev/null 2>&1 || {
 echo "${name}: perf not installed? Aborting..."
 exit 1
}
[ ! -d ${FLMGR} ] && {
 echo "${name}: FlameGraph not installed?
 You need to install it...
 In your home dir, type:
 git clone https://github.com/brendangregg/FlameGraph"
 exit 1
}
#[ $(id -u) -ne 0 ] && {
# echo "${name}: must run as root user"
# exit 1
#}
[ $# -lt 1 ] && {
  usage
  exit 1
}
[ $1 = "--help" ] && {
  usage
  exit 0
}

HZ=99
SVG=""
PDIR=""
TOPDIR=${PWD}
PERF_RESULT_DIR_BASE=/tmp/flamegraphs # change to make it non-volatile

#--- Run the part 2 - generating the FG - on interrupt or exit
trap 'cd ${TOPDIR}; ./2flameg.sh ${PDIR} ${SVG} ${STYLE_INVERTED_ICICLE}' INT QUIT EXIT
#---

if [ $# -eq 2 ]; then  #------------------ Profile a particular process
 # Check if PID is valid
 kill -0 $1
 [ $? -ne 0 ] && {
   echo "${name}: PID $1 is an invalid (or dead) process?"
   exit 1
 }
 echo "### ${name}: recording samples on process PID ${1} now...
 Press ^C to stop..."
 SVG=${2}.svg
 PDIR=${PERF_RESULT_DIR_BASE}/${2}
 #echo "PDIR = ${PDIR} SVG=${SVG}"
 mkdir -p ${PDIR}
 sudo chown -R ${LOGNAME}:${LOGNAME} ${PERF_RESULT_DIR_BASE} 2>/dev/null
 cd ${PDIR}
 sudo perf record -F ${HZ} --call-graph dwarf -p $1 || exit 1  # generates perf.data
elif [ $# -eq 1 ]; then  #---------------- Profile system-wide
 echo "### ${name}: recording samples system-wide now...
 Press ^C to stop..."
 SVG=${1}.svg
 PDIR=${PERF_RESULT_DIR_BASE}/${1}
 #echo "PDIR = ${PDIR} SVG=${SVG} LOGNAME = $(logname)"

 mkdir -p ${PDIR}
 sudo chown -R $(logname):$(logname) ${PERF_RESULT_DIR_BASE} 2>/dev/null
 cd ${PDIR}
 sudo perf record -F ${HZ} --call-graph dwarf -a || exit 1  # generates perf.data
fi
cd ${TOPDIR}

exit 0  # this exit causes the 'trap' to run (as we've trapped the EXIT!)
