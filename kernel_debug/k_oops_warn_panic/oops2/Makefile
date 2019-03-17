ifeq ($(KERNELRELEASE),)  

#########################################
# To support cross-compiling for the ARM:
# For ARM, invoke make as:
# make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
ifeq ($(ARCH),arm)
# Update 'KDIR' below to point to the ARM Linux kernel source tree
KDIR ?= ~/scratchpad/SEALS_staging/SEALS_staging_vexpress/linux-4.9.1
else
KDIR ?= /lib/modules/$(shell uname -r)/build 
endif
#########################################

PWD := $(shell pwd)  

.PHONY: build clean  

build:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

else

$(info Building with KERNELRELEASE = ${KERNELRELEASE}) 
EXTRA_CFLAGS += -DDEBUG -g -ggdb -gdwarf-4 #-Wa,-a,-ad,-ah,-al
obj-m := oops2.o  

endif

