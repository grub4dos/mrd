#include "MyRamDisk.h"

//一个通用的打开文件的函数
EFI_FILE_HANDLE
	OpenFileInDevice(
		EFI_HANDLE			DeviceHandle,
		CHAR16				*AbsFileName
	)
	{
		EFI_STATUS							Status;
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL 	*DidoTempProtocol;
		EFI_FILE_PROTOCOL	 				*DidoVolumeHandle=NULL;
		EFI_FILE_PROTOCOL					*DidoFileHandle=NULL;
	
		Status=gBS->HandleProtocol(DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&DidoTempProtocol);
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
		//打开绝对路径指定的镜像文件
		Status=DidoVolumeHandle->Open(DidoVolumeHandle,&DidoFileHandle,AbsFileName,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
		if(EFI_ERROR (Status)){
			Print(L"Open image file failed.Error=[%r]\n",Status);
			return NULL;
			}
			
		//检查文件是不是目录
		Status=FileHandleIsDirectory(DidoFileHandle);
		if(EFI_SUCCESS==Status){
			Print(L"Not a file.Error=[%r]\n",Status);
			FileHandleClose(DidoFileHandle);
			return NULL;
			}
		return DidoFileHandle;
	
	}



///打开命令行指定的iso文件，返回句柄,以及延时和是否载入内存的参数
EFI_FILE_HANDLE
	OpenFileInOptionStatus(
		DIDO_OPTION_STATUS					*OptionStatus,
		EFI_FILE_HANDLE						CurrDirHandle
	)
	{
		EFI_STATUS							Status;
		EFI_LOADED_IMAGE_PROTOCOL			*ThisFileLIP;
		CHAR16								*IsoFileName=NULL;
		EFI_FILE_PROTOCOL	 				*DidoFileHandle=NULL;
		UINTN i;	
		EFI_DEVICE_PATH_PROTOCOL			*DevDevicePath;
		EFI_HANDLE							DevHandle;
		CHAR16 								*AbsFileName=NULL;

		//参数中有文件名？	
		if(NULL==OptionStatus->ImageFileName){
			Print(L"OptionStatus didn't have a file name\n");	
			return NULL;
			}
		Print(L"File name in OptionStatus is:%s\n",OptionStatus->ImageFileName);
		//将文件名字符串复制到临时变量
		IsoFileName=AllocateCopyPool(StrnSizeS(OptionStatus->ImageFileName,MAX_FILE_NAME_STRING_SIZE),OptionStatus->ImageFileName);
		//将斜杠L'/'替换为反斜杠L'\\'
		for(i=0;i<StrLen(IsoFileName);i++){
			if(IsoFileName[i]==L'/')IsoFileName[i]=L'\\';
			}				
		//得到绝对路径
		AbsFileName=AllocateZeroPool(StrnSizeS(OptionStatus->ImageFileName,MAX_FILE_NAME_STRING_SIZE)+2);
		if(IsoFileName[0]!=L'\\')
			AbsFileName[0]=L'\\';
		StrCatS(AbsFileName,StrnSizeS(OptionStatus->ImageFileName,MAX_FILE_NAME_STRING_SIZE)+2,IsoFileName);
		
	
		///未指定设备
		if(NULL==OptionStatus->DevicePathToFindImage){
			///相对路径
			if(IsoFileName[0]!=L'\\'){
				//打开镜像文件
				Status=CurrDirHandle->Open(CurrDirHandle,&DidoFileHandle,IsoFileName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
				if(EFI_ERROR (Status)){
					Print(L"Open image file fail\n");
					return NULL;
					}
				Print(L"Open image file success\n");
				return DidoFileHandle;
				}
			
			///处理绝对路径				
			if(IsoFileName[0]==L'\\'){
				//打开自己的映像DP信息
				Status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&ThisFileLIP);
				if(EFI_ERROR (Status)){
					Print(L"LoadedImageProtocol not found.Error=[%r]\n",Status);
					return NULL;
					}
				//打开镜像文件	
				DidoFileHandle=OpenFileInDevice(ThisFileLIP->DeviceHandle,IsoFileName);
				if(DidoFileHandle==NULL)
					Print(L"Open image file fail\n");
				if(DidoFileHandle!=NULL)
					Print(L"Open image file success\n");				
				return 	DidoFileHandle;
				}			
			
			}
		
					
		///在所有文件系统中查找指定的镜像
		if(0==StrCmp(OptionStatus->DevicePathToFindImage,L"auto")||0==StrCmp(OptionStatus->DevicePathToFindImage,L"AUTO")){
			///自动搜索镜像
			UINTN							BufferIndex;
			UINTN							BufferCount;
			EFI_HANDLE 						*Buffer=NULL;
			
			//列出所有disk设备
			Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiDiskIoProtocolGuid,NULL,&BufferCount,&Buffer);
			if(EFI_ERROR (Status)){
				Print(L"DiskIo Protocol not found.Error=[%r]\n",Status);
				return NULL;
				}
			for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
				//为每个磁盘设备安装驱动
				gBS->ConnectController (Buffer[BufferIndex], NULL, NULL, TRUE);				
				}
				
			//列出所有的简单文件系统设备
			Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&BufferCount,&Buffer);
			if(EFI_ERROR (Status)){
				Print(L"SimpleFileSystem Protocol not found.Error=[%r]\n",Status);
				return NULL;
				}
			Print(L"Device handles found %d\n",BufferCount);	
			for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
				//打开镜像文件
				DidoFileHandle=OpenFileInDevice(Buffer[BufferIndex],AbsFileName);
				if(DidoFileHandle!=NULL){
					Print(L"Device handles selected %d\n",BufferIndex+1);
					break;
					}
				}
			if(DidoFileHandle==NULL)
				Print(L"Handle selected none\n");
//			if(DidoFileHandle!=NULL)
//				Print(L"Open image file success\n");				
			return DidoFileHandle;	
			}
		
		///指定设备处理代码
		Print(L"DevDevicePath is:%s\n",OptionStatus->DevicePathToFindImage);
		DevDevicePath=ConvertTextToDevicePath(OptionStatus->DevicePathToFindImage);
		Status=gBS->LocateDevicePath(&gEfiDiskIoProtocolGuid,&DevDevicePath,&DevHandle);
		gBS->ConnectController (DevHandle, NULL, NULL, TRUE);
		//打开镜像文件
		DidoFileHandle=OpenFileInDevice(DevHandle,AbsFileName);
		//检查文件打开是否正常
		if(DidoFileHandle==NULL)
			Print(L"Open image file fail\n");
		if(DidoFileHandle!=NULL)
			Print(L"Open image file success\n");		
		return DidoFileHandle;
	}
	
	