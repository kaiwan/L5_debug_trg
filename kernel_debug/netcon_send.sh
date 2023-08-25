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

# Install the netconsole LKM
# netconsole module parameter format:
#  netconsole=[+][src-port]@[src-ip]/[<dev>],[tgt-port]@<tgt-ip>/[tgt-macaddr]
sudo modprobe netconsole netconsole=@${ip}/${SENDER_INTF},@${SENDTO_IP}/
[[ $? -ne 0 ]] && die "${name}: modprobe netconsole failed"

echo "OK, the netconsole module is running ...
$(lsmod|grep netconsole)

Receive this system's kernel printk's by doing this on the receiver:
 netcat -d -u -l 6666

(Can test with:
sudo sh -c \"echo 'Hello via netconsole' > /dev/kmsg\"
)"
exit 0
