#include "HardwareManager.h"

#define BIOS_INFO_TYPE 0
#define SYSTEM_INFO_TYPE 1
#define MOTHERBOARD_INFO_TYPE 2

HardwareManagerNamespaceBegin

CSMBiosPaser::CSMBiosPaser(const std::vector<unsigned char>& SMBiosData)
{
	// 按照Type解析SMBios信息
	unsigned TypeStartIndex = 0;// Type结构开始索引位置
	while (TypeStartIndex < SMBiosData.size()) {
		SMBiosType NewType;
		// 获取Type值
		NewType.Type = (unsigned int)SMBiosData[TypeStartIndex];
		// 获取格式区域长度
		unsigned int length = (unsigned int)SMBiosData[TypeStartIndex + 1];
		// 查找字符串区域结束位置
		unsigned int TypeEndIndex = 0;
		for (unsigned int i = length + TypeStartIndex; i < (SMBiosData.size() - 1); i++) {
			if (SMBiosData[i] == 0 && SMBiosData[i + 1] == 0) {
				TypeEndIndex = i + 1;
				break;
			}
		}
		if (TypeEndIndex == 0) break;
		NewType.TypeData.assign(SMBiosData.begin() + TypeStartIndex, SMBiosData.begin() + TypeEndIndex + 1);
		TypeStartIndex = TypeEndIndex + 1;
		m_SMBiosTypeList.push_back(NewType);
	}
}
CSMBiosPaser::~CSMBiosPaser()
{}

void CSMBiosPaser::GetTypeStringList(const SMBiosType &TypeInfo, std::vector<std::string> &strList)
{
	strList.clear();
	unsigned int structLength = (unsigned int)TypeInfo.TypeData[0x01];
	for (unsigned int i = structLength; i < TypeInfo.TypeData.size(); ) {
		if (TypeInfo.TypeData[i] == 0) break;
		std::string str((char*)&(TypeInfo.TypeData[i]));
		strList.push_back(str);
		i += str.length() + 1;
	}
}

bool CSMBiosPaser::GetBiosInfo(SMBiosBIOSInfo &Info)
{
	unsigned int Index = -1;
	for (unsigned int i = 0; i < m_SMBiosTypeList.size(); i++) {
		if (m_SMBiosTypeList[i].Type == BIOS_INFO_TYPE) {
			Index = i;
			break;
		}
	}
	if (Index == -1) return false;
	const SMBiosType &TypeInfo = m_SMBiosTypeList[Index];
	std::vector<std::string> stringList;
	GetTypeStringList(TypeInfo, stringList);
	Info.Vendor = stringList[TypeInfo.TypeData[0x04]-1];
	Info.Version = stringList[TypeInfo.TypeData[0x05]-1];
	Info.ReleaseDate = stringList[TypeInfo.TypeData[0x08]-1];
	Info.RomSize = 64 * ((unsigned int)TypeInfo.TypeData[0x09] + 1);
	return true;
}
bool CSMBiosPaser::GetSystemInfo(SMBiosSystemInfo &Info)
{
	unsigned int Index = -1;
	for (unsigned int i = 0; i < m_SMBiosTypeList.size(); i++) {
		if (m_SMBiosTypeList[i].Type == SYSTEM_INFO_TYPE) {
			Index = i;
			break;
		}
	}
	if (Index == -1) return false;
	const SMBiosType &TypeInfo = m_SMBiosTypeList[Index];
	std::vector<std::string> stringList;
	GetTypeStringList(TypeInfo, stringList);
	Info.Manufacturer = stringList[TypeInfo.TypeData[0x04]-1];
	Info.ProductName = stringList[TypeInfo.TypeData[0x05]-1];
	Info.Version = stringList[TypeInfo.TypeData[0x06]-1];
	Info.SerialNumber = stringList[TypeInfo.TypeData[0x07]-1];
	for (unsigned int i = 0; i < 16; i++) {
		Info.UUID[i] = TypeInfo.TypeData[0x08 + i];
	}
	return true;
}
bool CSMBiosPaser::GetBaseBoardInfo(SMBiosMotherBoardInfo &Info)
{
	unsigned int Index = -1;
	for (unsigned int i = 0; i < m_SMBiosTypeList.size(); i++) {
		if (m_SMBiosTypeList[i].Type == SYSTEM_INFO_TYPE) {
			Index = i;
			break;
		}
	}
	if (Index == -1) return false;
	const SMBiosType &TypeInfo = m_SMBiosTypeList[Index];
	std::vector<std::string> stringList;
	GetTypeStringList(TypeInfo, stringList);
	Info.Manufacturer = stringList[TypeInfo.TypeData[0x04]-1];
	Info.Product = stringList[TypeInfo.TypeData[0x05]-1];
	Info.Version = stringList[TypeInfo.TypeData[0x06]-1];
	Info.SerialNumber = stringList[TypeInfo.TypeData[0x07]-1];
	return true;
}

HardwareManagerNamespaceEnd
