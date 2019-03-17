# Makefile for 2.6 kernel module
# This Makefile idiom from the Rubini & Corbet (LDD3) book.

ifneq ($(KERNELRELEASE),)
	obj-m       := hang_panic.o
	#obj-m       += soln_hang_panic.o

	# turn on debug mode
	EXTRA_CFLAGS += -DDEBUG

else
	#	KDIR        := /lib/modules/$(shell uname -r)/build
	#########################################
	# To support cross-compiling for the ARM:
	# For ARM, invoke make as:
	# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
	ifeq ($(ARCH),arm)
	# Update 'KDIR' below to point to the ARM Linux kernel source tree
	  KDIR ?= ~/ARM_Balau/linux-3.2.11
	else
	  KDIR ?= /lib/modules/$(shell uname -r)/build 
	  #KDIR ?= /mnt/big1_200G/trg/linux/DG-L4/Qemu_kgdb/linux-3.1.5
endif
	#########################################

	PWD         := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
endif

