#pragma once

HardwareManagerNamespaceBegin

#define SMART_DATA_LENGTH 362
#define MAX_SMART_ATTRIBUTES 30

#pragma pack (1)

typedef struct tagSMARTAttribute
{
	enum SmartAttributeID {
		PowerOnHours = 0x09,// 通电时间
		Temperature = 0xC2// 温度
	};
	unsigned char ID;// 属性ID
	unsigned short StatusFlags;// 状态值
	unsigned char Current;// 当前值
	unsigned char Worst;// 最差值
	unsigned char RawValue[6];// 真实值
	unsigned char Reserved;// 保留
} SMARTAttribute;

typedef struct tagSMARTData
{
	unsigned short Version;// SMART版本
	SMARTAttribute AttributeArray[MAX_SMART_ATTRIBUTES];// SMART属性数组
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
