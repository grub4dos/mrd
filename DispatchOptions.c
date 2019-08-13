#include "MyRamDisk.h"
///分析命令行串，将结果送入OptionStatus的成员
EFI_STATUS
	DispatchOptions(
		DIDO_OPTION_STATUS					*OptionStatus
	)
	{
		EFI_STATUS							Status=EFI_SUCCESS;
		CHAR16								**Argv;
		UINTN								Argc=0;
		UINTN								StrStartPos=0;
		UINTN i,j;	

		//分解命令行，完成后可以像main函数一样使用命令行
		Argv=AllocatePool(OptionStatus->OptionStringSizeInByte/2);
		for(i=0;i<OptionStatus->OptionStringSizeInByte/2;i++){
			//找到分隔符前一个
			if(i==OptionStatus->OptionStringSizeInByte/2-1||
				((CHAR16*)OptionStatus->OptionString)[i+1]==L' '||
				((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x09'||
				((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x0a'||
				((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x0d'||
				((CHAR16*)OptionStatus->OptionString)[i+1]==L'\0'
				){
				Argv[Argc]=AllocatePool((i+2-StrStartPos)*2);
				for(j=0;j<i+1-StrStartPos;j++){
					Argv[Argc][j]=((CHAR16*)OptionStatus->OptionString)[j+StrStartPos];
					if(j==i+1-StrStartPos-1){
						Argv[Argc][j+1]=L'\0';
						}
					}
				//排除掉连续的分隔符	
				while(i<OptionStatus->OptionStringSizeInByte/2-1&&
					(((CHAR16*)OptionStatus->OptionString)[i+1]==L' '||
					((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x09'||
					((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x0a'||
					((CHAR16*)OptionStatus->OptionString)[i+1]==L'\x0d'||
					 ((CHAR16*)OptionStatus->OptionString)[i+1]==L'\0')
					){
					i=i+1;
					}						
				
//				Print(L"cmdline is %s\n",Argv[Argc]);
				Argc=Argc+1;
				StrStartPos=i+1;
//				Print(L"Argc is %d\n",Argc);
				}
			
			}


			
		///解析命令行的-mem -wait -type和-file参数
		for(i=0;i<Argc;i++){
			//防止grub2出错，提高健壮性
			UINTN StringLenth;
			StringLenth=StrnLenS(Argv[i],OptionStatus->OptionStringSizeInByte/2);
			//命令行-mem参数	
			if(	StringLenth>=4&&
				(0==StrCmp(Argv[i],L"-mem")||0==StrCmp(Argv[i],L"-MEM"))){
				OptionStatus->LoadInMemory=TRUE;
				continue;
				}

			//命令行-debug参数		
			if(	StringLenth>=6&&
				(0==StrCmp(Argv[i],L"-debug")||0==StrCmp(Argv[i],L"-DEBUG"))){
				OptionStatus->DebugDropToShell=TRUE;
				continue;
				}
			//命令行-wait参数		
			if(	StringLenth>=5&&i<Argc-1&&
				(0==StrCmp(Argv[i],L"-wait")||0==StrCmp(Argv[i],L"-WAIT"))){
				CHAR16					*TempStr;
				TempStr=AllocateZeroPool(StrSize(Argv[i+1])+2);
				TempStr[0]=L' ';
				StrCatS(TempStr, StrSize(Argv[i+1])+2,Argv[i+1]);	
				OptionStatus->WaitTimeSec=StrDecimalToUintn(TempStr);
				if(TempStr!=NULL)FreePool(TempStr);
				continue;
				}	
				
			//命令行-altsign参数		
			if(	StringLenth>=8&&i<Argc-1&&
				(0==StrCmp(Argv[i],L"-altsign")||0==StrCmp(Argv[i],L"-altsign"))){
				CHAR16					*TempStr;
				TempStr=AllocateZeroPool(StrSize(Argv[i+1])+2);
				TempStr[0]=L' ';
				StrCatS(TempStr, StrSize(Argv[i+1])+2,Argv[i+1]);	
				OptionStatus->AltDiskSign=(UINT32)StrDecimalToUintn(TempStr);
				if(TempStr!=NULL)FreePool(TempStr);
				continue;
				}					
				
			//命令行-file参数		
			if(	StringLenth>=5&&i<Argc-1&&
				(0==StrCmp(Argv[i],L"-file")||0==StrCmp(Argv[i],L"-FILE"))){
				OptionStatus->ImageFileName=AllocateCopyPool(StrSize(Argv[i+1])+2,Argv[i+1]);
				continue;
				}					
			//命令行-type参数		
			if(	StringLenth>=5&&i<Argc-1&&
				(0==StrCmp(Argv[i],L"-type")||0==StrCmp(Argv[i],L"-TYPE"))){
				CHAR16					*TempStr;
				TempStr=AllocateCopyPool(StrSize(Argv[i+1])+2,Argv[i+1]);
				if(0==StrCmp(TempStr,L"cd")||0==StrCmp(TempStr,L"CD")){
					OptionStatus->ImageFileType=ISOFILE;
					}
				if(0==StrCmp(TempStr,L"hd")||0==StrCmp(TempStr,L"HD")){
					OptionStatus->ImageFileType=HARDDISKFILE;
					}
				if(0==StrCmp(TempStr,L"fd")||0==StrCmp(TempStr,L"FD")){
					OptionStatus->ImageFileType=FLOPPYFILE;
					}	
						
				continue;
				}
			//命令行-dev参数
			if(	StringLenth>=4&&i<Argc-1&&
				(0==StrCmp(Argv[i],L"-dev")||0==StrCmp(Argv[i],L"-DEV"))){
				OptionStatus->DevicePathToFindImage=AllocateCopyPool(StrSize(Argv[i+1])+2,Argv[i+1]);
				continue;
				}
			//命令行-ntfs参数
			if(	StringLenth>=5&&
				(0==StrCmp(Argv[i],L"-ntfs")||0==StrCmp(Argv[i],L"-NTFS"))){
				OptionStatus->UseBuildInNtfsDriver=TRUE;
				continue;
				}				
								
			}
/*		
		for(i=0;i<Argc;i++){
			FreePool(argv[i]);
			}
		FreePool(argv);	
*/
		return Status;	
	}