#pragma once

HardwareManagerNamespaceBegin

typedef struct tagCPUCore
{
	unsigned long PhysicalCoresr;
	unsigned long LogicalCores;
} CPUCore;

typedef struct tagCPUTemperature
{
	unsigned int CoresNumber;
	unsigned int Temperatures[64];
	unsigned int Temperature;
} CPUTemperature;

typedef struct tagCPUPerformance
{
	unsigned long LoadPercentage;
	unsigned long SpeedPercentage;
} CPUPerformance;

// 需要管理员权限
class CCPUTemperature
{
public:
	CCPUTemperature();
	virtual ~CCPUTemperature();

protected:
	class CPdh *m_pPdh;

public:
	bool GetCoreNumber(CPUCore &Core);
	bool GetTemperature(CPUTemperature &Temperature);
	bool GetPerformance(CPUPerformance &Performance);
};

typedef struct tagGPUTemperature
{
	unsigned int SensorsNumber;
	unsigned int Temperatures[16];
	unsigned int Temperature;
} GPUTemperature;

class CGPUTemperature
{
public:
	CGPUTemperature();
	virtual ~CGPUTemperature();

private:
	GPUTemperature m_Temperature;

public:
	bool GetTemperature(GPUTemperature &Temperature);
};

HardwareManagerNamespaceEnd
