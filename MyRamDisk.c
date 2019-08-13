/** @file 
	在UEFI环境下帮助iso文件直接启动，也支持ixpe启动

**/
#include "MyRamDisk.h"

///全局私有数据指针,由驱动部分访问
DIDO_DISK_PRIVATE_DATA 			*pridata;

///全局命令行参数指针
DIDO_OPTION_STATUS				*OptionStatus;

///程序入口，作为启动器，貌似不能用main作为入口
EFI_STATUS EFIAPI UefiMain(
        IN EFI_HANDLE           ImageHandle,
        IN EFI_SYSTEM_TABLE     *SystemTable
		)
	{
        EFI_STATUS               		Status;
		EFI_FILE_HANDLE					BootFileHandleInRamDisk=NULL;
		EFI_DEVICE_PATH_PROTOCOL		*CurrDirDP;
		EFI_FILE_HANDLE					IsoFileHandle;
		EFI_FILE_HANDLE					CurrDirHandle;
		
		///显示版本号
		Print(L"imgboot version 47\n");
		///初始化命令行参数状态
		OptionStatus=AllocateZeroPool(sizeof(DIDO_OPTION_STATUS));
		OptionStatus->LoadInMemory=FALSE;
		OptionStatus->DebugDropToShell=FALSE;
		OptionStatus->ImageFileType=UNKNOWNTYPE;
		OptionStatus->ImageFileName=NULL;  
		OptionStatus->WaitTimeSec=0;
		OptionStatus->DevicePathToFindImage=NULL;
		OptionStatus->UseBuildInNtfsDriver=FALSE;

		///初始化全局指针pridata,分配3个私有数据结构，第一个用于整个盘，第二个未使用，第三个用于启动分区
		pridata=AllocateZeroPool(3*sizeof(DIDO_DISK_PRIVATE_DATA)+8);

/*//测试4g边界
{///获取当前目录
		CHAR8 *buffer;
		UINTN   sss=512;
		UINTN i;
		CurrDirDP=GetCurrDirDP(gImageHandle,L"ggg.vhd");
		if(NULL==CurrDirDP){
			Print(L"GetCurrDirDP Error\n");
			return EFI_SUCCESS;
			}
			
		///打开当前目录	
		CurrDirHandle=OpenFileByDevicePath(CurrDirDP);
		if(NULL==CurrDirHandle){
			Print(L"Open CurrDir Error\n");
			return EFI_SUCCESS;
			}
		buffer=AllocateZeroPool(512);	
		FileHandleSetPosition(CurrDirHandle,0x100000000);
		FileHandleRead(CurrDirHandle,&sss,buffer);
		for(i=0;i<505;i+=8)
			Print(L"%016llX   ",*(UINTN *)(buffer+i));
		if(NULL!=CurrDirHandle)return EFI_SUCCESS;
}
//测试结束
*/

		///获取当前目录
		CurrDirDP=GetCurrDirDP(gImageHandle,L"");
		if(NULL==CurrDirDP){
			Print(L"GetCurrDirDP Error\n");
			return EFI_SUCCESS;
			}
			
		///打开当前目录	
		CurrDirHandle=OpenFileByDevicePath(CurrDirDP);
		if(NULL==CurrDirHandle){
			Print(L"Open CurrDir Error\n");
			return EFI_SUCCESS;
			}
			
		///处理命令行参数
		Status=ProcCmdLine(OptionStatus);
		if(NULL==OptionStatus->ImageFileName){	
			///处理配置文件
			Status=ProcCfgFile(OptionStatus,CurrDirHandle,L"imgboot.cfg");
			}
			
		///加载ntfs驱动
		if(OptionStatus->UseBuildInNtfsDriver)
			Status=LoadNtfsDriver();
			
		///打开指定镜像文件		
		IsoFileHandle=OpenFileInOptionStatus(OptionStatus,CurrDirHandle);	
		if(NULL==IsoFileHandle){
			//在当前目录搜索第一个iso文件并打开
			OptionStatus->ImageFileType=ISOFILE;
			IsoFileHandle=OpenFirstIsoFileInDir(CurrDirHandle);

			if(NULL==IsoFileHandle){
				Print(L"Open ISO file Error\n");
				goto errordroptoshell;
				}
			}
		
		///安装虚拟盘
		Status=FileDiskInstall(IsoFileHandle);
		if(Status==EFI_NOT_FOUND) {
			Print(L"Sorry!Can't boot this file.\n");
			goto errordroptoshell;
			}
		
		///打开虚拟盘上的启动文件
		if(pridata[2].VirDiskHandle!=NULL)
			BootFileHandleInRamDisk=LoadBootFileInVirtualDisk(pridata[2].VirDiskHandle);
		///查找启动文件
		if(BootFileHandleInRamDisk==NULL) {
			BootFileHandleInRamDisk=FindAndLoadBootFileInVirtualDisk(); 
			}
		if(BootFileHandleInRamDisk==NULL) {	
			Print(L"Sorry!Can't boot this file.\n");
			goto errordroptoshell;
			}
			
		///检查调试开关
		if(OptionStatus->DebugDropToShell==TRUE){
			Print(L"Debug drop to shell\n");
			goto errordroptoshell;
			}
		
		///必须降低运行级别才能等待，并且避免实机进入pe的logo后死机重启
		gBS->RestoreTPL(TPL_APPLICATION);		
			
		///等待WaitTimeSec秒，并降低运行级别到application级，否则实机死机重启
		DidoWaitSec(OptionStatus->WaitTimeSec);
		
		///启动
		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		Print(L"Image returned\n");
		return EFI_SUCCESS;
			
errordroptoshell:			
		///失败载入一个shellx64.efi
		Print(L"Loading shellx64.efi\n");
		CurrDirDP=GetCurrDirDP(gImageHandle,L"shellx64.efi");
		if(NULL==CurrDirDP){
			return EFI_SUCCESS;
			}		
		Status=gBS->LoadImage(
			FALSE,
			gImageHandle,                   //parent不能为空，传入本文件的Handle
			CurrDirDP,			//文件的devicepath
			NULL,
			0,
			(VOID**)&BootFileHandleInRamDisk				//传入HANDLE地址	
			);
		if(EFI_ERROR(Status)){
			Print(L"Load shellx64.efi failed!\n");
			return EFI_SUCCESS;
			}
			
		///必须降低运行级别才能等待，并且避免实机进入pe的logo后死机重启
		gBS->RestoreTPL(TPL_APPLICATION);
	
		///等待30秒
		DidoWaitSec(OptionStatus->WaitTimeSec>30?OptionStatus->WaitTimeSec:30);	
		
		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		if(EFI_ERROR (Status)) {
			Print(L"Start shellx64.efi failed!\n");
			return EFI_SUCCESS;
			}			
/**/	return EFI_SUCCESS;
	}


