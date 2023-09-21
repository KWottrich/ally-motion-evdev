#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include "uinputdev.h"
#include "iioaccell.h"
#include "iiogyro.h"
#include "argpopt.h"

#define DEVNAME "Bosch BMI323 6DoF Sensor"
#define VENDOR 0x108c // Robert Bosch GmbH
#define PRODUCT 0x323
#define ACCEL_SCALE 255/9.81 // convert m/s^2 to g's, and scale x255 to increase precision when passed to evdev as an int
#define GYRO_SCALE 180/M_PI  // convert radians/s to degrees/s

bool stop = false;

void sigTerm(int _)
{
    stop = true;
}

int main(int argc, char** argv)
{
	UinputDevice dev;
	if(!dev.openDev("/dev/uinput", DEVNAME, VENDOR, PRODUCT))
	{
		std::cerr<<"Failed to open /dev/uinput: ";
		perror(NULL);
		return -1;
	}
	std::cout<<"Registered virtual device "<<DEVNAME<<std::endl;
	
	Config config;
	argp_parse(&argp, argc, argv, 0, 0, &config);
	
	if(config.device.empty())
	{
		config.device = Accelerometer::findAccelerometer();
		if(config.device.empty())
		{
			std::cout<<"No accelerometer specified and none found\n";
			return 0;
		}
	}
	std::cout<<"Found device "<<config.device<<std::endl;
	
	Accelerometer accel;
	if(!accel.openDevice(config.device))
	{
		std::cerr<<"failed to open iio device "<<config.device<<" as an accelerometer"<<std::endl;
		return -1;
	}
	int startingAccelRate = accel.getRate();

	Gyro gyro;
	if(!gyro.openDevice(config.device))
	{
		std::cerr<<"failed to open iio device "<<config.device<<" as a gyroscope"<<std::endl;
		return -1;
	}
	int startingGyroRate = gyro.getRate();
	
	signal(SIGINT, sigTerm);
	signal(SIGTERM, sigTerm);
	signal(SIGHUP, sigTerm);
	
	int sleep_ms = 0;
	if(config.rate > 0)
	{
		accel.setRate(config.rate);
		gyro.setRate(config.rate);
		sleep_ms = 1000/config.rate;
		std::cout<<"Setting sleep between frames at "<<sleep_ms<<"ms"<<std::endl;
	}
	if(!gyro.setScale(0.00106))
	{
		std::cerr<<"Failed to change gyro scale!"<<std::endl;
	}
	
	std::cout<<"Accel X | Accel Y | Accel Z | Gyro X | Gyro Y | Gyro Z"<<std::endl;

	while(!stop)
	{
		Accelerometer::Frame accelFrame = accel.getFrame();
		accelFrame.scale(ACCEL_SCALE); // Convert m/s^2 to 255 * g
		int accelX = static_cast<int>(accelFrame.x);
		int accelY = static_cast<int>(accelFrame.y);
		int accelZ = static_cast<int>(accelFrame.z);

		Gyro::Frame gyroFrame = gyro.getFrame();
		gyroFrame.scale(GYRO_SCALE); // Convert radians/s to degrees/s
		int gyroX = static_cast<int>(gyroFrame.x);
		int gyroY = static_cast<int>(gyroFrame.y);
		int gyroZ = static_cast<int>(gyroFrame.z);
		std::cout<<"\33[2K\r";
		printf("  %+4d  |   %+4d  |   %+4d  | %+5d  | %+5d  | %+5d", accelX, accelY, accelZ, gyroX, gyroY, gyroZ);
		std::cout<<std::flush;
		
		dev.sendAbs(accelX,accelY,accelZ,gyroFrame.x,gyroFrame.y,gyroFrame.z);
		if (sleep_ms) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
	}

	std::cout<<std::endl<<"Restoring original rate..."<<std::endl;
	accel.setRate(startingAccelRate);
	gyro.setRate(startingGyroRate);
	
	return 0;
}
