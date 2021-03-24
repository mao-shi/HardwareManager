#pragma once

#include <vector>
#include <string>

HardwareManagerNamespaceBegin

typedef struct tagSMBiosType 
{
	unsigned int Type;// ����
	std::vector<unsigned char> TypeData;// ����
} SMBiosType ;

typedef struct tagSMBiosBIOSInfo
{
	std::string Vendor; /// <������
	std::string Version;// �汾
	std::string ReleaseDate;// ��������
	unsigned long RomSize;// ROM��С, ��λK
}SMBiosBIOSInfo;

typedef struct tagSMBiosSystemInfo
{
	std::string Manufacturer;// ����������
	std::string ProductName;// ��������
	std::string Version;// ���԰汾
	std::string SerialNumber;// �������к�
	unsigned char UUID[16];// ����Ψһ��ʶ��
} SMBiosSystemInfo;

typedef struct tagSMBiosMotherBoardInfo
{
	std::string Manufacturer;// ����������
	std::string Product;// ������
	std::string Version;// ����汾
	std::string SerialNumber;// �������к�
} SMBiosMotherBoardInfo;

class CSMBiosPaser
{
public:
	CSMBiosPaser(const std::vector<unsigned char>& SMBiosData);
	virtual ~CSMBiosPaser();

private:
	std::vector<SMBiosType> m_SMBiosTypeList;// SMBIOS��ϢType�б�

private:
	void GetTypeStringList(const SMBiosType &TypeInfo, std::vector<std::string> &strList);

public:
	bool GetBiosInfo(SMBiosBIOSInfo &Info);
	bool GetSystemInfo(SMBiosSystemInfo &Info);
	bool GetBaseBoardInfo(SMBiosMotherBoardInfo &Info);
};

HardwareManagerNamespaceEnd
