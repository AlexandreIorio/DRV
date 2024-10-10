# Compilation du noyau
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN -j$(nproc)
# Compilation des modules
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN modules
# Copie des modules dans le sous-répertoire tmp/
rm ./tmp -rf
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN INSTALL_MOD_PATH="./tmp" modules_install
# Compilation du DT
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN socfpga_cyclone5_sockit.dtb

# Backup de l'ancien noyau/DT et copie des nouveaux fichiers
cp /srv/tftp/socfpga.dtb /srv/tftp/socfpga.dtb.old
cp /srv/tftp/zImage /srv/tftp/zImage.old
cp arch/arm/boot/dts/socfpga_cyclone5_sockit.dtb /srv/tftp/socfpga.dtb
cp arch/arm/boot/zImage  /srv/tftp/
cp ./tmp/lib/modules/6.1.55-g57cf7f3b7f73-dirty/ /export/drv/ -R
