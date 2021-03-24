#include "HardwareManager.h"

#pragma comment(lib, "Wlanapi.lib")
#pragma comment(lib, "Rpcrt4.lib")

HardwareManagerNamespaceBegin

CWIFI::CWIFI() :
	m_bInitSuccess(false),
	m_bWLANExist(false),
	m_hWLAN(NULL)
{
	Create();
}
CWIFI::~CWIFI()
{
	Release();
}

bool CWIFI::Create()
{
	if (m_bInitSuccess) return false;
	DWORD Version = 0;
	DWORD dwRet = ::WlanOpenHandle(2, NULL, &Version, &m_hWLAN);
	if (dwRet != ERROR_SUCCESS) return false;
	PWLAN_INTERFACE_INFO_LIST pWLANInterfaceList = NULL;
	dwRet = ::WlanEnumInterfaces(m_hWLAN, NULL, &pWLANInterfaceList);
	if (dwRet != ERROR_SUCCESS) {
		::WlanCloseHandle(m_hWLAN, NULL);
		m_hWLAN = NULL;
		return false;
	}
	if (pWLANInterfaceList->dwNumberOfItems > 0) {
		m_GUID = pWLANInterfaceList->InterfaceInfo[0].InterfaceGuid;
		m_Description = pWLANInterfaceList->InterfaceInfo[0].strInterfaceDescription;
		m_bWLANExist = true;
	}
	else m_bWLANExist = false;
	m_bInitSuccess = true;
	if (pWLANInterfaceList != NULL) ::WlanFreeMemory(pWLANInterfaceList);
	return true;
}
void CWIFI::Release()
{
	if (!m_bInitSuccess) return;
	if (m_hWLAN != NULL) {
		::WlanCloseHandle(m_hWLAN, NULL);
		m_hWLAN = NULL;
	}
	m_bInitSuccess = false;
}

bool CWIFI::IsExist()
{
	return (m_bInitSuccess && m_bWLANExist);
}
bool CWIFI::GetDescription(std::wstring &Description)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	Description = m_Description;
	return true;
}
bool CWIFI::GetGUID(GUID &GUID)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	GUID = m_GUID;
	return true;
}
bool CWIFI::IsHardwareEnabled()
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	PWLAN_INTERFACE_INFO_LIST pWLANInterfaceList = NULL;
	DWORD dwRet = ::WlanEnumInterfaces(m_hWLAN, NULL, &pWLANInterfaceList);
	if (dwRet != ERROR_SUCCESS) return false;
	if (pWLANInterfaceList->dwNumberOfItems < 1) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	DWORD dwDataSize = sizeof(WLAN_RADIO_STATE);
    PWLAN_RADIO_STATE pWLANRadioState = NULL;
	dwRet = ::WlanQueryInterface(
		m_hWLAN, 
		&(pWLANInterfaceList->InterfaceInfo[0].InterfaceGuid),
		wlan_intf_opcode_radio_state,
		NULL,
		&dwDataSize,
		(PVOID*)&pWLANRadioState,
		NULL);
	if (dwRet != ERROR_SUCCESS) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	if (pWLANRadioState->PhyRadioState[0].dot11HardwareRadioState != dot11_radio_state_on) {
		::WlanFreeMemory(pWLANRadioState);
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	::WlanFreeMemory(pWLANRadioState);
	::WlanFreeMemory(pWLANInterfaceList);
	return true;
}
bool CWIFI::IsSoftwareEnabled()
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	PWLAN_INTERFACE_INFO_LIST pWLANInterfaceList = NULL;
	DWORD dwRet = ::WlanEnumInterfaces(m_hWLAN, NULL, &pWLANInterfaceList);
	if (dwRet != ERROR_SUCCESS) return false;
	if (pWLANInterfaceList->dwNumberOfItems < 1) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	DWORD dwDataSize = sizeof(WLAN_RADIO_STATE);
    PWLAN_RADIO_STATE pWLANRadioState = NULL;
	dwRet = ::WlanQueryInterface(
		m_hWLAN, 
		&(pWLANInterfaceList->InterfaceInfo[0].InterfaceGuid),
		wlan_intf_opcode_radio_state,
		NULL,
		&dwDataSize,
		(PVOID*)&pWLANRadioState,
		NULL);
	if (dwRet != ERROR_SUCCESS) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	if (pWLANRadioState->PhyRadioState[0].dot11SoftwareRadioState != dot11_radio_state_on) {
		::WlanFreeMemory(pWLANRadioState);
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	::WlanFreeMemory(pWLANRadioState);
	::WlanFreeMemory(pWLANInterfaceList);
	return true;
}
bool CWIFI::SetSoftwareEnabled(bool Enabled)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	PWLAN_INTERFACE_INFO_LIST pWLANInterfaceList = NULL;
	DWORD dwRet = ::WlanEnumInterfaces(m_hWLAN, NULL, &pWLANInterfaceList);
	if (dwRet != ERROR_SUCCESS) return false;
	if (pWLANInterfaceList->dwNumberOfItems < 1) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	WLAN_PHY_RADIO_STATE RadioState = { 0 };
    RadioState.dwPhyIndex = 0;
    RadioState.dot11SoftwareRadioState = (Enabled ? dot11_radio_state_on : dot11_radio_state_off);
	dwRet = ::WlanSetInterface(
		m_hWLAN,
		&(pWLANInterfaceList->InterfaceInfo[0].InterfaceGuid),
		wlan_intf_opcode_radio_state,
		sizeof(WLAN_PHY_RADIO_STATE),
		(PVOID)&RadioState,
		NULL);
	if (dwRet != ERROR_SUCCESS) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	::WlanFreeMemory(pWLANInterfaceList);
	return true;
}
bool CWIFI::IsConnected(std::string *ssid)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	PWLAN_INTERFACE_INFO_LIST pWLANInterfaceList = NULL;
	DWORD dwRet = ::WlanEnumInterfaces(m_hWLAN, NULL, &pWLANInterfaceList);
	if (dwRet != ERROR_SUCCESS) return false;
	if (pWLANInterfaceList->dwNumberOfItems < 1) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	if (pWLANInterfaceList->InterfaceInfo[0].isState != wlan_interface_state_connected) {
		::WlanFreeMemory(pWLANInterfaceList);
		return false;
	}
	::WlanFreeMemory(pWLANInterfaceList);
	if (ssid) {
		std::vector<WLANInfo> List;
		GetAvailableNetworkList(List);
		for (std::vector<WLANInfo>::iterator itor = List.begin(); itor != List.end(); itor++) {
			if (itor->IsCurrentConnected) {
				*ssid = itor->SSID;
			}
		}
	}
	return true;
}
bool CWIFI::Disconnect()
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	DWORD dwRet = ::WlanDisconnect(m_hWLAN, &m_GUID, NULL);
	return (dwRet != ERROR_SUCCESS);
}
bool CWIFI::Connect(const std::string &ssid, std::string *key)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	// 获取网络列表
	PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = NULL;
	DWORD dwRet = ::WlanGetAvailableNetworkList(m_hWLAN, &m_GUID, 
	WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES, NULL, &pNetworkList);
	if (dwRet != ERROR_SUCCESS) return false;
	// 搜索指定网络
	int DestNetworkIndex = -1;
	for (DWORD i = 0; i < pNetworkList->dwNumberOfItems; i++) {
		if (ssid.compare((char*)(pNetworkList->Network[i].dot11Ssid.ucSSID)) == 0) {
			DestNetworkIndex = (int)i;
			break;
		}
	}
	// 没有找到指定的网络
	if (DestNetworkIndex == -1) {
		::WlanFreeMemory(pNetworkList);
		return false;
	}
	// 指定网络已经被连接
	if ((pNetworkList->Network[DestNetworkIndex].dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) == WLAN_AVAILABLE_NETWORK_CONNECTED) {
		::WlanFreeMemory(pNetworkList);
		return true;
	}
	if (key) {
		WLAN_REASON_CODE ReasonCode = 0;
		WLANProfileInfo Profile;
		std::wstring ProfileStr;
		// 创建网络配置文件
		Profile.SSID = ssid;
		Profile.Key = *key;
		Profile.AuthAlgo = pNetworkList->Network[DestNetworkIndex].dot11DefaultAuthAlgorithm;
		Profile.CipherAlgo = pNetworkList->Network[DestNetworkIndex].dot11DefaultCipherAlgorithm;
		CreateWLANProfile(Profile, ProfileStr);
		// 设置网络配置文件
		dwRet = ::WlanSetProfile(m_hWLAN, &(m_GUID), 0, ProfileStr.c_str(), NULL, TRUE, NULL, &ReasonCode);
		if (dwRet != ERROR_SUCCESS) {
			::WlanFreeMemory(pNetworkList);
			return false;
		}
	}
	WLAN_CONNECTION_PARAMETERS ConnParam;
	ConnParam.wlanConnectionMode = wlan_connection_mode_profile;
	ConnParam.strProfile =pNetworkList->Network[DestNetworkIndex].strProfileName;								
	ConnParam.pDot11Ssid = NULL;
	ConnParam.pDesiredBssidList = NULL;
	ConnParam.dwFlags = 0;
	ConnParam.dot11BssType = dot11_BSS_type_infrastructure;
	dwRet = ::WlanConnect(m_hWLAN, &(m_GUID), &ConnParam, NULL);
	::WlanFreeMemory(pNetworkList);
	return (dwRet == ERROR_SUCCESS);
}
bool CWIFI::GetAvailableNetworkList(std::vector<WLANInfo> &List)
{
	if (!m_bInitSuccess || !m_bWLANExist) return false;
	PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = NULL;
	DWORD dwRet = WlanGetAvailableNetworkList(m_hWLAN, &m_GUID, WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES, NULL, &pNetworkList);
	if (dwRet != ERROR_SUCCESS) return false;
	WLANInfo info;
	for (DWORD i = 0; i < pNetworkList->dwNumberOfItems; i++) {
		info.IsCurrentConnected = false;
		info.SSID = (char*)(pNetworkList->Network[i].dot11Ssid.ucSSID);
		if ((pNetworkList->Network[i].dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) == WLAN_AVAILABLE_NETWORK_CONNECTED) {
			info.IsCurrentConnected = true;
		}
		List.push_back(info);
	}
	::WlanFreeMemory(pNetworkList);
	return true;
}
void CWIFI::CreateWLANProfile(const WLANProfileInfo &Profile, std::wstring &ProfileString)
{
	std::string AuthAlgo;
	switch (Profile.AuthAlgo)
	{
	case DOT11_AUTH_ALGO_80211_OPEN:
	case DOT11_AUTH_ALGO_IHV_START:
	case DOT11_AUTH_ALGO_IHV_END:
		AuthAlgo = "open";
		break;
	case DOT11_AUTH_ALGO_80211_SHARED_KEY:
		AuthAlgo = "shared";
		break;
	case DOT11_AUTH_ALGO_WPA:
	case DOT11_AUTH_ALGO_WPA_NONE:
		AuthAlgo = "WPA";
		break;
	case DOT11_AUTH_ALGO_WPA_PSK:
		AuthAlgo = "WPAPSK";
		break;
	case DOT11_AUTH_ALGO_RSNA:
		AuthAlgo = "WPA2";
		break;
	case DOT11_AUTH_ALGO_RSNA_PSK:
		AuthAlgo = "WPA2PSK";
		break;
	default:
		AuthAlgo = "open";
		break;
	}
	std::string CipherAlgo;
	switch (Profile.CipherAlgo)
	{
	case DOT11_CIPHER_ALGO_NONE:
	case DOT11_CIPHER_ALGO_IHV_START:
	case DOT11_CIPHER_ALGO_IHV_END:
		CipherAlgo = "none";
		break;
	case  DOT11_CIPHER_ALGO_WEP40:
	case DOT11_CIPHER_ALGO_WEP:
		CipherAlgo = "WEP";
		break;
	case DOT11_CIPHER_ALGO_CCMP:
	case DOT11_CIPHER_ALGO_WEP104:
	case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
	//case DOT11_CIPHER_ALGO_RSN_USE_GROUP:
		CipherAlgo = "AES";
		break;
	case DOT11_CIPHER_ALGO_TKIP:
		CipherAlgo = "TKIP";
		break;
	default:
		CipherAlgo = "none";
		break;
	}
	std::string KeyType;
	if (CipherAlgo.compare("WEP") == 0) KeyType = "networkKey";
	else KeyType = "passPhrase";
	const int BUF_SIZE = 2048; 
	wchar_t ProfileBuffer[BUF_SIZE] = { 0 };
	swprintf_s(ProfileBuffer, BUF_SIZE,
		L"<?xml version=\"1.0\" encoding=\"US-ASCII\"?> \
		<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\"> \
		<name>%S</name> \
		<SSIDConfig> \
		<SSID> \
		<name>%S</name> \
		</SSID> \
		</SSIDConfig> \
		<connectionType>ESS</connectionType> \
		<connectionMode>auto</connectionMode> \
		<autoSwitch>false</autoSwitch> \
		<MSM> \
		<security> \
		<authEncryption> \
		<authentication>%S</authentication> \
		<encryption>%S</encryption> \
		<useOneX>false</useOneX> \
		</authEncryption> \
		<sharedKey> \
		<keyType>%S</keyType> \
		<protected>false</protected> \
		<keyMaterial>%S</keyMaterial> \
		</sharedKey> \
		</security> \
		</MSM> \
		</WLANProfile>",
		Profile.SSID.c_str(),
		Profile.SSID.c_str(),
		AuthAlgo.c_str(),
		CipherAlgo.c_str(),
		KeyType.c_str(),
		Profile.Key.c_str());
	ProfileString = ProfileBuffer;
}

HardwareManagerNamespaceEnd
