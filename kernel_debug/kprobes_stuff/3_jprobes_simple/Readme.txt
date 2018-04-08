Tips:
-----
Jprobes is a kernel feature; first, check whether Jprobes is enabled
for the current kernel:

grep -w register_jprobe /boot/System.map-$(uname -r)


Enable Kprobes within the kernel with:
   cd <kernel-src-tree>
   make menuconfig
     General Setup / Kprobes : turn it ON
     Exit with Save
   <rebuild kernel, reboot from new kernel>.

