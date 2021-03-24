#include "HardwareManager.h"
#include <WbemIdl.h>
#include <WbemCli.h>
#include <comutil.h>

#pragma comment(lib, "WbemUuid.lib")
#pragma comment(lib, "comsuppw.lib")

#ifndef NAMESPACE_ROOT_WMI
#define NAMESPACE_ROOT_WMI L"ROOT\\WMI"
#endif
#ifndef NAMESPACE_ROOT_CIMV2
#define NAMESPACE_ROOT_CIMV2 L"ROOT\\cimv2"
#endif

HardwareManagerNamespaceBegin
	
CWMICoreManager::CWMICoreManager(const wchar_t *Namespace, const wchar_t *Query) :
	m_pWbemLocator(NULL),
	m_pWbemServices(NULL),
	m_pWbemRefresher(NULL),
	m_pWbemConfigRefresher(NULL),
	m_pEnumObject(NULL),
	m_pObjectArray(NULL),
	m_nObjectCount(0),
	m_pRefreshAbleObjectArray(NULL)
{
	if (!Create(Namespace)) return;
	if (!WQLQuery(Query)) return;
}
CWMICoreManager::~CWMICoreManager()
{
	CleanupQueried();
	CleanupCreated();
}

void CWMICoreManager::CleanupCreated()
{
	if (m_pWbemLocator) {
		m_pWbemLocator->Release();
		m_pWbemLocator = NULL;
	}
	if (m_pWbemServices) {
		m_pWbemServices->Release();
		m_pWbemServices = NULL;
	}
	if (m_pWbemRefresher) {
		m_pWbemRefresher->Release();
		m_pWbemRefresher = NULL;
	}
	if (m_pWbemConfigRefresher) {
		m_pWbemConfigRefresher->Release();
		m_pWbemConfigRefresher = NULL;
	}
}
void CWMICoreManager::CleanupQueried()
{
	if (m_pObjectArray) {
		for (int i = 0; i < m_nObjectCount; i ++) {
			if (m_pObjectArray[i]) {
				m_pObjectArray[i]->Release();
				m_pObjectArray[i] = NULL;
			}
		}
		delete[] m_pObjectArray;
		m_pObjectArray = NULL;
	}
	if (m_pRefreshAbleObjectArray) {
		for (int i = 0; i < m_nObjectCount; i ++) {
			if (m_pRefreshAbleObjectArray[i]) {
				m_pRefreshAbleObjectArray[i]->Release();
				m_pRefreshAbleObjectArray[i] = NULL;
			}
		}
		delete[] m_pRefreshAbleObjectArray;
		m_pRefreshAbleObjectArray = NULL;
	}
	if (m_pEnumObject) {
		m_pEnumObject->Release();
		m_pEnumObject = NULL;
	}
	m_nObjectCount = 0;
}
bool CWMICoreManager::Create(const wchar_t *Namespace)
{
	if (!Namespace) return false;
	HRESULT hr = ::CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&m_pWbemLocator);
	if (FAILED(hr)) {
		CleanupCreated();
		return false;
	}
	hr = m_pWbemLocator->ConnectServer(_bstr_t(Namespace), NULL, NULL, NULL, 0, NULL, NULL, &m_pWbemServices);
	if (FAILED(hr)) {
		CleanupCreated();
		return false;
	}
	hr = ::CoSetProxyBlanket(m_pWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hr)) {
		CleanupCreated();
		return false;
	}
	hr = ::CoCreateInstance(CLSID_WbemRefresher, NULL, CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (LPVOID*)&m_pWbemRefresher);
	if (FAILED(hr)) {
		CleanupCreated();
		return false;
	}
	hr = m_pWbemRefresher->QueryInterface(IID_IWbemConfigureRefresher, (LPVOID*)&m_pWbemConfigRefresher);
	if (FAILED(hr)) {
		CleanupCreated();
		return false;
	}
	return true;
}
bool CWMICoreManager::WQLQuery(const wchar_t *Query)
{
	if (!Query || !m_pWbemServices) return false;
	CleanupQueried();
	HRESULT hr = m_pWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(Query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &m_pEnumObject);
	if (FAILED(hr)) return false;
	int MAX_SIZE = 5;
	m_pObjectArray = new IWbemClassObject*[MAX_SIZE];
	memset(m_pObjectArray, 0, MAX_SIZE * sizeof(IWbemClassObject*));
	ULONG ulReturn = 0;
	while (true) {
		if (m_nObjectCount >= MAX_SIZE) {
			IWbemClassObject **pTempArray = new IWbemClassObject*[MAX_SIZE * 2];
			memset(pTempArray, 0, MAX_SIZE * 2 * sizeof(IWbemClassObject*));
			memcpy_s(pTempArray, MAX_SIZE * 2 * sizeof(IWbemClassObject*), m_pObjectArray, MAX_SIZE * sizeof(IWbemClassObject*));
			delete[] m_pObjectArray;
			m_pObjectArray = pTempArray;
			pTempArray = NULL;
			MAX_SIZE *= 2;
		}
		hr = m_pEnumObject->Next(WBEM_INFINITE, 1, &m_pObjectArray[m_nObjectCount], &ulReturn);
		if (ulReturn == 0) break;
		m_nObjectCount++;
	}
	if (m_nObjectCount > 0) m_pRefreshAbleObjectArray = new IWbemClassObject*[m_nObjectCount];
	if (m_pRefreshAbleObjectArray) {
		long lid = 0;
		for (int i = 0; i < m_nObjectCount; i++) {
			m_pRefreshAbleObjectArray[i] = NULL;
			hr = m_pWbemConfigRefresher->AddObjectByTemplate(m_pWbemServices, m_pObjectArray[i], 0, NULL, &(m_pRefreshAbleObjectArray[i]), &lid);
		}
	}
	return true;
}

int CWMICoreManager::GetObjectsCount()
{
	return m_nObjectCount;
}
bool CWMICoreManager::GetStringProperty(int Index, const wchar_t *PropertyName, std::wstring &PropertyValue)
{
	PropertyValue.clear();
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue.assign(vtProp.bstrVal);
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetStringPropertyRefreshed(int Index, const wchar_t *PropertyName, std::wstring &PropertyValue)
{
	PropertyValue.clear();
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pWbemRefresher) return false;
	HRESULT hr = m_pWbemRefresher->Refresh(WBEM_FLAG_REFRESH_AUTO_RECONNECT);
	if (hr != WBEM_S_NO_ERROR) return false;
	if (!m_pRefreshAbleObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	hr = m_pRefreshAbleObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue.assign(vtProp.bstrVal);
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT8Property(int Index, const wchar_t* PropertyName, UINT8 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue = vtProp.bVal;
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT8ArrayProperty(int Index, const wchar_t* PropertyName, std::vector<UINT8> &PropertyValues)
{
	PropertyValues.clear();
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValues.resize(vtProp.parray->rgsabound->cElements);
	for (long i = 0; i < (long)PropertyValues.size(); i++) {
		::SafeArrayGetElement(vtProp.parray, &i, &PropertyValues[i]);
	}
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT16Property(int Index, const wchar_t* PropertyName, UINT16 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue = vtProp.uiVal;
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT16PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT16 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pWbemRefresher) return false;
	HRESULT hr = m_pWbemRefresher->Refresh(WBEM_FLAG_REFRESH_AUTO_RECONNECT);
	if (hr != WBEM_S_NO_ERROR) return false;
	if (!m_pRefreshAbleObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	hr = m_pRefreshAbleObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue = vtProp.uiVal;
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT32Property(int Index, const wchar_t* PropertyName, UINT32 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue = vtProp.uintVal;
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT32PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT32 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pWbemRefresher) return false;
	HRESULT hr = m_pWbemRefresher->Refresh(WBEM_FLAG_REFRESH_AUTO_RECONNECT);
	if (hr != WBEM_S_NO_ERROR) return false;
	if (!m_pRefreshAbleObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	hr = m_pRefreshAbleObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	PropertyValue = vtProp.uintVal;
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT64Property(int Index, const wchar_t* PropertyName, UINT64 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	HRESULT hr = m_pObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	// 64位整数以字符串的方式保存在VARIANT中
	PropertyValue = _wcstoui64(vtProp.bstrVal, L'\0', 0);
	::VariantClear(&vtProp);
	return true;
}
bool CWMICoreManager::GetUINT64PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT64 &PropertyValue)
{
	PropertyValue = 0;
	if (Index < 0 || Index >= m_nObjectCount || !PropertyName) return false;
	if (!m_pWbemRefresher) return false;
	HRESULT hr = m_pWbemRefresher->Refresh(WBEM_FLAG_REFRESH_AUTO_RECONNECT);
	if (hr != WBEM_S_NO_ERROR) return false;
	if (!m_pRefreshAbleObjectArray[Index]) return false;
	VARIANT vtProp = { 0 };
	::VariantInit(&vtProp);
	hr = m_pRefreshAbleObjectArray[Index]->Get(PropertyName, 0, &vtProp, 0, 0);
	if (hr != WBEM_S_NO_ERROR) {
		::VariantClear(&vtProp);
		return false;
	}
	if (vtProp.vt == VT_EMPTY||vtProp.vt == VT_NULL) {
		::VariantClear(&vtProp);
		return false;
	}
	// 64位整数以字符串的方式保存在VARIANT中
	PropertyValue = _wcstoui64(vtProp.bstrVal, L'\0', 0);
	::VariantClear(&vtProp);
	return true;
}

// -----------------------------------------------------------------

CMotherBoardManager::CMotherBoardManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_BaseBoard");
}
CMotherBoardManager::~CMotherBoardManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CMotherBoardManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CMotherBoardManager::GetCaption(int Index, std::wstring &Caption)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Caption", Caption) : false);
}
bool CMotherBoardManager::GetDescription(int Index, std::wstring &Description)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Description", Description) : false);
}
bool CMotherBoardManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CMotherBoardManager::GetSerialNumber(int Index, std::wstring &SerialNumber)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SerialNumber", SerialNumber) : false);
}

// -----------------------------------------------------------------

CBIOSManager::CBIOSManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_BIOS");
}
CBIOSManager::~CBIOSManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CBIOSManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CBIOSManager::GetDescription(int Index, std::wstring &Description)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Description", Description) : false);
}
bool CBIOSManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CBIOSManager::GetSerialNumber(int Index, std::wstring &SerialNumber)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SerialNumber", SerialNumber) : false);
}
bool CBIOSManager::GetVersion(int Index, std::wstring &Version)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Version", Version) : false);
}
bool CBIOSManager::GetSMBIOSVersion(int Index, std::wstring &SMBIOSVersion)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SMBIOSBIOSVersion", SMBIOSVersion) : false);
}

// -----------------------------------------------------------------

CCPUManager::CCPUManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_Processor");
}
CCPUManager::~CCPUManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CCPUManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CCPUManager::GetName(int Index, std::wstring &Name)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Name", Name) : false);
}
bool CCPUManager::GetDescription(int Index, std::wstring &Description)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Description", Description) : false);
}
bool CCPUManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CCPUManager::GetArchitecture(int Index, CCPUManager::Architecture &Architecture)
{
	return (m_pManager ? m_pManager->GetUINT16Property(Index, L"Architecture", (UINT16&)Architecture) : false);
}
bool CCPUManager::GetMaxClockSpeed(int Index, BigLong &MaxClockSpeed)
{
	return (m_pManager ? m_pManager->GetUINT32Property(Index, L"MaxClockSpeed", (UINT32&)MaxClockSpeed) : false);
}
bool CCPUManager::GetCoresNumber(int Index, BigLong &CoresNumber)
{
	return (m_pManager ? m_pManager->GetUINT32Property(Index, L"NumberOfCores", (UINT32&)CoresNumber) : false);
}
bool CCPUManager::GetLogicalCoresNumber(int Index, BigLong &LogicalCoresNumber)
{
	return (m_pManager ? m_pManager->GetUINT32Property(Index, L"NumberOfLogicalProcessors", (UINT32&)LogicalCoresNumber) : false);
}
bool CCPUManager::GetLoadPercentage(int Index, BigLong &LoadPercentage)
{
	return (m_pManager ? m_pManager->GetUINT16Property(Index, L"LoadPercentage", (UINT16&)LoadPercentage) : false);
}

// -----------------------------------------------------------------

CMemoryManager::CMemoryManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_PhysicalMemory");
}
CMemoryManager::~CMemoryManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CMemoryManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CMemoryManager::GetDescription(int Index, std::wstring &Description)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Description", Description) : false);
}
bool CMemoryManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CMemoryManager::GetSerialNumber(int Index, std::wstring &SerialNumber)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SerialNumber", SerialNumber) : false);
}
bool CMemoryManager::GetCapacity(int Index, BigLong &Capacity)
{
	return (m_pManager ? m_pManager->GetUINT64Property(Index, L"Capacity", (UINT64&)Capacity) : false);
}
bool CMemoryManager::GetSpeed(int Index, BigLong &Speed)
{
	return (m_pManager ? m_pManager->GetUINT32Property(Index, L"Speed", (UINT32&)Speed) : false);
}
bool CMemoryManager::GetPartNumber(int Index, std::wstring &PartNumber)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"PartNumber", PartNumber) : false);
}

// -----------------------------------------------------------------

CGPUManager::CGPUManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_VideoController");
}
CGPUManager::~CGPUManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CGPUManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CGPUManager::GetDescription(int Index, std::wstring &Description)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Description", Description) : false);
}
bool CGPUManager::GetRAMCapacity(int Index, BigLong &RAMCapacity)
{
	return (m_pManager ? m_pManager->GetUINT32Property(Index, L"AdapterRAM", (UINT32&)RAMCapacity) : false);
}

// -----------------------------------------------------------------

CDiskManager::CDiskManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_DiskDrive");
}
CDiskManager::~CDiskManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CDiskManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CDiskManager::GetModel(int Index, std::wstring &Model)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Model", Model) : false);
}
bool CDiskManager::GetSerialNumber(int Index, std::wstring &SerialNumber)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SerialNumber", SerialNumber) : false);
}
bool CDiskManager::GetSize(int Index, BigLong &Size)
{
	return (m_pManager ? m_pManager->GetUINT64Property(Index, L"Size", (UINT64&)Size) : false);
}
bool CDiskManager::GetDeviceID(int Index, std::wstring &DeviceID)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"DeviceID", DeviceID) : false);
}
bool CDiskManager::GetPNPDeviceID(int Index, std::wstring &PNPDeviceID)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"PNPDeviceID", PNPDeviceID) : false);
}
bool CDiskManager::GetInterfaceType(int Index, std::wstring &InterfaceType)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"InterfaceType", InterfaceType) : false);
}
bool CDiskManager::GetType(int Index, CDiskManager::Type &Type)
{
	std::wstring DeviceID, InterfaceType;
	if (!GetDeviceID(Index, DeviceID)) return false;
	if (!GetInterfaceType(Index, InterfaceType)) return false;
	// 打开设备句柄
	HANDLE hDiskDrive = ::CreateFileW(DeviceID.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDiskDrive == INVALID_HANDLE_VALUE) return false;
	STORAGE_PROPERTY_QUERY PropertyQuery;
	memset(&PropertyQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
	PropertyQuery.PropertyId = STORAGE_PROPERTY_ID::StorageDeviceProperty;
	PropertyQuery.QueryType = PropertyStandardQuery;
	ULONG nBytes = 0;
	STORAGE_DEVICE_DESCRIPTOR DeviceDescriptor;
	BOOL bRet = ::DeviceIoControl(
		hDiskDrive, 
		IOCTL_STORAGE_QUERY_PROPERTY, 
		&PropertyQuery, 
		sizeof(PropertyQuery), 
		&DeviceDescriptor, 
		sizeof(DeviceDescriptor), 
		&nBytes, 
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDiskDrive);
		return false;
	}
	STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (PSTORAGE_DEVICE_DESCRIPTOR)malloc(nBytes);
	if (!pDeviceDescriptor) {
		::CloseHandle(hDiskDrive);
		return false;
	}
	bRet = ::DeviceIoControl(
		hDiskDrive, 
		IOCTL_STORAGE_QUERY_PROPERTY, 
		&PropertyQuery, 
		sizeof(PropertyQuery), 
		pDeviceDescriptor, 
		nBytes, 
		&nBytes, 
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDiskDrive);
		free(pDeviceDescriptor);
		return false;
	}
	if (pDeviceDescriptor->RemovableMedia == TRUE) {
		Type = REMOVABLE_DISK;
	}
	else if (pDeviceDescriptor->RemovableMedia == FALSE) {
		switch (pDeviceDescriptor->BusType)
		{
		case BusTypeVirtual:
		case BusTypeFileBackedVirtual:
			Type = VIRTUAL_DISK;
			break;
		default:
			if (InterfaceType.find(L"USB") != std::wstring::npos)
				Type = EXTERNAL_USB_DISK;
			else if (InterfaceType.find(L"IDE") != std::wstring::npos ||
				InterfaceType.find(L"SCSI") != std::wstring::npos)
				Type = FIXED_DISK;
			else
				Type = UNKNOWN_DISK;
			break;
		}
	}
	::CloseHandle(hDiskDrive);
	free(pDeviceDescriptor);
	return true;
}
bool CDiskManager::GetLogicalName(int Index, std::wstring &LogicalName)
{
	std::wstring DeviceID, PartitionQuery;
	if (!GetDeviceID(Index, DeviceID)) return false;
    PartitionQuery = L"ASSOCIATORS OF {Win32_DiskDrive.DeviceID='";
    PartitionQuery += DeviceID;
    PartitionQuery += L"'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
	CWMICoreManager DiskPartitionManager(NAMESPACE_ROOT_CIMV2, PartitionQuery.c_str());
	LogicalName.clear();
	for (int i = 0; i < DiskPartitionManager.GetObjectsCount(); i++) {
		std::wstring PartitionDeviceID;
		if (!DiskPartitionManager.GetStringProperty(i, L"DeviceID", PartitionDeviceID)) continue;
		std::wstring LogicalDiskQuery;
		LogicalDiskQuery = L"ASSOCIATORS OF {Win32_DiskPartition.DeviceID='";
		LogicalDiskQuery += PartitionDeviceID;
		LogicalDiskQuery += L"'} WHERE AssocClass = Win32_LogicalDiskToPartition";
		CWMICoreManager LogicalDiskManager(NAMESPACE_ROOT_CIMV2, LogicalDiskQuery.c_str());
		for (int j = 0; j < LogicalDiskManager.GetObjectsCount(); j++) {
			std::wstring Name;
			if (!LogicalDiskManager.GetStringProperty(j, L"Name", Name)) continue;
			LogicalName += Name;
			LogicalName += L";";
		}
	}
	// 删除最后一个分号
	if (!LogicalName.empty()) {
		LogicalName.erase(LogicalName.size() - 1, 1);
	}
	return true;
}

// -----------------------------------------------------------------

CBatteryManager::CBatteryManager()
{
	for (int i = 0; i < Max; i++) {
		m_pManager[i] = NULL;
	}
	m_pManager[Basic] = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_Battery");
	m_pManager[StaticData] = new CWMICoreManager(NAMESPACE_ROOT_WMI, L"SELECT * FROM BatteryStaticData");
	m_pManager[FullCapacity] = new CWMICoreManager(NAMESPACE_ROOT_WMI, L"SELECT * FROM BatteryFullChargedCapacity");
}
CBatteryManager::~CBatteryManager()
{
	for (int i = 0; i < Max; i++) {
		if (m_pManager[i]) {
			delete m_pManager[i];
			m_pManager[i] = NULL;
		}
	}
}

int CBatteryManager::GetCount()
{
	return (m_pManager[Basic] ? m_pManager[Basic]->GetObjectsCount() : 0);
}
bool CBatteryManager::GetName(int Index, std::wstring &Name)
{
	return (m_pManager[Basic] ? m_pManager[Basic]->GetStringProperty(Index, L"Name", Name) : 0);

}
bool CBatteryManager::GetDeviceID(int Index, std::wstring &DeviceID)
{
	return (m_pManager[Basic] ? m_pManager[Basic]->GetStringProperty(Index, L"DeviceID", DeviceID) : 0);
}
bool CBatteryManager::GetDesignedVoltage(int Index, BigLong &Voltage)
{
	return (m_pManager[Basic] ? m_pManager[Basic]->GetUINT64Property(Index, L"DesignVoltage", (UINT64&)Voltage) : 0);
}

bool CBatteryManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager[StaticData] ? m_pManager[StaticData]->GetStringProperty(Index, L"ManufactureName", Manufacturer) : 0);
}
bool CBatteryManager::GetUniqueID(int Index, std::wstring &UniqueID)
{
	return (m_pManager[StaticData] ? m_pManager[StaticData]->GetStringProperty(Index, L"UniqueID", UniqueID) : 0);
}
bool CBatteryManager::GetSerialNumber(int Index, std::wstring &SerialNumber)
{
	return (m_pManager[StaticData] ? m_pManager[StaticData]->GetStringProperty(Index, L"SerialNumber", SerialNumber) : 0);
}
bool CBatteryManager::GetDesignedCapacity(int Index, BigLong &Capacity)
{
	return (m_pManager[StaticData] ? m_pManager[StaticData]->GetUINT32Property(Index, L"DesignedCapacity", (UINT32&)Capacity) : 0);
}

bool CBatteryManager::GetFullChargedCapacity(int Index, BigLong &Capacity)
{
	return (m_pManager[FullCapacity] ? m_pManager[FullCapacity]->GetUINT32Property(Index, L"FullChargedCapacity", (UINT32&)Capacity) : 0);
}

// -----------------------------------------------------------------

CNetworkCardManager::CNetworkCardManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_NetworkAdapter Where PhysicalAdapter = true");
}
CNetworkCardManager::~CNetworkCardManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CNetworkCardManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CNetworkCardManager::GetName(int Index, std::wstring &Name)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Name", Name) : false);
}
bool CNetworkCardManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CNetworkCardManager::GetMAC(int Index, std::wstring &MAC)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"MACAddress", MAC) : false);
}
bool CNetworkCardManager::GetConnectionID(int Index, std::wstring &ConnectionID)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"NetConnectionID", ConnectionID) : false);
}
bool CNetworkCardManager::GetGUID(int Index, std::wstring &GUID)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"GUID", GUID) : false);
}
bool CNetworkCardManager::GetPNPDeviceID(int Index, std::wstring &PNPDeviceID)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"PNPDeviceID", PNPDeviceID) : false);
}

// -----------------------------------------------------------------

CCDManager::CCDManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_CDROMDrive");
}
CCDManager::~CCDManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CCDManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CCDManager::GetName(int Index, std::wstring &Name)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Name", Name) : false);
}

// -----------------------------------------------------------------

CComputerManager::CComputerManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_ComputerSystem");
}
CComputerManager::~CComputerManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CComputerManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CComputerManager::GetModel(int Index, std::wstring &Model)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Model", Model) : false);
}
bool CComputerManager::GetManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Manufacturer", Manufacturer) : false);
}
bool CComputerManager::GetPCType(int Index, Type &PCType)
{
	UINT16 u16Type1 = 0, u16Type2 = 0;
	bool ret1 = (m_pManager ? m_pManager->GetUINT16Property(Index, L"PCSystemType", u16Type1) : false);
	bool ret2 = (m_pManager ? m_pManager->GetUINT16Property(Index, L"PCSystemTypeEx", u16Type2) : false);
	PCType = CComputerManager::UNKNOWN;
	if (ret1) {
		if (1 == u16Type1)
			PCType = CComputerManager::DESKTOP;
		else if (2 == u16Type1)
			PCType = CComputerManager::NOTEBOOK;
	}
	if (ret2) {
		if (8 == u16Type2)
			PCType = CComputerManager::TABLET;
	}
	return (ret1 || ret2);
}
bool CComputerManager::GetType(int Index, std::wstring &Type)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemType", Type) : false);
}

// -----------------------------------------------------------------

COSManager::COSManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_OperatingSystem");
}
COSManager::~COSManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int COSManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool COSManager::GetCaption(int Index, std::wstring &Caption)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Caption", Caption) : false);
}
bool COSManager::GetArchitecture(int Index, std::wstring &Architecture)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"OSArchitecture", Architecture) : false);
}
bool COSManager::GetVersion(int Index, std::wstring &Version)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"Version", Version) : false);
}
bool COSManager::GetSystemDrive(int Index, std::wstring &SystemDrive)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemDrive", SystemDrive) : false);
}

// -----------------------------------------------------------------

CInformationManager::CInformationManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_WMI, L"SELECT * FROM MS_SystemInformation");
}
CInformationManager::~CInformationManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CInformationManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CInformationManager::GetMotherBoardManufacturer(int Index, std::wstring &Manufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"BaseBoardManufacturer", Manufacturer) : false);
}
bool CInformationManager::GetMotherBoardProductName(int Index, std::wstring &ProductName)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"BaseBoardProduct", ProductName) : false);
}
bool CInformationManager::GetBIOSReleaseDate(int Index, std::wstring &BIOSReleaseDate)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"BIOSReleaseDate", BIOSReleaseDate) : false);
}
bool CInformationManager::GetBIOSVendor(int Index, std::wstring &BIOSVendor)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"BIOSVendor", BIOSVendor) : false);
}
bool CInformationManager::GetBIOSVersion(int Index, std::wstring &BIOSVersion)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"BIOSVersion", BIOSVersion) : false);
}
bool CInformationManager::GetSystemFamily(int Index, std::wstring &SystemFamily)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemFamily", SystemFamily) : false);
}
bool CInformationManager::GetSystemManufacturer(int Index, std::wstring &SystemManufacturer)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemManufacturer", SystemManufacturer) : false);
}
bool CInformationManager::GetSystemProductName(int Index, std::wstring &SystemProductName)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemProductName", SystemProductName) : false);
}
bool CInformationManager::GetSystemSKU(int Index, std::wstring &SystemSKU)
{
	return (m_pManager ? m_pManager->GetStringProperty(Index, L"SystemSKU", SystemSKU) : false);
}

// -----------------------------------------------------------------

CRawSMBiosTablesManager::CRawSMBiosTablesManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_WMI, L"SELECT * FROM MSSmBios_RawSMBiosTables");
}
CRawSMBiosTablesManager::~CRawSMBiosTablesManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CRawSMBiosTablesManager::GetCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CRawSMBiosTablesManager::GetMajorVersion(int Index, UINT8 &MajorVersion)
{
	return (m_pManager ? m_pManager->GetUINT8Property(Index, L"SmbiosMajorVersion", MajorVersion) : false);
}
bool CRawSMBiosTablesManager::GetMinorVersion(int Index, UINT8 &MinorVersion)
{
	return (m_pManager ? m_pManager->GetUINT8Property(Index, L"SmbiosMinorVersion", MinorVersion) : false);
}
bool CRawSMBiosTablesManager::GetData(int Index, std::vector<UINT8> &Data)
{
	return (m_pManager ? m_pManager->GetUINT8ArrayProperty(Index, L"SMBiosData", Data) : false);
}

// -----------------------------------------------------------------

CPerfRawDataManager::CPerfRawDataManager() :
	m_pManager(NULL)
{
	m_pManager = new CWMICoreManager(NAMESPACE_ROOT_CIMV2, L"SELECT * FROM Win32_PerfRawData_PerfOS_Memory");
}
CPerfRawDataManager::~CPerfRawDataManager()
{
	if (m_pManager) {
		delete m_pManager;
		m_pManager = NULL;
	}
}

int CPerfRawDataManager::GetMemoryPerfDataCount()
{
	return (m_pManager ? m_pManager->GetObjectsCount() : 0);
}
bool CPerfRawDataManager::GetMemoryAvailableBytes(int Index, BigLong &Bytes)
{
	return (m_pManager ? m_pManager->GetUINT64Property(Index, L"AvailableMBytes", (UINT64&)Bytes) : false);
}
bool CPerfRawDataManager::GetMemoryUnusedBytes(int Index, BigLong &Bytes)
{
	return (m_pManager ? m_pManager->GetUINT64Property(Index, L"FreeAndZeroPageListBytes", (UINT64&)Bytes) : false);
}

HardwareManagerNamespaceEnd
