#include "HardwareManager.h"
#include <ntddscsi.h>

HardwareManagerNamespaceBegin

CGeneralStorageController::CGeneralStorageController(const std::wstring &DevicePath)
    : m_DevicePath(DevicePath)
{}
CGeneralStorageController::~CGeneralStorageController()
{}

HANDLE CGeneralStorageController::OpenDeviceHandle()
{
	return ::CreateFileW(m_DevicePath.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING,
		0,
		NULL);
}

bool CGeneralStorageController::GetLogicalDriveFreeSpace(const std::wstring &LogicalDrive, unsigned long long &FreeSpace)
{
	if (LogicalDrive.empty()) return false;
	ULARGE_INTEGER _FreeSpace;
	if (::GetDiskFreeSpaceExW(LogicalDrive.c_str(), &_FreeSpace, 0, 0) == FALSE) return false;
	FreeSpace = _FreeSpace.QuadPart;
	return true;
}

bool CGeneralStorageController::IsDeviceExist()
{
	HANDLE hDevice =  OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	::CloseHandle(hDevice);
	return true;
}
void CGeneralStorageController::ResetDevicePath(const std::wstring &DevicePath)
{
	m_DevicePath = DevicePath;
}
bool CGeneralStorageController::GetDeviceProperty(StorageDeviceProperty &DevicePrperty)
{
	// 打开设备
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	ULONG nBytes; // 存储返回的字节数
	STORAGE_PROPERTY_QUERY PropertyQuery; // 查询的属性
	STORAGE_DEVICE_DESCRIPTOR DeviceDescriptor; // 属性描述结构
	// 填充需要查询的属性
	memset(&PropertyQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
	PropertyQuery.PropertyId = STORAGE_PROPERTY_ID::StorageDeviceProperty;
	PropertyQuery.QueryType = PropertyStandardQuery;
	// 第一次查询, 获取属性需要的字节数
	BOOL bRet = ::DeviceIoControl(hDevice, 
		IOCTL_STORAGE_QUERY_PROPERTY, 
		&PropertyQuery, 
		sizeof(PropertyQuery), 
		&DeviceDescriptor, 
		sizeof(DeviceDescriptor), 
		&nBytes, 
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)malloc(DeviceDescriptor.Size);// 存储查询到的属性
	if (!pDeviceDescriptor) {
		::CloseHandle(hDevice);
		return false;
	}
	// 第二次查询, 获取属性
	bRet = ::DeviceIoControl(hDevice, 
		IOCTL_STORAGE_QUERY_PROPERTY, 
		&PropertyQuery, 
		sizeof(PropertyQuery), 
		pDeviceDescriptor, 
		DeviceDescriptor.Size, 
		&nBytes, 
		NULL);
	if (bRet == FALSE) {
		free(pDeviceDescriptor);
		::CloseHandle(hDevice);
		return false;
	}
	DevicePrperty.BusType = pDeviceDescriptor->BusType;
	DevicePrperty.DeviceType = pDeviceDescriptor->DeviceType;
	DevicePrperty.DeviceTypeModifier = pDeviceDescriptor->DeviceTypeModifier;
	DevicePrperty.RemovableMedia = pDeviceDescriptor->RemovableMedia == TRUE ? true : false;
	// 序列号以及其他属性需要根据偏移来取得, 如果偏移值为0, 则表示没有该值
	if (pDeviceDescriptor->SerialNumberOffset != 0) {
		DevicePrperty.SerialNumber = (char*)pDeviceDescriptor + pDeviceDescriptor->SerialNumberOffset;
	}
	if (pDeviceDescriptor->VendorIdOffset != 0) {
		DevicePrperty.VendorId = (char*)pDeviceDescriptor + pDeviceDescriptor->VendorIdOffset;
	}
	if (pDeviceDescriptor->ProductIdOffset != 0) {
		DevicePrperty.ProductId = (char*)pDeviceDescriptor + pDeviceDescriptor->ProductIdOffset;
	}
	if (pDeviceDescriptor->ProductRevisionOffset != 0) {
		DevicePrperty.ProductRevision = (char*)pDeviceDescriptor + pDeviceDescriptor->ProductRevisionOffset;
	}
	free(pDeviceDescriptor);
	::CloseHandle(hDevice);
	return true;
}
bool CGeneralStorageController::GetMediaType(DEVICE_MEDIA_INFO &MediaInfo)
{
	// 打开设备
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	ULONG nBytes = 0;// 存储返回的字节数
	GET_MEDIA_TYPES MediaTypes; // 媒体类型结构
	// 第一次查询需要的大小
	BOOL bRet = ::DeviceIoControl(hDevice, 
		IOCTL_STORAGE_GET_MEDIA_TYPES_EX, 
		NULL, 
		0, 
		&MediaTypes, 
		sizeof(MediaTypes), 
		&nBytes, 
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	ULONG RequiredBytes = sizeof(GET_MEDIA_TYPES) + MediaTypes.MediaInfoCount  *sizeof(DEVICE_MEDIA_INFO);// 存储需要的字节数
	// 分配足够的空间
	GET_MEDIA_TYPES *pMediaTypes = (GET_MEDIA_TYPES*)malloc(RequiredBytes);// 存储媒体类型
	// 第二次查询, 获取类型
	bRet = ::DeviceIoControl(hDevice, 
		IOCTL_STORAGE_GET_MEDIA_TYPES_EX, 
		NULL, 
		0, 
		pMediaTypes, 
		RequiredBytes, 
		&nBytes, 
		NULL);
	// 虽然可能获取到多组媒体信息, 但我们只取第一组, 以后如果有其他需求, 
	// 你可以重载本方法
	MediaInfo = *(pMediaTypes->MediaInfo);
	::CloseHandle(hDevice);
	free(pMediaTypes);
	return (bRet != FALSE);
}

// -----------------------------------------------------------------

CDiskController::CDiskController(const std::wstring &DevicePath) :
	CGeneralStorageController(DevicePath)
{}
CDiskController::~CDiskController()
{}

bool CDiskController::GetGeometry(DISK_GEOMETRY &Geometry)
{
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	DWORD nBytes = 0;
	BOOL bRet = ::DeviceIoControl(hDevice, 
		IOCTL_DISK_GET_DRIVE_GEOMETRY, 
		NULL, 
		0, 
		&Geometry, 
		sizeof(Geometry), 
		&nBytes, 
		NULL);
	::CloseHandle(hDevice);
	return (bRet != FALSE);
}
bool CDiskController::GetVersionInfor(GETVERSIONINPARAMS &VersionParams)
{
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	DWORD nBytes = 0;
	BOOL bRet = ::DeviceIoControl(hDevice, 
		SMART_GET_VERSION, 
		NULL, 
		0, 
		&VersionParams, 
		sizeof(VersionParams), 
		&nBytes, 
		NULL);
	::CloseHandle(hDevice);
	return (bRet != FALSE);
}

// -----------------------------------------------------------------

CIDEDiskController::CIDEDiskController(const std::wstring &DevicePath) :
	CDiskController(DevicePath)
{}
CIDEDiskController::~CIDEDiskController()
{}

bool CIDEDiskController::ATACmdCheckMediaCardType()
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_CHECK_MEDIA_CARD_TYPE; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdCheckPowerMode(BYTE &Mode)
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_CHECK_POWER_MODE; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// 查阅ATA8 SPEC 可知, 执行命令后TaskFile中的第2个字节值为电源模式的值
	Mode = pATACmd->CurrentTaskFile[1];
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdIDLEImmediate()
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_IDELE_IMMEDIATE; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdStandbyImmediate()
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_STANDBY_IMMEDIATE; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdSleep()
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_SLEEP; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdDeviceReset()
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_DEVICE_RESET; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdExecuteDeviceDiagnostic(BYTE &State)
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_EXECUTE_DEVICE_DIAGNOSTIC; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// 获取错误寄存器中的值
	// 执行命令后TaskFile中的第1个字节值为错误寄存器的值
	State = pATACmd->CurrentTaskFile[0];
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdReadVerifySectorExt(ULONGLONG lbaAddress, unsigned long SectorCount)
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	IDEREGS *pCurrentTaskFile = (IDEREGS*)pATACmd->CurrentTaskFile;
	IDEREGS *pPreviousTaskFile = (IDEREGS*)pATACmd->PreviousTaskFile;
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_48BIT_COMMAND; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	ULONG nBytes = 0; // 存储返回的字节数
	// 设置命令寄存器
	pCurrentTaskFile->bCommandReg = pPreviousTaskFile->bCommandReg = ATA_READ_VERIFY_SECTOR_EXT;
	/*	
	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  L  |  1  | DRV | HS3 | HS2 | HS1 | HS0 |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	|           |   \_____________________/
	|           |              |
	|           |              `------------ If L=0, Head Select.
	|           |                                   These four bits select the head number.
	|           |                                   HS0 is the least significant.
	|           |                            If L=1, HS0 through HS3 contain bit 24-27 of the LBA.
	|           `--------------------------- Drive. When DRV=0, drive 0 (master) is selected. 
	|                                               When DRV=1, drive 1 (slave) is selected.
	`--------------------------------------- LBA mode. This bit selects the mode of operation.
	When L=0, addressing is by 'CHS' mode.
	When L=1, addressing is by 'LBA' mode.
	*/
	// 设置驱动器头寄存器
	pCurrentTaskFile->bDriveHeadReg = pPreviousTaskFile->bDriveHeadReg = 0xE0; // 驱动器头寄存器设置为0xE0表示使用LBA寻址方式
	// 设置读取扇区数目寄存器
	pCurrentTaskFile->bSectorCountReg = (BYTE)SectorCount;
	pPreviousTaskFile->bSectorCountReg = (BYTE)(SectorCount >> 8);
	// 设置起始寄存器LBA地址
	pCurrentTaskFile->bSectorNumberReg = (BYTE)lbaAddress;
	pCurrentTaskFile->bCylLowReg = (BYTE)(lbaAddress >> 8);
	pCurrentTaskFile->bCylHighReg = (BYTE)(lbaAddress >> 16);
	pPreviousTaskFile->bSectorNumberReg = (BYTE)(lbaAddress >> 24);
	pPreviousTaskFile->bCylLowReg = (BYTE)(lbaAddress >> 32);
	pPreviousTaskFile->bCylHighReg = (BYTE)(lbaAddress >> 40);
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}

	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdIdentifyDevice(SATA8IdentifyData &IdentifyData)
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	pATACmd->CurrentTaskFile[6] = ATA_IDENTIFY_DEVICE; // 命令寄存器
	ULONG nBytes = 0; // 存储返回的字节数
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	memcpy_s(&IdentifyData, sizeof(IdentifyData), ParamBuffer + sizeof(ATA_PASS_THROUGH_EX), DATA_BUFFER_LEN);
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdSMARTReadData(unsigned char SmartData[SMART_DATA_LENGTH])
{
	HANDLE hDevice = OpenDeviceHandle();// 设备句柄
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // 数据缓冲长度
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // 参数缓冲区, ATA命令+输出缓冲区
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA命令参数
	IDEREGS *pCurrentTaskFile = (IDEREGS*)pATACmd->CurrentTaskFile;
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // 读取数据
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // 数据缓冲区的偏移值
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // 数据缓冲区的长度
	pATACmd->TimeOutValue = 3; // 命令执行的超时时间(秒)
	ULONG nBytes = 0; // 存储返回的字节数
	// 设置命令寄存器
	pCurrentTaskFile->bCommandReg = SMART_CMD;
	/*	
	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  L  |  1  | DRV | HS3 | HS2 | HS1 | HS0 |
	+-----+-----+-----+-----+-----+-----+-----+-----+
	|           |   \_____________________/
	|           |              |
	|           |              `------------ If L=0, Head Select.
	|           |                                   These four bits select the head number.
	|           |                                   HS0 is the least significant.
	|           |                            If L=1, HS0 through HS3 contain bit 24-27 of the LBA.
	|           `--------------------------- Drive. When DRV=0, drive 0 (master) is selected. 
	|                                               When DRV=1, drive 1 (slave) is selected.
	`--------------------------------------- LBA mode. This bit selects the mode of operation.
	When L=0, addressing is by 'CHS' mode.
	When L=1, addressing is by 'LBA' mode.
	*/
	// 设置驱动器头寄存器
	pCurrentTaskFile->bDriveHeadReg = 0xA0; // 驱动器头寄存器设置为0xA0表示使用CHS寻址方式
	// 设置特征寄存器
	pCurrentTaskFile->bFeaturesReg = READ_ATTRIBUTES;
	pCurrentTaskFile->bCylLowReg = SMART_CYL_LOW;
	pCurrentTaskFile->bCylHighReg = SMART_CYL_HI;
	BOOL bRet = ::DeviceIoControl(hDevice,
		IOCTL_ATA_PASS_THROUGH,
		pATACmd,
		sizeof(ATA_PASS_THROUGH_EX),
		ParamBuffer,
		sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN,
		&nBytes,
		NULL);
	if (bRet == FALSE) {
		::CloseHandle(hDevice);
		return false;
	}
	// 执行命令后TaskFile中的第7个字节值为状态寄存器的值, 检测该值可以知道命令的执行结果
	// 如果状态寄存器的值得0位为1, 表示发生了错误
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// 返回的数据中, 前362个数据为SMART属性数据
	memcpy_s(SmartData, SMART_DATA_LENGTH, ParamBuffer + sizeof(ATA_PASS_THROUGH_EX), SMART_DATA_LENGTH);
	::CloseHandle(hDevice);
	return true;
}

HardwareManagerNamespaceEnd
