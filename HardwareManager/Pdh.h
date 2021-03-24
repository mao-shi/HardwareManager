#pragma once

#include <Pdh.h>

HardwareManagerNamespaceBegin

class CPdh
{
public:
	explicit CPdh(const TCHAR *type);
	virtual ~CPdh();

protected:
	HQUERY m_hQuery;
    HCOUNTER m_hCounter;

public:
	struct Type
	{
		static const TCHAR *CPU;
	};

public:
	bool CollectData(long &Value);
};

HardwareManagerNamespaceEnd
