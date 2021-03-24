#include "HardwareManager.h"

HardwareManagerNamespaceBegin

CSMARTParser::CSMARTParser(unsigned char SmartData[SMART_DATA_LENGTH])
{
	if (SmartData) memcpy_s(&m_SmartData, sizeof(SMARTData), SmartData, SMART_DATA_LENGTH);
	else memset(&m_SmartData, 0, sizeof(SMARTData));
}
CSMARTParser::~CSMARTParser()
{}

bool CSMARTParser::GetTemperature(unsigned int &Temperature)
{
	for (int i = 0; i < MAX_SMART_ATTRIBUTES; i++) {
		if (m_SmartData.AttributeArray[i].ID == SMARTAttribute::SmartAttributeID::Temperature) {
			Temperature = m_SmartData.AttributeArray[i].RawValue[0];
			return true;
		}
	}
	return false;
}
bool CSMARTParser::GetPowerOnHours(unsigned long &Hours)
{
	for (int i = 0; i < MAX_SMART_ATTRIBUTES; i++) {
		if (m_SmartData.AttributeArray[i].ID == SMARTAttribute::SmartAttributeID::PowerOnHours) {
			Hours = *((unsigned long*)(m_SmartData.AttributeArray[i].RawValue));
			return true;
		}
	}
	return false;
}

HardwareManagerNamespaceEnd
