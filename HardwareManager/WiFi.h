#pragma once

#include <wlanapi.h>
#include <string>
#include <vector>

HardwareManagerNamespaceBegin

typedef struct tagWLANInfo 
{
    std::string SSID;// 网络名称
    bool IsCurrentConnected;// 是否为当前连接的网络
} WLANInfo;

typedef struct tagWLANProfileInfo 
{
	std::string SSID;// 网络SSID
	std::string Key;// 网络密钥
	DOT11_AUTH_ALGORITHM AuthAlgo;// 认证算法
	DOT11_CIPHER_ALGORITHM CipherAlgo;// 加密算法
} WLANProfileInfo;

class CWIFI
{
public:
	CWIFI();
	virtual ~CWIFI();

private:
	bool m_bInitSuccess;// 标识是否已经初始化成功
	bool m_bWLANExist;// 标识WLAN是否存在
	HANDLE m_hWLAN;// 无线网卡句柄
	GUID m_GUID;// 无线网卡接口GUID
	std::wstring m_Description;// 无线网卡接口描述

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
