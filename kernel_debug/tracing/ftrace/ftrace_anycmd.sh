#!/bin/bash
# ftrace1.sh
# A simple demo of using the powerful kernel ftrace facility.
# The program the user passes is ftrace-d; BUT, it's simplistic:
# whatever else is running at the time is traced as well.
#
# Kaiwan NB, kaiwanTECH, May16.
# License: MIT
name=$(basename $0)
PFX=/sys/kernel/debug/tracing
TRC_FILE=/tmp/trc.txt

reset_ftrc()
{
 echo 0 > ${PFX}/tracing_on
 echo nop > ${PFX}/current_tracer
 echo 1 > ${PFX}/options/latency-format
 echo 0 > ${PFX}/options/context-info
 echo 0 > ${PFX}/options/userstacktrace
 echo 0 > ${PFX}/tracing_max_latency # reset

 echo "" > ${PFX}/set_ftrace_filter
 echo "" > ${PFX}/set_ftrace_notrace
 echo "" > ${PFX}/set_ftrace_pid
 echo 2048 > ${PFX}/buffer_size_kb
}

init_ftrc()
{
 echo function_graph > ${PFX}/current_tracer
 echo 1 > ${PFX}/options/latency-format
 echo 1 > ${PFX}/options/context-info
 echo funcgraph-proc > ${PFX}/trace_options
 echo 1 > ${PFX}/options/userstacktrace

 echo "" > ${PFX}/set_ftrace_filter
 echo "" > ${PFX}/set_ftrace_notrace
 echo "" > ${PFX}/set_ftrace_pid
 echo 20480 > ${PFX}/buffer_size_kb
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
[ ! -d ${PFX} ] && {
 echo "${name}: directory \"${PFX}\" unavailable? Aborting..."
 exit 2
}
echo " [OK]"

[ $# -lt 1 ] && {
 echo "Usage: ${name} program-to-ftrace
 Eg. sudo ./${name} ps -LA
 [NOTE: other stuff running _also_ gets ftraced (this is non-exclusive).
 If you'd prefer _only_ tracing a particular process, it's easier to setup
 with 'trace-cmd record -F <app>']"
 exit 3
}

#cd /sys/kernel/debug/tracing || exit 3
echo "[+] ${name}: ftrace init ..."
reset_ftrc
init_ftrc
#echo "Available tracers:"
#cat available_tracers

echo "[+] ${name}: Running \"$@\" now ..."

#--- perform the ftrace
 echo 1 > ${PFX}/tracing_on ; eval "$@" ; echo 0 > ${PFX}/tracing_on 
#---

echo "[+] ${name}: Setting up ${TRC_FILE}, pl wait ..."
cp ${PFX}/trace ${TRC_FILE}
echo "[+] ${name}: Done. Trace file in ${TRC_FILE}"
ls -lh ${TRC_FILE}

###########################################
# TIP:
# filter out everything else by:
#grep "^ [0-9]).*<prcsnm>" ${TRC_FILE}
# NOTE: the process-name is truncated to just 7 chars, so don't use
# any more than 7 in the grep regex!
###########################################

reset_ftrc
exit 0
