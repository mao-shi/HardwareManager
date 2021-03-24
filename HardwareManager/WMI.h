#pragma once

#include <string>
#include <vector>

struct IWbemLocator;
struct IWbemServices;
struct IEnumWbemClassObject;
struct IWbemClassObject;
struct IWbemRefresher;
struct IWbemConfigureRefresher;
typedef unsigned long long BigLong;

HardwareManagerNamespaceBegin

class CWMICoreManager
{
public:
	CWMICoreManager(const wchar_t *Namespace, const wchar_t *Query);
	virtual ~CWMICoreManager();

private:
	IWbemLocator *m_pWbemLocator;
	IWbemServices *m_pWbemServices;
	IWbemRefresher *m_pWbemRefresher;
	IWbemConfigureRefresher *m_pWbemConfigRefresher;
	IEnumWbemClassObject *m_pEnumObject;
	IWbemClassObject **m_pObjectArray;
	int m_nObjectCount;
	IWbemClassObject **m_pRefreshAbleObjectArray;

private:
	void CleanupCreated();
	void CleanupQueried();
	bool Create(const wchar_t *Namespace);
	bool WQLQuery(const wchar_t *Query);

public:
	int GetObjectsCount();
	bool GetStringProperty(int Index, const wchar_t *PropertyName, std::wstring &PropertyValue);
	bool GetStringPropertyRefreshed(int Index, const wchar_t *PropertyName, std::wstring &PropertyValue);
	bool GetUINT8Property(int Index, const wchar_t* PropertyName, UINT8 &PropertyValue);
	bool GetUINT8ArrayProperty(int Index, const wchar_t* PropertyName, std::vector<UINT8> &PropertyValues);
	bool GetUINT16Property(int Index, const wchar_t* PropertyName, UINT16 &PropertyValue);
	bool GetUINT16PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT16 &PropertyValue);
	bool GetUINT32Property(int Index, const wchar_t* PropertyName, UINT32 &PropertyValue);
	bool GetUINT32PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT32 &PropertyValue);
	bool GetUINT64Property(int Index, const wchar_t* PropertyName, UINT64 &PropertyValue);
	bool GetUINT64PropertyRefreshed(int Index, const wchar_t* PropertyName, UINT64 &PropertyValue);
};

class CMotherBoardManager
{
public:
	CMotherBoardManager();
	virtual ~CMotherBoardManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CMotherBoardManager(const CMotherBoardManager&) {}
	CMotherBoardManager& operator = (const CMotherBoardManager&) { return *this; }

public:
	int GetCount();
	bool GetCaption(int Index, std::wstring &Caption);
	bool GetDescription(int Index, std::wstring &Description);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetSerialNumber(int Index, std::wstring &SerialNumber);
};

class CBIOSManager
{
public:
	CBIOSManager();
	virtual ~CBIOSManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CBIOSManager(const CBIOSManager&) {}
	CBIOSManager& operator = (const CBIOSManager&) { return *this; }
		
public:
	int GetCount();
	bool GetDescription(int Index, std::wstring &Description);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetSerialNumber(int Index, std::wstring &SerialNumber);
	bool GetVersion(int Index, std::wstring &Version);
	bool GetSMBIOSVersion(int Index, std::wstring &SMBIOSVersion);
};

class CCPUManager
{
public:
	CCPUManager();
	virtual ~CCPUManager();

private:
	CWMICoreManager *m_pManager;

public:
	enum Architecture {
		X86 = 0,
		MIPS = 1,
		ALPHA = 2,
		POWER_PC = 3,
		ARM = 5,
		IA64 = 6,
		X64 = 9
	};
	
private:
	CCPUManager(const CCPUManager&) {}
	CCPUManager& operator = (const CCPUManager&) { return *this; }

public:
	int GetCount();
	bool GetName(int Index, std::wstring &Name);
	bool GetDescription(int Index, std::wstring &Description);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetArchitecture(int Index, Architecture &Architecture);
	bool GetMaxClockSpeed(int Index, BigLong &MaxClockSpeed);
	bool GetCoresNumber(int Index, BigLong &CoresNumber);
	bool GetLogicalCoresNumber(int Index, BigLong &LogicalCoresNumber);
	bool GetLoadPercentage(int Index, BigLong &LoadPercentage);
};

class CMemoryManager
{
public:
	CMemoryManager();
	virtual ~CMemoryManager();

private:
	CWMICoreManager *m_pManager;

private:
	CMemoryManager(const CMemoryManager&) {}
	CMemoryManager& operator = (const CMemoryManager&) { return *this; }

public:
	int GetCount();
	bool GetDescription(int Index, std::wstring &Description);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetSerialNumber(int Index, std::wstring &SerialNumber);
	bool GetCapacity(int Index, BigLong &Capacity);
	bool GetSpeed(int Index, BigLong &Speed);
	bool GetPartNumber(int Index, std::wstring &PartNumber);
};

class CGPUManager
{
public:
	CGPUManager();
	virtual ~CGPUManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CGPUManager(const CGPUManager&) {}
	CGPUManager& operator = (const CGPUManager&) { return *this; }

public:
	int GetCount();
	bool GetDescription(int Index, std::wstring &Description);
	bool GetRAMCapacity(int Index, BigLong &RAMCapacity);
};

class CDiskManager
{
public:
	CDiskManager();
	virtual ~CDiskManager();

private:
	CWMICoreManager *m_pManager;
	
public:
	enum Type {
		UNKNOWN_DISK = 0, // 未知类型
		FIXED_DISK = 1, // 固定磁盘(如本地硬盘)
		EXTERNAL_USB_DISK = 2, // 扩展磁盘(如USB移动硬盘)
		VIRTUAL_DISK = 3, // 虚拟硬盘(如VHD)
		REMOVABLE_DISK = 4, // 可移动盘
	};
	
private:
	CDiskManager(const CDiskManager&) {}
	CDiskManager& operator = (const CDiskManager&) { return *this; }

public:
	int GetCount();
	bool GetModel(int Index, std::wstring &Model);
	bool GetSerialNumber(int Index, std::wstring &SerialNumber);
	bool GetSize(int Index, BigLong &Size);
	bool GetDeviceID(int Index, std::wstring &DeviceID);
	bool GetPNPDeviceID(int Index, std::wstring &PNPDeviceID);
	bool GetInterfaceType(int Index, std::wstring &InterfaceType);
	bool GetType(int Index, Type &Type);
	bool GetLogicalName(int Index, std::wstring &LogicalName);
};

class CBatteryManager
{
public:
	CBatteryManager();
	virtual ~CBatteryManager();

private:
	enum {
		Basic = 0,
		StaticData,
		FullCapacity,
		Max
	};
	CWMICoreManager *m_pManager[Max];
	
private:
	CBatteryManager(const CBatteryManager&) {}
	CBatteryManager& operator = (const CBatteryManager&) { return *this; }

public:
	int GetCount();
	bool GetName(int Index, std::wstring &Name);
	bool GetDeviceID(int Index, std::wstring &DeviceID);
	bool GetDesignedVoltage(int Index, BigLong &Voltage);

public:
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetUniqueID(int Index, std::wstring &UniqueID);
	bool GetSerialNumber(int Index, std::wstring &SerialNumber);
	bool GetDesignedCapacity(int Index, BigLong &Capacity);

public:
	bool GetFullChargedCapacity(int Index, BigLong &Capacity);
};

class CNetworkCardManager
{
public:
	CNetworkCardManager();
	virtual ~CNetworkCardManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CNetworkCardManager(const CNetworkCardManager&) {}
	CNetworkCardManager& operator = (const CNetworkCardManager&) { return *this; }

public:
	int GetCount();
	bool GetName(int Index, std::wstring &Name);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetMAC(int Index, std::wstring &MAC);
	bool GetConnectionID(int Index, std::wstring &ConnectionID);
	bool GetGUID(int Index, std::wstring &GUID);
	bool GetPNPDeviceID(int Index, std::wstring &PNPDeviceID);
};

class CCDManager
{
public:
	CCDManager();
	virtual ~CCDManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CCDManager(const CCDManager&) {}
	CCDManager& operator = (const CCDManager&) { return *this; }

public:
	int GetCount();
	bool GetName(int Index, std::wstring &Name);
};

class CComputerManager
{
public:
	CComputerManager();
	virtual ~CComputerManager();

private:
	CWMICoreManager *m_pManager;

public:
	enum Type {
		UNKNOWN = 0, ///< 未知
		DESKTOP = 1, ///< 台式机
		NOTEBOOK = 2, ///< 笔记本
		TABLET = 3 ///< 平板电脑
	};
	
private:
	CComputerManager(const CComputerManager&) {}
	CComputerManager& operator = (const CComputerManager&) { return *this; }

public:
	int GetCount();
	bool GetModel(int Index, std::wstring &Model);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetPCType(int Index, Type &PCType);
	bool GetType(int Index, std::wstring &Type);
};

class COSManager
{
public:
	COSManager();
	virtual ~COSManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	COSManager(const COSManager&) {}
	COSManager& operator = (const COSManager&) { return *this; }

public:
	int GetCount();
	bool GetCaption(int Index, std::wstring &Caption);
	bool GetArchitecture(int Index, std::wstring &Architecture);
	bool GetVersion(int Index, std::wstring &Version);
	bool GetSystemDrive(int Index, std::wstring &SystemDrive);
};

class CInformationManager
{
public:
	CInformationManager();
	virtual ~CInformationManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CInformationManager(const CInformationManager&) {}
	CInformationManager& operator = (const CInformationManager&) { return *this; }

public:
	int GetCount();
	bool GetMotherBoardManufacturer(int Index, std::wstring &Manufacturer);
	bool GetMotherBoardProductName(int Index, std::wstring &ProductName);
	bool GetBIOSReleaseDate(int Index, std::wstring &BIOSReleaseDate);
	bool GetBIOSVendor(int Index, std::wstring &BIOSVendor);
	bool GetBIOSVersion(int Index, std::wstring &BIOSVersion);
	bool GetSystemFamily(int Index, std::wstring &SystemFamily);
	bool GetSystemManufacturer(int Index, std::wstring &SystemManufacturer);
	bool GetSystemProductName(int Index, std::wstring &SystemProductName);
	bool GetSystemSKU(int Index, std::wstring &SystemSKU);
};

class CRawSMBiosTablesManager
{
public:
	CRawSMBiosTablesManager();
	virtual ~CRawSMBiosTablesManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CRawSMBiosTablesManager(const CRawSMBiosTablesManager&) {}
	CRawSMBiosTablesManager& operator = (const CRawSMBiosTablesManager&) { return *this; }

public:
	int GetCount();
	bool GetMajorVersion(int Index, UINT8 &MajorVersion);
	bool GetMinorVersion(int Index, UINT8 &MinorVersion);
	bool GetData(int Index, std::vector<UINT8> &Data);
};

class CPerfRawDataManager
{
public:
	CPerfRawDataManager();
	virtual ~CPerfRawDataManager();

private:
	CWMICoreManager *m_pManager;
	
private:
	CPerfRawDataManager(const CPerfRawDataManager&) {}
	CPerfRawDataManager& operator = (const CPerfRawDataManager&) { return *this; }

public:
	int GetMemoryPerfDataCount();
	bool GetMemoryAvailableBytes(int Index, BigLong &Bytes);
	bool GetMemoryUnusedBytes(int Index, BigLong &Bytes);
};

HardwareManagerNamespaceEnd
