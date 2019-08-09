#ifndef		_MY_RAM_DISK_H_
#define 	_MY_RAM_DISK_H_

#include 	<Uefi.h>
#include 	<Library/UefiApplicationEntryPoint.h>
#include 	<Library/BaseLib.h>
#include	<Library/UefiBootServicesTableLib.h>
#include 	<Library/UefiLib.h>
#include 	<Library/DevicePathLib.h>
#include 	<Library/FileHandleLib.h>
#include 	<Library/MemoryAllocationLib.h>
#include	<Library/BaseMemoryLib.h>
#include 	<Protocol/SimpleFileSystem.h>
#include 	<Protocol/RamDisk.h>
#include 	<Protocol/DevicePathToText.h>
#include 	<Protocol/LoadedImage.h>
#include 	<Protocol/BlockIo.h>
#include 	<Protocol/BlockIo2.h>
#include 	<Protocol/DiskIo.h>
#include 	<IndustryStandard/ElTorito.h>
#include 	<IndustryStandard/Mbr.h>

//
// Block size for RAM disk
//
#define RAM_DISK_BLOCK_SIZE 2048        //被我修改以适应虚拟光驱 原来是512
#define FLOPPY_DISK_BLOCK_SIZE 512
#define BLOCK_OF_1_44MB	0xB40
#define CD_BOOT_SECTOR	17
#define IS_EFI_SYSTEM_PARTITION 239
#define DIDO_DISK_PRIVATE_DATA_BLOCKIO_TO_PARENT(a) ((DIDO_DISK_PRIVATE_DATA *)((CHAR8 *)(a)-(CHAR8 *) &(((DIDO_DISK_PRIVATE_DATA *) 0)->BlockIo)))
#define DIDO_DISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(a) ((DIDO_DISK_PRIVATE_DATA *)((CHAR8 *)(a)-(CHAR8 *) &(((DIDO_DISK_PRIVATE_DATA *) 0)->BlockIo2)))
#define MAX_FILE_NAME_STRING_SIZE 255
#define MBR_START_LBA 0
// {CF03E624-DD29-426D-86A8-CB21E97E4C9B}
static const EFI_GUID MyGuid = 
{ 0xcf03e624, 0xdd29, 0x426d, { 0x86, 0xa8, 0xcb, 0x21, 0xe9, 0x7e, 0x4c, 0x9b } };

typedef enum {
	///指出镜像文件类型
	ISOFILE,
	HARDDISKFILE,
	FLOPPYFILE
	}IMAGE_FILE_TYPE;


typedef struct {
	///命令行参数的结构
	CHAR16								*OptionString;
	UINTN								OptionStringSizeInByte;	
	BOOLEAN								LoadInMemory;
	BOOLEAN								DebugDropToShell;
	IMAGE_FILE_TYPE						ImageFileType;
	CHAR16								*DevicePathToFindImage;
	CHAR16								*ImageFileName;  
	UINTN								WaitTimeSec;
	BOOLEAN								UseBuildInNtfsDriver;
	}DIDO_OPTION_STATUS;



typedef struct {

	///虚拟设备的私有数据结构
	BOOLEAN							Present;					//不同
	EFI_HANDLE                      VirDiskHandle;				//不同
	EFI_DEVICE_PATH_PROTOCOL        *VirDiskDevicePath;			//不同
	EFI_FILE_HANDLE				  	VirDiskFileHandle;			//相同
	BOOLEAN						  	InRam;						//相同
	UINTN                          	StartAddr;					//不同
	UINT64	                        Size;						//不同
	UINT32							UniqueMbrSignature;        	//硬盘分区表前面的签名
	//以下三条由blockinit填充，其余由install填充
	EFI_BLOCK_IO_PROTOCOL           BlockIo;
	EFI_BLOCK_IO2_PROTOCOL          BlockIo2;
	EFI_BLOCK_IO_MEDIA              Media;
	


} DIDO_DISK_PRIVATE_DATA;

extern EFI_BLOCK_IO_PROTOCOL  mFileDiskBlockIoTemplate;
extern EFI_BLOCK_IO2_PROTOCOL  mFileDiskBlockIo2Template;
extern DIDO_DISK_PRIVATE_DATA *pridata;






///说明FileBlockIo的函数



EFI_STATUS
EFIAPI
FileDiskBlkIoReset (
  IN EFI_BLOCK_IO_PROTOCOL        *This,
  IN BOOLEAN                      ExtendedVerification
  );
  
EFI_STATUS
EFIAPI
FileDiskBlkIoReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL        *This,
  IN UINT32                       MediaId,
  IN EFI_LBA                      Lba,
  IN UINTN                        BufferSize,
  IN OUT VOID                     *Buffer
  );  

EFI_STATUS
EFIAPI
FileDiskBlkIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL        *This,
  IN UINT32                       MediaId,
  IN EFI_LBA                      Lba,
  IN UINTN                        BufferSize,
  IN VOID                         *Buffer
  );

EFI_STATUS
EFIAPI
FileDiskBlkIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL        *This
  );

EFI_STATUS
EFIAPI
FileDiskBlkIo2Reset (
  IN EFI_BLOCK_IO2_PROTOCOL       *This,
  IN BOOLEAN                      ExtendedVerification
  );

EFI_STATUS
EFIAPI
FileDiskBlkIo2ReadBlocksEx (
  IN     EFI_BLOCK_IO2_PROTOCOL   *This,
  IN     UINT32                   MediaId,
  IN     EFI_LBA                  Lba,
  IN OUT EFI_BLOCK_IO2_TOKEN      *Token,
  IN     UINTN                    BufferSize,
     OUT VOID                     *Buffer
  );

EFI_STATUS
EFIAPI
FileDiskBlkIo2WriteBlocksEx (
  IN     EFI_BLOCK_IO2_PROTOCOL   *This,
  IN     UINT32                   MediaId,
  IN     EFI_LBA                  Lba,
  IN OUT EFI_BLOCK_IO2_TOKEN      *Token,
  IN     UINTN                    BufferSize,
  IN     VOID                     *Buffer
  );

EFI_STATUS
EFIAPI
FileDiskBlkIo2FlushBlocksEx (
  IN     EFI_BLOCK_IO2_PROTOCOL   *This,
  IN OUT EFI_BLOCK_IO2_TOKEN      *Token
  );






///取得当前目录
EFI_DEVICE_PATH_PROTOCOL* 
	GetCurrDirDP(
		IN	EFI_HANDLE 			FileHandle,
		IN 	const 	CHAR16		*FileName
		);

///载入并分析指定的配置文件，返回打开的iso文件句柄，由主函数调用	
EFI_FILE_HANDLE
	OpenFileByDevicePath(
		IN 		EFI_DEVICE_PATH_PROTOCOL 			*CurrDevicePath			//当前设备的设备路径
		);

///载入句柄指定的设备(由光盘上的bootimage虚拟)中的/efi/boot/bootx64.efi，并返回启动文件的句柄
///如果BootImageDiskHandle为NULL，则会搜索虚拟盘
EFI_STATUS
LoadBootFileInVirtualDisk(
	IN		EFI_HANDLE							*BootImageDiskHandle, 
	OUT		EFI_FILE_HANDLE						*BootMyFileHandle
	);
///等待指定时间秒
EFI_STATUS
	DidoWaitSec(
		UINTN 		Sec
		);

///搜索并启动任意根目录的第一个iso文件，兼容pxe	
EFI_FILE_HANDLE
	OpenFirstIsoFileInDir(
		IN	EFI_FILE_HANDLE						DirToSearch
		);

///将配置文件的内容传到OptionStatus->ImageFileName
EFI_STATUS
	ProcCfgFile(
		DIDO_OPTION_STATUS						*OptionStatus,	
		EFI_FILE_HANDLE							CurrDirHandle,
		IN 				 CHAR16					*ConfigFileName			//配置文件名
		);
///找出光盘分区信息，返回到后4个参数中
EFI_STATUS
	FindPartitionInIsoFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINTN					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINTN					*BootStartAddr,
		OUT		UINT64					*BootSize
		);
///找出硬盘分区信息，返回到后4个参数中
EFI_STATUS
	FindPartitionInHdFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINTN					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINTN					*BootStartAddr,
		OUT		UINT64					*BootSize,
		OUT		UINT32					*UniqueMbrSignature	
		);
///打开OptionStatus指定的文件，返回句柄	
EFI_FILE_HANDLE
	OpenFileInOptionStatus(
		DIDO_OPTION_STATUS					*OptionStatus,
		EFI_FILE_HANDLE						CurrDirHandle
	);

///检查启动分区是否成功安装

EFI_HANDLE
	FindBootPartitionHandle();

///处理命令行，将结果送入OptionStatus
EFI_STATUS
	ProcCmdLine(
		DIDO_OPTION_STATUS					*OptionStatus
	);
///安装虚拟硬盘
EFI_STATUS
	HdFileDiskInstall(
		IN 	EFI_FILE_HANDLE 	FileDiskFileHandle,
		IN	BOOLEAN				DiskInRam
	);
///安装虚拟光盘
EFI_STATUS
	IsoFileDiskInstall(
		IN 	EFI_FILE_HANDLE 	FileDiskFileHandle,
		IN	BOOLEAN				DiskInRam
	);
///加载ntfs驱动
EFI_STATUS
	LoadNtfsDriver(
	);
	
///分析命令行串，将结果送入OptionStatus的成员
EFI_STATUS
	DispatchOptions(
		DIDO_OPTION_STATUS					*OptionStatus
	);	
	
	
	
#endif