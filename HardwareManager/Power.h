#pragma once

HardwareManagerNamespaceBegin

class CPower
{
public:
	CPower();
	virtual ~CPower();

public:
	bool EnterHibernate();
	bool EnterSleep();
};

HardwareManagerNamespaceEnd
