#include "MyRamDisk.h"

///根据配置文件名打开iso文件句柄
EFI_STATUS
	ProcCfgFile(
		DIDO_OPTION_STATUS						*OptionStatus,	
		EFI_FILE_HANDLE							CurrDirHandle,
		IN 				 CHAR16					*ConfigFileName			//配置文件名
		)
		{
			EFI_STATUS							Status;
			EFI_FILE_HANDLE						CfgFileHandle;
			CHAR8								*ConfigFileLine;
			UINT64 								ConfigFileSize;
			//打开配置文件
			Print(L"Process config file\n");
			Status=CurrDirHandle->Open(CurrDirHandle,&CfgFileHandle,ConfigFileName,EFI_FILE_MODE_READ,EFI_FILE_DIRECTORY);
			if(EFI_ERROR (Status)){
				Print(L"Can't find cfg file 'imgboot.cfg' in current dir\n");
				return EFI_NOT_FOUND;
				}
			//读配置文件	
			FileHandleGetSize(CfgFileHandle	,&ConfigFileSize);
			ConfigFileLine=AllocateZeroPool((UINTN)ConfigFileSize+1);
			FileHandleRead(CfgFileHandle,(UINTN*)&ConfigFileSize,ConfigFileLine);
			
			//要处理ASCII到UNICODE的转换

			OptionStatus->OptionStringSizeInByte=(UINTN)ConfigFileSize*2;
			OptionStatus->OptionString=AllocateZeroPool(OptionStatus->OptionStringSizeInByte+2);
			AsciiStrToUnicodeStrS(ConfigFileLine,OptionStatus->OptionString,(UINTN)ConfigFileSize+1);
			//将字符串分解到各成员
			DispatchOptions(OptionStatus);
			FreePool(OptionStatus->OptionString);
			if(OptionStatus->ImageFileName==NULL){
				Print(L"Config file didn't have a file name\n");
				return EFI_NOT_FOUND;
				}
			Print(L"Image file name is:%s\n",OptionStatus->ImageFileName);	
			return 	EFI_SUCCESS;
			
	

		}