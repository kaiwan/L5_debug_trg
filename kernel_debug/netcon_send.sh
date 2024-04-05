#!/bin/bash
# netcon_send.sh

die()
{
echo >&2 "FATAL: $@"
exit 1
}

name=$(basename $0)
lsmod|grep netconsole >/dev/null && die "${name}: netconsole already loaded"
[[ $# -ne 2 ]] && die "Usage: ${name} sender-net-interface receiver-IP-address"

SENDER_INTF="$1"
SENDTO_IP="$2"
ip a | grep -w ${SENDER_INTF} > /dev/null || die "Specified sender n/w interface - ${SENDER_INTF} - not found?"

ip=$(ip a|grep  -w "${SENDER_INTF}" |grep "^ *inet "|awk '{print $2}'|sed 's/...$//')
[[ -z "${ip}" ]] && die "${name}: couldn't fetch sender's IP addr"

# Raise printk --console-level to 8 (all msgs goto console device)
sudo dmesg -n 8

# Install the netconsole LKM
# netconsole module parameter format:
#  netconsole=[+][src-port]@[src-ip]/[<dev>],[tgt-port]@<tgt-ip>/[tgt-macaddr]
sudo modprobe netconsole netconsole=@${ip}/${SENDER_INTF},@${SENDTO_IP}/
[[ $? -ne 0 ]] && die "${name}: modprobe netconsole failed"

# TIP
# Must see klog looking like this (then it typically works!):
# [  260.203296] netpoll: netconsole: local port 6666
# [  260.203299] netpoll: netconsole: local IPv4 address 192.168.1.22
# [  260.203300] netpoll: netconsole: interface 'enp0s8'
# [  260.203301] netpoll: netconsole: remote port 6666
# [  260.203302] netpoll: netconsole: remote IPv4 address 192.168.1.25
# [  260.203303] netpoll: netconsole: remote ethernet address ff:ff:ff:ff:ff:ff
# [  260.203350] printk: console [netcon0] enabled
# [  260.203353] netconsole: network logging started

echo "OK, the netconsole module is running ...
$(lsmod|grep netconsole)

Receive this system's kernel printk's by doing this on the receiver:
 netcat -d -u -l 6666

(Can test with:
sudo sh -c \"echo 'Hello via netconsole' > /dev/kmsg\"
)"
exit 0
