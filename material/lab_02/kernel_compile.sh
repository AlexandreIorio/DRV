# Compilation du noyau
echo "On entre dans linux-socfpga"
cd linux-socfpga
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN -j$(nproc)
# Compilation des modules
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN modules
# Copie des modules dans le sous-répertoire tmp/
rm ./tmp -rf
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN INSTALL_MOD_PATH="./tmp" modules_install
# Compilation du DT
make ARCH=arm CROSS_COMPILE=$TOOLCHAIN socfpga_cyclone5_sockit.dtb

# Backup de l'ancien noyau/DT et copie des nouveaux fichiers
sudo cp /srv/tftp/socfpga.dtb /srv/tftp/socfpga.dtb.old
sudo cp /srv/tftp/zImage /srv/tftp/zImage.old
sudo cp arch/arm/boot/dts/socfpga_cyclone5_sockit.dtb /srv/tftp/socfpga.dtb
sudo cp arch/arm/boot/zImage  /srv/tftp/
sudo cp ./tmp/lib/modules/6.1.55-g57cf7f3b7f73-dirty/ /export/drv/ -R
echo "On sort de linux-socfpga"
cd ..
