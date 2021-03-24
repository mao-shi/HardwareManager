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
	// 获取逻辑处理器数量
	::GetNativeSystemInfo(&systemInfo);
	Core.LogicalCores = systemInfo.dwNumberOfProcessors;
	delete[] processorInforList;
	return true;
}
bool CCPUTemperature::GetTemperature(CPUTemperature &Temperature)
{
	if (WinRing0Namespace::GetRefCount() == 0) return false;
	// 获取CPU厂商信息
	char vendorBuffer[20] = { 0 };
	bool ret = WinRing0Namespace::Cpuid(0,
		(unsigned long*)&vendorBuffer[0],
		(unsigned long *)&vendorBuffer[4],
		(unsigned long*)&vendorBuffer[8],
		(unsigned long*)&vendorBuffer[12]);
	if (!ret) return false;
	// AMD:		"AuthenticAMD"
	// Intel:	"GenuineIntel"
	// 注意这里：edx与 ecx的顺序和字符串中二者的顺序是反的, 所以调换后两个寄存器
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
		DTS( Digital Thermal Senser)方式获取CPU温度，通过读取MSR来实现
		现在Intel处理器CPU中每个都集合了DTS，用来实时监测CPU的温度， 
		当温度达到TCC( Thermal Control Circuit)激活温度时， 将会通过降低电压、主频、风扇调节等形式调节温度， 我们称这个温度为Tjunction
		而我们从MSR（Model Specific Register）读到的温度是距离Tjunction的温差，而不是实际的温度，称之为Delta
		所以实际的温度为：Tjunction- Delta
		并且不同CPU的Tjunction还不一样，可以为85℃、100℃、105℃等, 需要查阅Intel用户手册
		以下为Intel 酷睿CPU温度的读取过程:
		*/
		// 使用CPUID指令0获取CPU支持的最大命令数
		// 如果最大命令数小于6, 那么CPU不支持DTS
		DWORD eax = 0;
		DWORD ebx = 0;
		DWORD ecx = 0;
		DWORD edx = 0;
		WinRing0Namespace::Cpuid(0, &eax, &ebx, &ecx, &edx);
		DWORD maxCmdNum = eax;
		if (maxCmdNum < 6) return false;
		// 使用CPUID指令6, 查看CPU是否支持DTS
		// eax第一位为1则表示支持DTS, 反之不支持DTS
		eax = 0;
		ebx = 0;
		ecx = 0;
		edx = 0;
		WinRing0Namespace::Cpuid(6, &eax, &ebx, &ecx, &edx);
		if (0 == (eax & 1)) return false;
		// 使用0xee执行rdmsr指令, 如果exa的第30位为1, 则表示Tjunction为85, 否则为100
		eax = 0;
		edx = 0;
		WinRing0Namespace::Rdmsr(0xee, &eax, &edx);
		DWORD tjunction = 0;
		if (1 == (eax&0x20000000)) tjunction = 85;
		else tjunction = 100;
		DWORD step =  Core.LogicalCores / Core.PhysicalCoresr;
		// 使用0x19c执行rdmsr指令, eax的16:23位表示当前DTS值
		// 分别获取每个逻辑处理器的温度
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
		// 扫描所有PCI设备找到设备: VID 0x1022 DID 0x141D
		for(unsigned int bus = 0; bus <= 255; bus++) {
			for(unsigned int dev = 0; dev < 32; dev++) {
				for(unsigned int func = 0; func < 8; func++) {
					DWORD dwAddr = 0X80000000+(bus<<16)+(dev<<11)+(func<<8);
					// 读取设备ID和厂商ID
					WinRing0Namespace::WriteIoPortDword(PCI_CONFIG_ADDRESS, dwAddr);
					DWORD dwData = 0;
					WinRing0Namespace::ReadIoPortDword(PCI_CONFIG_DATA, &dwData);
					// 值为0XFFFFFFFF表示不存在该PCI设备
					if (dwData == 0XFFFFFFFF) continue;
					DWORD vid=dwData&0XFFFF;
					DWORD did=(dwData>>16)&0XFFFF;
					if (vid == 0X1022 && did == 0X141D) {
						// 寄存器的值高8位为温度值
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

// CGPUTemperature类的实现是通过使用英伟达开发包nvapi来实现的
// CGPUTemperature类的实现是通过使用AMD开发包ADL(AMD Display Library)来实现的
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
				NvPhysicalGpuHandle hNvPhysicalGpuList[NVAPI_MAX_PHYSICAL_GPUS];// 物理Gpu句柄表
				NvRet = ::NvAPI_EnumPhysicalGPUs(hNvPhysicalGpuList, &GpuCount);
				if (NvRet != NVAPI_OK || GpuCount == 0) return;
				// 获取温度前先获取其他信息, 不然获取温度会失败
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
				// 安装了AMD显卡驱动的系统目录System32下存在atiadlxx.dll(32位或64位)
				// 在64位系统上, 32位的程序装载64位的dll会失败, 所以需要在装载SysWOW64目录下的atiadlxy.dll
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
