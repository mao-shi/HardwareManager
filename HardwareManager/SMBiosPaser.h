#pragma once

#include <vector>
#include <string>

HardwareManagerNamespaceBegin

typedef struct tagSMBiosType 
{
	unsigned int Type;// 类型
	std::vector<unsigned char> TypeData;// 数据
} SMBiosType ;

typedef struct tagSMBiosBIOSInfo
{
	std::string Vendor; /// <厂商名
	std::string Version;// 版本
	std::string ReleaseDate;// 发布日期
	unsigned long RomSize;// ROM大小, 单位K
}SMBiosBIOSInfo;

typedef struct tagSMBiosSystemInfo
{
	std::string Manufacturer;// 电脑制造商
	std::string ProductName;// 电脑名称
	std::string Version;// 电脑版本
	std::string SerialNumber;// 电脑序列号
	unsigned char UUID[16];// 电脑唯一标识符
} SMBiosSystemInfo;

typedef struct tagSMBiosMotherBoardInfo
{
	std::string Manufacturer;// 主板制造商
	std::string Product;// 主板名
	std::string Version;// 主板版本
	std::string SerialNumber;// 主板序列号
} SMBiosMotherBoardInfo;

class CSMBiosPaser
{
public:
	CSMBiosPaser(const std::vector<unsigned char>& SMBiosData);
	virtual ~CSMBiosPaser();

private:
	std::vector<SMBiosType> m_SMBiosTypeList;// SMBIOS信息Type列表

private:
	void GetTypeStringList(const SMBiosType &TypeInfo, std::vector<std::string> &strList);

public:
	bool GetBiosInfo(SMBiosBIOSInfo &Info);
	bool GetSystemInfo(SMBiosSystemInfo &Info);
	bool GetBaseBoardInfo(SMBiosMotherBoardInfo &Info);
};

HardwareManagerNamespaceEnd
