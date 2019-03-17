# Makefile for kernel module
### Specifically: Kprobe helpers !

ifneq ($(KERNELRELEASE),)
	
	 
	# If you choose to keep the define USE_FTRACE_PRINT , we'll use
	# trace_printk() , else the regular printk()
	EXTRA_CFLAGS += -DDEBUG  # use regular printk()
	#EXTRA_CFLAGS += -DDEBUG -DUSE_FTRACE_PRINT  # use ftrace trace_printk()
	obj-m := helper_kp-do_filp_open-17Mar19.o

else
	#########################################
	# To support cross-compiling for the ARM:
	# For ARM, invoke make as:
	# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
	ifeq ($(ARCH),arm)
	# Update 'KDIR' below to point to the ARM Linux kernel source tree
		KDIR ?= ~/3.10.24
	else
		KDIR ?= /lib/modules/$(shell uname -r)/build 
	endif
	#########################################
	PWD   := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
