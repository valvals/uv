#! /bin/bash

# Set up GPIO20 and set to output
echo "20" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio20/direction
echo "1" > /sys/class/gpio/gpio20/value

# Set up GPIO21 and set to input
echo "21" > /sys/class/gpio/export
echo "in" > /sys/class/gpio/gpio21/direction

while ( true ) 
do
    # check if the pin is connected to GND and, if so, halt the system
    if [ $(</sys/class/gpio/gpio21/value) == 1 ]
    then
        echo "20" > /sys/class/gpio/unexport
        echo "21" > /sys/class/gpio/unexport
   sudo umount /dev/sda1
        shutdown -h now "System halted by a GPIO action"
    fi 
    sleep 1
done
