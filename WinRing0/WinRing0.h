#pragma once

#if defined(WINRING0_EXPORTS)
#define WinRing0Interface __declspec(dllexport)
#else
#define WinRing0Interface __declspec(dllimport)
#endif

#define WinRing0Namespace WinRing0Namespace
#define WinRing0NamespaceBegin namespace WinRing0Namespace {
#define WinRing0NamespaceEnd }

#include <Windows.h>

#include "define.h"

WinRing0NamespaceBegin

//extern Type Var;

bool Initialize();
void Release();

//extern WinRing0Interface Type Var;

WinRing0Interface unsigned long GetRefCount();
WinRing0Interface bool IsCurrentUserLocalAdministrator();
// CPU
WinRing0Interface bool IsCpuid();
WinRing0Interface bool IsMsr();
WinRing0Interface bool IsTsc();
WinRing0Interface bool Rdmsr(DWORD index, PDWORD eax, PDWORD edx);
WinRing0Interface bool RdmsrTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask);
WinRing0Interface bool RdmsrPx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask);
WinRing0Interface bool Wrmsr(DWORD index, DWORD eax, DWORD edx);
WinRing0Interface bool WrmsrTx(DWORD index, DWORD eax, DWORD edx, DWORD_PTR threadAffinityMask);
WinRing0Interface bool WrmsrPx(DWORD index, DWORD eax, DWORD edx, DWORD_PTR processAffinityMask);
WinRing0Interface bool Rdpmc(DWORD index, PDWORD eax, PDWORD edx);
WinRing0Interface bool RdpmcTx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask);
WinRing0Interface bool RdpmcPx(DWORD index, PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask);
WinRing0Interface bool Cpuid(DWORD index, DWORD *pEAX, DWORD *pEBX, DWORD *pECX, DWORD *pEDX);
WinRing0Interface bool CpuidTx(DWORD index, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR threadAffinityMask);
WinRing0Interface bool CpuidPx(DWORD index, PDWORD eax, PDWORD ebx, PDWORD ecx, PDWORD edx, DWORD_PTR processAffinityMask);
WinRing0Interface bool Rdtsc(PDWORD eax, PDWORD edx);
WinRing0Interface bool RdtscTx(PDWORD eax, PDWORD edx, DWORD_PTR threadAffinityMask);
WinRing0Interface bool RdtscPx(PDWORD eax, PDWORD edx, DWORD_PTR processAffinityMask);
WinRing0Interface bool Hlt();
WinRing0Interface bool HltTx(DWORD_PTR threadAffinityMask);
WinRing0Interface bool HltPx(DWORD_PTR processAffinityMask);
// I/O
WinRing0Interface bool ReadIoPortByte(WORD port, PBYTE value);
WinRing0Interface bool ReadIoPortWord(WORD port, PWORD value);
WinRing0Interface bool ReadIoPortDword(WORD port, PDWORD value);
WinRing0Interface bool WriteIoPortByte(WORD port, BYTE value);
WinRing0Interface bool WriteIoPortWord(WORD port, WORD value);
WinRing0Interface bool WriteIoPortDword(WORD port, DWORD value);
// PCI
WinRing0Interface bool ReadPciConfigByte(DWORD pciAddress, DWORD regAddress, PBYTE value);
WinRing0Interface bool ReadPciConfigWord(DWORD pciAddress, DWORD regAddress, PWORD value);
WinRing0Interface bool ReadPciConfigDword(DWORD pciAddress, DWORD regAddress, PDWORD value);
WinRing0Interface bool WritePciConfigByte(DWORD pciAddress, DWORD regAddress, BYTE value);
WinRing0Interface bool WritePciConfigWord(DWORD pciAddress, DWORD regAddress, WORD value);
WinRing0Interface bool WritePciConfigDword(DWORD pciAddress, DWORD regAddress, DWORD value);
// Find PCI Device
WinRing0Interface DWORD FindPciDeviceById(WORD vendorId, WORD deviceId, BYTE index, BYTE maxBus);
WinRing0Interface DWORD FindPciDeviceByClass(BYTE baseClass, BYTE subClass, BYTE programIf, BYTE index, BYTE maxBus);
// Physical Memory
WinRing0Interface DWORD ReadDmiMemory(PBYTE buffer, DWORD count, DWORD unitSize);
WinRing0Interface DWORD ReadPhysicalMemory(DWORD_PTR address, PBYTE buffer, DWORD count, DWORD unitSize);
WinRing0Interface DWORD WritePhysicalMemory(DWORD_PTR address, PBYTE buffer, DWORD count, DWORD unitSize);

WinRing0NamespaceEnd

