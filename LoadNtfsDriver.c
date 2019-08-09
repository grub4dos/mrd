/** 加载NTFS驱动

**/
#include "ntfs.c"
#include "MyRamDisk.h"
EFI_STATUS
	LoadNtfsDriver(
	)
	{
        EFI_STATUS               		Status;
//		EFI_FILE_HANDLE					NtfsDriverHandle;
		EFI_HANDLE 						BootFileHandleInRamDisk;
		
		
		Print(L"Loading NTFS driver\n");
				
		Status=gBS->LoadImage(
			FALSE,
			gImageHandle,                   //parent不能为空，传入本文件的Handle
			NULL,						//文件的devicepath
			(VOID*)acntfs,
			(UINTN)&endofntfsdriver-(UINTN)acntfs,
			(VOID**)&BootFileHandleInRamDisk				//传入HANDLE地址	
			);
		if(EFI_ERROR(Status)){
			Print(L"Loading NTFS driver failed!\n");
			return EFI_NOT_FOUND;
			}			

		Status=gBS->StartImage(	BootFileHandleInRamDisk,0,	NULL);
		if(EFI_ERROR (Status)) {
			Print(L"Start NTFS driver failed!\n");
			return EFI_NOT_FOUND;
			}
			
		return EFI_SUCCESS;		
	}			