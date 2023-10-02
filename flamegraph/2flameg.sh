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
#echo "#p = $# p= $*"

# Find whether run from the command-line !
# ref: https://stackoverflow.com/a/4262107/779269
tok=$(ps -o stat= -p $PPID)  # yields 'S+' via script and 'Ss' via cmdline
#echo "${tok}"
[ "${tok:1:1}" = "s" ] && {
 echo "${name}: should be invoked by the flame_grapher.sh script, not directly!"
 exit 1
}
[ $# -lt 4 ] && {
# echo "${name}: should be invoked by the flame_grapher.sh script, not directly!"
 echo "Usage: ${name} result-folder SVG-filename style-to-display(1 for icicle) type(1 for FlameChart)"
 exit 1
}

INFILE=perf.data
OUTFILE=${2}

TOPDIR=${PWD}
cd ${1} || exit 1
#pwd

INFILE=perf.data
OUTFILE=${2}
STYLE=${3}
TYPE=${4}

[ ! -f ${INFILE} ] && {
  echo "${name} : perf data file ${INFILE} invalid? Aborting..."
  cd ${TOPDIR}
  exit 1
}
if [ $3 -ne 0 -a $3 -ne 1 ] ; then
	echo "${name}: third parameter graph-style must be 0 or 1"
	exit 1
fi

echo
echo "${name}: Working ... generating SVG file \"${1}/${OUTFILE}\"..."

#---
# Interesting options to flamegraph.pl:
# (ref: https://github.com/brendangregg/FlameGraph )
# --title TEXT     # change title text
# --subtitle TEXT  # second level title (optional)
# --inverted       # icicle graph ; downward-growing
# --flamechart     # produce a flame chart (sort by time, do not merge stacks)
# --width NUM      # width of image (default 1200)
# --hash           # colors are keyed by function name hash
# --cp             # use consistent palette (palette.map)
# --notes TEXT     # add notes comment in SVG (for debugging)
WIDTH=1900  # can make it v large; you'll just have to scroll horizontally...
TITLE="CPU mixed-mode Flame"
# ${name} result-folder SVG-filename style-to-display(1 for icicle) type(1 for FlameChart)"
#            p1               p2           p3:STYLE                      p4:TYPE
[ ${TYPE} -eq 1 ] && PTYPE=--flamechart
NOTES="notes text: "
if [ ${STYLE} -eq 0 ] ; then
   [ ${TYPE} -eq 0 ] && {
     TITLE="${TITLE}Graph ${OUTFILE} ; style is normal (upward-growing stacks), type is graph"
	 NOTES="${NOTES}FlameGraph, type normal"
   } || {
     TITLE="${TITLE}Graph ${OUTFILE} ; style is normal (upward-growing stacks), type is chart"
	 NOTES="${NOTES}FlameGraph, type chart"
   }
   sudo perf script --input ${INFILE} | ${FLMGR}/stackcollapse-perf.pl | \
	  ${FLMGR}/flamegraph.pl  --title "${TITLE}" --subtitle "${OUTFILE}" ${PTYPE} \
	  --notes "${NOTES}" --width ${WIDTH} > ${OUTFILE} || {
     echo "${name}: failed."
     exit 1
   }
else
  [ ${TYPE} -eq 1 ] && {
	 TITLE="${TITLE}Chart ${OUTFILE}; style is flamechart (all stacks, X-axis is timeline)"
	 NOTES="${NOTES}FlameChart, type flamechart"
  } || {
	 TITLE="${TITLE}Graph ${OUTFILE}; style is flamegraph (merged stacks)"
	 NOTES="${NOTES}FlameGraph, type normal"
  }
  sudo perf script --input ${INFILE} | ${FLMGR}/stackcollapse-perf.pl | \
	  ${FLMGR}/flamegraph.pl --title "${TITLE}" --subtitle "${OUTFILE}" --inverted ${PTYPE} \
	  --notes "${NOTES}" --width ${WIDTH} > ${OUTFILE} || {
     echo "${name}: failed."
     exit 1
  }
fi

USERNM=$(echo ${LOGNAME})
sudo chown -R ${USERNM}:${USERNM} ${TOPDIR}/${1}/ 2>/dev/null
cd ${TOPDIR}
ls -lh ${1}/${OUTFILE}
echo

echo "<NOTES:> in the SVG (if any):"
grep -w "NOTES\:" ${1}/${OUTFILE}

# Display in chrome !
if [[ -f $(which google-chrome-stable) ]] ; then
  nohup google-chrome-stable --incognito --new-window ${1}/${OUTFILE} &
else
  echo "View the above SVG file in your web browser to see and zoom into the CPU FlameGraph."
fi

#exit 0
