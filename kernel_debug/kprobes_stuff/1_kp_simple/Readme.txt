Tips:
-----

1. Kprobes is a kernel feature; first, check whether Kprobes is enabled
for the current kernel:

grep CONFIG_KPROBES /boot/config-$(uname -r)

2. Further, verify Kprobes support in this manner too:

sudo grep -w register_kprobe /boot/System.map-$(uname -r)

