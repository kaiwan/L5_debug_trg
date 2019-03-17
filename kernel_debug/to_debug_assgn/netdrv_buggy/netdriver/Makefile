ifeq ($(KERNELRELEASE),) 
######################################### 
# To support cross-compiling for the ARM: 
# For ARM, invoke make as:
# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
ifeq ($(ARCH),arm) 
# Update 'KDIR' below to point to the ARM Linux kernel (headers) source tree 
 KDIR ?= ~/DG-L3/linux-3.2.21
else 
 KDIR ?= /lib/modules/$(shell uname -r)/build 
endif 
######################################### 
PWD := $(shell pwd) 
.PHONY: build 
build: 
	$(MAKE) -Wall -C $(KDIR) M=$(PWD) modules 
install: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install 
clean: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
else 
  $(info Building with ARCH := ${ARCH} and KERNELRELEASE := ${KERNELRELEASE}) 
#  EXTRA_CFLAGS += -DDEBUG -g -ggdb
  obj-m := buggy_veth_dyn_ops.o
  buggy_veth_dyn_ops-objs := buggy_veth_netdrv.o debugfs_veth.o
endif
