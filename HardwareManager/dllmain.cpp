#include "HardwareManager.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		return (HardwareManagerNamespace::Initialize() ? TRUE : FALSE);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		HardwareManagerNamespace::Release();
		break;
	}
	return TRUE;
}
