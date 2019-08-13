#include "MyRamDisk.h"
///检查启动分区是否成功安装

EFI_HANDLE
	FindAndLoadBootFileInVirtualDisk()
	{
		EFI_STATUS							Status;
		UINTN 								BufferCount=0;
		UINTN								BufferIndex=0;
		EFI_HANDLE 							*Buffer=NULL;
		EFI_DEVICE_PATH_PROTOCOL			*FoundDevicePathToCmp;
		EFI_DEVICE_PATH_PROTOCOL			*BootFileDevicePath2;
		EFI_HANDLE							BootImageHandle=NULL;		
		CHAR16								*TextFoundDevicePathToCmp=NULL;
		///根据MyGuid，以及是否有efi/boot/bootx64.efi,来确定
		
		//列出所有的简单文件系统设备
		Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&BufferCount,&Buffer);
		if(EFI_ERROR (Status)){
			Print(L"SimpleFileSystemProtocol not found.Error=[%r]\n",Status);
			return NULL;
			}
		Print(L"Handles found %d\n",BufferCount);	
		for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
			//将句柄转换成DevicePath
			
			FoundDevicePathToCmp = DevicePathFromHandle  ( Buffer[BufferIndex]  );
			
			//测试父节点，不同则下一个
			if(((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Header.Type!=HARDWARE_DEVICE_PATH||
				((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Header.SubType!=HW_VENDOR_DP||
				!CompareGuid(&((VENDOR_DEVICE_PATH*)FoundDevicePathToCmp)->Guid,&MyGuid))continue;
			/*不再需要测试子节点
			//测试子节点，不同则下一个	
			FoundDevicePathToCmp=NextDevicePathNode(FoundDevicePathToCmp);	
			if(
				(((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->Header.Type!=MEDIA_DEVICE_PATH||
				((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->Header.SubType!=MEDIA_CDROM_DP||
				((CDROM_DEVICE_PATH*)FoundDevicePathToCmp)->BootEntry!=1)
				&&
				(((HARDDRIVE_DEVICE_PATH*)FoundDevicePathToCmp)->Header.Type!=MEDIA_DEVICE_PATH||
				((HARDDRIVE_DEVICE_PATH*)FoundDevicePathToCmp)->Header.SubType!=MEDIA_HARDDRIVE_DP)
				)
				continue;
			*/	
			
			///载入内存镜像中的bootx64.efi
			//组装启动文件/EFI/BOOT/BOOTX64.EFI的devicepath
			BootFileDevicePath2=FileDevicePath(Buffer[BufferIndex],EFI_REMOVABLE_MEDIA_FILE_NAME);

			Status=gBS->LoadImage(
				TRUE,
				gImageHandle,                   //parent不能为空，传入本文件的Handle
				BootFileDevicePath2,			//文件的devicepath
				NULL,
				0,
				(VOID**)&BootImageHandle				//传入HANDLE地址	
				);							
			if(EFI_ERROR(Status)){
				continue;
				}
			
			break;
			}
			
		//没找到返回错误	
		if(NULL==BootImageHandle){
			Print(L"Handle selected none\n");
			return NULL;
			}
		//显示找到的信息	
		Print(L"Handle selected %d\n",BufferIndex);
		TextFoundDevicePathToCmp=ConvertDevicePathToText(BootFileDevicePath2, FALSE, FALSE);
		Print(L"Boot file path is:%s\n",TextFoundDevicePathToCmp);
		if(NULL!=TextFoundDevicePathToCmp)FreePool(TextFoundDevicePathToCmp);
	
		return BootImageHandle;
		
		
		
	}