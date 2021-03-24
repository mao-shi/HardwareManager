#pragma once

HardwareManagerNamespaceBegin

#define SMART_DATA_LENGTH 362
#define MAX_SMART_ATTRIBUTES 30

#pragma pack (1)

typedef struct tagSMARTAttribute
{
	enum SmartAttributeID {
		PowerOnHours = 0x09,// ͨ��ʱ��
		Temperature = 0xC2// �¶�
	};
	unsigned char ID;// ����ID
	unsigned short StatusFlags;// ״ֵ̬
	unsigned char Current;// ��ǰֵ
	unsigned char Worst;// ���ֵ
	unsigned char RawValue[6];// ��ʵֵ
	unsigned char Reserved;// ����
} SMARTAttribute;

typedef struct tagSMARTData
{
	unsigned short Version;// SMART�汾
	SMARTAttribute AttributeArray[MAX_SMART_ATTRIBUTES];// SMART��������
} SMARTData;

#pragma pack ()

class CSMARTParser
{
public:
	CSMARTParser(unsigned char SmartData[SMART_DATA_LENGTH]);
	virtual ~CSMARTParser();

private:
	SMARTData m_SmartData;

public:
	bool GetTemperature(unsigned int &Temperature);
    bool GetPowerOnHours(unsigned long &Hours);
};

HardwareManagerNamespaceEnd
