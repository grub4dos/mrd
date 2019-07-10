#include "MyRamDisk.h"

///根据配置文件名打开iso文件句柄
EFI_FILE_HANDLE
	ProcCfgFile(
		EFI_DEVICE_PATH_PROTOCOL				*CurrDirDP,
		EFI_FILE_HANDLE							CurrDirHandle,
		IN 				 CHAR16					*ConfigFileName			//配置文件名
		)
		{
			EFI_STATUS							Status;
			EFI_HANDLE 							CurrDeviceHandle;		//当前设备的句柄
			EFI_FILE_HANDLE						CfgFileHandle;
			EFI_FILE_HANDLE						OutPutIsoFileHandle;
			EFI_SIMPLE_FILE_SYSTEM_PROTOCOL 	*DidoTempProtocol;
			EFI_FILE_PROTOCOL 					*DidoVolumeHandle=NULL,*DidoFileHandle;
			CHAR16								*MyTempFileName=NULL;
			BOOLEAN 							IsAscii=FALSE;
			CHAR16								*ConfigFileLine;
			//打开配置文件
			Status=CurrDirHandle->Open(CurrDirHandle,&CfgFileHandle,ConfigFileName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
			if(EFI_ERROR (Status)){
				Print(L"Can't find cfg file 'isoboot.cfg' in current dir\n");
				return NULL;
				}
			//读配置文件	
			ConfigFileLine=FileHandleReturnLine(CfgFileHandle	,&IsAscii);
			//可能要处理ASCII到UNICODE的转换
			MyTempFileName=ConfigFileLine;
			Print(L"Iso file in cfg is:%s\n",MyTempFileName);
			//将斜杠L'/'替换为反斜杠L'\\'
			for(UINTN i=0;i<StrLen(MyTempFileName);i++){
				if(MyTempFileName[i]==L'/')MyTempFileName[i]=L'\\';
				}				
			if(MyTempFileName[0]!=L'\\'){
				Status=CurrDirHandle->Open(CurrDirHandle,&OutPutIsoFileHandle,MyTempFileName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
				if(EFI_ERROR (Status)){
					return NULL;
					}else{
						return OutPutIsoFileHandle;
						}
				}
			//处理绝对路径	
			Status=gBS->LocateDevicePath(&gEfiSimpleFileSystemProtocolGuid,&CurrDirDP,&CurrDeviceHandle);
			//通过设备句柄打开找到的文件系统协议
			Status=gBS->HandleProtocol(CurrDeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&DidoTempProtocol);
			if(EFI_ERROR (Status)){
				Print(L"SimpleFileSystemProtocol not found.Error=[%r]\n",Status);
				return NULL;
				}					
			//打开根卷
			Status=DidoTempProtocol->OpenVolume(DidoTempProtocol,&DidoVolumeHandle);
			if(EFI_ERROR (Status)){
				Print(L"OpenCurrVolume failed.Error=[%r]\n",Status);
				return NULL;
				}
			
			
			//打开绝对路径指定的iso文件
			
			Status=DidoVolumeHandle->Open(DidoVolumeHandle,&DidoFileHandle,MyTempFileName,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
			if(EFI_ERROR (Status)){
				Print(L"Config File Setting Is Wrong.Error=[%r]\n",Status);
				//return EFI_SUCCESS;
				//continue;
				return NULL;
				}
			//检查文件是不是目录
			Status=FileHandleIsDirectory(DidoFileHandle);
			if(EFI_SUCCESS==Status){
				Print(L"Not a file.Error=[%r]\n",Status);
				FileHandleClose(DidoFileHandle);
				return NULL;
				}	
			return 	DidoFileHandle;
			
	

		}