### Path to kernel sources ###
KERNELDIR := /home/aiorio/heig/drv/labos/linux-socfpga/
TOOLCHAIN := /opt/toolchains/arm-linux-gnueabihf_6.4.1/bin/arm-linux-gnueabihf-

DEPLOY_DIR := /export/drv/

# Nom du module kernel
obj-m := drivify_player.o

# Fichiers supplémentaires pour le module kernel
drivify_player-y := drivify.o playlist.o player.o keys.o hex.o led.o 

PWD := $(shell pwd)
WARN := -W -Wall -Wstrict-prototypes -Wmissing-prototypes

CC := $(TOOLCHAIN)gcc
CFLAGS := -I$(KERNELDIR)/include $(WARN)

all: drivify add_music deploy

drivify:
	@echo "Building kernel module drivify with kernel sources in $(KERNELDIR)"
	$(MAKE) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN) -C $(KERNELDIR) M=$(PWD) modules

add_music: insert_music.c
	@echo "Building user-space application add_music"
	$(CC) $(CFLAGS) -o add_music insert_music.c

deploy:
	@echo "Deploying drivify.ko and add_music to $(DEPLOY_DIR)"
	cp drivify_player.ko $(DEPLOY_DIR)
	cp add_music $(DEPLOY_DIR)

clean:
	@echo "Cleaning up build files"
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod *.mod.c .tmp_versions modules.order Module.symvers *.a add_music
