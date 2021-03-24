#include "HardwareManager.h"
#include <powrprof.h>

#pragma comment(lib, "powrprof.lib")

HardwareManagerNamespaceBegin

CPower::CPower()
{}
CPower::~CPower()
{}

bool CPower::EnterHibernate()
{
	return (::SetSuspendState(TRUE, TRUE, FALSE) != FALSE);
}
bool CPower::EnterSleep()
{
	return (::SetSuspendState(FALSE, TRUE, FALSE) != FALSE);
}

HardwareManagerNamespaceEnd
