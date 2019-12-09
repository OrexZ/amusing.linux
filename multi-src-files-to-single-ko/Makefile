# coding=UTF-8

ifneq ($(KERNELRELEASE),)#{

soga-objs += foo_01.o
soga-objs += foo_02.o
soga-objs += foo_init.o foo_exit.o
soga-objs += sub/sub_foo.o

obj-m := foo.o
foo-objs := $(soga-objs)

else

PWD           := $(shell pwd)

ARCH          ?= arm
KDIR          ?= /labs/linux-lab/output/arm/linux-v2.6.36-versatilepb/
CROSS_COMPILE ?= arm-linux-gnueabi-

all:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(H)rm -rf *.ko *.o *.symvers *.cmd *.cmd.o *.mod.c *.order .tmp_versions/ .*.o.cmd .*.mod.o.cmd .*.ko.cmd */*.o

endif
