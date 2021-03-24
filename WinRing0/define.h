#pragma once

#define Ring0DriverName9X _T("WinRing0.vxd")
#define Ring0DriverNameNT _T("WinRing0.sys")
#define Ring0DriverNameNTX64 _T("WinRing0x64.sys")
#define Ring0DriverNameNTIA64 _T("WinRing0ia64.sys")

#define Ring0DriverID _T("WinRing0_1_2_0")
#define OLS_TYPE 40000
//-----------------------------------------------------------------------------
//
// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
//
//-----------------------------------------------------------------------------
#define IOCTL_OLS_GET_DRIVER_VERSION \
	CTL_CODE(OLS_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_GET_REFCOUNT \
	CTL_CODE(OLS_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_READ_MSR \
	CTL_CODE(OLS_TYPE, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_WRITE_MSR \
	CTL_CODE(OLS_TYPE, 0x822, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_READ_PMC \
	CTL_CODE(OLS_TYPE, 0x823, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_HALT \
	CTL_CODE(OLS_TYPE, 0x824, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_OLS_READ_IO_PORT \
	CTL_CODE(OLS_TYPE, 0x831, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_WRITE_IO_PORT \
	CTL_CODE(OLS_TYPE, 0x832, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_OLS_READ_IO_PORT_BYTE \
	CTL_CODE(OLS_TYPE, 0x833, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_READ_IO_PORT_WORD \
	CTL_CODE(OLS_TYPE, 0x834, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_READ_IO_PORT_DWORD \
	CTL_CODE(OLS_TYPE, 0x835, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_WRITE_IO_PORT_BYTE \
	CTL_CODE(OLS_TYPE, 0x836, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_OLS_WRITE_IO_PORT_WORD \
	CTL_CODE(OLS_TYPE, 0x837, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_OLS_WRITE_IO_PORT_DWORD \
	CTL_CODE(OLS_TYPE, 0x838, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_OLS_READ_MEMORY \
	CTL_CODE(OLS_TYPE, 0x841, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_WRITE_MEMORY \
	CTL_CODE(OLS_TYPE, 0x842, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_OLS_READ_PCI_CONFIG \
	CTL_CODE(OLS_TYPE, 0x851, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_OLS_WRITE_PCI_CONFIG \
	CTL_CODE(OLS_TYPE, 0x852, METHOD_BUFFERED, FILE_WRITE_ACCESS)

// Bus Number, Device Number and Function Number to PCI Device Address
#define PciBusDevFunc(Bus, Dev, Func)	((Bus&0xFF)<<8) | ((Dev&0x1F)<<3) | (Func&7)
// PCI Device Address to Bus Number
#define PciGetBus(address)				((address>>8) & 0xFF)
// PCI Device Address to Device Number
#define PciGetDev(address)				((address>>3) & 0x1F)
// PCI Device Address to Function Number
#define PciGetFunc(address)				(address&7)

#pragma pack(push,4)

typedef struct  _OLS_WRITE_MSR_INPUT {
	ULONG		Register;
	ULARGE_INTEGER	Value;
}   OLS_WRITE_MSR_INPUT;

typedef struct  _OLS_WRITE_IO_PORT_INPUT {
	ULONG	PortNumber; 
	union {
		ULONG   LongData;
		USHORT  ShortData;
		UCHAR   CharData;
	};
}   OLS_WRITE_IO_PORT_INPUT;

typedef struct  _OLS_READ_PCI_CONFIG_INPUT {
	ULONG PciAddress;
	ULONG PciOffset;
}   OLS_READ_PCI_CONFIG_INPUT;

typedef struct  _OLS_WRITE_PCI_CONFIG_INPUT {
	ULONG PciAddress;
	ULONG PciOffset;
	UCHAR Data[1];
}   OLS_WRITE_PCI_CONFIG_INPUT;

typedef LARGE_INTEGER PHYSICAL_ADDRESS;

typedef struct  _OLS_READ_MEMORY_INPUT {
	PHYSICAL_ADDRESS Address;
	ULONG UnitSize;
	ULONG Count;
}   OLS_READ_MEMORY_INPUT;

typedef struct  _OLS_WRITE_MEMORY_INPUT {
	PHYSICAL_ADDRESS Address;	 
	ULONG UnitSize;
	ULONG Count;
	UCHAR Data[1];
}   OLS_WRITE_MEMORY_INPUT;

#pragma pack(pop)