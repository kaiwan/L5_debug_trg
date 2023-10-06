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
export FLMGR=~/FlameGraph  # location of code
PERF_RESULT_DIR_BASE=/tmp/flamegraphs # change to make it non-volatile
STYLE_INVERTED_ICICLE=0
TYPE_CHART=0
HZ=99

# TODO - 
#  [ ] add -d duration param
#  [ ] show current config on run
# ISSUES
#  [ ] why does 2flameg.sh seem to run twice?? the 'exit' and ^C ?

usage()
{
  echo "Usage: ${name} -o svg-out-filename(without .svg) [options ...]
  -o svg-out-filename(without .svg) : name of SVG file to generate (saved under ${PERF_RESULT_DIR_BASE}/)
Optional switches:
 [-p PID]     : PID = generate a FlameGraph for ONLY this process or thread
                 If not passed, the *entire system* is sampled...
 [-s <style>] : normal = draw the stack frames growing upward   [default]
                icicle = draw the stack frames growing downward
 [-t <type>]  : graph  = produce a flame graph (X axis is NOT time, merges stacks) [default]
                   Good for performance outliers (who's eating CPU? using max stack?); works well for multi-threaded apps
                chart  = produce a flame chart (sort by time, do not merge stacks)
                   Good for seeing all calls; works well for single-threaded apps
 [-f <frequency>] : frequency (HZ) to have perf sample the system/process at [default=${HZ}]
                      Too high a value here can cause issues
 -h|-?        : show this help screen.

Note that the FlameGraph SVG (and perf.data file) are stored in the volatile ${PERF_RESULT_DIR_BASE} dir;
copy them to a non-volatile location to save them."
}

function die
{
echo >&2 "$@"
exit 1
}


### "main" here

which perf >/dev/null 2>&1 || {
 echo "${name}: perf not installed? Aborting...
 Tip- (Deb/Ubuntu) sudo apt install linux-tools-$(uname -r) linux-tools-generic"
 exit 1
}
[ ! -f ./2flameg.sh ] && {
 echo "${name}: the part-2 script 2flameg.sh is missing? Aborting..."
 exit 1
}
[ ! -d ${FLMGR} ] && {
 echo "${name}: I find that the original FlameGraph GitHub repo isn't installed.
 You need to (one-time) install it...
 In your terminal window/shell, type (including the parentheses):
 (cd; git clone https://github.com/brendangregg/FlameGraph)"
 exit 1
}
[ $# -lt 1 ] && {
  usage
  exit 1
}
[ $1 = "--help" ] && {
  usage
  exit 0
}

#--- getopts processing
optspec=":o:p:s:t:f:h?" # a : after an arg implies it expects an argument
unset PID
while getopts "${optspec}" opt
do
    #echo "opt=${opt} optarg=${OPTARG}"
    case "${opt}" in
			  o)
	 	        OUTFILE=${OPTARG}
		        #echo "-o passed; OUTFILE=${OUTFILE}"
			    ;;
			  p)
	 	        PID=${OPTARG}
		        #echo "-p passed; PID=${PID}"
				# Check if PID is valid
				sudo kill -0 ${PID} 2>/dev/null
			    [ $? -ne 0 ] && {
			      echo "${name}: PID ${PID} is an invalid (or dead) process/thread?"
			      exit 1
			    }
			    ;;
			  s)
	 	        STYLE=${OPTARG}
		        #echo "-s passed; STYLE=${STYLE}"
				if [ "${STYLE}" != "normal" -a  "${STYLE}" != "icicle" ]; then
					usage ; exit 1
				fi
				[ "${STYLE}" = "normal" ] && STYLE_INVERTED_ICICLE=0
			    ;;
			  t)
	 	        TYPE=${OPTARG}
		        #echo "-f passed; TYPE=${TYPE}"
				if [ "${TYPE}" != "graph" -a  "${TYPE}" != "chart" ]; then
					usage ; exit 1
				fi
				[ "${TYPE}" = "chart" ] && TYPE_CHART=1
			    ;;
			  f)
	 	        HZ=${OPTARG}
		        echo "-f passed; HZ=${HZ}"
			    ;;
			  h|?)
			    usage
				exit 0
				;;
			  *) echo "Unknown option '-${OPTARG}'" ; usage; exit 1
				;;
  	  esac
done
shift $((OPTIND-1))

[ -z "${OUTFILE}" ] && {
		usage ; exit 1
}

SVG=${OUTFILE}.svg
PDIR=${PERF_RESULT_DIR_BASE}/${OUTFILE}
TOPDIR=${PWD}

#--- Run the part 2 - generating the FG - on interrupt or exit
trap 'ls -lh perf.data; cd ${TOPDIR}; sync ; ./2flameg.sh ${PDIR} ${SVG} ${STYLE_INVERTED_ICICLE} ${TYPE_CHART}' INT EXIT
#trap 'cd ${TOPDIR}; echo Aborting run... ; sync ; exit 1' QUIT
#---

mkdir -p ${PDIR} || die "mkdir -p ${PDIR}"
sudo chown -R ${LOGNAME}:${LOGNAME} ${PERF_RESULT_DIR_BASE} 2>/dev/null
cd ${PDIR}

if [ ! -z "${PID}" ]; then  #------------------ Profile a particular process
 echo "### ${name}: recording samples on process PID ${PID} now...
 Press ^C to stop..."
 sudo perf record -F ${HZ} --call-graph dwarf -p ${PID} || exit 1  # generates perf.data
else                        #---------------- Profile system-wide
 echo "### ${name}: recording samples system-wide now...
 Press ^C to stop..."
 sudo perf record -F ${HZ} --call-graph dwarf -a || exit 1  # generates perf.data
fi
cd ${TOPDIR}

#exit 0  # this exit causes the 'trap' to run (as we've trapped the EXIT!)
