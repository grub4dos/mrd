/** @file
	在UEFI环境下帮助iso文件直接启动，也支持ixpe启动

**/
#include "MyRamDisk.h"
///全局私有数据指针,由驱动部分访问
DIDO_DISK_PRIVATE_DATA *pridata;

///程序入口，作为启动器，貌似不能用main作为入口
EFI_STATUS EFIAPI UefiMain(
        IN EFI_HANDLE           ImageHandle,
        IN EFI_SYSTEM_TABLE     *SystemTable
		)
	{
        EFI_STATUS               		Status;
		DIDO_CMDLINE_STATUS				*CmdLineStatus;
		EFI_FILE_HANDLE					BootFileHandleInRamDisk;
		EFI_DEVICE_PATH_PROTOCOL		*CurrDirDP;
		EFI_FILE_HANDLE					IsoFileHandle;
		EFI_FILE_HANDLE					CurrDirHandle;
		
		///初始化命令行参数状态
		CmdLineStatus=AllocateZeroPool(sizeof(DIDO_CMDLINE_STATUS));

		
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
		IsoFileHandle=OpenIsoFileInCmdLineStr(CmdLineStatus,CurrDirHandle);
		if(NULL==IsoFileHandle){	
			///处理配置文件
			IsoFileHandle=ProcCfgFile(CurrDirDP,CurrDirHandle,L"isoboot.cfg");
			if(NULL==IsoFileHandle){
				///在当前目录搜索第一个iso文件并打开
				IsoFileHandle=OpenFirstIsoFileInDir(CurrDirHandle);
				if(NULL==IsoFileHandle){
					Print(L"Open ISO file Error\n");
					goto errordroptoshell;
					}
				}
			}		
		///安装虚拟盘	
		Status=MyFileDiskInstall(IsoFileHandle,CmdLineStatus->LoadIsoInMemory);
		
		///打开虚拟盘上的启动文件
		
		Status=LoadBootFileInVirtualDisk(EFI_ERROR(Status)?NULL:pridata[2].VirDiskHandle,&BootFileHandleInRamDisk);
		if(EFI_ERROR (Status)) {
			Print(L"Sorry!Can't boot this iso file.\n");
			goto errordroptoshell;
			}
		///检查调试开关
		if(CmdLineStatus->DebugDropToShell==TRUE)goto errordroptoshell;
		///等待WaitTimeSec秒，并降低运行级别到application级，否则实机死机重启
		DidoWaitSec(CmdLineStatus->WaitTimeSec);
		///启动
		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		if(EFI_ERROR (Status)) {
			Print(L"Startimage failed!\n");
			goto errordroptoshell;
			}
errordroptoshell:			
		///失败载入一个shellx64.efi
		CurrDirDP=GetCurrDirDP(gImageHandle,L"shellx64.efi");
		if(NULL==CurrDirDP){
			Print(L"Start shellx64.efi failed!\n");
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
		///等待WaitTimeSec秒
		DidoWaitSec(CmdLineStatus->WaitTimeSec);	
		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		if(EFI_ERROR (Status)) {
			Print(L"Start shellx64.efi failed!\n");
			return EFI_SUCCESS;
			}			
/**/	return EFI_SUCCESS;
	}


