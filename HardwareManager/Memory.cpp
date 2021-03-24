#include "HardwareManager.h"

HardwareManagerNamespaceBegin

CMemory::CMemory()
{}
CMemory::~CMemory()
{}

int CMemory::GetLoadPercent()
{
	MEMORYSTATUSEX MemoryStatus = { 0 };
    MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (::GlobalMemoryStatusEx(&MemoryStatus) == FALSE) return -1;
    return MemoryStatus.dwMemoryLoad;
}
unsigned long long CMemory::GetTotalSize()
{
	MEMORYSTATUSEX MemoryStatus = { 0 };
    MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (::GlobalMemoryStatusEx(&MemoryStatus) == FALSE) return -1;
    return MemoryStatus.ullTotalPhys;
}
unsigned long long CMemory::GetAvailableSize()
{
	MEMORYSTATUSEX MemoryStatus = { 0 };
    MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (::GlobalMemoryStatusEx(&MemoryStatus) == FALSE) return -1;
    return MemoryStatus.ullAvailPhys;
}

HardwareManagerNamespaceEnd
