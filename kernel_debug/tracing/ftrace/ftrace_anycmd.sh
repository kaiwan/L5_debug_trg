#!/bin/bash
# ftrace1.sh
# A simple demo of using the powerful kernel ftrace facility.
# The program the user passes is ftrace-d; BUT, it's simplistic:
# whatever else is running at the time is traced as well.
#
# Kaiwan NB, kaiwanTECH
# License: MIT
name=$(basename $0)
TRC_FILE=/tmp/trc.txt
# Setup a large buffer (200 MiB) to capture info
BUFFERSZ=204800

reset_ftrc()
{
 echo 0 > ${TRCMNT}/tracing_on
 echo nop > ${TRCMNT}/current_tracer
 echo 1 > ${TRCMNT}/options/latency-format
 echo 0 > ${TRCMNT}/options/context-info
 echo 0 > ${TRCMNT}/options/display-graph
 echo 0 > ${TRCMNT}/options/userstacktrace
 echo 0 > ${TRCMNT}/tracing_max_latency # reset

 echo "" > ${TRCMNT}/set_ftrace_filter
 echo "" > ${TRCMNT}/set_ftrace_notrace
 echo "" > ${TRCMNT}/set_ftrace_pid
 echo 2048 > ${TRCMNT}/buffer_size_kb
}

init_ftrc()
{
 echo function_graph > ${TRCMNT}/current_tracer
 echo 1 > ${TRCMNT}/options/latency-format
 echo 1 > ${TRCMNT}/options/context-info
 echo 1 > ${TRCMNT}/options/display-graph
 echo funcgraph-proc > ${TRCMNT}/trace_options
 echo 1 > ${TRCMNT}/options/userstacktrace

 echo "" > ${TRCMNT}/set_ftrace_filter
 echo "" > ${TRCMNT}/set_ftrace_notrace
 echo "" > ${TRCMNT}/set_ftrace_pid
 echo ${BUFFERSZ} > ${TRCMNT}/buffer_size_kb
}


## "main" here
[ $(id -u) -ne 0 ] && {
 echo "${name}: Need to be root."
 exit 1
}
echo -n "[+] Checking for ftrace support ..."
mount | grep debugfs > /dev/null 2>&1 || {
 echo "${name}: debugfs not mounted? Aborting..."
 exit 1
}
mount |grep -q -w debugfs || {
 echo "${name}: debugfs filesystem not mounted? Aborting..."
 exit 2
}
TRCMNT=$(mount |grep -w debugfs |awk '{print $3}')
export TRCMNT=${TRCMNT}/tracing
[ ! -d ${TRCMNT} ] && {
 echo "${name}: ${TRCMNT} not mounted as debugfs? Aborting..."
 exit 2
}
echo " [OK] (ftrace loc: ${TRCMNT})"

[ $# -lt 1 ] && {
 echo "Usage: ${name} program-to-ftrace
 Eg. sudo ./${name} ps -LA
 NOTE: 
 (a) other stuff running _also_ gets ftraced (this is non-exclusive).
 If you'd prefer _only_ tracing a particular process, it's easier to setup
 with trace-cmd (or our 'trccmd' front-end to it)]
 (b) to help filter, we deliberately perform the run on only a single CPU core
     (core #0)"
 exit 3
}

echo "[+] ${name}: ftrace init ..."
reset_ftrc
init_ftrc
#echo "Available tracers:"
#cat available_tracers

echo "[+] ${name}: Running \"sudo taskset -c 0 $@\" now ..."
#--- perform the ftrace on CPU #0 only
 echo 1 > ${TRCMNT}/tracing_on ; sudo taskset -c 0 "$@" ; echo 0 > ${TRCMNT}/tracing_on 
#---

echo "[+] ${name}: Setting up full tracing report \"${TRC_FILE}\", pl wait ..."
cp ${TRCMNT}/per_cpu/cpu0/trace ${TRC_FILE}
cp ${TRCMNT}/per_cpu/cpu0/stats ${TRC_FILE}.stats
sync
#cp ${TRCMNT}/trace ${TRC_FILE} ; sync
echo "[+] ${name}: Done. Full trace file in ${TRC_FILE} (stats in ${TRC_FILE}.stats)"
ls -lh ${TRC_FILE}*

###
# TIP:
# filter out everything else by:
#grep "^ [0-9]).*<prcsnm>" ${TRC_FILE}
# NOTE: the process-name is truncated to just 7 chars, so don't use
# any more than 7 in the grep regex!
###
### Filtered report: it's a bit iffy :-/   YMMV
prg="$@"
prg2=$(echo "${prg}" |awk '{print $1}')
prgname=$(basename ${prg2})
echo "[+] ${name}: now generating *filtered* trace report for process/thread \"${prgname}\" only here... "
egrep "^ [0-9]) * ${prgname}[- ]" ${TRC_FILE} > trc_${prgname}.txt
#grep "^ [0-9]).*${prgname}" ${TRC_FILE} > trc_${prgname}.txt
sz=$(stat --printf="%s" trc_${prgname}.txt)
[ ${sz} -ne 0 ] && ls -lh trc_${prgname}.txt || {
  echo " Couldn't seem to get the filtered trace, sorry!"
}

reset_ftrc
exit 0
