# ally-motion-evdev
Tool to redirect ROG Ally accelerometer and gyro data from iio to evdev for use in games and applications.

Requires NeroReflex's kernel driver for the bmi323 sensor (see https://github.com/NeroReflex/linux-chimeraos/tree/bmi323)

## Build Instructions
1. `cmake .`
2. `make`
3. `chmod +x ally-motion-evdev`

## udev Rule
By default, the virtual device will be created at `/dev/input/eventXX` with read permissions restricted to root. Access to this device can be granted to logged-in users by adding the following `udev` rule:

`ACTION=="add|change", KERNEL=="event[0-9]*", SUBSYSTEM=="input", ATTRS{id/vendor}=="108c", ATTRS{id/product}=="0323", MODE="0660", TAG+="uaccess"`

Adding a `udev` rule can be done by creating a new file in `/usr/lib/udev/rules.d/` (filename doesn't matter) and pasting in the above rule. Then you can either reboot your system or run `sudo udevadm control -R`.

## Running
`./ally-motion-evdev`

To get the names of available iio devices, run `ls /sys/bus/iio/devices/`
