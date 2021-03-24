#pragma once

#if defined(HARDWAREMANAGER_EXPORTS)
#define HardwareManagerInterface __declspec(dllexport)
#else
#define HardwareManagerInterface __declspec(dllimport)
#endif

#define HardwareManagerNamespace HardwareManagerNamespace
#define HardwareManagerNamespaceBegin namespace HardwareManagerNamespace {
#define HardwareManagerNamespaceEnd }

#include <Windows.h>
#include <tchar.h>
#include <comdef.h>

#if defined(HARDWAREMANAGER_EXPORTS)
#include "..\WinRing0\WinRing0.h"
#endif

#if defined(HARDWAREMANAGER_EXPORTS)
#include "Pdh.h"
#include "WMI.h"
#include "Temperature.h"
#include "Power.h"
#include "SystemMetrics.h"
#include "TaskScheduler.h"
#include "Memory.h"
#include "WiFi.h"
#include "Device.h"
#include "SMBiosPaser.h"
#include "SMARTParser.h"
#include "IDEDiskController.h"
#endif

#include "Computer.h"

#if defined(HARDWAREMANAGER_EXPORTS)
#pragma comment(lib, "..\\Bin\\WinRing0.lib")
#endif

HardwareManagerNamespaceBegin

//extern Type Var;

bool Initialize();
void Release();

//extern HardwareManagerInterface Type Var;

//HardwareManagerInterface DWORD WritePhysicalMemory(DWORD_PTR address, PBYTE buffer, DWORD count, DWORD unitSize);

HardwareManagerNamespaceEnd
