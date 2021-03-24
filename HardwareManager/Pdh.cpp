#include "HardwareManager.h"

#pragma comment(lib, "Pdh.lib")

HardwareManagerNamespaceBegin

const TCHAR *CPdh::Type::CPU = _T("\\Processor Information(_Total)\\% Processor Performance");

CPdh::CPdh(const TCHAR *type) :
	m_hQuery(NULL),
	m_hCounter(NULL)
{
	PDH_STATUS lStatus = ::PdhOpenQuery(NULL, NULL, &m_hQuery);
    if (lStatus != ERROR_SUCCESS) {
		return;
	}
	lStatus = ::PdhAddCounter(m_hQuery, type, NULL, &m_hCounter);
}
CPdh::~CPdh()
{
	if (m_hCounter) {
		::PdhRemoveCounter(m_hCounter);
		m_hCounter = NULL;
	}
	if (m_hQuery) {
		::PdhCloseQuery(m_hQuery);
		m_hQuery = NULL;
	}
}

bool CPdh::CollectData(long &Value)
{
	if (!m_hCounter) return false;
	PDH_STATUS lStatus = ::PdhCollectQueryData(m_hQuery);
	if (lStatus != ERROR_SUCCESS) return false;
	lStatus = ::PdhCollectQueryData(m_hQuery);
	if (lStatus != ERROR_SUCCESS) return false;
	PDH_FMT_COUNTERVALUE CounterValue = { 0 };
	lStatus = ::PdhGetFormattedCounterValue(m_hCounter, PDH_FMT_LONG, NULL, &CounterValue);
	if (lStatus != ERROR_SUCCESS) return false;
	Value = CounterValue.longValue;
	return true;
}

HardwareManagerNamespaceEnd
