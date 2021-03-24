#include "HardwareManager.h"
#include <tchar.h>
#include <io.h>
#include <string>

HardwareManagerNamespaceBegin

bool Initialize()
{
	HRESULT hr = ::CoInitialize(NULL);
	if (hr != S_OK && hr != S_FALSE) return false;
	return true;
}
void Release()
{
	::CoUninitialize();
}

HardwareManagerNamespaceEnd
