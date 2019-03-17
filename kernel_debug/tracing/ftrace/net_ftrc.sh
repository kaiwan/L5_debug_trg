#!/bin/bash
# (c) kaiwanTECH

name=$(basename $0)

net_ftrc_setup()
{
 # "Include-only" approach yields more precise traces but at the cost of possibly
 # millions of comparisons - v slow.
 echo "---------> Including functions to ftrace, pl wait (takes a while!)... <---------"                 
 echo $(grep -i net /sys/kernel/debug/tracing/available_filter_functions) >> ${FTRCDIR}/set_ftrace_filter
[ 1 -eq 1 ] && {
 echo `grep -i tcp /sys/kernel/debug/tracing/available_filter_functions` >> ${FTRCDIR}/set_ftrace_filter
 echo `grep -i udp /sys/kernel/debug/tracing/available_filter_functions` >> ${FTRCDIR}/set_ftrace_filter
 echo `grep -i sock /sys/kernel/debug/tracing/available_filter_functions` >> ${FTRCDIR}/set_ftrace_filter
 echo `grep -i '^ip' /sys/kernel/debug/tracing/available_filter_functions` >> ${FTRCDIR}/set_ftrace_filter
 echo `grep -i xmit /sys/kernel/debug/tracing/available_filter_functions` >> ${FTRCDIR}/set_ftrace_filter
}
 echo "Functions to ftrace ::"
 cat ${FTRCDIR}/set_ftrace_filter
}

do_ftrc()
{
 # We want to only trace this process.
 # So have it delay for 'n' secs (3rd param); we can then pass it's pid
 # onto ftrace..
 #./${PRG2TRC} ${PRGARGS} &
 ./${PRG2TRC} 192.168.2.10 hello-net 5 &
 pid=$(ps -A|grep "${PRG2TRC}"|awk '{print $1}')
 [ -z ${pid} ] && {
   echo "${name}: could not detect pid of ${PRG2TRC} , aborting.."
   reset_ftrc
   exit 1
 }
 echo "pid = ${pid}"
 echo ${pid} > ${FTRCDIR}/set_ftrace_pid   # only ftrace this guy!!

 echo 1 > ${FTRCDIR}/tracing_on
 wait ${pid}
 echo 0 > ${FTRCDIR}/tracing_on

 rm -f ${FTRC_FILE}
 cp ${FTRCDIR}/trace ${FTRC_FILE}
 ls -lh ${FTRC_FILE}
}

ftrace_init()
{
 echo 20480 > ${FTRCDIR}/buffer_size_kb
 echo function_graph > ${FTRCDIR}/current_tracer
 # show process name/PID pair in the trace
 echo funcgraph-proc > ${FTRCDIR}/trace_options
 # turn ON latency-format
 echo 1 > ${FTRCDIR}/options/latency-format
 echo > ${FTRCDIR}/trace  # empty trace buffer
}

reset_ftrc()
{
  echo -1 > ${FTRCDIR}/set_ftrace_pid
  echo > ${FTRCDIR}/set_ftrace_filter
}


### "main" here

FTRCDIR=/sys/kernel/debug/tracing
FTRC_FILE=/tmp/trc.txt
PRG2TRC=talker_dgram
PRGARGS=192.168.2.10 hello-net 5

 [ $(id -u) -ne 0 ] && {
  echo "${name}: need to be root."
  exit 1
 }

 mount |grep debugfs >/dev/null 2>&1 || {
  echo "${name}: need to support debugfs (& ftrace)"
  exit 1
 }
 [ ! -d ${FTRCDIR} ] && { 
  echo "${name}: need to support ftrace"
  exit 1
 }
 echo "${name}: Ftrace seems to be present..."

 ftrace_init
 net_ftrc_setup
 do_ftrc
 reset_ftrc

 exit 0
