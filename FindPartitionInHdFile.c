#include "MyRamDisk.h"

///找出分区信息，返回到后4个参数中
EFI_STATUS
	FindPartitionInHdFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINTN					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINTN					*BootStartAddr,
		OUT		UINT64					*BootSize,
		OUT		UINT32					*UniqueMbrSignature	
		
		)
		{
			MASTER_BOOT_RECORD     		*MasterBootRecord=NULL;
			UINTN						MasterBootRecordSize=FLOPPY_DISK_BLOCK_SIZE;	

			MasterBootRecord = AllocatePool (MasterBootRecordSize);
			if (MasterBootRecord == NULL) {
				return EFI_NOT_FOUND;
				}
			//判断MBR是否有效
			FileHandleSetPosition(FileDiskFileHandle,MBR_START_LBA); 	
			FileHandleRead(FileDiskFileHandle,&MasterBootRecordSize,MasterBootRecord);
			if(	MasterBootRecord->Signature!=MBR_SIGNATURE){
				FreePool(MasterBootRecord);
				return EFI_NOT_FOUND;
				}
			//判断分区类型，如果是mbr则处理后返回
			if(MasterBootRecord->Partition[0].OSIndicator!=PMBR_GPT_PARTITION){
				*BootStartAddr=*(UINT32*)(MasterBootRecord->Partition[0].StartingLBA);
				*BootStartAddr=(*BootStartAddr)*FLOPPY_DISK_BLOCK_SIZE;
				*BootSize=*(UINT32*)(MasterBootRecord->Partition[0].SizeInLBA);
				*BootSize=(*BootSize)*FLOPPY_DISK_BLOCK_SIZE;
				*UniqueMbrSignature=*(UINT32*)(MasterBootRecord->UniqueMbrSignature);
				FreePool(MasterBootRecord);
				return EFI_SUCCESS;
				}
/*		
			//gpt分区	
			TempCatalog = (ELTORITO_CATALOG*)VolDescriptor;
			FileHandleSetPosition(FileDiskFileHandle,*((UINT32*)VolDescriptor->BootRecordVolume.EltCatalog)*RAM_DISK_BLOCK_SIZE); 	
			FileHandleRead(FileDiskFileHandle,&DescriptorSize,TempCatalog);	
			if( TempCatalog[0].Catalog.Indicator!=ELTORITO_ID_CATALOG){
				FreePool(VolDescriptor);
				return EFI_NOT_FOUND;
				}
			for(i=0;i<64;i++){
				if( TempCatalog[i].Catalog.Indicator==ELTORITO_ID_CATALOG&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE&&
					TempCatalog[i+1].Boot.LoadSegment==0x7c0){
					*NoBootStartAddr	=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*NoBootSize	 		=TempCatalog[i+1].Boot.SectorCount*FLOPPY_DISK_BLOCK_SIZE;
					
					}
				
				
				if( TempCatalog[i].Section.Indicator==ELTORITO_ID_SECTION_HEADER_FINAL&&
					TempCatalog[i].Section.PlatformId==IS_EFI_SYSTEM_PARTITION&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE ){
						
					*BootStartAddr	=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*BootSize		=TempCatalog[i+1].Boot.SectorCount*FLOPPY_DISK_BLOCK_SIZE;
					//有些光盘的映像大小设定不正确，太小就读取软盘映像的dbr
					FileHandleSetPosition(FileDiskFileHandle,*BootStartAddr+0x13); 	
					FileHandleRead(FileDiskFileHandle,&DbrImageSize,&DbrImageSizeBuffer);
					
					DbrImageSize=DbrImageSizeBuffer*FLOPPY_DISK_BLOCK_SIZE;
					*BootSize=*BootSize>DbrImageSize?*BootSize:DbrImageSize;
						
					//仍然太小则设为固定值	
					if(*BootSize<BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE){
						*BootSize=BLOCK_OF_1_44MB*FLOPPY_DISK_BLOCK_SIZE;
						}
					FreePool(VolDescriptor);	
					return EFI_SUCCESS;
					}
				
				}				
			FreePool(VolDescriptor);
*/	
					
			return 	EFI_NOT_FOUND;		
		}