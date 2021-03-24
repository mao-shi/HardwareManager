#pragma once

#include <vector>
#include <algorithm>
#include <SetupAPI.h>
#include <devguid.h>

HardwareManagerNamespaceBegin

class CDevice
{
public:
	CDevice();
	virtual ~CDevice();

private:
	HDEVINFO m_hDevInfoSet; // �豸��Ϣ��
	int m_nDevCount;// �豸��Ŀ
	SP_DEVINFO_DATA *m_pDevInfoList;// �豸��Ϣ�б� 

private:
	void Cleanup();
	bool GetRegistryPropertyStr(int Index, DWORD dwProperty, std::wstring &strProperty);
	bool GetRegistryPropertyUInt(int Index, DWORD dwProperty, unsigned int &PropertyValue);
	bool GetPropertyStr(int Index, const DEVPROPKEY *pPropertyKey, std::wstring &strProperty);
	bool GetPropertyStrList(int Index, const DEVPROPKEY *pPropertyKey, std::vector<std::wstring> &strPropertyList);
	bool ChangeState(int Index, DWORD NewState);

public:
	bool Create(const GUID *GUID);
	int GetCount();
	bool SetEnabled(int Index, bool Enabled);
	bool GetDescription(int Index, std::wstring &Description);
	bool GetHardwareID(int Index, std::wstring &ID);
	bool GetFriendlyName(int Index, std::wstring &FriendlyName);
	bool GetLoctionInfo(int Index, std::wstring &LoctionInfo);
	bool GetInstanceID(int Index, std::wstring &InstanceID);
	bool GetParentInstanceID(int Index, std::wstring &InstanceID);
	bool GetChildren(int Index, std::vector<std::wstring> Children);
	bool GetDriverKeyName(int Index, std::wstring &Name);
	bool GetClass(int Index, std::wstring &Class);
	bool GetClassGUID(int Index, std::wstring &ClassGUID);
	bool GetBusNumber(int Index, unsigned int &BusNumber);
	bool GetManufacturer(int Index, std::wstring &Manufacturer);
	bool GetMatchingDeviceID(int Index, std::wstring &ID);

};

#pragma pack (1)
typedef struct tagMonitorEDID
{
	unsigned char HeadInfor[8];// ͷ��Ϣ, 8���ֽ�
	unsigned char VendorID[2];// ����ID
	unsigned char ProductID[2];// ��ƷID
	unsigned char SerialNumber[4];// ���к�
	unsigned char Date[2];// ��������
	unsigned char EDIDVersion[2];// EDID�汾
	unsigned char BasicInfor[5];// ��ʾ��������Ϣ(��Դ, ���߶�, ���)
	unsigned char ColorFeature[10];// ��ʾ����ɫ����
	unsigned char OtherInfor[93];// ������Ϣ
} MonitorEDID;
#pragma pack()

class CDeviceMonitor : public CDevice
{
public:
	CDeviceMonitor();
	virtual ~CDeviceMonitor();

public:
	bool GetEDID(int Index, MonitorEDID &EDID);
};

HardwareManagerNamespaceEnd
