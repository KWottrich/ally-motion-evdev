#pragma once
#include <string>
#include <fstream>

#define IIO_DIRECTORY "/sys/bus/iio/devices/"

class Gyro
{
public:
	
	class Frame
	{
	public:
		double x;
		double y;
		double z;
		inline void scale(double scale)
		{
			x = x*scale;
			y = y*scale;
			z = z*scale;
		}
	};
	
private:
	std::fstream fileX;
	std::fstream fileY;
	std::fstream fileZ;
	double scale;
	
	std::string deviceDir;
	
	double readFile(const std::string& fileName, bool& status);
	double readFile(std::fstream& file, bool& status);
	int readRaw(const std::string& fileName, bool& status);
	
public:
	static std::string findGyro();
	bool openDevice(const std::string& device);
	bool setRate(unsigned int rate);
	bool setScale(double scale);
	int getRate();
	Frame getFrame();
	~Gyro();
};
