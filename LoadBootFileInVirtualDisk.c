#include "MyRamDisk.h"
///载入句柄指定的设备(由光盘上的bootimage虚拟)中的/efi/boot/bootx64.efi，并返回启动文件的句柄
EFI_HANDLE
	LoadBootFileInVirtualDisk(
		IN		EFI_HANDLE							*BootImagePartitionHandle 
		)
		{
			EFI_STATUS 							Status;
			EFI_HANDLE							BootImageHandle;
			
			CHAR16								*TextFoundDevicePathToCmp=NULL;
			EFI_DEVICE_PATH_PROTOCOL			*BootFileDevicePath2;
			

			if(BootImagePartitionHandle==NULL){
				return NULL;
				}
					
			///载入内存镜像中的bootx64.efi
			//组装启动文件/EFI/BOOT/BOOTX64.EFI的devicepath
			BootFileDevicePath2=FileDevicePath(BootImagePartitionHandle,EFI_REMOVABLE_MEDIA_FILE_NAME);

			Status=gBS->LoadImage(
				TRUE,
				gImageHandle,                   //parent不能为空，传入本文件的Handle
				BootFileDevicePath2,			//文件的devicepath
				NULL,
				0,
				(VOID**)&BootImageHandle				//传入HANDLE地址	
				);				
			TextFoundDevicePathToCmp=ConvertDevicePathToText(BootFileDevicePath2, FALSE, FALSE);
			Print(L"Boot file path is:%s\n",TextFoundDevicePathToCmp);
			if(NULL!=TextFoundDevicePathToCmp)FreePool(TextFoundDevicePathToCmp);
			if(EFI_ERROR (Status)) {
				Print(L"Loadimage failed! Error=[%r]\n",Status);
				return NULL;
				}
				
			return BootImageHandle;
		}