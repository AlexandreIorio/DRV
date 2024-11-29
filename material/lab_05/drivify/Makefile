### Path to kernel sources ###
KERNELDIR := /home/aiorio/heig/drv/labos/linux-socfpga/
TOOLCHAIN := /opt/toolchains/arm-linux-gnueabihf_6.4.1/bin/arm-linux-gnueabihf-

DEPLOY_DIR := /export/drv/

# Nom du module
obj-m := drivify.o

# Ajout des fichiers supplémentaires nécessaires
drivify-y := ex1.o music.o keys.o 
#button.o

PWD := $(shell pwd)
WARN := -W -Wall -Wstrict-prototypes -Wmissing-prototypes

all: drivify deploy

drivify:
	@echo "Building with kernel sources in $(KERNELDIR)"
	$(MAKE) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN) -C $(KERNELDIR) M=$(PWD) ${WARN} modules

deploy:
	@echo "Deploying drivify.ko to $(DEPLOY_DIR)"
	cp drivify.ko $(DEPLOY_DIR)

clean:
	@echo "Cleaning up build files"
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod *.mod.c .tmp_versions modules.order Module.symvers *.a
