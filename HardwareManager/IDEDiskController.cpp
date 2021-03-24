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
	// ���豸
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	ULONG nBytes; // �洢���ص��ֽ���
	STORAGE_PROPERTY_QUERY PropertyQuery; // ��ѯ������
	STORAGE_DEVICE_DESCRIPTOR DeviceDescriptor; // ���������ṹ
	// �����Ҫ��ѯ������
	memset(&PropertyQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
	PropertyQuery.PropertyId = STORAGE_PROPERTY_ID::StorageDeviceProperty;
	PropertyQuery.QueryType = PropertyStandardQuery;
	// ��һ�β�ѯ, ��ȡ������Ҫ���ֽ���
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
	STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)malloc(DeviceDescriptor.Size);// �洢��ѯ��������
	if (!pDeviceDescriptor) {
		::CloseHandle(hDevice);
		return false;
	}
	// �ڶ��β�ѯ, ��ȡ����
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
	// ���к��Լ�����������Ҫ����ƫ����ȡ��, ���ƫ��ֵΪ0, ���ʾû�и�ֵ
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
	// ���豸
	HANDLE hDevice = OpenDeviceHandle();
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	ULONG nBytes = 0;// �洢���ص��ֽ���
	GET_MEDIA_TYPES MediaTypes; // ý�����ͽṹ
	// ��һ�β�ѯ��Ҫ�Ĵ�С
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
	ULONG RequiredBytes = sizeof(GET_MEDIA_TYPES) + MediaTypes.MediaInfoCount  *sizeof(DEVICE_MEDIA_INFO);// �洢��Ҫ���ֽ���
	// �����㹻�Ŀռ�
	GET_MEDIA_TYPES *pMediaTypes = (GET_MEDIA_TYPES*)malloc(RequiredBytes);// �洢ý������
	// �ڶ��β�ѯ, ��ȡ����
	bRet = ::DeviceIoControl(hDevice, 
		IOCTL_STORAGE_GET_MEDIA_TYPES_EX, 
		NULL, 
		0, 
		pMediaTypes, 
		RequiredBytes, 
		&nBytes, 
		NULL);
	// ��Ȼ���ܻ�ȡ������ý����Ϣ, ������ֻȡ��һ��, �Ժ��������������, 
	// ��������ر�����
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
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_CHECK_MEDIA_CARD_TYPE; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdCheckPowerMode(BYTE &Mode)
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_CHECK_POWER_MODE; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// ����ATA8 SPEC ��֪, ִ�������TaskFile�еĵ�2���ֽ�ֵΪ��Դģʽ��ֵ
	Mode = pATACmd->CurrentTaskFile[1];
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdIDLEImmediate()
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_IDELE_IMMEDIATE; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdStandbyImmediate()
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_STANDBY_IMMEDIATE; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdSleep()
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_SLEEP; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdDeviceReset()
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_DEVICE_RESET; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdExecuteDeviceDiagnostic(BYTE &State)
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_EXECUTE_DEVICE_DIAGNOSTIC; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// ��ȡ����Ĵ����е�ֵ
	// ִ�������TaskFile�еĵ�1���ֽ�ֵΪ����Ĵ�����ֵ
	State = pATACmd->CurrentTaskFile[0];
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdReadVerifySectorExt(ULONGLONG lbaAddress, unsigned long SectorCount)
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	IDEREGS *pCurrentTaskFile = (IDEREGS*)pATACmd->CurrentTaskFile;
	IDEREGS *pPreviousTaskFile = (IDEREGS*)pATACmd->PreviousTaskFile;
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_48BIT_COMMAND; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	ULONG nBytes = 0; // �洢���ص��ֽ���
	// ��������Ĵ���
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
	// ����������ͷ�Ĵ���
	pCurrentTaskFile->bDriveHeadReg = pPreviousTaskFile->bDriveHeadReg = 0xE0; // ������ͷ�Ĵ�������Ϊ0xE0��ʾʹ��LBAѰַ��ʽ
	// ���ö�ȡ������Ŀ�Ĵ���
	pCurrentTaskFile->bSectorCountReg = (BYTE)SectorCount;
	pPreviousTaskFile->bSectorCountReg = (BYTE)(SectorCount >> 8);
	// ������ʼ�Ĵ���LBA��ַ
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

	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	::CloseHandle(hDevice);
	return true;
}
bool CIDEDiskController::ATACmdIdentifyDevice(SATA8IdentifyData &IdentifyData)
{
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	pATACmd->CurrentTaskFile[6] = ATA_IDENTIFY_DEVICE; // ����Ĵ���
	ULONG nBytes = 0; // �洢���ص��ֽ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
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
	HANDLE hDevice = OpenDeviceHandle();// �豸���
	if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL) return false;
	const int DATA_BUFFER_LEN = 512; // ���ݻ��峤��
	BYTE ParamBuffer[sizeof(ATA_PASS_THROUGH_EX) + DATA_BUFFER_LEN] = { 0 }; // ����������, ATA����+���������
	ATA_PASS_THROUGH_EX *pATACmd = (ATA_PASS_THROUGH_EX*)ParamBuffer; // ATA�������
	IDEREGS *pCurrentTaskFile = (IDEREGS*)pATACmd->CurrentTaskFile;
	pATACmd->Length = sizeof(ATA_PASS_THROUGH_EX);
	pATACmd->AtaFlags = ATA_FLAGS_DATA_IN; // ��ȡ����
	pATACmd->DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX); // ���ݻ�������ƫ��ֵ
	pATACmd->DataTransferLength = DATA_BUFFER_LEN; // ���ݻ������ĳ���
	pATACmd->TimeOutValue = 3; // ����ִ�еĳ�ʱʱ��(��)
	ULONG nBytes = 0; // �洢���ص��ֽ���
	// ��������Ĵ���
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
	// ����������ͷ�Ĵ���
	pCurrentTaskFile->bDriveHeadReg = 0xA0; // ������ͷ�Ĵ�������Ϊ0xA0��ʾʹ��CHSѰַ��ʽ
	// ���������Ĵ���
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
	// ִ�������TaskFile�еĵ�7���ֽ�ֵΪ״̬�Ĵ�����ֵ, ����ֵ����֪�������ִ�н��
	// ���״̬�Ĵ�����ֵ��0λΪ1, ��ʾ�����˴���
	if (pATACmd->CurrentTaskFile[6] & 0x1) {
		::CloseHandle(hDevice);
		return false;
	}
	// ���ص�������, ǰ362������ΪSMART��������
	memcpy_s(SmartData, SMART_DATA_LENGTH, ParamBuffer + sizeof(ATA_PASS_THROUGH_EX), SMART_DATA_LENGTH);
	::CloseHandle(hDevice);
	return true;
}

HardwareManagerNamespaceEnd
