/** 加载NTFS驱动

**/
#include "ntfs.c"
#include "MyRamDisk.h"
EFI_STATUS
	LoadNtfsDriver(
	)
	{
        EFI_STATUS               		Status;
		EFI_DEVICE_PATH_PROTOCOL		*NtfsDriverFileDevicePath=NULL;
		EFI_HANDLE 						BootFileHandleInRamDisk;
		
		///卸载华硕主板的ntfs驱动
		EFI_HANDLE							NtfsDriverHandle=NULL;
		EFI_HANDLE							*Buffer;
		UINTN								BufferCount=0;
		UINTN								BufferIndex;
		EFI_COMPONENT_NAME2_PROTOCOL		*DriverNameProtocol;
		CHAR16								*DriverName=NULL;

		//下面的语句在系统中搜索ntfs驱动，然后将其卸载，因为华硕主板带的ntfs驱动兼容性不好		
		//列出所有的支持驱动名字的句柄
		Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiComponentName2ProtocolGuid,NULL,&BufferCount,&Buffer);
		if(EFI_ERROR (Status)){
			Print(L"ComponentNameProtocol not found.Error=[%r]\n",Status);
			}
		//循环缓冲区中的所有句柄	
		for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
			gBS->HandleProtocol(Buffer[BufferIndex],&gEfiComponentName2ProtocolGuid,(VOID**)&DriverNameProtocol);
			DriverNameProtocol->GetDriverName(DriverNameProtocol,"en-us",&DriverName);
			//是否NTFS驱动
			//Print(L"%s       ",DriverName);
			if(NULL!=DriverName&&
				NULL!=StrStr(DriverName,L"AMI NTFS Driver")){
					
				NtfsDriverHandle=Buffer[BufferIndex];
				Print(L"Found AMI NTFS Driver\n");
				break;
				}
			}
		//是否找到了NTFS驱动	
		if(NtfsDriverHandle!=NULL){
			Status=gBS->UnloadImage(NtfsDriverHandle);
			Print(L"Unload AMI NTFS Driver %r\n",Status);
			}else{
				Print(L"Can't find AMI NTFS Driver\n");
				}
				
					
		Print(L"Loading new NTFS driver\n");
		///获取当前目录
		NtfsDriverFileDevicePath=GetCurrDirDP(gImageHandle,L"ntfs.efi");
		if(NULL==NtfsDriverFileDevicePath){
			Print(L"Loading 'ntfs.efi' failed!\n");
			return EFI_NOT_FOUND;
			}



				
		Status=gBS->LoadImage(
			FALSE,
			gImageHandle,                   //parent不能为空，传入本文件的Handle
			NtfsDriverFileDevicePath,						//文件的devicepath
			NULL,
			0,
			(VOID**)&BootFileHandleInRamDisk				//传入HANDLE地址	
			);
		if(EFI_ERROR(Status)){
			Print(L"Loading 'ntfs.efi' failed!\n");
			return EFI_NOT_FOUND;
			}			

		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		if(EFI_ERROR (Status)) {
			Print(L"Start 'ntfs.efi' failed!\n");
			return EFI_NOT_FOUND;
			}
			
		return EFI_SUCCESS;		
	}			