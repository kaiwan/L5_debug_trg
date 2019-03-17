#!/bin/bash
[ `id -u` -ne 0 ] && {
 echo "Need root"
 exit 1
}
[ ! -f /sys/kernel/debug/kmemleak ] && {
 echo "Verify that kmemleak is enabled! 
If required, pass the kernel cmdline param 'kmemleak=on'"
 exit 1
}

echo clear > /sys/kernel/debug/kmemleak
make && {
 echo "--- Running test case now ..."
 ./tst.sh
}
DELAY_SEC=100
echo "--- Delaying for ${DELAY_SEC} seconds now, pl wait ..."
sleep ${DELAY_SEC}
echo
echo scan > /sys/kernel/debug/kmemleak
echo "--- Kmemleak report:
----------------"
cat /sys/kernel/debug/kmemleak
