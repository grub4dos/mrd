#include "MyRamDisk.h"
///打开参数指定的文件或目录并返回句柄	
EFI_FILE_HANDLE
	OpenFileByDevicePath(
		IN 		EFI_DEVICE_PATH_PROTOCOL 			*CurrDevicePath			//当前设备的设备路径
		)
		{
			EFI_STATUS								Status;
			EFI_DEVICE_PATH_PROTOCOL				*NextNodeDP;	
			EFI_HANDLE 								CurrDeviceHandle;		//当前设备的句柄			
			EFI_SIMPLE_FILE_SYSTEM_PROTOCOL 		*DidoTempProtocol;
			EFI_FILE_PROTOCOL 						*DidoVolumeHandle=NULL,*DidoDirHandle,*DidoFileHandle=NULL;

			//分解DP
			Status=gBS->LocateDevicePath(&gEfiSimpleFileSystemProtocolGuid,&CurrDevicePath,&CurrDeviceHandle);
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
			//打开第一节点目录
			
			Status=DidoVolumeHandle->Open(DidoVolumeHandle,&DidoDirHandle,((FILEPATH_DEVICE_PATH*)CurrDevicePath)->PathName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
			if(EFI_ERROR (Status)){
				Print(L"Open CurrDir1 failed.Error=[%r]\n",Status);
				return DidoVolumeHandle;
				}
			//测试第二节点	
			NextNodeDP=NextDevicePathNode(CurrDevicePath);	
			if(MEDIA_DEVICE_PATH!=NextNodeDP->Type||
				MEDIA_FILEPATH_DP!=NextNodeDP->SubType||
				0==StrLen(((FILEPATH_DEVICE_PATH*)NextNodeDP)->PathName)){
				Print(L"Node2 not open\n");	
				return 	DidoDirHandle;
				}
			//打开第二节点目录或者文件
			Status=DidoDirHandle->Open(DidoDirHandle,&DidoFileHandle,((FILEPATH_DEVICE_PATH*)NextNodeDP)->PathName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
			if(EFI_ERROR (Status)){
				Print(L"Open Node2 failed.Error=[%r]\n",Status);
				return DidoDirHandle;
				}
			return	DidoFileHandle;
	
			
		}