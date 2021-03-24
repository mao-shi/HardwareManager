#include "HardwareManager.h"
#include <devguid.h>
#include "nvapi\nvapi.h"
#include "adlapi\adl_sdk.h"

#pragma comment(lib, ".\\nvapi\\nvapi.lib")

HardwareManagerNamespaceBegin

CCPUTemperature::CCPUTemperature() :
	m_pPdh(NULL)
{
	m_pPdh = new CPdh(CPdh::Type::CPU);
}
CCPUTemperature::~CCPUTemperature()
{
	if (m_pPdh) {
		delete m_pPdh;
		m_pPdh = NULL;
	}
}

bool CCPUTemperature::GetCoreNumber(CPUCore &Core)
{
	if (WinRing0Namespace::GetRefCount() == 0) return false;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION processorInforList = NULL;
	DWORD dwRequireSize = 0;
	SYSTEM_INFO systemInfo = { 0 };
	DWORD listCount = 0;
	BOOL bRet = ::GetLogicalProcessorInformation(processorInforList, &dwRequireSize);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) return false;
	processorInforList = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)new char[dwRequireSize];
	if (!processorInforList) return false;
	bRet = ::GetLogicalProcessorInformation(processorInforList, &dwRequireSize);
	if (bRet == FALSE) {
		delete[] processorInforList;
		return false;
	}
	listCount = dwRequireSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	Core.PhysicalCoresr = 0;
	for (unsigned long i = 0; i < listCount; i++) {
		if (processorInforList[i].Relationship == RelationProcessorCore) {
			Core.PhysicalCoresr += 1;
		}
	}
	// ��ȡ�߼�����������
	::GetNativeSystemInfo(&systemInfo);
	Core.LogicalCores = systemInfo.dwNumberOfProcessors;
	delete[] processorInforList;
	return true;
}
bool CCPUTemperature::GetTemperature(CPUTemperature &Temperature)
{
	if (WinRing0Namespace::GetRefCount() == 0) return false;
	// ��ȡCPU������Ϣ
	char vendorBuffer[20] = { 0 };
	bool ret = WinRing0Namespace::Cpuid(0,
		(unsigned long*)&vendorBuffer[0],
		(unsigned long *)&vendorBuffer[4],
		(unsigned long*)&vendorBuffer[8],
		(unsigned long*)&vendorBuffer[12]);
	if (!ret) return false;
	// AMD:		"AuthenticAMD"
	// Intel:	"GenuineIntel"
	// ע�����edx�� ecx��˳����ַ����ж��ߵ�˳���Ƿ���, ���Ե����������Ĵ���
	unsigned long ecx = *((unsigned long *)&vendorBuffer[8]);
	unsigned long edx = *((unsigned long *)&vendorBuffer[12]);
	*((unsigned long *)&vendorBuffer[12]) = ecx;
	*((unsigned long *)&vendorBuffer[8]) = edx;
	const DWORD temperaturesSize = 64;
	CPUCore Core = { 0 };
	if (!GetCoreNumber(Core)) return false;
	Temperature.CoresNumber = Core.PhysicalCoresr;
	if (0 == strcmp((char*)(&vendorBuffer[4]), "GenuineIntel")) {
		/*
		DTS( Digital Thermal Senser)��ʽ��ȡCPU�¶ȣ�ͨ����ȡMSR��ʵ��
		����Intel������CPU��ÿ����������DTS������ʵʱ���CPU���¶ȣ� 
		���¶ȴﵽTCC( Thermal Control Circuit)�����¶�ʱ�� ����ͨ�����͵�ѹ����Ƶ�����ȵ��ڵ���ʽ�����¶ȣ� ���ǳ�����¶�ΪTjunction
		�����Ǵ�MSR��Model Specific Register���������¶��Ǿ���Tjunction���²������ʵ�ʵ��¶ȣ���֮ΪDelta
		����ʵ�ʵ��¶�Ϊ��Tjunction- Delta
		���Ҳ�ͬCPU��Tjunction����һ��������Ϊ85�桢100�桢105���, ��Ҫ����Intel�û��ֲ�
		����ΪIntel ���CPU�¶ȵĶ�ȡ����:
		*/
		// ʹ��CPUIDָ��0��ȡCPU֧�ֵ����������
		// ������������С��6, ��ôCPU��֧��DTS
		DWORD eax = 0;
		DWORD ebx = 0;
		DWORD ecx = 0;
		DWORD edx = 0;
		WinRing0Namespace::Cpuid(0, &eax, &ebx, &ecx, &edx);
		DWORD maxCmdNum = eax;
		if (maxCmdNum < 6) return false;
		// ʹ��CPUIDָ��6, �鿴CPU�Ƿ�֧��DTS
		// eax��һλΪ1���ʾ֧��DTS, ��֮��֧��DTS
		eax = 0;
		ebx = 0;
		ecx = 0;
		edx = 0;
		WinRing0Namespace::Cpuid(6, &eax, &ebx, &ecx, &edx);
		if (0 == (eax & 1)) return false;
		// ʹ��0xeeִ��rdmsrָ��, ���exa�ĵ�30λΪ1, ���ʾTjunctionΪ85, ����Ϊ100
		eax = 0;
		edx = 0;
		WinRing0Namespace::Rdmsr(0xee, &eax, &edx);
		DWORD tjunction = 0;
		if (1 == (eax&0x20000000)) tjunction = 85;
		else tjunction = 100;
		DWORD step =  Core.LogicalCores / Core.PhysicalCoresr;
		// ʹ��0x19cִ��rdmsrָ��, eax��16:23λ��ʾ��ǰDTSֵ
		// �ֱ��ȡÿ���߼����������¶�
		DWORD loopMax = (temperaturesSize < Core.LogicalCores ? temperaturesSize : Core.LogicalCores);
		for (DWORD processorIndex = 0; processorIndex < loopMax; processorIndex += step) {
			DWORD threadMask = 1;
			threadMask = threadMask << processorIndex;
			DWORD oldMask = ::SetThreadAffinityMask(::GetCurrentThread(), threadMask);
			if (0 == oldMask) return false;
			eax = 0;
			edx = 0;
			WinRing0Namespace::Rdmsr(0x19c, &eax, &edx);
			DWORD delta = (eax&0x007f0000) >> 16;
			Temperature.Temperatures[processorIndex/step] = tjunction - delta;
			::SetThreadAffinityMask(::GetCurrentThread(), oldMask);
		}
	}
	else if (0 == strcmp((char*)(&vendorBuffer[4]), "AuthenticAMD")) {
		unsigned int coreTemp = 0;
		const WORD PCI_CONFIG_ADDRESS = 0XCF8;
		const WORD PCI_CONFIG_DATA = 0XCFC;
		// ɨ������PCI�豸�ҵ��豸: VID 0x1022 DID 0x141D
		for(unsigned int bus = 0; bus <= 255; bus++) {
			for(unsigned int dev = 0; dev < 32; dev++) {
				for(unsigned int func = 0; func < 8; func++) {
					DWORD dwAddr = 0X80000000+(bus<<16)+(dev<<11)+(func<<8);
					// ��ȡ�豸ID�ͳ���ID
					WinRing0Namespace::WriteIoPortDword(PCI_CONFIG_ADDRESS, dwAddr);
					DWORD dwData = 0;
					WinRing0Namespace::ReadIoPortDword(PCI_CONFIG_DATA, &dwData);
					// ֵΪ0XFFFFFFFF��ʾ�����ڸ�PCI�豸
					if (dwData == 0XFFFFFFFF) continue;
					DWORD vid=dwData&0XFFFF;
					DWORD did=(dwData>>16)&0XFFFF;
					if (vid == 0X1022 && did == 0X141D) {
						// �Ĵ�����ֵ��8λΪ�¶�ֵ
						WinRing0Namespace::WriteIoPortDword(PCI_CONFIG_ADDRESS, dwAddr|0XA4);
						DWORD dwTemp = 0;
						WinRing0Namespace::ReadIoPortDword(PCI_CONFIG_DATA, &dwTemp);
						dwTemp = dwTemp >> 24;
						dwTemp = dwTemp & 0XFF;
						coreTemp = (int)dwTemp;
						break;
					}
				}
				if (coreTemp != 0) break;
			}
			if (coreTemp != 0) break;
		}
		unsigned int loopMax = (temperaturesSize < Core.PhysicalCoresr ? temperaturesSize : Core.PhysicalCoresr);
		for (unsigned int i = 0; i < loopMax; i++) {
			Temperature.Temperatures[i] = coreTemp;
		}
		if (coreTemp == 0) return false;
	}
	else return false;
	Temperature.Temperature = 0;
	for (unsigned int c = 0; c < Temperature.CoresNumber; c++) {
		Temperature.Temperature += Temperature.Temperatures[c] / Temperature.CoresNumber;
	}
	return true;
}
bool CCPUTemperature::GetPerformance(CPUPerformance &Performance)
{
	if (!m_pPdh) return false;
	long Speed = 0;
	if (!m_pPdh->CollectData(Speed)) return false;
	Performance.SpeedPercentage = Speed;
	CCPUManager CPUManager;
	unsigned long long LoadPercentage = 0;
	if (!CPUManager.GetLoadPercentage(0, LoadPercentage)) return false;
	Performance.LoadPercentage = (unsigned long)LoadPercentage;
	return true;
}

// -----------------------------------------------------------------

// CGPUTemperature���ʵ����ͨ��ʹ��Ӣΰ�￪����nvapi��ʵ�ֵ�
// CGPUTemperature���ʵ����ͨ��ʹ��AMD������ADL(AMD Display Library)��ʵ�ֵ�
void* __stdcall ADL_Main_Memory_Alloc (int iSize)
{
	void* lpBuffer = malloc (iSize);
	return lpBuffer;
}
void __stdcall ADL_Main_Memory_Free (void** lpBuffer)
{
	if (*lpBuffer) {
		free(*lpBuffer);
		*lpBuffer = NULL;
	}
}

CGPUTemperature::CGPUTemperature()
{
	memset(&m_Temperature, 0, sizeof(GPUTemperature));
	CGPUManager Gpu;
	int Count = Gpu.GetCount();
	std::wstring Description;
	for (int i = 0; i < Count; i++) {
		if (Gpu.GetDescription(i, Description)) {
			if (Description.find(L"NVIDIA") != std::wstring::npos) {
				NvAPI_Status NvRet = ::NvAPI_Initialize();
				if (NvRet != NVAPI_OK) return;
				NvU32 GpuCount = 0;
				NvPhysicalGpuHandle hNvPhysicalGpuList[NVAPI_MAX_PHYSICAL_GPUS];// ����Gpu�����
				NvRet = ::NvAPI_EnumPhysicalGPUs(hNvPhysicalGpuList, &GpuCount);
				if (NvRet != NVAPI_OK || GpuCount == 0) return;
				// ��ȡ�¶�ǰ�Ȼ�ȡ������Ϣ, ��Ȼ��ȡ�¶Ȼ�ʧ��
				unsigned int UsingGpuIndex = 0;
				NvU32 BusID = 0;
				::NvAPI_GPU_GetBusId(hNvPhysicalGpuList[UsingGpuIndex], &BusID);
				NV_GPU_THERMAL_SETTINGS Thermal;
				Thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
				NvRet = ::NvAPI_GPU_GetThermalSettings(hNvPhysicalGpuList[UsingGpuIndex], NVAPI_THERMAL_TARGET_ALL, &Thermal);
				if (NvRet != NVAPI_OK) return;
				m_Temperature.SensorsNumber = Thermal.count;
				for (unsigned int i = 0; i < Thermal.count; i++) {
					m_Temperature.Temperatures[i] = Thermal.sensor[i].currentTemp;
					m_Temperature.Temperature += (Thermal.sensor[i].currentTemp / Thermal.count);
				}
				NvRet = ::NvAPI_Unload();
			}
			else {
				typedef int (*ADL2_MAIN_CONTROL_CREATE )(IN ADL_MAIN_MALLOC_CALLBACK callback, IN int iEnumConnectedAdapters, OUT ADL_CONTEXT_HANDLE* context);
				typedef int (*ADL2_MAIN_CONTROL_DESTROY )(IN ADL_CONTEXT_HANDLE context);
				typedef int (*ADL2_ADAPTER_NUMBEROFADAPTERS_GET ) (IN ADL_CONTEXT_HANDLE context, OUT int* lpNumAdapters);
				typedef int (*ADL2_ADAPTER_ADAPTERINFO_GET)(IN ADL_CONTEXT_HANDLE context, OUT LPAdapterInfo lpInfo, IN int iInputSize);
				typedef int (*ADL2_OVERDRIVE5_TEMPERATURE_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iThermalControllerIndex, ADLTemperature* lpTemperature);
				// ��װ��AMD�Կ�������ϵͳĿ¼System32�´���atiadlxx.dll(32λ��64λ)
				// ��64λϵͳ��, 32λ�ĳ���װ��64λ��dll��ʧ��, ������Ҫ��װ��SysWOW64Ŀ¼�µ�atiadlxy.dll
				HMODULE hADLDll = ::LoadLibraryA("atiadlxx.dll");
				if (hADLDll == NULL) hADLDll = ::LoadLibraryA("atiadlxy.dll");
				if (hADLDll == NULL) return;
				ADL2_MAIN_CONTROL_CREATE ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)::GetProcAddress(hADLDll, "ADL2_Main_Control_Create");
				ADL2_MAIN_CONTROL_DESTROY ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)::GetProcAddress(hADLDll, "ADL2_Main_Control_Destroy");
				ADL2_ADAPTER_NUMBEROFADAPTERS_GET ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)::GetProcAddress(hADLDll, "ADL2_Adapter_NumberOfAdapters_Get");
				ADL2_ADAPTER_ADAPTERINFO_GET ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)::GetProcAddress(hADLDll, "ADL2_Adapter_AdapterInfo_Get");
				ADL2_OVERDRIVE5_TEMPERATURE_GET ADL2_Overdrive5_Temperature_Get = (ADL2_OVERDRIVE5_TEMPERATURE_GET)::GetProcAddress(hADLDll, "ADL2_Overdrive5_Temperature_Get");
				if (ADL2_Main_Control_Create == NULL ||
					ADL2_Main_Control_Destroy == NULL ||
					ADL2_Adapter_NumberOfAdapters_Get == NULL ||
					ADL2_Adapter_AdapterInfo_Get == NULL ||
					ADL2_Overdrive5_Temperature_Get == NULL) {
					return;
				}
				ADL_CONTEXT_HANDLE hADLContext = NULL;
				int iRet = ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &hADLContext);
				if (iRet != ADL_OK) return;
				int AdapterCount = 0;
				iRet = ADL2_Adapter_NumberOfAdapters_Get(hADLContext, &AdapterCount);
				if (iRet != ADL_OK) return;
				if (AdapterCount < 1) return;
				unsigned int UsingGpuIndex = 0;
				ADLTemperature adlTemp;
				if (ADL2_Overdrive5_Temperature_Get(hADLContext, UsingGpuIndex, 0, &adlTemp) != ADL_OK) return;
				m_Temperature.SensorsNumber = 1;
				m_Temperature.Temperatures[0] = (unsigned int)adlTemp.iTemperature / 1000;
				m_Temperature.Temperature = m_Temperature.Temperatures[0];
				ADL2_Main_Control_Destroy(hADLContext);
				::FreeLibrary(hADLDll);
			}
			break;
		}
	}
}
CGPUTemperature::~CGPUTemperature()
{}

bool CGPUTemperature::GetTemperature(GPUTemperature &Temperature)
{
	if (m_Temperature.SensorsNumber == 0) return false;
	memcpy_s(&Temperature, sizeof(GPUTemperature), &m_Temperature, sizeof(GPUTemperature));
	return true;
}

HardwareManagerNamespaceEnd
