#pragma once

HardwareManagerNamespaceBegin

class CSystemMetrics
{
public:
	CSystemMetrics();
	virtual ~CSystemMetrics();

public:
	bool IsTouchScreenSupported();
};

HardwareManagerNamespaceEnd
