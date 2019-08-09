#include "MyRamDisk.h"
///处理命令行，将结果送入OptionStatus
EFI_STATUS
	ProcCmdLine(
		DIDO_OPTION_STATUS					*OptionStatus
		)
		{
			EFI_STATUS							Status;
			EFI_LOADED_IMAGE_PROTOCOL			*ThisFileLIP;

			//打开自己的映像DP信息
			Print(L"Process cmdline\n");		
			Status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&ThisFileLIP);
			if(EFI_ERROR (Status)){
				Print(L"LoadedImageProtocol not found.Error=[%r]\n",Status);
				return EFI_NOT_FOUND;
				}
			//字符串复制到OptionStatus	
			OptionStatus->OptionStringSizeInByte=ThisFileLIP->LoadOptionsSize;
			OptionStatus->OptionString=AllocateCopyPool(ThisFileLIP->LoadOptionsSize,ThisFileLIP->LoadOptions);
			//将字符串分解到各成员
			DispatchOptions(OptionStatus);
			FreePool(OptionStatus->OptionString);
			if(OptionStatus->ImageFileName==NULL){
				Print(L"Cmdline didn't have a file name\n");
				return EFI_NOT_FOUND;
				}		
			Print(L"Image file name is:%s\n",OptionStatus->ImageFileName);
			return EFI_SUCCESS;	
		}