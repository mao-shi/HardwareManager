#include "HardwareManager.h"

HardwareManagerNamespaceBegin

CSystemMetrics::CSystemMetrics()
{}
CSystemMetrics::~CSystemMetrics()
{}

bool CSystemMetrics::IsTouchScreenSupported()
{
	int Metrics = ::GetSystemMetrics(SM_DIGITIZER);
	if ((Metrics & NID_INTEGRATED_TOUCH) == NID_INTEGRATED_TOUCH) return true;
	if ((Metrics & NID_EXTERNAL_TOUCH) == NID_EXTERNAL_TOUCH) return true;
	return false;
}

HardwareManagerNamespaceEnd
