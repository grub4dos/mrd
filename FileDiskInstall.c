

#include "MyRamDisk.h"

extern DIDO_OPTION_STATUS				*OptionStatus;


//区分镜像类型
IMAGE_FILE_TYPE
	CheckImageType()		

		{
			MASTER_BOOT_RECORD     			*MasterBootRecord=NULL;
			UINTN							MasterBootRecordSize=FLOPPY_DISK_BLOCK_SIZE;	
			
			MasterBootRecord = AllocateZeroPool (MasterBootRecordSize);
			if (MasterBootRecord == NULL) {
				return FLOPPYFILE;
				}
			//判断MBR是否有效
			FileHandleSetPosition(pridata[0].VirDiskFileHandle,0); 	
			FileHandleRead(pridata[0].VirDiskFileHandle,&MasterBootRecordSize,MasterBootRecord);
			if(	MasterBootRecord->Signature!=MBR_SIGNATURE){
				FreePool(MasterBootRecord);
				return ISOFILE;
				}
			//判断分区类型，如果是mbr则处理后返回
			if(MasterBootRecord->Partition[0].OSIndicator!=PMBR_GPT_PARTITION){
				FreePool(MasterBootRecord);
				return MBR;	
				}else{
					FreePool(MasterBootRecord);
					return GPT;
					}
		}


//处理内存盘的协议安装
EFI_STATUS
	FileDiskInstall(
		IN 	EFI_FILE_HANDLE 	FileDiskFileHandle
	)
	{
		EFI_STATUS 							Status=EFI_SUCCESS;
		EFI_DEVICE_PATH_PROTOCOL 			*TempPath1;
		EFI_DEVICE_PATH_PROTOCOL			*VirDevicePath;
		UINTN								StartAddr;
		UINT64								Size;
		IMAGE_FILE_TYPE						RealImageType;
		

		
		pridata[0].VirDiskFileHandle=FileDiskFileHandle;		
		RealImageType=CheckImageType();		
		
		//决定块大小
		if(OptionStatus->ImageFileType==ISOFILE||RealImageType==ISOFILE){
			pridata[0].BlockSize=CD_BLOCK_SIZE;
			}else{
				pridata[0].BlockSize=FLOPPY_DISK_BLOCK_SIZE;
				}

		//未指定-type参数
		if(OptionStatus->ImageFileType==UNKNOWNTYPE||OptionStatus->ImageFileType==HARDDISKFILE){
			OptionStatus->ImageFileType=RealImageType;
			}		
				
		//获取文件大小
		Status=FileHandleGetSize(FileDiskFileHandle,&Size);
		Print(L"ImageSize is %ld byte\n",Size);
		//是否需要载入内存
		if(OptionStatus->LoadInMemory){				
			//分配内存
			StartAddr = (UINTN)AllocatePool ((UINTN)Size+8); 
			if(NULL==(VOID*)StartAddr) {
				Print(L"Allocate Memory failed!\n");
				return EFI_NOT_FOUND;
				}
			Print(L"Creating ramdisk.Please wait.\n");
			//全读iso文件
			FileHandleSetPosition(FileDiskFileHandle,0);	
			Status=FileHandleRead(FileDiskFileHandle,(UINTN*)&Size,(VOID*)StartAddr);
			if(EFI_ERROR (Status)){
				Print(L"ReadFile failed.Error=[%r]\n",Status);
				Status=gBS->FreePool((VOID*)StartAddr);
				return EFI_NOT_FOUND;
				}
			//不载入内存则StartAddr代表文件起始偏移量
			}else{
				StartAddr=0;
				}
				

					
						
		//安装整个硬盘镜像的DP
		TempPath1=CreateDeviceNode  ( HARDWARE_DEVICE_PATH ,HW_VENDOR_DP ,  sizeof(VENDOR_DEVICE_PATH));
		((VENDOR_DEVICE_PATH*)TempPath1)->Guid=MyGuid;
		VirDevicePath=AppendDevicePathNode(NULL,TempPath1);
		if(TempPath1!=NULL)FreePool(TempPath1);

		///初始磁盘的私有数据
		pridata[0].Present=TRUE;
		pridata[0].VirDiskHandle=NULL;
		pridata[0].VirDiskDevicePath=VirDevicePath;	
//		pridata[0].VirDiskFileHandle=FileDiskFileHandle;
		pridata[0].InRam=OptionStatus->LoadInMemory;
		pridata[0].StartAddr= StartAddr;
		pridata[0].Size = Size;
		pridata[0].ImageType= OptionStatus->ImageFileType;
		pridata[0].AltDiskSign=OptionStatus->AltDiskSign;
		
		//blockio的初始化
		CopyMem (&pridata[0].BlockIo, &mFileDiskBlockIoTemplate, sizeof (EFI_BLOCK_IO_PROTOCOL));
		CopyMem (&pridata[0].BlockIo2, &mFileDiskBlockIo2Template, sizeof (EFI_BLOCK_IO2_PROTOCOL));
		
		//Media的初始化
		pridata[0].BlockIo.Media          		= &pridata[0].Media;
		pridata[0].BlockIo2.Media      	 		= &pridata[0].Media;
		pridata[0].Media.MediaId		  		= VIRTUAL_MEDIA_ID;	
		pridata[0].Media.RemovableMedia   		= FALSE;				
		pridata[0].Media.MediaPresent     		= TRUE;
		pridata[0].Media.LogicalPartition 		= FALSE;						
		pridata[0].Media.ReadOnly         		= TRUE;								
		pridata[0].Media.WriteCaching     		= FALSE;
		pridata[0].Media.IoAlign 		  		= 16;	//不添加这个参数不能被diskio识别,小于16在ntfs vdf,ntfs盘上无法找到bootx64.efi
		pridata[0].Media.BlockSize        		= pridata[0].BlockSize;
		pridata[0].Media.LastBlock        		= DivU64x32 (pridata[0].Size + pridata[0].BlockSize - 1, pridata[0].BlockSize) - 1;

		//安装分区的块协议
		if(pridata[0].ImageType!=FLOPPYFILE){
			Status=PartitionInstall();
			if(EFI_ERROR (Status)) {
				Print(L"No bootable partition on the disk image!\n");
				}					
			}
		//将磁盘放到分区后面来安装是因为virtualbox的固件下，生成的分区有时候有错误
		//安装磁盘的块协议		
		Print(L"installing disk blockio protocol\n");
		Status=gBS->InstallMultipleProtocolInterfaces (
			&pridata[0].VirDiskHandle,
			
			&gEfiDevicePathProtocolGuid,
			pridata[0].VirDiskDevicePath,
			&gEfiBlockIoProtocolGuid,
			&pridata[0].BlockIo,
			&gEfiBlockIo2ProtocolGuid,
			&pridata[0].BlockIo2,

			NULL
			);		
		if(EFI_ERROR(Status)){
			Print(L"%r\n",Status);
			Exit(Status);
			}
		gBS->ConnectController (pridata[0].VirDiskHandle, NULL, NULL, TRUE);
		

		return EFI_SUCCESS;
	}