[ `id -u` -ne 0 ] && {
 echo "Need root"
 exit 1
}
make && ./tst.sh ; dmesg
