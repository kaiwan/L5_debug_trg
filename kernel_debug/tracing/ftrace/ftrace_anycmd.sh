#!/bin/sh
# ftrace_anycmd.sh
# A simple demo of using the powerful kernel ftrace facility.
# The program the user passes is ftrace-d; BUT, it's simplistic:
# whatever else is running at the time is traced as well.
# To help, we do attempt to filter out the noise and display just the
# items of relevance - the process being traced; but, YMMV! (Your Mileage
# May Vary :-)
#
# (c) Kaiwan N Billimoria, kaiwanTECH
# License: MIT
name=$(basename $0)
TRC_FILE=/tmp/trc.txt
# Setup buffer size of BUFFERSZ KB to capture info
BUFFERSZ=10240  #204800  # in KiB

#-------------- r u n c m d -------------------------------------------
# Display and run the provided command.
# Parameter 1 : the command to run
runcmd()
{
SEP="------------------------------"
[ $# -eq 0 ] && return
echo "${SEP}
$*"
eval "$*"
}

#-------------- r u n c m d _ f a i l c h k ---------------------------
# Display and run the provided command; check for failure case
#  Parameter 1 : 0 => non-fatal, +ve => fatal exit with this error val
#  Parameter 2... : the command to run
runcmd_failchk()
{
SEP="------------------------------"
[ $# -eq 0 ] && return
local errcode=$1
shift
echo "${SEP}
$*"
eval "$*" || {
  echo -n " *WARNING* execution failure"
  if [ ${errcode} -ne 0 ] ; then
    echo " : FATAL error (${errcode}), aborting now"
    exit ${errcode}
  fi
}
}

reset_ftrc()
{
 echo 0 > ${TRCMNT}/tracing_on
 runcmd_failchk 1 "echo nop > ${TRCMNT}/current_tracer"
 echo 1 > ${TRCMNT}/options/latency-format
 echo 0 > ${TRCMNT}/options/context-info
 echo 0 > ${TRCMNT}/options/display-graph
 echo 0 > ${TRCMNT}/options/userstacktrace
 echo 0 > ${TRCMNT}/options/verbose
 echo 0 > ${TRCMNT}/tracing_max_latency # reset

 echo "" > ${TRCMNT}/set_ftrace_filter
 echo "" > ${TRCMNT}/set_ftrace_notrace
 echo "" > ${TRCMNT}/set_ftrace_pid
 echo 2048 > ${TRCMNT}/buffer_size_kb
 cat /dev/null > ${TRCMNT}/trace
 echo -n ${orig_cpumask} > ${TRCMNT}/tracing_cpumask
}

init_ftrc()
{
 cat /dev/null > ${TRCMNT}/trace
 runcmd_failchk 1 "echo function_graph > ${TRCMNT}/current_tracer"
 echo 1 > ${TRCMNT}/options/latency-format
 echo 1 > ${TRCMNT}/options/context-info
 echo 1 > ${TRCMNT}/options/display-graph
 runcmd_failchk 1 "echo funcgraph-proc > ${TRCMNT}/trace_options"
 echo 1 > ${TRCMNT}/options/userstacktrace
 echo 1 > ${TRCMNT}/options/verbose

 echo "" > ${TRCMNT}/set_ftrace_filter
 echo "" > ${TRCMNT}/set_ftrace_notrace
 echo "" > ${TRCMNT}/set_ftrace_pid
 runcmd_failchk 1 "echo ${BUFFERSZ} > ${TRCMNT}/buffer_size_kb"
}


## "main" here
[ $(id -u) -ne 0 ] && {
 echo "${name}: Need to be root."
 exit 1
}

trap 'reset_ftrc' INT QUIT EXIT

# TODO: >= 4.1 it's become the 'tracefs' filesystem!
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

at=$(cat /sys/kernel/debug/tracing/available_tracers)
if [ "${at}" = "nop" ] ; then
 echo "${name}: Appears that Ftrace is present But Only the 'nop' plugin is
available; this typically implies that ftrace has not been fully configured
within the kernel. Pl re-configure, rebuild the kernel, install it and then
retry this script. Aborting..."
 exit 5
fi

echo " [OK] (ftrace loc: ${TRCMNT})"

which taskset >/dev/null 2>&1 || {
 echo "${name}: taskset(1) utility is required, pl install it. Aborting..."
 exit 3
}

[ $# -lt 1 ] && {
 echo "Usage: ${name} program-to-ftrace
 Eg. sudo ./${name} ps -LA
 NOTE: 
 (a) other stuff running _also_ gets ftraced (this is non-exclusive).
 If you'd prefer _only_ tracing a particular process, it's easier to setup
 with trace-cmd (or our 'trccmd' front-end to it)]
 (b) to help filter, we deliberately perform the run on only a single CPU core
     (core #0)"
 exit 4
}


echo "[+] ${name}: ftrace init ..."
reset_ftrc
init_ftrc
#echo "Available tracers:"
#cat available_tracers

#--- perform the ftrace on CPU #0 only (bit 1 set)
orig_cpumask=$(cat ${TRCMNT}/tracing_cpumask)
echo 1 > ${TRCMNT}/tracing_cpumask
echo "    orig cpumask = ${orig_cpumask}"
echo "    curr cpumask = $(cat ${TRCMNT}/tracing_cpumask)"

echo "[+] ${name}: Running \"sudo taskset -c 0 $@\" now ..."
#--- perform the ftrace on CPU #0 only
echo 1 > ${TRCMNT}/tracing_on ; sudo taskset -c 0 "$@" ; echo 0 > ${TRCMNT}/tracing_on
#---
# 'Should' work via just the eval $@ but it causes the filtering to fail.. unsure why??
# With doing the explicit taskset it does work...
 #echo 1 > ${TRCMNT}/tracing_on ; eval "$@" ; echo 0 > ${TRCMNT}/tracing_on

echo "[+] ${name}: Setting up full tracing report \"${TRC_FILE}\", pl wait ..."
[ -s ${TRCMNT}/per_cpu/cpu0/trace ] && cp ${TRCMNT}/per_cpu/cpu0/trace ${TRC_FILE}
[ -s ${TRCMNT}/per_cpu/cpu0/stats ] && cp ${TRCMNT}/per_cpu/cpu0/stats ${TRC_FILE}.stats
sync
echo "[+] ${name}: Done. Full trace file in ${TRC_FILE} (stats in ${TRC_FILE}.stats)"
ls -lh ${TRC_FILE}*

###
# TIP:
# filter out everything else by:
#grep "^ [0-9]) * <prcsnm>[- ]" ${TRC_FILE}
# NOTE: the process-name is truncated to just 7 chars, so don't use
# any more than 7 in the grep regex!
###
### Filtered report: it's a bit iffy :-/   YMMV
ret=0
prg="$@"
prg2=$(echo "${prg}" |awk '{print $1}')
prgname=$(basename ${prg2})
echo "[+] ${name}: now generating *filtered* trace report for process/thread \"${prgname}\" only ... "
egrep "^ [0-9]+) * ${prgname}[- ]" ${TRC_FILE} > trc_${prgname}.txt
#grep "^ [0-9]+).*${prgname}" ${TRC_FILE} > trc_${prgname}.txt

sz=$(stat trc_${prgname}.txt |grep "Size:"|awk  '{print $2}')
if [ ${sz} -eq 0 ] ; then
  echo " Couldn't seem to get the filtered trace directly, trying fallback approach..."
  rm -f trc_${prgname}.txt 2>/dev/null

  # fallback: look for first 7 chars of this script name foll by "-<PID>"
  first7=$(echo "${name}" |awk '{printf("%s", substr($0,1,7))}')
  searchstr="${first7}-$$"
  #echo "searchstr=${searchstr}"
  egrep "^ [0-9]) * ${searchstr}" ${TRC_FILE} > trc_${prgname}.txt
  sz=$(stat trc_${prgname}.txt |grep "Size:"|awk  '{print $2}')
  if [ ${sz} -eq 0 ] ; then
     echo " Couldn't seem to get the filtered trace, sorry!"
     rm -f trc_${prgname}.txt 2>/dev/null
     ret=1
  else
     echo " Got it:"
     ls -l trc_${prgname}.txt
  fi
else
  echo " Got it:"
  ls -l trc_${prgname}.txt
fi

reset_ftrc
exit ${ret}
