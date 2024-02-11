obj-m += networkfs.o
networkfs-objs += entrypoint.o http_client.o
ccflags-y := -std=gnu11

CONFIG_MODULE_SIG=n

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

load: all
	sudo insmod networkfs.ko
	sudo mount -t networkfs  8c6a65c8-5ca6-49d7-a33d-daec00267011 ../mount_point/

unload:
	sudo umount ../mount_point
	sudo rmmod networkfs
	make clean

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
