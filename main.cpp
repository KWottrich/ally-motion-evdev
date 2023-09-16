#include <iostream>
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
#define GYRO_SCALE 255/4.9

bool stop = false;

void sigTerm(int dummy) 
{
    stop = true;
}

int main(int argc, char** argv)
{
	UinputDevice dev;
	if(!dev.openDev("/dev/uinput", "VirtMotionController", 0x46d, 0xc214))
	{
		std::cerr<<"Failed to open /dev/uinput: ";
		perror(NULL);
		return -1;
	}
	
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
		//gyroFrame.scale(GYRO_SCALE);
		std::cout<<"\r";
		printf("%+4.6f | %+4.6f | %+4.6f", gyroFrame.x, gyroFrame.y, gyroFrame.z);
		/*
		if(abs(gyroFrame.x) > 512)
			gyroFrame.x = 0;
		if(abs(gyroFrame.y) > 512)
			gyroFrame.y = 0;
		if(abs(gyroFrame.z) > 512)
			gyroFrame.z = 0;
		*/
		dev.sendAbs(accelFrame.x,accelFrame.y,accelFrame.z,gyroFrame.x,gyroFrame.y,gyroFrame.z);
	}
	std::cout<<std::endl<<"Restoring original rate..."<<std::endl;
	
	accel.setRate(startingAccelRate);
	gyro.setRate(startingGyroRate);
	
	return 0;
}
