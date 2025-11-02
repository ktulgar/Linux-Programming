obj-m += bmp180.o
KDIR ?= /home/ktulgar/Desktop/buildroot/output/build/linux-custom
CROSS ?= arm-linux-gnueabihf-


CFLAGS_bmp180.o += -fno-stack-protector

.PHONY: all clean
all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

