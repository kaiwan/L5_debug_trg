Tips:
-----
Jprobes is a kernel feature; first, check whether Jprobes is enabled
for the current kernel:

grep -w register_jprobe /boot/System.map-$(uname -r)

