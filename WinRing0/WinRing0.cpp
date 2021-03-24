#include "WinRing0.h"
#include <tchar.h>
#include <io.h>
#include <string>

WinRing0NamespaceBegin

enum {
	ManageDriverInstall,
	ManageDriverRemove,
	ManageDriverSystemInstall,
	ManageDriverSystemRemove
};
class CriticalSection
{
public:
	CriticalSection()
	{
		::InitializeCriticalSection((LPCRITICAL_SECTION)&cs);
	}
	virtual ~CriticalSection()
	{
		::DeleteCriticalSection((LPCRITICAL_SECTION)&cs);
	}

private:
	CRITICAL_SECTION cs;

public:
	void Lock() const
	{
		::EnterCriticalSection((LPCRITICAL_SECTION)&cs);
	}
	void Unlock() const
	{
		::LeaveCriticalSection((LPCRITICAL_SECTION)&cs);
	}
};
class AutoCriticalSection
{
public:
	AutoCriticalSection(const CriticalSection &_cs) :
		cs(_cs)
	{
		cs.Lock();
	}
	virtual ~AutoCriticalSection()
	{
		cs.Unlock();
	}

private:
	const CriticalSection &cs;
};

volatile unsigned long bInit = 0;
CriticalSection csInit;
HANDLE hDriverHandle = NULL;

bool IsWow64()
{
	typedef BOOL (WINAPI *lpfnIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
	lpfnIsWow64Process IsWow64Process = (lpfnIsWow64Process)::GetProcAddress(::GetModuleHandle(_T("kernel32")), "IsWow64Process");
	if (!IsWow64Process) return false;
	BOOL bIsWow64 = FALSE;
	if (IsWow64Process(::GetCurrentProcess(), &bIsWow64) == FALSE) return false;
	return (bIsWow64 != FALSE);
}
bool IsX64()
{
	typedef BOOL (WINAPI *lpfnGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
	lpfnGetNativeSystemInfo GetNativeSystemInfo = (lpfnGetNativeSystemInfo)::GetProcAddress(::GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");
	if (!GetNativeSystemInfo) return false;
	SYSTEM_INFO sysi = { 0 };
	if (GetNativeSystemInfo(&sysi) == FALSE) return false;
	return (sysi.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
}
bool IsOnNetworkDrive(LPCTSTR Path)
{
	if (!Path) return false;
	TCHAR Root[4];
	Root[0] = Path[0];
	Root[1] = _T(':');
	Root[2] = _T('\\');
	Root[3] = _T('\0');
	if (Root[0] == _T('\\') || ::GetDriveType(Root) == DRIVE_REMOTE) return true;
	return false;
}
bool IsNT()
{
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osvi);
	return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
}
void GetAppPath(TCHAR *Buffer, size_t Size = MAX_PATH)
{
	if (!Buffer) return;
	HMODULE hModule = ::GetModuleHandle(NULL);
	::GetModuleFileName(hModule, Buffer, Size);
	if (TCHAR *Ptr = _tcsrchr(Buffer, _T('\\'))) Buffer[Ptr - Buffer + 1] = _T('\0');
}
bool GetRing0DriverName(TCHAR *Buffer, size_t Size = MAX_PATH)
{
	if (!Buffer) return false;
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osvi);
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			_tcscpy_s(Buffer, Size, Ring0DriverName9X);
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		{
			if (IsWow64()) {
				if (IsX64()) _tcscpy_s(Buffer, Size, Ring0DriverNameNTX64);
				else _tcscpy_s(Buffer, Size, Ring0DriverNameNTIA64);
			}
			else {
				_tcscpy_s(Buffer, Size, Ring0DriverNameNT);
			}
		}
		break;
	default:
		return false;
	}
	return true;
}
bool GetRing0DriverPath(TCHAR *Buffer, size_t Size = MAX_PATH)
{
	if (!Buffer) return false;
	GetAppPath(Buffer, Size);
	size_t Length = _tcslen(Buffer);
	if (Length >= Size) return false;
	if (!GetRing0DriverName(&Buffer[Length], Size - Length)) return false;
	return true;
}
bool InstallDriver(SC_HANDLE hSCManager, LPCTSTR DriverId, LPCTSTR DriverPath)
{
	SC_HANDLE hService = ::CreateService(hSCManager,
							DriverId,
							DriverId,
							SERVICE_ALL_ACCESS,
							SERVICE_KERNEL_DRIVER,
							SERVICE_DEMAND_START,
							SERVICE_ERROR_NORMAL,
							DriverPath,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL);
	if (!hService && ::GetLastError() != ERROR_SERVICE_EXISTS) return false;
	if (hService) ::CloseServiceHandle(hService);
	return true;
}
bool SystemInstallDriver(SC_HANDLE hSCManager, LPCTSTR DriverId, LPCTSTR DriverPath)
{
	SC_HANDLE hService = ::OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	if (!hService) return false;
	BOOL bRet = ::ChangeServiceConfig(hService,
							SERVICE_KERNEL_DRIVER,
							SERVICE_AUTO_START,
							SERVICE_ERROR_NORMAL,
							DriverPath,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL);
	::CloseServiceHandle(hService);
	return (bRet != FALSE);
}
bool RemoveDriver(SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	SC_HANDLE hService = ::OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	if (!hService) return true;
	BOOL bRet = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	return (bRet != FALSE);
}
bool StartDriver(SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	SC_HANDLE hService = ::OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	if (!hService) return false;
	BOOL bRet = ::StartService(hService, 0, NULL);
	if (!bRet && ::GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) return false;
	::CloseServiceHandle(hService);
	return true;
}
bool StopDriver(SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	SC_HANDLE hService = ::OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	if (!hService) return false;
	SERVICE_STATUS ServiceStatus;
	BOOL bRet = ::ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);
	::CloseServiceHandle(hService);
	return (bRet != FALSE);
}
bool OpenDriver()
{
	if (hDriverHandle && hDriverHandle != INVALID_HANDLE_VALUE) return false;
	hDriverHandle = ::CreateFile(_T("\\\\.\\")Ring0DriverID,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	if (hDriverHandle != INVALID_HANDLE_VALUE) return true;
	hDriverHandle = NULL;
	return false;
}
bool CloseDriver()
{
	if (hDriverHandle && hDriverHandle != INVALID_HANDLE_VALUE) ::CloseHandle(hDriverHandle);
	hDriverHandle = NULL;
	return true;
}
bool IsSystemInstallDriver(SC_HANDLE hSCManager, LPCTSTR DriverId, LPCTSTR DriverPath)
{
	SC_HANDLE hService = ::OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	if (!hService) return false;
	DWORD dwSize = 0;
	::QueryServiceConfig(hService, NULL, 0, &dwSize);
	LPQUERY_SERVICE_CONFIG lpServiceConfig = (LPQUERY_SERVICE_CONFIG)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
	if (!lpServiceConfig) return false;
	bool ret = (lpServiceConfig->dwStartType == SERVICE_AUTO_START);
	::CloseServiceHandle(hService);
	::HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpServiceConfig);
	return ret;
}
bool ManageDriver(LPCTSTR DriverId, LPCTSTR DriverPath, unsigned short Function)
{
	if (!DriverId || !DriverPath) return false;
	SC_HANDLE hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager) return false;
	bool ret = false;
	switch (Function)
	{
	case ManageDriverInstall:
		{
			if (InstallDriver(hSCManager, DriverId, DriverPath)) {
				ret = StartDriver(hSCManager, DriverId);
			}
		}
		break;
	case ManageDriverRemove:
		{
			if (!IsSystemInstallDriver(hSCManager, DriverId, DriverPath)) {
				StopDriver(hSCManager, DriverId);
				ret = RemoveDriver(hSCManager, DriverId);
			}
		}
		break;
	case ManageDriverSystemInstall:
		{
			if (IsSystemInstallDriver(hSCManager, DriverId, DriverPath)) ret = true;
			else {
				if (!OpenDriver()) {
					StopDriver(hSCManager, DriverId);
					RemoveDriver(hSCManager, DriverId);
					if (InstallDriver(hSCManager, DriverId, DriverPath)) {
						StartDriver(hSCManager, DriverId);
					}
					OpenDriver();
				}
				ret = SystemInstallDriver(hSCManager, DriverId, DriverPath);
			}
		}
		break;
	case ManageDriverSystemRemove:
		{
			if (IsSystemInstallDriver(hSCManager, DriverId, DriverPath)) ret = true;
			else {
				CloseDriver();
				if (StopDriver(hSCManager, DriverId)) {
					ret = RemoveDriver(hSCManager, DriverId);
				}
			}
		}
		break;
	default:
		break;
	}
	::CloseServiceHandle(hSCManager);
	return ret;
}

bool Initialize()
{
	if (!IsCurrentUserLocalAdministrator()) return false;
	AutoCriticalSection acs(csInit);
	if (bInit > 0) return true;
	TCHAR Ring0DriverPath[MAX_PATH] = { 0 };
	if (!Ring0DriverPath) return false;
	if (!GetRing0DriverPath(Ring0DriverPath)) return false;
	if (_taccess(Ring0DriverPath, 0) != 0) return false;
	if (IsOnNetworkDrive(Ring0DriverPath)) return false;
	if (IsNT()) {
		if (!OpenDriver()) {
			ManageDriver(Ring0DriverID, Ring0DriverPath, ManageDriverRemove);
			if (!ManageDriver(Ring0DriverID, Ring0DriverPath, ManageDriverInstall)) {
				ManageDriver(Ring0DriverID, Ring0DriverPath, ManageDriverRemove);
				return false;
			}
			if (!OpenDriver()) return false;
		}
	}
	else {
		hDriverHandle = ::CreateFile(_T("\\\\.\\")Ring0DriverName9X,
			0,
			0,
			NULL,
			0,
			FILE_FLAG_DELETE_ON_CLOSE,
			NULL);
		if (hDriverHandle == INVALID_HANDLE_VALUE) return false;
	}
	bInit++;
	return true;
}
void Release()
{
	AutoCriticalSection acs(csInit);
	bInit--;
	if (bInit > 0) return;
	CloseDriver();
	TCHAR Ring0DriverPath[MAX_PATH] = { 0 };
	if (GetRing0DriverPath(Ring0DriverPath)) {
		ManageDriver(Ring0DriverID, Ring0DriverPath, ManageDriverRemove);
	}
	CloseDriver();
}

//WinRing0Interface Type Var;

WinRing0Interface unsigned long GetRefCount()
{
	AutoCriticalSection acs(csInit);
	return bInit;
}
WinRing0Interface bool IsCurrentUserLocalAdministrator()
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;


   /*
      Determine if the current thread is running as a user that is a member of
      the local admins group.  To do this, create a security descriptor that
      has a DACL which has an ACE that allows only local aministrators access.
      Then, call AccessCheck with the current thread's token and the security
      descriptor.  It will say whether the user could access an object if it
      had that security descriptor.  Note: you do not need to actually create
      the object.  Just checking access against the security descriptor alone
      will be sufficient.
   */
   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   __try
   {

      /*
         AccessCheck() requires an impersonation token.  We first get a primary
         token and then create a duplicate impersonation token.  The
         impersonation token is not actually assigned to the thread, but is
         used in the call to AccessCheck.  Thus, this function itself never
         impersonates, but does use the identity of the thread.  If the thread
         was impersonating already, this function uses that impersonation context.
      */
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation, &hImpersonationToken))
          __leave;

      /*
        Create the binary representation of the well-known SID that
        represents the local administrators group.  Then create the security
        descriptor and DACL with an ACE that allows only local admins access.
        After that, perform the access check.  This will determine whether
        the current user is a local admin.
      */
      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
         __leave;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
         __leave;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

      /*
         AccessCheck validates a security descriptor somewhat; set the group
         and owner so that enough of the security descriptor is filled out to
         make AccessCheck happy.
      */
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         __leave;

      dwAccessDesired = ACCESS_READ;

      /*
         Initialize GenericMapping structure even though you
         do not use generic rights.
      */
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                       &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                       &fReturn))
      {
         fReturn = FALSE;
         __leave;
      }
   }
   __finally
   {
      // Clean up.
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);
      if (psidAdmin) FreeSid(psidAdmin);
      if (hImpersonationToken) CloseHandle (hImpersonationToken);
      if (hToken) CloseHandle (hToken);
   }

   return (fReturn != FALSE);
}
// CPU
WinRing0Interface bool IsCpuid()
{
	__try {
		int info[4] = { 0 };
		__cpuid(info, 0x0);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
	return true;
}
WinRing0Interface bool IsMsr()
{
	// MSR : Standard Feature Flag EDX, Bit 5 
	int info[4] = { 0 };
	__cpuid(info, 0x1);
	return ((info[3] >> 5) & 1);
}
WinRing0Interface bool IsTsc()
{
	// TSC : Standard Feature Flag EDX, Bit 4 
	int info[4] = { 0 };
	__cpuid(info, 0x1);
	return ((info[3] >> 4) & 1);
}
WinRing0Interface bool Rdmsr(DWORD index, PDWORD eax, PDWORD edx)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!eax || !edx || !IsMsr()) return false;
	DWORD Returned = 0;
	BYTE OutBuffer[8] = { 0 };
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_MSR,
					&index,
					sizeof(index),
					&OutBuffer,
					sizeof(OutBuffer),
					&Returned,
					NULL);
	if (bRet == FALSE) return false;
	memcpy_s(eax, sizeof(DWORD), OutBuffer, 4);
	memcpy_s(edx, sizeof(DWORD), OutBuffer + 4, 4);
	return true;
}
WinRing0Interface bool RdmsrTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Rdmsr(index, eax, edx);
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool RdmsrPx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Rdmsr(index, eax, edx);
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
WinRing0Interface bool Wrmsr(DWORD index, DWORD eax, DWORD edx)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!IsMsr()) return false;
	DWORD Returned = 0;
	OLS_WRITE_MSR_INPUT InBuffer = { 0 };
	DWORD OutBuffer = 0;
	InBuffer.Register = index;
	InBuffer.Value.HighPart = edx;
	InBuffer.Value.LowPart = eax;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_WRITE_MSR,
					&InBuffer,
					sizeof(InBuffer),
					&OutBuffer,
					sizeof(OutBuffer),
					&Returned,
					NULL);
	return (bRet != FALSE);
}
WinRing0Interface bool WrmsrTx(DWORD index, DWORD eax, DWORD edx, DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Wrmsr(index, eax, edx);
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool WrmsrPx(DWORD index, DWORD eax, DWORD edx, DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Wrmsr(index, eax, edx);
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
WinRing0Interface bool Rdpmc(DWORD index, PDWORD eax, PDWORD edx)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!eax || !edx || !IsMsr()) return false;
	DWORD Returned = 0;
	BYTE OutBuffer[8] = { 0 };
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_PMC,
					&index,
					sizeof(index),
					&OutBuffer,
					sizeof(OutBuffer),
					&Returned,
					NULL);
	if (bRet == FALSE) return false;
	memcpy_s(eax, sizeof(DWORD), OutBuffer, 4);
	memcpy_s(edx, sizeof(DWORD), OutBuffer + 4, 4);
	return true;
}
WinRing0Interface bool RdpmcTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Rdpmc(index, eax, edx);
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool RdpmcPx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Rdpmc(index, eax, edx);
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
#ifdef _M_X64
extern "C" {
void __fastcall _CPUIDx64(DWORD index, DWORD *pEAX, DWORD *pEBX, DWORD *pECX, DWORD *pEDX);
}
#endif
WinRing0Interface bool Cpuid(DWORD index, DWORD *pEAX, DWORD *pEBX, DWORD *pECX, DWORD *pEDX)
{
	if (!pEAX || !pEBX || !pECX || !pEDX || !IsCpuid()) return false;
#ifdef _M_X64
	*pECX = 0;
	_CPUIDx64(index, pEAX, pEBX, pECX, pEDX);
#else
	int info[4] = { 0 };
	__asm { mov ecx, 0 }
	__cpuid(info, index);
	*pEAX = info[0];
	*pEBX = info[1];
	*pECX = info[2];
	*pEDX = info[3];
#endif
	return true;
}
WinRing0Interface bool CpuidTx(DWORD index, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Cpuid(index, eax, ebx, ecx, edx);
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool CpuidPx(DWORD index, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Cpuid(index, eax, ebx, ecx, edx);
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
WinRing0Interface bool Rdtsc(PDWORD eax, PDWORD edx)
{
	if (!eax || !edx || !IsTsc()) return false;
	ULONGLONG value = 0;
	value = __rdtsc();
	*eax = (DWORD)((value >>  0) & 0xFFFFFFFF);
	*edx = (DWORD)((value >> 32) & 0xFFFFFFFF);
	return true;
}
WinRing0Interface bool RdtscTx(PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Rdtsc(eax, edx);
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool RdtscPx(PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Rdtsc(eax, edx);
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
WinRing0Interface bool Hlt()
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_HALT,
					NULL,
					0,
					NULL,
					0,
					&Returned,
					NULL);
	return (bRet != FALSE);
}
WinRing0Interface bool HltTx(DWORD_PTR threadAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hThread = NULL;
	DWORD_PTR Mask = 0;
	if (bIsNT) {
		hThread = ::GetCurrentThread();
		Mask = ::SetThreadAffinityMask(hThread, threadAffinityMask);
		if (Mask == 0) return false;
	}
	bool ret = Hlt();
	if (bIsNT) {
		::SetThreadAffinityMask(hThread, Mask);
	}
	return ret;
}
WinRing0Interface bool HltPx(DWORD_PTR processAffinityMask)
{
	bool bIsNT = IsNT();
	HANDLE hProcess = NULL;
	DWORD_PTR ProcessMask = 0, SystemMask = 0;
	if (bIsNT) {
		hProcess = ::GetCurrentProcess();
		::GetProcessAffinityMask(hProcess, &ProcessMask, &SystemMask);
		if (::SetProcessAffinityMask(hProcess, processAffinityMask) == FALSE) return false;
	}
	bool ret = Hlt();
	if (bIsNT) {
		::SetProcessAffinityMask(hProcess, ProcessMask);
	}
	return ret;
}
// I/O
WinRing0Interface bool ReadIoPortByte(WORD port, PBYTE value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	WORD Val = 0;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_IO_PORT_BYTE,
					&port,
					sizeof(port),
					&Val,
					sizeof(Val),
					&Returned,
					NULL);
	if (bRet == FALSE) return false;
	*value = (BYTE)Val;
	return true;
}
WinRing0Interface bool ReadIoPortWord(WORD port, PWORD value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	WORD Val = 0;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_IO_PORT_WORD,
					&port,
					sizeof(port),
					&Val,
					sizeof(Val),
					&Returned,
					NULL);
	if (bRet == FALSE) return false;
	*value = Val;
	return true;
}
WinRing0Interface bool ReadIoPortDword(WORD port, PDWORD value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	DWORD Val = 0;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_IO_PORT_DWORD,
					&port,
					sizeof(port),
					&Val,
					sizeof(Val),
					&Returned,
					NULL);
	if (bRet == FALSE) return false;
	*value = Val;
	return true;
}
WinRing0Interface bool WriteIoPortByte(WORD port, BYTE value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	OLS_WRITE_IO_PORT_INPUT InBuffer = { 0 };
	InBuffer.CharData = value;
	InBuffer.PortNumber = port;
	DWORD Length = offsetof(OLS_WRITE_IO_PORT_INPUT, CharData) + sizeof(InBuffer.CharData);
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_WRITE_IO_PORT_BYTE,
					&InBuffer,
					Length,
					NULL,
					0,
					&Returned,
					NULL);
	return (bRet != FALSE);
}
WinRing0Interface bool WriteIoPortWord(WORD port, WORD value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	OLS_WRITE_IO_PORT_INPUT InBuffer = { 0 };
	InBuffer.CharData = (UCHAR)value;
	InBuffer.PortNumber = port;
	DWORD Length = offsetof(OLS_WRITE_IO_PORT_INPUT, CharData) + sizeof(InBuffer.CharData);
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_WRITE_IO_PORT_WORD,
					&InBuffer,
					Length,
					NULL,
					0,
					&Returned,
					NULL);
	return (bRet != FALSE);
}
WinRing0Interface bool WriteIoPortDword(WORD port, DWORD value)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	DWORD Returned = 0;
	OLS_WRITE_IO_PORT_INPUT InBuffer = { 0 };
	InBuffer.CharData = (UCHAR)value;
	InBuffer.PortNumber = port;
	DWORD Length = offsetof(OLS_WRITE_IO_PORT_INPUT, CharData) + sizeof(InBuffer.CharData);
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_WRITE_IO_PORT_DWORD,
					&InBuffer,
					Length,
					NULL,
					0,
					&Returned,
					NULL);
	return (bRet != FALSE);
}
// PCI
bool ReadPciConfig(DWORD pciAddress, DWORD regAddress, PBYTE value, DWORD size, PDWORD error)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!value) return false;
	if (size == 2 && (regAddress & 1) != 0) return false;// alignment check
	if (size == 4 && (regAddress & 3) != 0) return false;// alignment check
	DWORD Returned = 0;
	OLS_READ_PCI_CONFIG_INPUT InBuffer = { 0 };
	InBuffer.PciAddress = pciAddress;
	InBuffer.PciOffset = regAddress;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_READ_PCI_CONFIG,
					&InBuffer,
					sizeof(InBuffer),
					value,
                    size,
					&Returned,
					NULL);
	return (bRet != FALSE);
}
WinRing0Interface bool ReadPciConfigByte(DWORD pciAddress, DWORD regAddress, PBYTE value)
{
	return ReadPciConfig(pciAddress, regAddress, (PBYTE)value, sizeof(BYTE), NULL);
}
WinRing0Interface bool ReadPciConfigWord(DWORD pciAddress, DWORD regAddress, PWORD value)
{
	return ReadPciConfig(pciAddress, regAddress, (PBYTE)value, sizeof(WORD), NULL);
}
WinRing0Interface bool ReadPciConfigDword(DWORD pciAddress, DWORD regAddress, PDWORD value)
{
	return ReadPciConfig(pciAddress, regAddress, (PBYTE)value, sizeof(DWORD), NULL);
}
bool WritePciConfig(DWORD pciAddress, DWORD regAddress, PBYTE value, DWORD size)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!value) return false;
	if (size == 2 && (regAddress & 1) != 0) return false;// alignment check
	if (size == 4 && (regAddress & 3) != 0) return false;// alignment check
	DWORD Returned = 0;
	DWORD Length = offsetof(OLS_WRITE_PCI_CONFIG_INPUT, Data) + size;
	OLS_WRITE_PCI_CONFIG_INPUT *InBuffer = (OLS_WRITE_PCI_CONFIG_INPUT*)malloc(Length);
	if (!InBuffer) return false;
	InBuffer->PciAddress = pciAddress;
	InBuffer->PciOffset = regAddress;
	BOOL bRet = ::DeviceIoControl(hDriverHandle,
					IOCTL_OLS_WRITE_PCI_CONFIG,
					InBuffer,
					Length,
					NULL,
                    0,
					&Returned,
					NULL);
	free(InBuffer);
	return (bRet != FALSE);
}
WinRing0Interface bool WritePciConfigByte(DWORD pciAddress, DWORD regAddress, BYTE value)
{
	return WritePciConfig(pciAddress, regAddress, (PBYTE)&value, sizeof(value));
}
WinRing0Interface bool WritePciConfigWord(DWORD pciAddress, DWORD regAddress, WORD value)
{
	return WritePciConfig(pciAddress, regAddress, (PBYTE)&value, sizeof(value));
}
WinRing0Interface bool WritePciConfigDword(DWORD pciAddress, DWORD regAddress, DWORD value)
{
	return WritePciConfig(pciAddress, regAddress, (PBYTE)&value, sizeof(value));
}
// Find PCI Device
WinRing0Interface DWORD FindPciDeviceById(WORD vendorId, WORD deviceId, BYTE index, BYTE maxBus)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (vendorId == 0xFFFF) return 0xFFFFFFFF;
	const BYTE maxDevice = 32;
	const BYTE maxFunction = 8;
	bool bMultiFuncFlag = false;
	DWORD pciAddress = 0xFFFFFFFF, id = 0, error = 0, count = 0;
	BYTE type = 0;
	for (DWORD bus = 0; bus <= maxBus; bus++) {
		for (DWORD dev = 0; dev < maxDevice; dev++) {
			bMultiFuncFlag = false;
			for (DWORD func = 0; func < maxFunction; func++) {
				if (!bMultiFuncFlag && func > 0) break;
				pciAddress = PciBusDevFunc(bus, dev, func);
				if (ReadPciConfig(pciAddress, 0, (PBYTE)&id, sizeof(id), &error)) {
					if (func == 0) {// Is Multi Function Device
						if (ReadPciConfig(pciAddress, 0x0E, (PBYTE)&type, sizeof(type), NULL)) {
							if (type & 0x80) bMultiFuncFlag = true;
						}
					}
					if (id == (vendorId | ((DWORD)deviceId << 16))) {
						if (count == index) return pciAddress;
						count++;
						continue;
					}
				}
			}
		}
	}
	return 0xFFFFFFFF;
}
WinRing0Interface DWORD FindPciDeviceByClass(BYTE baseClass, BYTE subClass, BYTE programIf, BYTE index, BYTE maxBus)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	const BYTE maxDevice = 32;
	const BYTE maxFunction = 8;
	bool bMultiFuncFlag = false;
	DWORD pciAddress = 0xFFFFFFFF, conf[3] = { 0 }, error = 0, count = 0;
	BYTE type = 0;
	for (DWORD bus = 0; bus <= maxBus; bus++) {
		for (DWORD dev = 0; dev < maxDevice; dev++) {
			bMultiFuncFlag = false;
			for (DWORD func = 0; func < maxFunction; func++) {
				if (!bMultiFuncFlag && func > 0) break;
				pciAddress = PciBusDevFunc(bus, dev, func);
				if (ReadPciConfig(pciAddress, 0, (PBYTE)conf, sizeof(conf), &error)) {
					if (func == 0) {// Is Multi Function Device
						if (ReadPciConfig(pciAddress, 0x0E, (PBYTE)&type, sizeof(type), NULL)) {
							if (type & 0x80) bMultiFuncFlag = true;
						}
					}
					if ((conf[2] & 0xFFFFFF00) == 
							(((DWORD)baseClass << 24) |
							((DWORD)subClass << 16) |
							((DWORD)programIf << 8))
						) {
						if (count == index) return pciAddress;
						count++;
						continue;
					}
				}
			}
		}
	}
	return 0xFFFFFFFF;
}
// Physical Memory
WinRing0Interface DWORD ReadDmiMemory(PBYTE buffer, DWORD count, DWORD unitSize)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!buffer) return 0;
	DWORD Returned = 0;
	OLS_READ_MEMORY_INPUT InBuffer = { 0 };
	if (sizeof(DWORD_PTR) == 4) {
		InBuffer.Address.HighPart = 0;
		InBuffer.Address.LowPart = (DWORD)0x000F0000;
	}
	else {
		InBuffer.Address.QuadPart = 0x000F0000;
	}
	InBuffer.UnitSize = unitSize;
	InBuffer.Count = count;
	DWORD Length = InBuffer.UnitSize * InBuffer.Count;
	if (Length > 65536) return 0;
	BOOL bRet = ::DeviceIoControl(
                        hDriverHandle,
                        IOCTL_OLS_READ_MEMORY,
                        &InBuffer,
                        sizeof(OLS_READ_MEMORY_INPUT),
                        buffer,
                        Length,
                        &Returned,
                        NULL);
	if (bRet != FALSE && Returned == Length) return count * unitSize;
	return 0;
}
WinRing0Interface DWORD ReadPhysicalMemory(DWORD_PTR address, PBYTE buffer, DWORD count, DWORD unitSize)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!buffer) return 0;
	DWORD Returned = 0;
	OLS_READ_MEMORY_INPUT InBuffer = { 0 };
	if (sizeof(DWORD_PTR) == 4) {
		InBuffer.Address.HighPart = 0;
		InBuffer.Address.LowPart = (DWORD)address;
	}
	else {
		InBuffer.Address.QuadPart = address;
	}
	InBuffer.UnitSize = unitSize;
	InBuffer.Count = count;
	DWORD Length = InBuffer.UnitSize * InBuffer.Count;
	BOOL bRet = ::DeviceIoControl(
                        hDriverHandle,
                        IOCTL_OLS_READ_MEMORY,
                        &InBuffer,
                        sizeof(OLS_READ_MEMORY_INPUT),
                        buffer,
                        Length,
                        &Returned,
                        NULL);
	if (bRet != FALSE && Returned == Length) return count * unitSize;
	return 0;
}
WinRing0Interface DWORD WritePhysicalMemory(DWORD_PTR address, PBYTE buffer, DWORD count, DWORD unitSize)
{
	if (!hDriverHandle || hDriverHandle == INVALID_HANDLE_VALUE) return false;
	if (!buffer) return 0;
	DWORD Returned = 0;
	DWORD Length = offsetof(OLS_WRITE_MEMORY_INPUT, Data) + count * unitSize;
	OLS_WRITE_MEMORY_INPUT *InBuffer = (OLS_WRITE_MEMORY_INPUT*)malloc(Length);
	if (sizeof(DWORD_PTR) == 4) {
		InBuffer->Address.HighPart = 0;
		InBuffer->Address.LowPart = (DWORD)address;
	}
	else {
		InBuffer->Address.QuadPart = address;
	}
	InBuffer->UnitSize = unitSize;
	InBuffer->Count = count;
	memcpy(&InBuffer->Data, buffer, count * unitSize);
	BOOL bRet = ::DeviceIoControl(
                        hDriverHandle,
                        IOCTL_OLS_WRITE_MEMORY,
                        &InBuffer,
                        Length,
                        NULL,
                        0,
                        &Returned,
                        NULL);
	free(InBuffer);
	if (bRet != FALSE) return count * unitSize;
	return 0;
}

WinRing0NamespaceEnd
