#!/bin/bash
name=$(basename $0)
KMOD=membugs_kasan

[ ! -f ${KMOD}.ko ] && {
  echo "${name}: kernel module \"${KMOD}.ko\" not present? Aborting ..."
  exit 1
}
ls -l ${KMOD}.ko

[ $# -ne 1 ] && {
 echo "Usage: ${name} testcase#"
 echo "
	test case  1 : uninitialized var test case
	test case  2 : out-of-bounds : write overflow [on compile-time memory]
	test case  3 : out-of-bounds : write overflow [on dynamic memory]
	test case  4 : out-of-bounds : write underflow
	test case  5 : out-of-bounds : read overflow [on compile-time memory]
	test case  6 : out-of-bounds : read overflow [on dynamic memory]
	test case  7 : out-of-bounds : read underflow
	test case  8 : UAF (use-after-free)
	test case  9 : UAR (use-after-return)
	test case 10 : double-free
	test case 11 : memory leak : simple leak
"
 exit 1
}

echo "sudo rmmod ${KMOD} 2>/dev/null ; sudo dmesg -C; sudo insmod ./${KMOD}.ko testcase=${1}; dmesg"
sudo rmmod ${KMOD} 2>/dev/null ; sudo dmesg -C; sudo insmod ./${KMOD}.ko testcase=${1}; dmesg
