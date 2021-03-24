#pragma once

#include <wlanapi.h>
#include <string>
#include <vector>

HardwareManagerNamespaceBegin

typedef struct tagWLANInfo 
{
    std::string SSID;// ��������
    bool IsCurrentConnected;// �Ƿ�Ϊ��ǰ���ӵ�����
} WLANInfo;

typedef struct tagWLANProfileInfo 
{
	std::string SSID;// ����SSID
	std::string Key;// ������Կ
	DOT11_AUTH_ALGORITHM AuthAlgo;// ��֤�㷨
	DOT11_CIPHER_ALGORITHM CipherAlgo;// �����㷨
} WLANProfileInfo;

class CWIFI
{
public:
	CWIFI();
	virtual ~CWIFI();

private:
	bool m_bInitSuccess;// ��ʶ�Ƿ��Ѿ���ʼ���ɹ�
	bool m_bWLANExist;// ��ʶWLAN�Ƿ����
	HANDLE m_hWLAN;// �����������
	GUID m_GUID;// ���������ӿ�GUID
	std::wstring m_Description;// ���������ӿ�����

private:
	bool Create();
	void Release();

public:
	bool IsExist();
	bool GetDescription(std::wstring &Description);
	bool GetGUID(GUID &GUID);
	bool IsHardwareEnabled();
	bool IsSoftwareEnabled();
	bool SetSoftwareEnabled(bool Enabled);
	bool IsConnected(std::string *ssid = NULL);
	bool Disconnect();
	bool Connect(const std::string &ssid, std::string *key = NULL);
	bool GetAvailableNetworkList(std::vector<WLANInfo> &List);
	void CreateWLANProfile(const WLANProfileInfo &Profile, std::wstring &ProfileString);
};

HardwareManagerNamespaceEnd
