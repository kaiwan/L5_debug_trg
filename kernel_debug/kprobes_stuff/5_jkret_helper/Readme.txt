Tips:
-----

----------------- IMP!
Jprobes has been *REMOVED* from kernel ver 4.15
The API wrappers are still present, but it won't really work!
------------------------

1. Kprobes is a kernel feature; first, check whether Kprobes is enabled
for the current kernel:
grep CONFIG_KPROBES /boot/config-$(uname -r)

2. Further, verify Jprobes support in this manner too:
sudo grep -w register_jprobe /boot/System.map-$(uname -r)

3. Enable Kprobes within the kernel with:
   cd <kernel-src-tree>
   make menuconfig
     General Setup / Kprobes : turn it ON
     Exit with Save
   <rebuild kernel, reboot from new kernel>.
