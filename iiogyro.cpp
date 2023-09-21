#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include "iiogyro.h"

bool Gyro::openDevice(const std::string& device)
{
	deviceDir = device;
	
	std::fstream statusFile;
	
	bool status = false;
	
	fileX.open(deviceDir + "/in_anglvel_x_raw", std::ios_base::in);
	if(!fileX.is_open())
	{
		std::cerr<<"in_anglvel_x_raw is required but not available\n";
		return false;
	}
	//fileX.close();
	
	fileY.open(deviceDir + "/in_anglvel_y_raw", std::ios_base::in);
	if(!fileY.is_open())
	{
		std::cerr<<"in_anglvel_y_raw is required but not available\n";
		return false;
	}
	//fileY.close();
	
	fileZ.open(deviceDir + "/in_anglvel_z_raw", std::ios_base::in);
	if(!fileZ.is_open())
	{
		std::cerr<<"in_anglvel_z_raw is required but not available\n";
		return false;
	}
	//fileZ.close();
	
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
	const std::string rateFile = deviceDir + "/in_anglvel_sampling_frequency";
	int rate = readFile(rateFile, status);
	if(status)
	{
		//std::cout<<"read rate "<<rate<<"hz from "<<rateFile<<std::endl;
		return rate;
	}
	else
	{
		std::cerr<<"failed to read rate from "<<rateFile<<std::endl;
		return -1;
	}
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
	
	double curRate=-1;
	while(availableRatesFile >> curRate) 
	{
		//std::cout<<"available gyro sampling rate "<<curRate<<'\n';
		if(curRate >= static_cast<int>(rate))
		{
			std::cout<<"setting gyro rate: "<<curRate<<'\n';
			rateFile<<curRate;
			break;
		}
	}
	availableRatesFile.close();
	rateFile.close();
	return true;
}

bool Gyro::setScale(double scale)
{
	std::fstream availableScalesFile;
	availableScalesFile.open(deviceDir + "/in_anglvel_scale_available", std::ios_base::in);
	if(!availableScalesFile.is_open())
	{
		std::cerr<<"Failed to open "<<deviceDir<<"/in_anglvel_scale_available"<<std::endl;
		return false;
	}
	
	std::fstream scaleFile;
	scaleFile.open(deviceDir + "/in_anglvel_scale", std::ios_base::out);
	if(!scaleFile.is_open())
	{
		availableScalesFile.close();
		std::cerr<<"Failed to open "<<deviceDir<<"/in_anglvel_scale for writing"<<std::endl;
		return false;
	}
	
	double curScale=-1;
	while(availableScalesFile >> curScale) 
	{
		//std::cout<<"available gyro sampling rate "<<curRate<<'\n';
		if(curScale >= static_cast<double>(scale))
		{
			std::cout<<"setting gyro scale: "<<curScale<<'\n';
			scaleFile<<curScale;
			break;
		}
	}
	availableScalesFile.close();
	scaleFile.close();
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

int Gyro::readRaw(const std::string& fileName, bool& status)
{
	FILE* fd = NULL;
	char buff[7]; // 6 characters for sign and 5 digits, plus one for null-terminator
	memset(buff, 0, sizeof(buff));

	fd = fopen(fileName.c_str(), "r");

	if (NULL == fd)
	{
		status = false;
		return 0;
	}

	int numRead = fread(buff, sizeof(char)*6, 1, fd);
	if (numRead < 0)
	{
		std::cerr<<"failed to read from "<<fileName<<", fread reported "<<numRead<<" chunks"<<std::endl;
		status = false;
		return 0;
	}
	fclose(fd);

	buff[6] = '\0'; //null-terminate the string
	int result = 0;
	sscanf(buff, "%d", &result);
	status = result;
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

