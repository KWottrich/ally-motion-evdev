#include <filesystem>
#include <iostream>

#include "iiogyro.h"

bool Gyro::openDevice(const std::string& device)
{
	deviceDir = device;
	
	std::fstream statusFile;
	
	bool status = false;
	
	fileX.open(deviceDir + "/in_anglvel_x_raw", std::ios_base::in);
	if(!fileX.is_open())
	{
		std::cerr<<"in_anglvel_x_raw is required but not availble\n";
		return false;
	}
	
	fileY.open(deviceDir + "/in_anglvel_y_raw", std::ios_base::in);
	if(!fileX.is_open())
	{
		std::cerr<<"in_anglvel_y_raw is required but not availble\n";
		return false;
	}
	
	fileZ.open(deviceDir + "/in_anglvel_z_raw", std::ios_base::in);
	if(!fileZ.is_open())
	{
		std::cerr<<"in_anglvel_z_raw is required but not availble\n";
		return false;
	}
	
	scale = readFile(deviceDir + "/in_anglvel_scale", status);
	if(!status) 
	{
		std::cerr<<"Device not reporting in_anglvel_scale, assuming scale=1\n";
		scale = 1;
	}
	
	return true;
}

Gyro::Frame Gyro::getFrame()
{
	Frame frame;
	bool status;
	frame.x = readFile(fileX, status)*scale;
	frame.y = readFile(fileY, status)*scale;
	frame.z = readFile(fileZ, status)*scale;
	return frame;
}

int Gyro::getRate()
{
	bool status = false;
	int rate = readFile(deviceDir + "/in_anglvel_sampling_frequency", status);
	if(status) return rate;
	else return -1;
}

bool Gyro::setRate(unsigned int rate)
{
	std::fstream availableRatesFile;
	availableRatesFile.open(deviceDir + "/in_anglvel_sampling_frequency_available", std::ios_base::in);
	if(!availableRatesFile.is_open()) return false;
	
	std::fstream rateFile;
	rateFile.open(deviceDir + "/in_anglvel_sampling_frequency", std::ios_base::out);
	if(!rateFile.is_open())
	{
		availableRatesFile.close();
		return false;
	}
	
	int curRate=-1;
	while(availableRatesFile >> curRate) 
	{
		std::cout<<"available gyro sampling rate "<<curRate<<'\n';
		if(curRate >= static_cast<int>(rate))
		{
			std::cout<<"closest available rate "<<curRate<<'\n';
			rateFile<<curRate;
			break;
		}
	}
	availableRatesFile.close();
	rateFile.close();
	return true;
}

double Gyro::readFile(const std::string& fileName, bool& status)
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

double Gyro::readFile(std::fstream& file, bool& status)
{
	double result = 0;
	file.seekg(std::ios_base::beg);
	file >> result;
	status = !file.rdstate();
	return result;
}

Gyro::~Gyro()
{
	fileX.close();
	fileY.close();
	fileZ.close();
}

std::string Gyro::findGyro()
{
	for(const auto & entry : std::filesystem::directory_iterator(IIO_DIRECTORY))
    {
		if(entry.is_directory())
		{
			bool xFile = std::filesystem::exists(entry.path() / "in_anglvel_x_raw");
			bool yFile = std::filesystem::exists(entry.path() / "in_anglvel_y_raw");
			bool zFile = std::filesystem::exists(entry.path() / "in_anglvel_z_raw");
			if(xFile && yFile && zFile) return entry.path();
		}
    }
    return "";
}

