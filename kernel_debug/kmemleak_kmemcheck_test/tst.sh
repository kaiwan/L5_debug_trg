MOD=kmemchk_test
echo 0 > /proc/sys/kernel/kmemcheck
dmesg -C
rmmod ${MOD} 2>/dev/null
echo 1 > /proc/sys/kernel/kmemcheck
insmod ${MOD}.ko
sleep 1
echo 0 > /proc/sys/kernel/kmemcheck
dmesg > out.txt

