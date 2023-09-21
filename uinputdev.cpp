#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <linux/uinput.h>
#include <linux/input.h>

#include "uinputdev.h"

#define ACCEL_RANGE 512
#define GYRO_RANGE  2000 // max range is +/- 35 radian/s

UinputDevice::UinputDevice()
{
}

bool UinputDevice::openDev(const std::string& uinputPath, const std::string& name, uint16_t vendor, uint16_t product)
{
	fd = open(uinputPath.c_str(), O_WRONLY | O_NONBLOCK);
	if(fd < 0) return false;
	
	struct uinput_setup dev = {0};
	if (name.length() <= UINPUT_MAX_NAME_SIZE) strcpy(dev.name, name.c_str());
	else strcpy(dev.name, name.substr(0, UINPUT_MAX_NAME_SIZE).c_str());
	dev.id.bustype = BUS_USB;
	dev.id.vendor = vendor;
	dev.id.product = product;
	
	ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_ACCELEROMETER);
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	//ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_MSC);
	ioctl(fd, UI_SET_MSCBIT, MSC_TIMESTAMP);

	ioctl(fd, UI_SET_ABSBIT, ABS_X);
	ioctl(fd, UI_SET_ABSBIT, ABS_Y);
	ioctl(fd, UI_SET_ABSBIT, ABS_Z);
	ioctl(fd, UI_SET_ABSBIT, ABS_RX);
	ioctl(fd, UI_SET_ABSBIT, ABS_RY);
	ioctl(fd, UI_SET_ABSBIT, ABS_RZ);

	//ioctl(fd, UI_SET_KEYBIT, BTN_TRIGGER);
	//ioctl(fd, UI_SET_KEYBIT, BTN_THUMB);
	
	struct uinput_abs_setup devAbsX = {0};
	devAbsX.code = ABS_X;
	devAbsX.absinfo.minimum = -ACCEL_RANGE;
	devAbsX.absinfo.maximum = ACCEL_RANGE;
	devAbsX.absinfo.resolution = 255; // 255 units = 1g
	devAbsX.absinfo.fuzz = 5;
	devAbsX.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsX) < 0) return false;

	struct uinput_abs_setup devAbsY = {0};
	devAbsY.code = ABS_Y;
	devAbsY.absinfo.minimum = -ACCEL_RANGE;
	devAbsY.absinfo.maximum = ACCEL_RANGE;
	devAbsY.absinfo.resolution = 255; // 255 units = 1g
	devAbsY.absinfo.fuzz = 5;
	devAbsY.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsY) < 0) return false;
	
	struct uinput_abs_setup devAbsZ = {0};
	devAbsZ.code = ABS_Z;
	devAbsZ.absinfo.minimum = -ACCEL_RANGE;
	devAbsZ.absinfo.maximum = ACCEL_RANGE;
	devAbsZ.absinfo.resolution = 255; // 255 units = 1g
	devAbsZ.absinfo.fuzz = 5;
	devAbsZ.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsZ) < 0) return false;
	
	struct uinput_abs_setup devAbsRX = {0};
	devAbsRX.code = ABS_RX;
	devAbsRX.absinfo.minimum = -GYRO_RANGE;
	devAbsRX.absinfo.maximum = GYRO_RANGE;
	devAbsRX.absinfo.resolution = 1; // 1 unit = 1 degree/s
	devAbsRX.absinfo.fuzz = 16;
	devAbsRX.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsRX) < 0) return false;

	struct uinput_abs_setup devAbsRY = {0};
	devAbsRY.code = ABS_RY;
	devAbsRY.absinfo.minimum = -GYRO_RANGE;
	devAbsRY.absinfo.maximum = GYRO_RANGE;
	devAbsRY.absinfo.resolution = 1; // 1 unit = 1 degree/s
	devAbsRY.absinfo.fuzz = 16;
	devAbsRY.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsRY) < 0) return false;
	
	struct uinput_abs_setup devAbsRZ = {0};
	devAbsRZ.code = ABS_RZ;
	devAbsRZ.absinfo.minimum = -GYRO_RANGE;
	devAbsRZ.absinfo.maximum = GYRO_RANGE;
	devAbsRZ.absinfo.resolution = 1; // 1 unit = 1 degree/s
	devAbsRZ.absinfo.fuzz = 16;
	devAbsRZ.absinfo.flat = 0;
	if(ioctl(fd, UI_ABS_SETUP, &devAbsRZ) < 0) return false;
	
		
	if(ioctl(fd, UI_DEV_SETUP, &dev) < 0) return false;
	if(ioctl(fd, UI_DEV_CREATE) < 0) return false;

	timeval now = {0};
	gettimeofday(&now, NULL);
	secAtInit = now.tv_sec;
	usecAtInit = now.tv_usec;
	
	return true;
}

bool UinputDevice::sendAbs(int x, int y, int z, int rx, int ry, int rz)
{
	if(fd < 0) return false;
	
	struct input_event ev = {0};
	
	ev.type = EV_ABS;
	ev.code = ABS_X;
	ev.value = x;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;

	ev.code = ABS_Y;
	ev.value = y;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;

	ev.code = ABS_Z;
	ev.value = z;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;
	
	ev.code = ABS_RX;
	ev.value = rx;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;

	ev.code = ABS_RY;
	ev.value = ry;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;

	ev.code = ABS_RZ;
	ev.value = rz;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;
	
	timeval now = {0};
	gettimeofday(&now, NULL);
	ev.type = EV_MSC;
	ev.code = MSC_TIMESTAMP;
	ev.value = (now.tv_sec - secAtInit)*1000000 + (now.tv_usec - usecAtInit);
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;

	ev.time = now;
	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	if(write(fd, &ev, sizeof(ev)) != sizeof(ev)) 
		return false;
	
	return true;
}

UinputDevice::~UinputDevice()
{
	if(fd > 0) 
	{
		ioctl(fd, UI_DEV_DESTROY);
		close(fd);
	}
}

