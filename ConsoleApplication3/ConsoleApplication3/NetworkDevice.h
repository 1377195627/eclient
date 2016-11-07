#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H

#include "pcaphelper.h"
#include "WinSock2.h"
#include <string>

#pragma comment(lib,"ws2_32.lib")

struct NetworkDevice
{
	string name;
	string description;
};


#endif