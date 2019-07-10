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

#include 	<IndustryStandard/ElTorito.h>


//
// Block size for RAM disk
//
#define RAM_DISK_BLOCK_SIZE 2048        //被我修改以适应虚拟光驱 原来是512
#define CD_BOOT_SECTOR	17
#define IS_EFI_SYSTEM_PARTITION 239
#define DIDO_DISK_PRIVATE_DATA_BLOCKIO_TO_PARENT(a) ((DIDO_DISK_PRIVATE_DATA *)((CHAR8 *)(a)-(CHAR8 *) &(((DIDO_DISK_PRIVATE_DATA *) 0)->BlockIo)))
#define DIDO_DISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(a) ((DIDO_DISK_PRIVATE_DATA *)((CHAR8 *)(a)-(CHAR8 *) &(((DIDO_DISK_PRIVATE_DATA *) 0)->BlockIo2)))

// {E4F3CE96-064F-4DA5-8C71-33232B9D9F2A}
static const EFI_GUID MyGuid = 
{ 0xe4f3ce96, 0x64f, 0x4da5, { 0x8c, 0x71, 0x33, 0x23, 0x2b, 0x9d, 0x9f, 0x2a } };




typedef EFI_STATUS (*FUN)(   IN EFI_HANDLE ,  IN EFI_SYSTEM_TABLE*);



typedef struct {

	///整个光盘的信息
	BOOLEAN							Present;					//不同
	EFI_HANDLE                      VirDiskHandle;				//不同
	EFI_DEVICE_PATH_PROTOCOL        *VirDiskDevicePath;			//不同
	EFI_FILE_HANDLE				  	VirDiskFileHandle;			//相同
	BOOLEAN						  	InRam;						//相同
	UINT64                          StartAddr;					//不同
	UINT64                          Size;						//不同
	
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





///创建虚拟盘
EFI_STATUS
	MyFileDiskInstall(
		IN 	EFI_FILE_HANDLE 	FileDiskFileHandle,
		IN	BOOLEAN				DiskInRam
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
///载入bootx64.efi准备启动
EFI_STATUS
LoadBootFileInVirtualDisk(
	IN		EFI_DEVICE_PATH_PROTOCOL			*RegedRamDiskDevicePath,
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

///根据配置文件名打开iso文件句柄
EFI_FILE_HANDLE
	ProcCfgFile(
		EFI_DEVICE_PATH_PROTOCOL		*CurrDirDP,
		EFI_FILE_HANDLE					CurrDirHandle,
		IN 		 CHAR16					*ConfigFileName			//配置文件名
		);
///找出分区信息，返回到后4个参数中
EFI_STATUS
	FindPartitionInFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINT64					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINT64					*BootStartAddr,
		OUT		UINT64					*BootSize
		);

///打开命令行指定的iso文件，返回句柄	
EFI_FILE_HANDLE
	OpenIsoFileInCmdLineStr(
//		IN	CHAR16*			CmdLine,
		EFI_FILE_HANDLE			CurrDirHandle
	);



#endif