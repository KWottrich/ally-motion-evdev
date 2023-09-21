# ally-motion-evdev
Tool to redirect ROG Ally accelerometer and gyro data from iio to evdev for use in games and applications.

Requires NeroReflex's kernel driver for the bmi323 sensor (see https://github.com/NeroReflex/linux-chimeraos/tree/bmi323)

## Build Instructions
1. `cmake .`
2. `make`
3. `chmod +x iio-uinput`

## Running
`./iio-uinput`

To get the names of available iio devices, run `ls /sys/bus/iio/devices/`
