#include "MyRamDisk.h"

//分析mbr分区
BOOLEAN
	GetMbrBootPartitionInfo()
		{
			MASTER_BOOT_RECORD     			*MasterBootRecord=NULL;
			UINTN							MasterBootRecordSize=FLOPPY_DISK_BLOCK_SIZE;	
			UINT32							UniqueMbrSignature;				
			UINTN							BootPartitionStartAddr;
			UINTN							BootPartitionSize;
			EFI_DEVICE_PATH_PROTOCOL		*TempPath1;
			UINT32							i;
			UINT32							BootPartitionNumber=0;
			//读MBR
			MasterBootRecord = AllocateZeroPool (MasterBootRecordSize);
			if (MasterBootRecord == NULL) {
				return FALSE;
				}
			FileHandleSetPosition(pridata[0].VirDiskFileHandle,MBR_START_LBA); 	
			FileHandleRead(pridata[0].VirDiskFileHandle,&MasterBootRecordSize,MasterBootRecord);
			//获取传统分区表的信息
			for(i=0;i<4;i++){
				if(MasterBootRecord->Partition[i].BootIndicator==0x80){
					BootPartitionStartAddr=*(UINT32*)(MasterBootRecord->Partition[i].StartingLBA);
					pridata[2].StartAddr=BootPartitionStartAddr*FLOPPY_DISK_BLOCK_SIZE;
					BootPartitionSize=*(UINT32*)(MasterBootRecord->Partition[i].SizeInLBA);
					pridata[2].Size=BootPartitionSize*FLOPPY_DISK_BLOCK_SIZE;
					BootPartitionNumber=i+1;
					break;
					}
				}
			if(BootPartitionNumber==0){
				FreePool(MasterBootRecord);
				return FALSE;
				}				
			UniqueMbrSignature=*(UINT32*)(MasterBootRecord->UniqueMbrSignature);

			//使用指定磁盘签名
			if(pridata[0].AltDiskSign!=0){
				UniqueMbrSignature=pridata[0].AltDiskSign;
				}
				
			
			pridata[2].StartAddr = pridata[2].StartAddr + pridata[0].StartAddr;		

			//分区的DP
			TempPath1=CreateDeviceNode  ( MEDIA_DEVICE_PATH ,MEDIA_HARDDRIVE_DP ,  sizeof(HARDDRIVE_DEVICE_PATH));
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionNumber	=BootPartitionNumber;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionStart		=BootPartitionStartAddr;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionSize		=BootPartitionSize;
			*(UINT32*)((HARDDRIVE_DEVICE_PATH*)TempPath1)->Signature=UniqueMbrSignature;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->MBRType			=1;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->SignatureType		=1;
			pridata[2].VirDiskDevicePath=AppendDevicePathNode(pridata[0].VirDiskDevicePath,TempPath1);
			FreePool(TempPath1);
			FreePool(MasterBootRecord);			
			return 	TRUE;				
		}
		
//分析gpt分区		
BOOLEAN
	GetGptBootPartitionInfo()
		{
			EFI_PARTITION_TABLE_HEADER		*GptHeader=NULL;
			UINTN							GptHeaderSize=FLOPPY_DISK_BLOCK_SIZE;			
			EFI_PARTITION_ENTRY				*GptPartitionEntry;
			UINTN							GptPartitionEntrySize;
			UINT64							GptPartitionEntryPos;
			UINT64							BootPartitionStartAddr;
			UINT64							BootPartitionSize;
			EFI_GUID						GptPartitionSignature;
			UINT32							BootPartitionNumber=0;
			EFI_DEVICE_PATH_PROTOCOL		*TempPath1;
			UINT32							i;
			//读GPT头
			GptHeader=AllocateZeroPool(GptHeaderSize);
			if(GptHeader==NULL){
				return FALSE;
				}
			FileHandleSetPosition(pridata[0].VirDiskFileHandle,PRIMARY_PART_HEADER_LBA*FLOPPY_DISK_BLOCK_SIZE);
			FileHandleRead(pridata[0].VirDiskFileHandle,&GptHeaderSize,GptHeader);
			if(GptHeader->Header.Signature!=EFI_PTAB_HEADER_ID){
				FreePool(GptHeader);
				return 	FALSE; 
				}
				
			//读分区条目	
			GptPartitionEntryPos = GptHeader->PartitionEntryLBA * FLOPPY_DISK_BLOCK_SIZE;
			GptPartitionEntrySize = GptHeader->SizeOfPartitionEntry;	
			GptPartitionEntry = AllocateZeroPool(GptHeader->SizeOfPartitionEntry * GptHeader->NumberOfPartitionEntries);
			if(GptPartitionEntry==NULL){
				return FALSE;
				}
			for(i=0;i<GptHeader->NumberOfPartitionEntries;i++){
				FileHandleSetPosition(pridata[0].VirDiskFileHandle,GptPartitionEntryPos+i*GptPartitionEntrySize);
				FileHandleRead(pridata[0].VirDiskFileHandle,&GptPartitionEntrySize,GptPartitionEntry);
				if(CompareGuid (&GptPartitionEntry->PartitionTypeGUID,&gEfiPartTypeSystemPartGuid)){
					BootPartitionStartAddr=GptPartitionEntry->StartingLBA*FLOPPY_DISK_BLOCK_SIZE;
					BootPartitionSize=(GptPartitionEntry->EndingLBA - GptPartitionEntry->StartingLBA)*FLOPPY_DISK_BLOCK_SIZE;
					CopyGuid(&GptPartitionSignature,&GptPartitionEntry->UniquePartitionGUID); 
					BootPartitionNumber=i+1;
					break;
					}
				}
			if(BootPartitionNumber==0){
				FreePool(GptPartitionEntry);
				FreePool(GptHeader);
				return FALSE;
				}				
			pridata[2].StartAddr = (UINTN)BootPartitionStartAddr + pridata[0].StartAddr;
			pridata[2].Size = BootPartitionSize	;

			//分区的DP
			TempPath1=CreateDeviceNode  ( MEDIA_DEVICE_PATH ,MEDIA_HARDDRIVE_DP ,  sizeof(HARDDRIVE_DEVICE_PATH));
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionNumber	=BootPartitionNumber;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionStart		=BootPartitionStartAddr;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->PartitionSize		=BootPartitionSize;
			CopyGuid((EFI_GUID *)&(((HARDDRIVE_DEVICE_PATH*)TempPath1)->Signature[0]),&GptPartitionSignature);
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->MBRType			=2;
			((HARDDRIVE_DEVICE_PATH*)TempPath1)->SignatureType		=2;
			pridata[2].VirDiskDevicePath=AppendDevicePathNode(pridata[0].VirDiskDevicePath,TempPath1);
			FreePool(TempPath1);
			FreePool(GptPartitionEntry);
			FreePool(GptHeader);			
			return TRUE;	
		}
		
//分析iso分区		
BOOLEAN
	GetIsoBootPartitionInfo()
		{
			CDROM_VOLUME_DESCRIPTOR     *VolDescriptor=NULL;
			ELTORITO_CATALOG            *TempCatalog=NULL;
			UINTN						DescriptorSize=CD_BLOCK_SIZE;	
			UINTN						DbrImageSize=2;
			UINT16						DbrImageSizeBuffer;
			BOOLEAN						FoundBootEntry=FALSE;
			UINTN						i;
			EFI_DEVICE_PATH_PROTOCOL	*TempPath1;
//			EFI_DEVICE_PATH_PROTOCOL	*TempPath2;			
			VolDescriptor = AllocatePool (DescriptorSize);
			if (VolDescriptor == NULL) {
				return FALSE;
				}
			//判断卷
			FileHandleSetPosition(pridata[0].VirDiskFileHandle,CD_BOOT_SECTOR*CD_BLOCK_SIZE); 	
			FileHandleRead(pridata[0].VirDiskFileHandle,&DescriptorSize,VolDescriptor);
			if(	VolDescriptor->Unknown.Type!=CDVOL_TYPE_STANDARD||
				CompareMem (VolDescriptor->BootRecordVolume.SystemId, CDVOL_ELTORITO_ID, sizeof (CDVOL_ELTORITO_ID) - 1) != 0){
				FreePool(VolDescriptor);
				return FALSE;
				}
			//判断启动目录	
			TempCatalog = (ELTORITO_CATALOG*)VolDescriptor;
			FileHandleSetPosition(pridata[0].VirDiskFileHandle,*((UINT32*)VolDescriptor->BootRecordVolume.EltCatalog)*CD_BLOCK_SIZE); 	
			FileHandleRead(pridata[0].VirDiskFileHandle,&DescriptorSize,TempCatalog);	
			if( TempCatalog[0].Catalog.Indicator!=ELTORITO_ID_CATALOG){
				FreePool(VolDescriptor);
				return FALSE;
				}
			//判断启动卷	
			for(i=0;i<64;i++){
				if( TempCatalog[i].Section.Indicator==ELTORITO_ID_SECTION_HEADER_FINAL&&
					TempCatalog[i].Section.PlatformId==IS_EFI_SYSTEM_PARTITION&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE ){
					FoundBootEntry=TRUE;	
					pridata[2].StartAddr	=TempCatalog[i+1].Boot.Lba*CD_BLOCK_SIZE;
					pridata[2].Size		=TempCatalog[i+1].Boot.SectorCount*FLOPPY_DISK_BLOCK_SIZE;
					
					//有些光盘的映像大小设定不正确，太小就读取软盘映像的dbr
					FileHandleSetPosition(pridata[0].VirDiskFileHandle,pridata[2].StartAddr+0x13); 	
					FileHandleRead(pridata[0].VirDiskFileHandle,&DbrImageSize,&DbrImageSizeBuffer);
					DbrImageSize=DbrImageSizeBuffer*FLOPPY_DISK_BLOCK_SIZE;
					pridata[2].Size=pridata[2].Size>DbrImageSize?pridata[2].Size:DbrImageSize;
						
					//仍然太小则设为固定值	
					if(pridata[2].Size<BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE){
						pridata[2].Size=BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE;
						}
					break;	
					}
				}
			if(FoundBootEntry==FALSE){
				FreePool(VolDescriptor);
				return FALSE;
				}
			pridata[2].StartAddr = pridata[2].StartAddr + pridata[0].StartAddr;
								
			//efi启动映像，一般是一个包含/efi/boot/bootx64.efi的软盘
			TempPath1=CreateDeviceNode  ( MEDIA_DEVICE_PATH ,MEDIA_CDROM_DP ,  sizeof(CDROM_DEVICE_PATH));		
			((CDROM_DEVICE_PATH*)TempPath1)->BootEntry=1;
			((CDROM_DEVICE_PATH*)TempPath1)->PartitionStart=(pridata[2].StartAddr-pridata[0].StartAddr)/CD_BLOCK_SIZE;
			((CDROM_DEVICE_PATH*)TempPath1)->PartitionSize=pridata[2].Size/CD_BLOCK_SIZE;
			pridata[2].VirDiskDevicePath=AppendDevicePathNode(pridata[0].VirDiskDevicePath,TempPath1);
			FreePool(TempPath1);
			FreePool(VolDescriptor);
			return 	TRUE;	
		}
		


///安装启动分区的块协议
EFI_STATUS
	PartitionInstall()
		{
			EFI_STATUS					Status;
			
	
			//根据不同的分区类型获取分区信息
			/*
			由这些函数填写	VirDiskDevicePath
							StartAddr
							Size
			*/			
			switch(pridata[0].ImageType){
				case ISOFILE:
					pridata[2].Present=GetIsoBootPartitionInfo();
					Print(L"Found ISO Partition\n");
					break;
				case MBR:
					pridata[2].Present=GetMbrBootPartitionInfo();
					Print(L"Found MBR Partition\n");
					break;
				case GPT:
					pridata[2].Present=GetGptBootPartitionInfo();
					Print(L"Found GPT Partition\n");
					break;
				case UNKNOWNTYPE:
					break;
				}
				
			
			//检查分区分析结果
			if(pridata[2].Present==FALSE)
				return EFI_NOT_FOUND;
			
			///	开始安装分区
			///初始分区的私有数据
//			pridata[2].Present=TRUE;
			pridata[2].VirDiskHandle=NULL;
//			pridata[2].VirDiskDevicePath=VirDevicePath;	
			pridata[2].VirDiskFileHandle=pridata[0].VirDiskFileHandle;
			pridata[2].InRam=pridata[0].InRam;
//			pridata[2]StartAddr= StartAddr;
//			pridata[2].Size = Size;
			pridata[2].ImageType=pridata[0].ImageType;
			pridata[2].AltDiskSign=0;
			
			//blockio的初始化
			CopyMem (&pridata[2].BlockIo, &mFileDiskBlockIoTemplate, sizeof (EFI_BLOCK_IO_PROTOCOL));
			CopyMem (&pridata[2].BlockIo2, &mFileDiskBlockIo2Template, sizeof (EFI_BLOCK_IO2_PROTOCOL));
			
			//Media的初始化
			pridata[2].BlockIo.Media          		= &pridata[2].Media;
			pridata[2].BlockIo2.Media      	 		= &pridata[2].Media;
			pridata[2].Media.MediaId		  		= VIRTUAL_MEDIA_ID;	
			pridata[2].Media.RemovableMedia   		= FALSE;				
			pridata[2].Media.MediaPresent     		= TRUE;
			pridata[2].Media.LogicalPartition 		= TRUE;						
			pridata[2].Media.ReadOnly         		= TRUE;								
			pridata[2].Media.WriteCaching     		= FALSE;
			pridata[2].Media.IoAlign 		  		= 16;	//不添加这个参数不能被diskio识别,小于16在ntfs vdf,ntfs盘上无法找到bootx64.efi
			pridata[2].Media.BlockSize        		= pridata[0].BlockSize;
			pridata[2].Media.LastBlock        		= DivU64x32 (pridata[2].Size + pridata[0].BlockSize - 1, pridata[0].BlockSize )- 1;


				
			//安装启动分区的块协议
			Print(L"installing partition blockio protocol\n");
			Status = gBS->InstallMultipleProtocolInterfaces (
				&pridata[2].VirDiskHandle,
				
				&gEfiDevicePathProtocolGuid,
				pridata[2].VirDiskDevicePath,
				&gEfiBlockIoProtocolGuid,
				&pridata[2].BlockIo,
				&gEfiBlockIo2ProtocolGuid,
				&pridata[2].BlockIo2,

				NULL
				);
			if(EFI_ERROR(Status)){
				Print(L"Install partition blockio protocol fail,error=%r\n",Status);
				return 	EFI_NOT_FOUND;
				}

			gBS->ConnectController (pridata[2].VirDiskHandle, NULL, NULL, TRUE);
			
			//光盘才需要执行下面的代码
			if(pridata[0].ImageType!=ISOFILE)
				return EFI_SUCCESS;
		
			//实验表明执行完ConnectController后大部分固件会自动为pridata[2].VirDiskHandle安装好fat协议		
			//但是由bios转uefi见此贴http://bbs.wuyou.net/forum.php?mod=viewthread&tid=378197&extra=的固件会错误的
			//给pridata[2].VirDiskHandle再安装一个partition协议，而不是fat协议，导致在此条件下不能正常启动。
			{

				EFI_HANDLE							FatDriverHandle=NULL;
				EFI_HANDLE							*Buffer;
				UINTN								BufferCount;
				UINTN								BufferIndex;
				EFI_COMPONENT_NAME2_PROTOCOL		*DriverNameProtocol;
				CHAR16								*DriverName;
				EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*PartitionSFSProtocol=NULL;
				

				
				//检验分区是否正确的被固件识别并安装了文件协议
				Status=gBS->HandleProtocol(pridata[2].VirDiskHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&PartitionSFSProtocol);
				if(Status==EFI_SUCCESS){
					return EFI_SUCCESS;
					}

				//下面的语句在系统中搜索fat驱动，然后为pridata[2].VirDiskHandle强制安装fat协议
				//如果固件没有自动为pridata[2].VirDiskHandle安装fat协议，则找到fat驱动句柄并强制安装fat协议		
				
				
				
				//列出所有的支持驱动名字的句柄
				Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiComponentName2ProtocolGuid,NULL,&BufferCount,&Buffer);
				if(EFI_ERROR (Status)){
					Print(L"ComponentNameProtocol not found.Error=[%r]\n",Status);
					//return Status;
					}
				//循环缓冲区中的所有句柄	
				for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
					gBS->HandleProtocol(Buffer[BufferIndex],&gEfiComponentName2ProtocolGuid,(VOID**)&DriverNameProtocol);
					DriverNameProtocol->GetDriverName(DriverNameProtocol,"en-us",&DriverName);
					//是否fat驱动
					if(NULL!=DriverName&&
						NULL!=StrStr(DriverName,L"FAT File System Driver")){
							
						FatDriverHandle=Buffer[BufferIndex];
						Print(L"FatDriverHandle is:%X\n",FatDriverHandle);
						break;
						}
					}
				//是否找到了fat驱动	
				if(FatDriverHandle!=NULL){
					Status=gBS->ConnectController (pridata[2].VirDiskHandle, &FatDriverHandle, NULL, TRUE);
					Print(L"ConnectController Fat %r\n",Status);
					return EFI_SUCCESS;
					}else{
						Print(L"Can't find FAT Driver\n");
						return EFI_NOT_FOUND;
						}
				
			}	
			
					
		}