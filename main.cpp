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

#define ACCEL_SCALE 255/9.81
#define GYRO_SCALE 180/M_PI
#define DEVNAME "VirtMotionController"

bool stop = false;

void sigTerm(int dummy) 
{
    stop = true;
}

int main(int argc, char** argv)
{
	UinputDevice dev;
	if(!dev.openDev("/dev/uinput", DEVNAME, 0x46d, 0xc214))
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
	
	if(config.rate > 0)
	{
		accel.setRate(config.rate);
		gyro.setRate(config.rate);
	}
	gyro.setScale(0.00106);
	
	double maxGyroX = 0, maxGyroY = 0, maxGyroZ = 0;
	std::cout<<"   Gyro X    |    Gyro Y    |    Gyro Z"<<std::endl;
	while(!stop)
	{
		Accelerometer::Frame accelFrame = accel.getFrame();
		accelFrame.scale(ACCEL_SCALE);
		if(abs(accelFrame.x) > 512)
			accelFrame.x = 0;
		if(abs(accelFrame.y) > 512)
			accelFrame.y = 0;
		if(abs(accelFrame.z) > 512)
			accelFrame.z = 0;
		
		Gyro::Frame gyroFrame = gyro.getFrame();
		gyroFrame.scale(GYRO_SCALE);
		std::cout<<"\33[2K\r";
		printf("%+012.6f | %+012.6f | %+012.6f", gyroFrame.x, gyroFrame.y, gyroFrame.z);
		std::cout<<std::flush;
		if (abs(gyroFrame.x) > maxGyroX)
			maxGyroX = abs(gyroFrame.x);
		if (abs(gyroFrame.y) > maxGyroY)
			maxGyroY = abs(gyroFrame.y);
		if (abs(gyroFrame.z) > maxGyroZ)
			maxGyroZ = abs(gyroFrame.z);
		
		dev.sendAbs(accelFrame.x,accelFrame.y,accelFrame.z,gyroFrame.x,gyroFrame.y,gyroFrame.z);
		if (config.rate > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds((1000/config.rate)));
		}
	}
	std::cout<<std::endl<<"Restoring original rate..."<<std::endl;
	
	accel.setRate(startingAccelRate);
	gyro.setRate(startingGyroRate);

	std::cout<<"Max gyro values reported:"<<std::endl;
	std::cout<<"X: "<<maxGyroX<<std::endl;
	std::cout<<"Y: "<<maxGyroY<<std::endl;
	std::cout<<"Z: "<<maxGyroZ<<std::endl;
	
	return 0;
}
