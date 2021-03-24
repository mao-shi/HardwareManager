#include "HardwareManager.h"

HardwareManagerNamespaceBegin

CComputer::CComputer()
{}
CComputer::~CComputer()
{}

void CComputer::Test()
{
	std::string str;
	std::wstring wstr;
	bool b;

	CGPUTemperature c;
	GPUTemperature g;
	c.GetTemperature(g);

	CDeviceMonitor monitor;
	MonitorEDID EDID;
	int n  = monitor.GetCount();
	for (int i = 0; i < n; i++){
		monitor.GetDescription(i, wstr);
		monitor.GetEDID(i, EDID);
	}

	CWIFI wifi;
	b = wifi.IsExist();

	CIDEDiskController disk(L"D");
	b = disk.IsDeviceExist();
}

HardwareManagerNamespaceEnd
