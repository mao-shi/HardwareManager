#pragma once

#include <string>

HardwareManagerNamespaceBegin

#define SMART_DATA_LENGTH 362

typedef struct tagStorageDeviceProperty
{
	bool RemovableMedia; // �Ƿ�Ϊ���ƶ�ý��
	UCHAR DeviceType; // �豸����, ��SCSI�淶����
	UCHAR DeviceTypeModifier; // �豸��������, ��SCSI�淶����, Ϊ0��ʾû�и�ֵ
	STORAGE_BUS_TYPE BusType; // ��������
	std::string VendorId; // ����ID
	std::string ProductId; // ��ƷID
	std::string ProductRevision; // ��Ʒ�޶�
	std::string SerialNumber; // ���к�
} StorageDeviceProperty;

#pragma pack (1)

typedef struct tagSATA8IdentifyData
{
	USHORT GeneralConfig; // WORD 0: ������Ϣ��
	USHORT Obsolete1; // WORD 1: ����
	USHORT SpecConfig; // WORD 2: ��������
	USHORT Obsolete3; // WORD 3: ����
	USHORT Obsolete4; // WORD 4: ����
	USHORT Obsolete5; // WORD 5: ����
	USHORT Obsolete6; // WORD 6: ����
	USHORT CompactFlashReserved[2]; // WORD 7-8: ����
	USHORT Obsolete9; // WORD 9: ����
	CHAR SerialNumber[20]; // WORD 10-19:���к�
	USHORT Obsolete20[3]; // WORD 20-22: ����
	CHAR FirmwareRev[8]; // WORD 23-26: �̼��汾
	CHAR ModelNumber[40]; // WORD 27-46: �ͺ�
	USHORT Reserved47; // WORD 47: ����
	USHORT Reserved48; // WORD 48: ����
	struct 
	{
		USHORT Obsolete0:8; // bit 0-7: ����
		USHORT DMASupport:1; // bit 8: 1=֧��DMA
		USHORT LBASupport:1; // bit 9: 1=֧��LBA
		USHORT IORDYDisabled:1; // bit 10: 1=IORDY���Ա�����
		USHORT IORDYSupport:1; // bit 11: 1=֧��IORDY
		USHORT Reserved12:4; // bit 12-15: ����
	}Capabilities; // WORD 49: һ������
	USHORT Reserved50; // WORD 50: ����
	USHORT Obsolete51; // WORD 51: ����
	USHORT Obsolete52; // WORD 52: ����
	USHORT Reserved53; // WORD 53: ����
	USHORT Obsolete54[5]; // WORD 54-58: ����
	USHORT Reserved59; // WORD 59: ����
	ULONG LBATotalSectors; // WORD 60-61: LBA��Ѱַ��������
	USHORT Obsolete62; // WORD 62: ����
	struct 
	{
		USHORT Mode0:1; // bit 0: 1=֧��ģʽ0 (4.17Mb/s)
		USHORT Mode1:1; // bit 1: 1=֧��ģʽ1 (13.3Mb/s)
		USHORT Mode2:1; // bit 2: 1=֧��ģʽ2 (16.7Mb/s)
		USHORT Reserved5:5; // bit 3-7: ����
		USHORT Mode0Sel:1; // bit8: 1=��ѡ��ģʽ0
		USHORT Mode1Sel:1; // bit9: 1=��ѡ��ģʽ1
		USHORT Mode2Sel:1; // bit10: 1=��ѡ��ģʽ2
		USHORT Reserved11:5; // bit 11-15: ����
	} MultiWordDMA; // WORD 63: ���ֽ�DMA֧������
	struct 
	{
		USHORT AdvPOIModes:8; // bit 0-7: ֧�ָ߼�POIģʽ��
		USHORT Reserved8:8; // bit 8-15: ����
	} PIOCapacity; // WORD 64: �߼�PIO֧������
	USHORT MinMultiWordDMACycle; // WORD 65: ���ֽ�DMA�������ڵ���Сֵ
	USHORT RecMultiWordDMACycle; // WORD 66: ���ֽ�DMA�������ڵĽ���ֵ
	USHORT MinPIONoFlowCycle; // WORD 67: ��������ʱPIO�������ڵ���Сֵ
	USHORT MinPIOFlowCycle; // WORD 68: ��������ʱPIO�������ڵ���Сֵ
	USHORT Reserved69[7]; // WORD 69-75: ����
	struct
	{
		USHORT Reserved0:1; // bit 0: ����
		USHORT SATAGen1:1; // bit1: 1=֧��SATA Gen1(1.5Gb/s)
		USHORT SATAGen2:1; // bit2: 1=֧��SATA Gen2(3.0Gb/s)
		USHORT SATAGen3:1; // bit3: 1=֧��SATA Gen3(6.0Gb/s)
		USHORT Reserved4:12; // bit4-15: ����
	}SATACapabilities; // WORD 76: SATA����
	USHORT Reserved77; // WORD 77: ����
	struct
	{
		USHORT Reserved0: 1; // bit0: Ӧ��Ϊ0
		USHORT NoneZeroBufferOffsets: 1; // bit1: 1=�豸֧�ַ�0����ƫ��
		USHORT DMASetupAutoActivation: 1; // bit2:
		USHORT InitiatePowerManagement: 1; // bit3: 1=�豸֧�ַ����Դ����
		USHORT InorderDataDelivery: 1; // bit4:
		USHORT Reserved11: 11; // bit5-15: ����
	}SATAFeaturesSupported; // WORD 78: SATA����֧��
	struct
	{
		USHORT Reserved0: 1; // bit0: Ӧ��Ϊ0
		USHORT NoneZeroBufferOffsets: 1; // bit1: 1=��0����ƫ�ƿ���
		USHORT DMASetupAutoActivation: 1; // bit2:
		USHORT InitiatePowerManagement: 1; // bit3: 1=�����Դ������
		USHORT InorderDataDelivery: 1; // bit4:
		USHORT Reserved11: 11; // bit5-15: ����
	}SATAFeaturesEnabled; // WORD 79: SATA��������
	struct 
	{
		USHORT Reserved0:1; // bit0: ����
		USHORT Obsolete1:3; // bit1-3: ����
		USHORT ATA4:1; // bit4: 1=֧��ATA/ATAPI-4
		USHORT ATA5:1; // bit5: 1=֧��ATA/ATAPI-5
		USHORT ATA6:1; // bit6: 1=֧��ATA/ATAPI-6
		USHORT ATA7:1; // bit7: 1=֧��ATA/ATAPI-7
		USHORT ATA8:1; // bit8: 1=֧��ATA8-ACS
		USHORT Reserved9:7; // bit9-15: ����
	} MajorVersion; // WORD 80: ���汾
	USHORT MinorVersion; // WORD 81: ���汾
	USHORT Reserved82;// WORD 82: ����
	struct 
	{
		USHORT Reserved0:3; // bit0-2: ����
		USHORT AdvancedPowerManagementFeatureSetSupported:1; // bit3: 1=�߼���Դ����������֧��
		USHORT Reserved4:12; // bit4-15: ����
	}CommandSetsSupported; // WORD 83: ���֧��
	USHORT Reserved84[2]; // WORD 84-85: ����
	struct 
	{
		USHORT Reserved0:3; // bit0-2: ����
		USHORT AdvancedPowerManagementFeatureSetEnabled:1; // bit3: 1=�߼���Դ��������������
		USHORT Reserved4:12; // bit4-15: ����
	}CommandSetFeatureEnabledSupported;  // WORD 86: ���������������֧��
	USHORT Reserved87; // WORD 87: ����
	struct 
	{
		USHORT Mode0:1;                // 1=֧��ģʽ0 (16.7Mb/s)
		USHORT Mode1:1;                // 1=֧��ģʽ1 (25Mb/s)
		USHORT Mode2:1;                // 1=֧��ģʽ2 (33Mb/s)
		USHORT Mode3:1;                // 1=֧��ģʽ3 (44Mb/s)
		USHORT Mode4:1;                // 1=֧��ģʽ4 (66Mb/s)
		USHORT Mode5:1;                // 1=֧��ģʽ5 (100Mb/s)
		USHORT Mode6:1;                // 1=֧��ģʽ6 (133Mb/s)
		USHORT Reserved7:1;          // ����
		USHORT Mode0Sel:1;             // 1=��ѡ��ģʽ0
		USHORT Mode1Sel:1;             // 1=��ѡ��ģʽ1
		USHORT Mode2Sel:1;             // 1=��ѡ��ģʽ2
		USHORT Mode3Sel:1;             // 1=��ѡ��ģʽ3
		USHORT Mode4Sel:1;             // 1=��ѡ��ģʽ4
		USHORT Mode5Sel:1;             // 1=��ѡ��ģʽ5
		USHORT Mode6Sel:1;             // 1=��ѡ��ģʽ6
		USHORT Reserved15:1;          // ����
	} UltraDMA; // WORD 88:  Ultra DMA֧������
	USHORT Reserved89[2];         // WORD 89-90: ����
	struct
	{
		BYTE LevelValue; // �߼���Դ������ֵ
		BYTE Reserved;
	}AdvancePowerManagementLevel; // WORD 91: �߼���Դ������
	USHORT Reserved92[125];         // WORD 92-216
	USHORT NominalMediaRotationRate; //  WORD 217 �궨ת��(RPM), ���ֵΪ1��ʾΪSSD��������
	USHORT Reserved218[38]; // WORD 218-255 ����
} SATA8IdentifyData;

#pragma pack ()

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff553891(v=vs.85).aspx
class CGeneralStorageController
{
public:
	/// @brief ���캯��
	/// @param[in] DevicePath �豸·��
	/// �豸·����ʽΪ(C����)"////.//DeviceName"
	/// �豸������: 
	/// Ӳ���߼�����: C:, D:, E:, ...
	/// ����������: PhysicalDrive0, PhysicalDrive1, ...
	/// CD-ROM, DVD/ROM: CDROM0, CDROM1, ...
	explicit CGeneralStorageController(const std::wstring &DevicePath);
	virtual ~CGeneralStorageController();

private:
	std::wstring m_DevicePath;

private:
	CGeneralStorageController(const CGeneralStorageController&);
	CGeneralStorageController &operator = (const CGeneralStorageController&);
	
protected:
	HANDLE OpenDeviceHandle();

public:
	static bool GetLogicalDriveFreeSpace(const std::wstring &LogicalDrive, unsigned long long &FreeSpace);
	
public:
	bool IsDeviceExist();
	void ResetDevicePath(const std::wstring &DevicePath);
	bool GetDeviceProperty(StorageDeviceProperty &DevicePrperty);
	bool GetMediaType(DEVICE_MEDIA_INFO &MediaInfo);
};

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff552626(v=vs.85).aspx
class CDiskController : public CGeneralStorageController
{
public:
	CDiskController(const std::wstring &DevicePath);
	virtual ~CDiskController();

public:
	bool GetGeometry(DISK_GEOMETRY &Geometry);
	bool GetVersionInfor(GETVERSIONINPARAMS &VersionParams);// �÷���Ҫ�����֧��SMART
};

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff559105(v=vs.85).aspx
// ��Ҫ����ԱȨ��
class CIDEDiskController : public CDiskController
{
public:
	CIDEDiskController(const std::wstring &DevicePath);
	virtual ~CIDEDiskController();

private:
	/// @brief ATA����
	enum ATA_COMMAND
	{
		ATA_DEVICE_RESET = 0x08, // ��������
		ATA_READ_VERIFY_SECTOR_EXT = 0x42, // �������չ����
		ATA_EXECUTE_DEVICE_DIAGNOSTIC = 0x90, // �������
		ATA_CHECK_MEDIA_CARD_TYPE = 0xD1, // ���ý����������
		ATA_STANDBY_IMMEDIATE = 0xE0, // �������õ�ԴģʽΪSTANDBY����
		ATA_IDELE_IMMEDIATE = 0xE1, // �������õ�ԴģʽΪIDELE����
		ATA_IDENTIFY_DEVICE = 0xEC, // ��ȡID��Ϣ����
		ATA_CHECK_POWER_MODE = 0xE5, // ����Դģʽ����
		ATA_SLEEP = 0xE6, // ��������
		ATA_SET_FEATURE = 0xFE // ������������
	};
	/// @brief ATA������������������
	enum SET_FEATURE_SUB_COMMAND
	{
		SET_FEATURE_ENABLE_ADVANCED_POWER_MANAGEMENT = 0x05, // �����߼���Դ����
		SET_FEATURE_DISABLE_ADVANCED_POWER_MANAGEMENT = 0x85 // �رո߼���Դ����
	};

public:
	bool ATACmdCheckMediaCardType();
	/// @brief ����Դģʽ
	/// @param[out] mode �洢ģʽ��ֵ
	/// mode ��ֵ����: (����ATA8 SPE)
	/// 0x00 Device is in Standby mode [ʡ��ģʽ]
	/// 0x40 Device is in NV Cache Power Mode and spindle is spun down or spinning down
	/// 0x41 Device is in NV Cache Power Mode and spindle is spun up or spinning up
	/// 0x80 Device is in Idle mode
	/// 0xFF Device is in Active mode or Idle mode
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdCheckPowerMode(BYTE &Mode);
	/// @brief ֱ�����õ�ԴģʽΪIDLE
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdIDLEImmediate();
	/// @brief ֱ�����õ�ԴģʽΪStandby
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdStandbyImmediate();
	/// @brief ˯������
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdSleep();
	/// @brief ��������
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdDeviceReset();
	/// @brief �������
	/// @param[out] state �洢״ֵ̬
	/// state ��ֵΪ0x01��0x81���ʾ�豸�Ǻõ�, �����ʾ�豸�ǻ��Ļ���û������
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdExecuteDeviceDiagnostic(BYTE &State);
	/// @brief �����������չ����(û�з��ض�ȡ������, For LBA48)
	/// @param[in] lbaAddress ��Ҫ��ȡ����ʼ������ʼ��ַ
	/// @param[in] sectorCount ��Ҫ��ȡ������������
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdReadVerifySectorExt(ULONGLONG lbaAddress, unsigned long SectorCount);
	/// @brief ��ȡID��Ϣ����
	/// @param[out] identifyData �洢ID��Ϣ
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdIdentifyDevice(SATA8IdentifyData &IdentifyData);
	/// @brief ��ȡSMART��������
	/// SMART���Զ���: http://en.wikipedia.org/wiki/S.M.A.R.T.
	/// @param[out] smartData �洢SMART����
	/// @return �ɹ�����true, ʧ�ܷ���false
	bool ATACmdSMARTReadData(unsigned char SmartData[SMART_DATA_LENGTH]);
};

HardwareManagerNamespaceEnd
