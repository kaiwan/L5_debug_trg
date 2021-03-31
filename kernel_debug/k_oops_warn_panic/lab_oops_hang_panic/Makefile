# Makefile for 2.6 kernel module
# This Makefile idiom from the Rubini & Corbet (LDD3) book.
ifneq ($(KERNELRELEASE),)
	obj-m       := hang_panic.o
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
	  KDIR ?= ~/seals_staging/linux-4.9.1
	#########################################
	else
	  KDIR ?= /lib/modules/$(shell uname -r)/build 
endif
$(info Building for ARCH ${ARCH}, KERNELRELEASE ${KERNELRELEASE} and CROSS_COMPILE=${CROSS_COMPILE})
	PWD         := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
