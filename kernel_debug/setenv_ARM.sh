# For cross-compilation env:
# Run as:
# . <path-to>/setenv_ARM.sh
export ARCH=arm
# this depends on the cross-compiler
export CROSS_COMPILE=arm-none-linux-gnueabi-
# this depends on the kernel src tree used for the QEMU-ARM system
export ARM_KERNEL=~/seals_staging/linux-4.9.1
