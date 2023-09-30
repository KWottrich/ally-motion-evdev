#include <filesystem>
#include <iostream>

#include "iioaccell.h"

bool Accelerometer::openDevice(const std::string& device)
{
	deviceDir = device;
	
	std::fstream statusFile;
	
	bool status = false;
	
	fileX.open(deviceDir + "/in_accel_x_raw", std::ios_base::in);
	if(!fileX.is_open())
	{
		std::cerr<<"in_accel_x_raw is required but not availble\n";
		return false;
	}
	
	fileY.open(deviceDir + "/in_accel_y_raw", std::ios_base::in);
	if(!fileX.is_open())
	{
		std::cerr<<"in_accel_y_raw is required but not availble\n";
		return false;
	}
	
	fileZ.open(deviceDir + "/in_accel_z_raw", std::ios_base::in);
	if(!fileZ.is_open())
	{
		std::cerr<<"in_accel_z_raw is required but not availble\n";
		return false;
	}

	xScale = readFile(deviceDir + "/in_accel_scale", status);
	if(status) 
	{
		yScale = xScale;
		zScale = xScale;
		return true;
	}
	
	xScale = readFile(deviceDir + "/in_accel_x_scale", status);
	if(!status) 
	{
		xScale = 1;
		std::cerr<<"Device not reporting in_accel_x_scale, assuming scale=1\n";
	}
	
	yScale = readFile(deviceDir + "/in_accel_y_scale", status);
	if(!status) 
	{
		yScale = 1;
		std::cerr<<"Device not reporting in_accel_y_scale, assuming scale=1\n";
	}
	
	zScale = readFile(deviceDir + "/in_accel_z_scale", status);
	if(!status) 
	{
		std::cerr<<"Device not reporting in_accel_z_scale, assuming scale=1\n";
		zScale = 1;
	}
	
	return true;
}

Accelerometer::Frame Accelerometer::getFrame()
{
	Frame frame;
	bool status;
	frame.x = readFile(fileX, status)*xScale;
	frame.y = readFile(fileY, status)*yScale;
	frame.z = readFile(fileZ, status)*zScale;
	return frame;
}

int Accelerometer::getRate()
{
	bool status = false;
	int rate = readFile(deviceDir + "/in_accel_sampling_frequency", status);
	if(status) return rate;
	else return -1;
}

bool Accelerometer::setRate(unsigned int rate)
{
	std::fstream availableRatesFile;
	availableRatesFile.open(deviceDir + "/in_accel_sampling_frequency_available", std::ios_base::in);
	if(!availableRatesFile.is_open()) return false;
	
	std::fstream rateFile;
	rateFile.open(deviceDir + "/in_accel_sampling_frequency", std::ios_base::out);
	if(!rateFile.is_open())
	{
		availableRatesFile.close();
		return false;
	}
	
	double curRate=-1;
	while(availableRatesFile >> curRate) 
	{
		//std::cout<<"available accel sampling rate "<<curRate<<'\n';
		if(curRate >= static_cast<int>(rate))
		{
			//std::cout<<"setting accel rate: "<<curRate<<'\n';
			rateFile<<curRate;
			break;
		}
	}
	availableRatesFile.close();
	rateFile.close();
	return true;
}

double Accelerometer::readFile(const std::string& fileName, bool& status)
{
	std::fstream file;
	file.open(fileName, std::ios_base::in);
	if(!file.is_open())
	{
		status = false;
		return 0;
	}
	double result = readFile(file, status);
	file.close();
	return result;
}

double Accelerometer::readFile(std::fstream& file, bool& status)
{
	double result = 0;
	file.seekg(std::ios_base::beg);
	file >> result;
	status = !file.rdstate();
	return result;
}

Accelerometer::~Accelerometer()
{
	fileX.close();
	fileY.close();
	fileZ.close();
}

std::string Accelerometer::findAccelerometer()
{
	for(const auto & entry : std::filesystem::directory_iterator(IIO_DIRECTORY))
    {
		if(entry.is_directory())
		{
			bool xFile = std::filesystem::exists(entry.path() / "in_accel_x_raw");
			bool yFile = std::filesystem::exists(entry.path() / "in_accel_y_raw");
			bool zFile = std::filesystem::exists(entry.path() / "in_accel_z_raw");
			if(xFile && yFile && zFile) return entry.path();
		}
    }
    return "";
}

