#pragma once

HardwareManagerNamespaceBegin

class CMemory
{
public:
	CMemory();
	virtual ~CMemory();

public:
	int GetLoadPercent();
	unsigned long long GetTotalSize();
	unsigned long long GetAvailableSize();
};

HardwareManagerNamespaceEnd
