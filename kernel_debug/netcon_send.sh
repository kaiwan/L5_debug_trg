#!/bin/bash
# netcon_send.sh
name=$(basename $0)

#-------- KEEP UPDATED
INTF=enp0s8
#---------------------

[ $# -ne 1 ] && {
  echo "Usage: ${name} receiver-IP-address"
  exit 1
}
SENDTO_IP="$1"

lsmod|grep -q netconsole && {
  echo "${name}: netconsole already loaded"
  exit 1
}

# get IP address
ip=$(ip a|grep  -w "${INTF}" |grep "^ *inet "|awk '{print $2}'|sed 's/...$//')
[ -z "${ip}" ] && {
  echo "${name}: couldn't fetch IP addr"
  exit 1
}

# Install the netconsole LKM
# netconsole module parameter format:
#  netconsole=[+][src-port]@[src-ip]/[<dev>],[tgt-port]@<tgt-ip>/[tgt-macaddr]
sudo modprobe netconsole netconsole=@${ip}/${INTF},@${SENDTO_IP}/
[ $? -ne 0 ] && {
  echo "${name}: modprobe netconsole failed"
  exit 1
}
echo "OK, netconsole LKM running ...
Receive this system's kernel printk's by doing this on the receiver:
 netcat -d -u -l 6666

(Can test with:
sudo sh -c "echo \"Hello via netconsole\" > /dev/kmsg"
)
"
#lsmod|grep netconsole
exit 0
