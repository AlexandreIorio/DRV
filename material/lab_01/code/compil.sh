echo "Compiling counter"
arm-linux-gnueabihf-gcc-6.4.1 -o counter counter.c hex.c led.c

echo "Setting the execution permission"
chmod +x counter

echo "Copying counter to /export/drv/labo01"
sudo cp counter /export/drv/labo01   
