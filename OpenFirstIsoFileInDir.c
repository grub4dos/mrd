#include "MyRamDisk.h"
///搜索并启动任意根目录的第一个iso文件，兼容pxe	
EFI_FILE_HANDLE
	OpenFirstIsoFileInDir(
		IN	EFI_FILE_HANDLE			DirToSearch
	)
	{
		EFI_STATUS							Status;
		CHAR16 								*DidoFoundStr=NULL;
		EFI_FILE_INFO 						*DidoFileInfoBuffer=NULL;
		BOOLEAN 							FoundIsoFile=FALSE;
		BOOLEAN 							DirHaveNoFile=FALSE;
		EFI_FILE_PROTOCOL					*DidoFileHandle=NULL;


		Print(L"Searching iso file in current dir\n");
		//找到根目录第一个iso文件
		Status=FileHandleFindFirstFile(DirToSearch,&DidoFileInfoBuffer);
		//简单的判断，有.iso字样则认为是iso文件	
		DidoFoundStr=StrStr(DidoFileInfoBuffer->FileName,L".iso");
		if(NULL!=DidoFoundStr){
			FoundIsoFile=TRUE;
			}	
		while(FALSE==FoundIsoFile){
			Status=FileHandleFindNextFile (DirToSearch,DidoFileInfoBuffer,&DirHaveNoFile);
			if(DirHaveNoFile){
				break;
				}
			//简单的判断，有.iso字样则认为是iso文件	
			DidoFoundStr=StrStr(DidoFileInfoBuffer->FileName,L".iso");
			if(NULL!=DidoFoundStr){
				FoundIsoFile=TRUE;
				break;
				}	
			}
		if(TRUE==FoundIsoFile){
			Print(L"Found iso file:%s\n",DidoFileInfoBuffer->FileName);	
			Status=DirToSearch->Open(DirToSearch,&DidoFileHandle,DidoFileInfoBuffer->FileName,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
			if(EFI_SUCCESS==Status){
				return DidoFileHandle;
				}		
			}
		//如果找到iso文件则释放临时内存
			
		if(NULL!=DidoFileInfoBuffer)gBS->FreePool(DidoFileInfoBuffer);
		//如果没找到，则下一个文件系统
			
			
		if(FALSE==FoundIsoFile){
			Print(L"Can not find iso file.Error=[%r]\n",Status);
			}			
		Print(L"Boot iso file failed.Error=[%r]\n",Status);
		return NULL;		
	}