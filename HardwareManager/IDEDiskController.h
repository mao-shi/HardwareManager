#pragma once

#include <string>

HardwareManagerNamespaceBegin

#define SMART_DATA_LENGTH 362

typedef struct tagStorageDeviceProperty
{
	bool RemovableMedia; // 是否为可移动媒体
	UCHAR DeviceType; // 设备类型, 由SCSI规范定义
	UCHAR DeviceTypeModifier; // 设备类型修饰, 由SCSI规范定义, 为0表示没有该值
	STORAGE_BUS_TYPE BusType; // 总线类型
	std::string VendorId; // 厂商ID
	std::string ProductId; // 产品ID
	std::string ProductRevision; // 产品修订
	std::string SerialNumber; // 序列号
} StorageDeviceProperty;

#pragma pack (1)

typedef struct tagSATA8IdentifyData
{
	USHORT GeneralConfig; // WORD 0: 基本信息字
	USHORT Obsolete1; // WORD 1: 废弃
	USHORT SpecConfig; // WORD 2: 具体配置
	USHORT Obsolete3; // WORD 3: 废弃
	USHORT Obsolete4; // WORD 4: 废弃
	USHORT Obsolete5; // WORD 5: 废弃
	USHORT Obsolete6; // WORD 6: 废弃
	USHORT CompactFlashReserved[2]; // WORD 7-8: 保留
	USHORT Obsolete9; // WORD 9: 废弃
	CHAR SerialNumber[20]; // WORD 10-19:序列号
	USHORT Obsolete20[3]; // WORD 20-22: 废弃
	CHAR FirmwareRev[8]; // WORD 23-26: 固件版本
	CHAR ModelNumber[40]; // WORD 27-46: 型号
	USHORT Reserved47; // WORD 47: 保留
	USHORT Reserved48; // WORD 48: 保留
	struct 
	{
		USHORT Obsolete0:8; // bit 0-7: 废弃
		USHORT DMASupport:1; // bit 8: 1=支持DMA
		USHORT LBASupport:1; // bit 9: 1=支持LBA
		USHORT IORDYDisabled:1; // bit 10: 1=IORDY可以被禁用
		USHORT IORDYSupport:1; // bit 11: 1=支持IORDY
		USHORT Reserved12:4; // bit 12-15: 保留
	}Capabilities; // WORD 49: 一般能力
	USHORT Reserved50; // WORD 50: 保留
	USHORT Obsolete51; // WORD 51: 废弃
	USHORT Obsolete52; // WORD 52: 废弃
	USHORT Reserved53; // WORD 53: 保留
	USHORT Obsolete54[5]; // WORD 54-58: 废弃
	USHORT Reserved59; // WORD 59: 保留
	ULONG LBATotalSectors; // WORD 60-61: LBA可寻址的扇区数
	USHORT Obsolete62; // WORD 62: 废弃
	struct 
	{
		USHORT Mode0:1; // bit 0: 1=支持模式0 (4.17Mb/s)
		USHORT Mode1:1; // bit 1: 1=支持模式1 (13.3Mb/s)
		USHORT Mode2:1; // bit 2: 1=支持模式2 (16.7Mb/s)
		USHORT Reserved5:5; // bit 3-7: 保留
		USHORT Mode0Sel:1; // bit8: 1=已选择模式0
		USHORT Mode1Sel:1; // bit9: 1=已选择模式1
		USHORT Mode2Sel:1; // bit10: 1=已选择模式2
		USHORT Reserved11:5; // bit 11-15: 保留
	} MultiWordDMA; // WORD 63: 多字节DMA支持能力
	struct 
	{
		USHORT AdvPOIModes:8; // bit 0-7: 支持高级POI模式数
		USHORT Reserved8:8; // bit 8-15: 保留
	} PIOCapacity; // WORD 64: 高级PIO支持能力
	USHORT MinMultiWordDMACycle; // WORD 65: 多字节DMA传输周期的最小值
	USHORT RecMultiWordDMACycle; // WORD 66: 多字节DMA传输周期的建议值
	USHORT MinPIONoFlowCycle; // WORD 67: 无流控制时PIO传输周期的最小值
	USHORT MinPIOFlowCycle; // WORD 68: 有流控制时PIO传输周期的最小值
	USHORT Reserved69[7]; // WORD 69-75: 保留
	struct
	{
		USHORT Reserved0:1; // bit 0: 保留
		USHORT SATAGen1:1; // bit1: 1=支持SATA Gen1(1.5Gb/s)
		USHORT SATAGen2:1; // bit2: 1=支持SATA Gen2(3.0Gb/s)
		USHORT SATAGen3:1; // bit3: 1=支持SATA Gen3(6.0Gb/s)
		USHORT Reserved4:12; // bit4-15: 保留
	}SATACapabilities; // WORD 76: SATA能力
	USHORT Reserved77; // WORD 77: 保留
	struct
	{
		USHORT Reserved0: 1; // bit0: 应该为0
		USHORT NoneZeroBufferOffsets: 1; // bit1: 1=设备支持非0缓冲偏移
		USHORT DMASetupAutoActivation: 1; // bit2:
		USHORT InitiatePowerManagement: 1; // bit3: 1=设备支持发起电源管理
		USHORT InorderDataDelivery: 1; // bit4:
		USHORT Reserved11: 11; // bit5-15: 保留
	}SATAFeaturesSupported; // WORD 78: SATA特征支持
	struct
	{
		USHORT Reserved0: 1; // bit0: 应该为0
		USHORT NoneZeroBufferOffsets: 1; // bit1: 1=非0缓冲偏移开启
		USHORT DMASetupAutoActivation: 1; // bit2:
		USHORT InitiatePowerManagement: 1; // bit3: 1=发起电源管理开启
		USHORT InorderDataDelivery: 1; // bit4:
		USHORT Reserved11: 11; // bit5-15: 保留
	}SATAFeaturesEnabled; // WORD 79: SATA特征开启
	struct 
	{
		USHORT Reserved0:1; // bit0: 保留
		USHORT Obsolete1:3; // bit1-3: 废弃
		USHORT ATA4:1; // bit4: 1=支持ATA/ATAPI-4
		USHORT ATA5:1; // bit5: 1=支持ATA/ATAPI-5
		USHORT ATA6:1; // bit6: 1=支持ATA/ATAPI-6
		USHORT ATA7:1; // bit7: 1=支持ATA/ATAPI-7
		USHORT ATA8:1; // bit8: 1=支持ATA8-ACS
		USHORT Reserved9:7; // bit9-15: 保留
	} MajorVersion; // WORD 80: 主版本
	USHORT MinorVersion; // WORD 81: 副版本
	USHORT Reserved82;// WORD 82: 保留
	struct 
	{
		USHORT Reserved0:3; // bit0-2: 保留
		USHORT AdvancedPowerManagementFeatureSetSupported:1; // bit3: 1=高级电源管理特征集支持
		USHORT Reserved4:12; // bit4-15: 保留
	}CommandSetsSupported; // WORD 83: 命令集支持
	USHORT Reserved84[2]; // WORD 84-85: 保留
	struct 
	{
		USHORT Reserved0:3; // bit0-2: 保留
		USHORT AdvancedPowerManagementFeatureSetEnabled:1; // bit3: 1=高级电源管理特征集开启
		USHORT Reserved4:12; // bit4-15: 保留
	}CommandSetFeatureEnabledSupported;  // WORD 86: 命令集或特征开启或支持
	USHORT Reserved87; // WORD 87: 保留
	struct 
	{
		USHORT Mode0:1;                // 1=支持模式0 (16.7Mb/s)
		USHORT Mode1:1;                // 1=支持模式1 (25Mb/s)
		USHORT Mode2:1;                // 1=支持模式2 (33Mb/s)
		USHORT Mode3:1;                // 1=支持模式3 (44Mb/s)
		USHORT Mode4:1;                // 1=支持模式4 (66Mb/s)
		USHORT Mode5:1;                // 1=支持模式5 (100Mb/s)
		USHORT Mode6:1;                // 1=支持模式6 (133Mb/s)
		USHORT Reserved7:1;          // 保留
		USHORT Mode0Sel:1;             // 1=已选择模式0
		USHORT Mode1Sel:1;             // 1=已选择模式1
		USHORT Mode2Sel:1;             // 1=已选择模式2
		USHORT Mode3Sel:1;             // 1=已选择模式3
		USHORT Mode4Sel:1;             // 1=已选择模式4
		USHORT Mode5Sel:1;             // 1=已选择模式5
		USHORT Mode6Sel:1;             // 1=已选择模式6
		USHORT Reserved15:1;          // 保留
	} UltraDMA; // WORD 88:  Ultra DMA支持能力
	USHORT Reserved89[2];         // WORD 89-90: 保留
	struct
	{
		BYTE LevelValue; // 高级电源管理级别值
		BYTE Reserved;
	}AdvancePowerManagementLevel; // WORD 91: 高级电源管理级别
	USHORT Reserved92[125];         // WORD 92-216
	USHORT NominalMediaRotationRate; //  WORD 217 标定转速(RPM), 如果值为1表示为SSD或者其他
	USHORT Reserved218[38]; // WORD 218-255 保留
} SATA8IdentifyData;

#pragma pack ()

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff553891(v=vs.85).aspx
class CGeneralStorageController
{
public:
	/// @brief 构造函数
	/// @param[in] DevicePath 设备路径
	/// 设备路径格式为(C语言)"////.//DeviceName"
	/// 设备名称如: 
	/// 硬盘逻辑分区: C:, D:, E:, ...
	/// 物理驱动器: PhysicalDrive0, PhysicalDrive1, ...
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
	bool GetVersionInfor(GETVERSIONINPARAMS &VersionParams);// 该方法要求磁盘支持SMART
};

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff559105(v=vs.85).aspx
// 需要管理员权限
class CIDEDiskController : public CDiskController
{
public:
	CIDEDiskController(const std::wstring &DevicePath);
	virtual ~CIDEDiskController();

private:
	/// @brief ATA命令
	enum ATA_COMMAND
	{
		ATA_DEVICE_RESET = 0x08, // 重置命令
		ATA_READ_VERIFY_SECTOR_EXT = 0x42, // 检验读扩展命令
		ATA_EXECUTE_DEVICE_DIAGNOSTIC = 0x90, // 诊断命令
		ATA_CHECK_MEDIA_CARD_TYPE = 0xD1, // 检测媒体类型命令
		ATA_STANDBY_IMMEDIATE = 0xE0, // 立即设置电源模式为STANDBY命令
		ATA_IDELE_IMMEDIATE = 0xE1, // 立即设置电源模式为IDELE命令
		ATA_IDENTIFY_DEVICE = 0xEC, // 获取ID信息命令
		ATA_CHECK_POWER_MODE = 0xE5, // 检测电源模式命令
		ATA_SLEEP = 0xE6, // 随眠命令
		ATA_SET_FEATURE = 0xFE // 设置特征命令
	};
	/// @brief ATA命令设置特征子命令
	enum SET_FEATURE_SUB_COMMAND
	{
		SET_FEATURE_ENABLE_ADVANCED_POWER_MANAGEMENT = 0x05, // 开启高级电源管理
		SET_FEATURE_DISABLE_ADVANCED_POWER_MANAGEMENT = 0x85 // 关闭高级电源管理
	};

public:
	bool ATACmdCheckMediaCardType();
	/// @brief 检测电源模式
	/// @param[out] mode 存储模式的值
	/// mode 的值如下: (查阅ATA8 SPE)
	/// 0x00 Device is in Standby mode [省电模式]
	/// 0x40 Device is in NV Cache Power Mode and spindle is spun down or spinning down
	/// 0x41 Device is in NV Cache Power Mode and spindle is spun up or spinning up
	/// 0x80 Device is in Idle mode
	/// 0xFF Device is in Active mode or Idle mode
	/// @return 成功返回true, 失败返回false
	bool ATACmdCheckPowerMode(BYTE &Mode);
	/// @brief 直接设置电源模式为IDLE
	/// @return 成功返回true, 失败返回false
	bool ATACmdIDLEImmediate();
	/// @brief 直接设置电源模式为Standby
	/// @return 成功返回true, 失败返回false
	bool ATACmdStandbyImmediate();
	/// @brief 睡眠命令
	/// @return 成功返回true, 失败返回false
	bool ATACmdSleep();
	/// @brief 重置命令
	/// @return 成功返回true, 失败返回false
	bool ATACmdDeviceReset();
	/// @brief 诊断命令
	/// @param[out] state 存储状态值
	/// state 的值为0x01或0x81则表示设备是好的, 否则表示设备是坏的或者没有连接
	/// @return 成功返回true, 失败返回false
	bool ATACmdExecuteDeviceDiagnostic(BYTE &State);
	/// @brief 检验读扇区扩展命令(没有返回读取的数据, For LBA48)
	/// @param[in] lbaAddress 需要读取的起始扇区开始地址
	/// @param[in] sectorCount 需要读取的扇区的数量
	/// @return 成功返回true, 失败返回false
	bool ATACmdReadVerifySectorExt(ULONGLONG lbaAddress, unsigned long SectorCount);
	/// @brief 获取ID信息命令
	/// @param[out] identifyData 存储ID信息
	/// @return 成功返回true, 失败返回false
	bool ATACmdIdentifyDevice(SATA8IdentifyData &IdentifyData);
	/// @brief 获取SMART数据命令
	/// SMART属性定义: http://en.wikipedia.org/wiki/S.M.A.R.T.
	/// @param[out] smartData 存储SMART数据
	/// @return 成功返回true, 失败返回false
	bool ATACmdSMARTReadData(unsigned char SmartData[SMART_DATA_LENGTH]);
};

HardwareManagerNamespaceEnd
