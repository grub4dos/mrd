#include "MyRamDisk.h"
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
		EFI_FILE_PROTOCOL	 				*DidoVolumeHandle=NULL,*DidoFileHandle=NULL;
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL 	*DidoTempProtocol;
		UINTN i;	
		EFI_DEVICE_PATH_PROTOCOL			*DevDevicePath;
		EFI_HANDLE							DevHandle;
		CHAR16 								*AbsFileName=NULL;
		EFI_DEVICE_PATH_PROTOCOL			*TempFileDevicePath1;
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
		//此处加入指定设备处理代码，如果指定设备，那么将路径加上绝对路径表示		

		if(OptionStatus->DevicePathToFindImage!=NULL){
			AbsFileName=AllocateZeroPool(StrnSizeS(OptionStatus->ImageFileName,MAX_FILE_NAME_STRING_SIZE)+2);
			if(IsoFileName[0]!=L'\\')
				AbsFileName[0]=L'\\';
			StrCatS(AbsFileName,StrnSizeS(OptionStatus->ImageFileName,MAX_FILE_NAME_STRING_SIZE)+2,IsoFileName);
			Print(L"DevDevicePath is:%s\n",OptionStatus->DevicePathToFindImage);
			DevDevicePath=ConvertTextToDevicePath(OptionStatus->DevicePathToFindImage);
			Status=gBS->LocateDevicePath(&gEfiBlockIoProtocolGuid,&DevDevicePath,&DevHandle);
			Status=gBS->ConnectController(DevHandle,NULL,NULL,TRUE);
			Print(L"%r",Status);
			TempFileDevicePath1=FileDevicePath(DevHandle,AbsFileName);
			
			DidoFileHandle=OpenFileByDevicePath(TempFileDevicePath1);
			FreePool(AbsFileName);
			FreePool(TempFileDevicePath1);

			if(DidoFileHandle!=NULL)
				return DidoFileHandle;
			}		
		
		
		
	

		
		
		if(IsoFileName[0]!=L'\\'){
			Status=CurrDirHandle->Open(CurrDirHandle,&DidoFileHandle,IsoFileName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
			if(EFI_ERROR (Status)){
				Print(L"Open image file fail\n");
				return NULL;
				}else{
					Print(L"Open image file success\n");
					return DidoFileHandle;
					}
			}
		//处理绝对路径	
		//打开自己的映像DP信息
		Status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&ThisFileLIP);
		if(EFI_ERROR (Status)){
			Print(L"LoadedImageProtocol not found.Error=[%r]\n",Status);
			return NULL;
			}
		Status=gBS->HandleProtocol(ThisFileLIP->DeviceHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&DidoTempProtocol);
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
		
		Status=DidoVolumeHandle->Open(DidoVolumeHandle,&DidoFileHandle,IsoFileName,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
		if(EFI_ERROR (Status)){
			Print(L"Image file name is wrong.Error=[%r]\n",Status);
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