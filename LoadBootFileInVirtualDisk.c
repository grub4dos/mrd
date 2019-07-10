#include "MyRamDisk.h"
///读取句柄指定的ISO文件，并注册成ramdisk，然后载入ramdisk里面的bootx64.efi
EFI_STATUS
LoadBootFileInVirtualDisk(
	IN		EFI_DEVICE_PATH_PROTOCOL			*RegedRamDiskDevicePath,
	OUT		EFI_FILE_HANDLE						*BootMyFileHandle
	)
	{
		EFI_STATUS 							Status;

		UINTN 								BufferCount=0;
		INTN								BufferIndex=0;
		EFI_HANDLE 							*Buffer=NULL;
		EFI_HANDLE 							FoundDeviceHandle=NULL;
		EFI_DEVICE_PATH_PROTOCOL			*FoundDevicePathToCmp;
		CHAR16	                           	*TextRamDiskDevicePathToBoot=NULL;
		CHAR16								*TextFoundDevicePathToCmp=NULL;
		EFI_DEVICE_PATH_PROTOCOL			*BootFileDevicePath2;
		
		//显示虚拟盘的DP
		TextRamDiskDevicePathToBoot=ConvertDevicePathToText(RegedRamDiskDevicePath, FALSE, FALSE); 
		Print(L"RamDisk DevicePath=%s\n", TextRamDiskDevicePathToBoot);

		
		
		//列出所有的简单文件系统设备
		Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&BufferCount,&Buffer);
		if(EFI_ERROR (Status)){
			Print(L"SimpleFileSystemProtocol not found.Error=[%r]\n",Status);
			return Status;
			}
		Print(L"Handles found %d\n",BufferCount);	
			
		for(BufferIndex=BufferCount-1;BufferIndex>=0;BufferIndex--){
			//将句柄转换成DevicePath
			if(TextFoundDevicePathToCmp)gBS->FreePool(TextFoundDevicePathToCmp);
			FoundDevicePathToCmp = DevicePathFromHandle  ( Buffer[BufferIndex]  );
			TextFoundDevicePathToCmp=ConvertDevicePathToText(FoundDevicePathToCmp,FALSE,FALSE);
			//测试父节点，不同则下一个
			if(((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Header.Type!=HARDWARE_DEVICE_PATH||
				((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Header.SubType!=HW_VENDOR_DP||
				!CompareGuid(&((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Guid,&MyGuid))continue;
			//测试子节点，不同则下一个	
			FoundDevicePathToCmp=NextDevicePathNode(FoundDevicePathToCmp);	
			if(((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->Header.Type!=MEDIA_DEVICE_PATH||
				((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->Header.SubType!=MEDIA_CDROM_DP||
				((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->BootEntry!=1)continue;
			FoundDeviceHandle=Buffer[BufferIndex];
			break;
			}
		//没找到返回错误	
		if(NULL==FoundDeviceHandle){
			
			return EFI_LOAD_ERROR;
			}

		Print(L"Handle selected %d\n",BufferIndex);
		Print(L"Partition DevicePath=%s\n", TextFoundDevicePathToCmp);
		if(TextFoundDevicePathToCmp)gBS->FreePool(TextFoundDevicePathToCmp);
			
		///载入内存镜像中的bootx64.efi
		//组装启动文件/EFI/BOOT/BOOTX64.EFI的devicepath
		BootFileDevicePath2=FileDevicePath(FoundDeviceHandle,EFI_REMOVABLE_MEDIA_FILE_NAME);
		TextFoundDevicePathToCmp=ConvertDevicePathToText(BootFileDevicePath2, FALSE, FALSE);
		Print(L"Boot file path is:%s\n",TextFoundDevicePathToCmp);
		Status=gBS->LoadImage(
			FALSE,
			gImageHandle,                   //parent不能为空，传入本文件的Handle
			BootFileDevicePath2,			//文件的devicepath
			NULL,
			0,
			(VOID**)BootMyFileHandle				//传入HANDLE地址	
			);				
		if(EFI_ERROR (Status)) {
			Print(L"Loadimage failed! Error=[%r]\n",Status);
			return Status;
			}
	return EFI_SUCCESS;
	}