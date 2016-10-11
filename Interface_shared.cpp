#include "Interface_shared.h"


Interface::Interface() {
	//Get number of cores
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	numCPU = sysinfo.dwNumberOfProcessors;
}