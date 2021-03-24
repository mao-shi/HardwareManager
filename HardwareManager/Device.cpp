#include "HardwareManager.h"
#include <InitGuid.h>
#include <DEVPKEY.H>

#pragma comment(lib, "Setupapi.lib")

HardwareManagerNamespaceBegin

CDevice::CDevice() :
	m_hDevInfoSet(NULL),
	m_nDevCount(0),
	m_pDevInfoList(NULL)
{}
CDevice::~CDevice()
{
	Cleanup();
}

void CDevice::Cleanup()
{
	if (m_hDevInfoSet) {
		::SetupDiDestroyDeviceInfoList(m_hDevInfoSet);
		m_hDevInfoSet = NULL;
	}
	if (m_pDevInfoList) {
		delete[] m_pDevInfoList;
		m_pDevInfoList = NULL;
	}
	m_nDevCount = 0;
}
bool CDevice::GetRegistryPropertyStr(int Index, DWORD dwProperty, std::wstring &strProperty)
{
	if (Index < 0 || Index >= m_nDevCount) return false;
	DWORD BufferSize = 0;
	LPWSTR pBuffer = NULL;
	DWORD DataType = 0;
	while (true) {
		BOOL bRet = ::SetupDiGetDeviceRegistryPropertyW(m_hDevInfoSet,
			&m_pDevInfoList[Index],
			dwProperty,
			&DataType,
			(PBYTE)pBuffer,
			BufferSize,
			&BufferSize);
		if (bRet == TRUE) break;
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (pBuffer) {
				::LocalFree(pBuffer);
				pBuffer = NULL;
			}
			pBuffer = (LPWSTR)::LocalAlloc(LPTR, BufferSize * 2);
		}
		else return false;
	}
	strProperty.clear();
    strProperty.append(pBuffer);
	::LocalFree(pBuffer);
	return true;
}
bool CDevice::GetRegistryPropertyUInt(int Index, DWORD dwProperty, unsigned int &PropertyValue)
{
	if (Index < 0 || Index >= m_nDevCount) return false;
	DWORD BufferSize = 0;
	BYTE *pBuffer = NULL;
	DWORD DataType = 0;
	while (true) {
		BOOL bRet = ::SetupDiGetDeviceRegistryPropertyA(m_hDevInfoSet,
			&m_pDevInfoList[Index],
			dwProperty,
			&DataType,
			(PBYTE)pBuffer,
			BufferSize,
			&BufferSize);
		if (bRet == TRUE) break;
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (pBuffer) {
				::LocalFree(pBuffer);
				pBuffer = NULL;
			}
			pBuffer = (BYTE*)::LocalAlloc(LPTR, BufferSize * 2);
		}
		else return false;
	}
	PropertyValue = (unsigned int)(*pBuffer);
	::LocalFree(pBuffer);
	return true;
}
bool CDevice::GetPropertyStr(int Index, const DEVPROPKEY *pPropertyKey, std::wstring &strProperty)
{
	if (Index < 0 || Index >= m_nDevCount || !pPropertyKey) return false;
	DWORD BufferSize = 0;
	LPWSTR pBuffer = NULL;
	DEVPROPTYPE DataType = 0;
	while (true) {
		BOOL bRet = ::SetupDiGetDevicePropertyW(m_hDevInfoSet,
			&m_pDevInfoList[Index],
			pPropertyKey,
			&DataType,
			(PBYTE)pBuffer,
			BufferSize,
			&BufferSize,
			0);
		if (bRet == TRUE) break;
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (pBuffer) {
				::LocalFree(pBuffer);
				pBuffer = NULL;
			}
			pBuffer = (LPWSTR)::LocalAlloc(LPTR, BufferSize);
		}
		else return false;
	}
	strProperty.clear();
    strProperty.append(pBuffer);
	::LocalFree(pBuffer);
	return true;
}
bool CDevice::GetPropertyStrList(int Index, const DEVPROPKEY *pPropertyKey, std::vector<std::wstring> &strPropertyList)
{
	if (Index < 0 || Index >= m_nDevCount || !pPropertyKey) return false;
	DWORD BufferSize = 0;
	LPWSTR pBuffer = NULL;
	DEVPROPTYPE DataType = 0;
	while (true) {
		BOOL bRet = ::SetupDiGetDevicePropertyW(m_hDevInfoSet,
			&m_pDevInfoList[Index],
			pPropertyKey,
			&DataType,
			(PBYTE)pBuffer,
			BufferSize,
			&BufferSize,
			0);
		if (bRet == TRUE) break;
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (pBuffer) {
				::LocalFree(pBuffer);
				pBuffer = NULL;
			}
			pBuffer = (LPWSTR)::LocalAlloc(LPTR, BufferSize);
		}
		else return false;
	}
	std::wstring strProperty;
	WCHAR LastWChar = L'\0';
	for (unsigned int i = 0; i < BufferSize / sizeof(WCHAR); i++) {
		if (LastWChar == L'\0') {
			if (pBuffer[i] != L'\0') {
				strProperty = pBuffer + i;
				strPropertyList.push_back(strProperty);
			}
		}
		LastWChar = pBuffer[i];
	}
	::LocalFree(pBuffer);
	return true;
}
bool CDevice::ChangeState(int Index, DWORD NewState)
{
	if (Index < 0 || Index >= m_nDevCount) return false;
	SP_PROPCHANGE_PARAMS PropChangeParam = { 0 };
	PropChangeParam.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	PropChangeParam.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	PropChangeParam.Scope = DICS_FLAG_GLOBAL;
	PropChangeParam.StateChange = NewState;
	PropChangeParam.HwProfile = 0;
	BOOL bRet = ::SetupDiSetClassInstallParamsA(m_hDevInfoSet, &m_pDevInfoList[Index], &PropChangeParam.ClassInstallHeader, sizeof(SP_PROPCHANGE_PARAMS));
	if (bRet == FALSE) return false;
	bRet = ::SetupDiChangeState(m_hDevInfoSet, &m_pDevInfoList[Index]);
	if (bRet == FALSE) return false;
	return true;
}

bool CDevice::Create(const GUID *GUID)
{
	Cleanup();
	if (GUID) m_hDevInfoSet = ::SetupDiGetClassDevsA(GUID, NULL, NULL, DIGCF_PRESENT);
	else m_hDevInfoSet = ::SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (m_hDevInfoSet == INVALID_HANDLE_VALUE) {
		m_hDevInfoSet = NULL;
		return false;
	}
	// 获取设备数目
	SP_DEVINFO_DATA DevInfoData;
	DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (int i = 0; ::SetupDiEnumDeviceInfo(m_hDevInfoSet, i, &DevInfoData); i++) {
		m_nDevCount = i + 1;
	}
	if (m_nDevCount == 0) return false;
	// 获取设备信息
	m_pDevInfoList = new SP_DEVINFO_DATA[m_nDevCount];
	for (int i = 0; i < m_nDevCount; i++) {
		m_pDevInfoList[i].cbSize =  sizeof(SP_DEVINFO_DATA);
		if (FALSE == ::SetupDiEnumDeviceInfo(m_hDevInfoSet, i, &m_pDevInfoList[i])) {
			Cleanup();
			return false;
		}
	}
	return true;
}
int CDevice::GetCount()
{
	return m_nDevCount;
}
bool CDevice::SetEnabled(int Index, bool Enabled)
{
	return ChangeState(Index, Enabled ? DICS_ENABLE : DICS_DISABLE);
}
bool CDevice::GetDescription(int Index, std::wstring &Description)
{
	return GetRegistryPropertyStr(Index, SPDRP_DEVICEDESC, Description);
}
bool CDevice::GetHardwareID(int Index, std::wstring &ID)
{
	return GetRegistryPropertyStr(Index, SPDRP_HARDWAREID, ID);
}
bool CDevice::GetFriendlyName(int Index, std::wstring &FriendlyName)
{
	return GetRegistryPropertyStr(Index, SPDRP_FRIENDLYNAME, FriendlyName);
}
bool CDevice::GetLoctionInfo(int Index, std::wstring &LoctionInfo)
{
	return GetRegistryPropertyStr(Index, SPDRP_LOCATION_INFORMATION, LoctionInfo);
}
bool CDevice::GetInstanceID(int Index, std::wstring &InstanceID)
{
	if (Index < 0 || Index >= m_nDevCount) return false;
	DWORD BufferSize = 0;
	PWSTR pBuffer = NULL;
	while (true) {
		BOOL bRet = ::SetupDiGetDeviceInstanceIdW(m_hDevInfoSet,
			&m_pDevInfoList[Index],
			pBuffer,
			BufferSize,
			&BufferSize);
		if (bRet == TRUE) break;
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (pBuffer) {
				::LocalFree(pBuffer);
				pBuffer = NULL;
			}
			pBuffer = (LPWSTR)::LocalAlloc(LPTR, BufferSize * 2);
		}
		else return false;
	}
	InstanceID.clear();
	InstanceID.append(pBuffer);
	::LocalFree(pBuffer);
	std::transform(InstanceID.begin(), InstanceID.end(), InstanceID.begin(),towupper);
	return true;
}
bool CDevice::GetParentInstanceID(int Index, std::wstring &InstanceID)
{
	bool ret = GetPropertyStr(Index, &DEVPKEY_Device_Parent, InstanceID);
	if (ret) std::transform(InstanceID.begin(), InstanceID.end(), InstanceID.begin(),towupper);
	return ret;
}
bool CDevice::GetChildren(int Index, std::vector<std::wstring> Children)
{
	return GetPropertyStrList(Index, &DEVPKEY_Device_Children, Children);
}
bool CDevice::GetDriverKeyName(int Index, std::wstring &Name)
{
	return GetRegistryPropertyStr(Index, SPDRP_DRIVER, Name);
}
bool CDevice::GetClass(int Index, std::wstring &Class)
{
	return GetRegistryPropertyStr(Index, SPDRP_CLASS, Class);
}
bool CDevice::GetClassGUID(int Index, std::wstring &ClassGUID)
{
	return GetRegistryPropertyStr(Index, SPDRP_CLASSGUID, ClassGUID);
}
bool CDevice::GetBusNumber(int Index, unsigned int &BusNumber)
{
	return GetRegistryPropertyUInt(Index, SPDRP_BUSNUMBER, BusNumber);
}
bool CDevice::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return GetPropertyStr(Index, &DEVPKEY_Device_Manufacturer, Manufacturer);
}
bool CDevice::GetMatchingDeviceID(int Index, std::wstring &ID)
{
	return GetPropertyStr(Index, &DEVPKEY_Device_MatchingDeviceId, ID);
}

// -----------------------------------------------------------------

CDeviceMonitor::CDeviceMonitor()
{
	Create(&GUID_DEVCLASS_MONITOR);
}
CDeviceMonitor::~CDeviceMonitor()
{}

bool CDeviceMonitor::GetEDID(int Index, MonitorEDID &EDID)
{
	// 理解以下代码前, 请查阅显示器EDID
	std::wstring InstanceID;
	if (!GetInstanceID(Index, InstanceID)) return false;
	std::wstring MonitorKeyName;
	MonitorKeyName = L"SYSTEM\\CurrentControlSet\\Enum\\";
	MonitorKeyName += InstanceID;
	MonitorKeyName += L"\\Device Parameters";
	HKEY hMonitorKey = NULL;
	LSTATUS lRet = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, MonitorKeyName.c_str(), 0, KEY_READ, &hMonitorKey);
	if (ERROR_SUCCESS != lRet) return false;
	DWORD DataType = REG_BINARY;
	DWORD DataLen = sizeof(MonitorEDID);
	lRet = RegQueryValueExW(hMonitorKey, L"EDID", 0, &DataType, (BYTE*)&EDID, &DataLen);
	::RegCloseKey(hMonitorKey);
	return (ERROR_SUCCESS == lRet);
}

HardwareManagerNamespaceEnd
