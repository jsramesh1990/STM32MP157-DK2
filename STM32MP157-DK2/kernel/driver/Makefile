obj-m += virtual_gpio.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install
	depmod -a

load:
	sudo insmod virtual_gpio.ko

unload:
	sudo rmmod virtual_gpio

reload: unload load

logs:
	dmesg | tail -50
