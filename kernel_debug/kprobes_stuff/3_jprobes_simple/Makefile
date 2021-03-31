# Makefile for 2.6 kernel module
# This Makefile idiom from the Rubini & Corbet (LDD3) book.

ifneq ($(KERNELRELEASE),)
	obj-m       := jprobe_eg.o

# turn on debug mode
#   By itself, -a defaults to -ahls.
#EXTRA_CFLAGS += -DDEBUG -g -Wa,-a,-ad
EXTRA_CFLAGS += -DDEBUG -g

else
	KDIR        := /lib/modules/$(shell uname -r)/build
	PWD         := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

