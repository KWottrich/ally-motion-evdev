#pragma once
#include <stdint.h>
#include <string>

class UinputDevice
{
private:
	int fd = -1;
	int64_t secAtInit = 0;
	int64_t usecAtInit = 0;
	
public:
	UinputDevice();
	~UinputDevice();
	bool openDev(const std::string& uinputPath, const std::string& name, uint16_t vendor, uint16_t product);
	bool sendAbs(int x, int y, int z, int rx, int ry, int rz);
};
