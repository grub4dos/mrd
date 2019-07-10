#include "MyRamDisk.h"
///检查启动分区是否成功安装

EFI_HANDLE
	FindBootPartitionHandle()
	{
		EFI_STATUS							Status;
		UINTN 								BufferCount=0;
		INTN								BufferIndex=0;
		EFI_HANDLE 							*Buffer=NULL;
		EFI_HANDLE 							FoundDeviceHandle=NULL;
		EFI_DEVICE_PATH_PROTOCOL			*FoundDevicePathToCmp;
		CHAR16								*TextFoundDevicePathToCmp=NULL;
		///根据MyGuid，以及是否有SFS,以及正确的分区dp,来确定前面生成的
		
		//列出所有的简单文件系统设备
		Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&BufferCount,&Buffer);
		if(EFI_ERROR (Status)){
			Print(L"SimpleFileSystemProtocol not found.Error=[%r]\n",Status);
			return NULL;
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
			if(TextFoundDevicePathToCmp)gBS->FreePool(TextFoundDevicePathToCmp);
			Print(L"Handle selected none\n");
			return NULL;
			}
		Print(L"Handle selected %d\n",BufferIndex);
		Print(L"Partition DevicePath=%s\n", TextFoundDevicePathToCmp);
		if(TextFoundDevicePathToCmp)gBS->FreePool(TextFoundDevicePathToCmp);
		return FoundDeviceHandle;
	}