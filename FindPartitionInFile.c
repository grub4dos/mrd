#include "MyRamDisk.h"

///找出分区信息，返回到后4个参数中
EFI_STATUS
	FindPartitionInFile(
		IN		EFI_FILE_HANDLE			FileDiskFileHandle,
		OUT		UINT64					*NoBootStartAddr,
		OUT		UINT64					*NoBootSize,
		OUT		UINT64					*BootStartAddr,
		OUT		UINT64					*BootSize
		)
		{
			CDROM_VOLUME_DESCRIPTOR     *VolDescriptor=NULL;
			ELTORITO_CATALOG            *TempCatalog=NULL;
			UINTN						DescriptorSize=RAM_DISK_BLOCK_SIZE;	
			

			VolDescriptor = AllocatePool (DescriptorSize);
			if (VolDescriptor == NULL) {
				return EFI_NOT_FOUND;
				}
			//判断卷
			FileHandleSetPosition(FileDiskFileHandle,CD_BOOT_SECTOR*RAM_DISK_BLOCK_SIZE); 	
			FileHandleRead(FileDiskFileHandle,&DescriptorSize,VolDescriptor);
			if(	VolDescriptor->Unknown.Type!=CDVOL_TYPE_STANDARD||
				CompareMem (VolDescriptor->BootRecordVolume.SystemId, CDVOL_ELTORITO_ID, sizeof (CDVOL_ELTORITO_ID) - 1) != 0){
				return EFI_NOT_FOUND;
				}
			//判断启动目录	
			TempCatalog = (ELTORITO_CATALOG*)VolDescriptor;
			FileHandleSetPosition(FileDiskFileHandle,*((UINT32*)VolDescriptor->BootRecordVolume.EltCatalog)*RAM_DISK_BLOCK_SIZE); 	
			FileHandleRead(FileDiskFileHandle,&DescriptorSize,TempCatalog);	
			if( TempCatalog[0].Catalog.Indicator!=ELTORITO_ID_CATALOG){
				return EFI_NOT_FOUND;
				}
			for(UINTN	i=0;i<64;i++){
				if( TempCatalog[i].Section.PlatformId!=IS_EFI_SYSTEM_PARTITION&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE&&
					TempCatalog[i+1].Boot.LoadSegment==0x7c00){
					*NoBootStartAddr	=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*NoBootSize	 	=TempCatalog[i+1].Boot.SectorCount*RAM_DISK_BLOCK_SIZE;
					return EFI_SUCCESS;
					}
				
				
				if( TempCatalog[i].Section.Indicator==ELTORITO_ID_SECTION_HEADER_FINAL&&
					TempCatalog[i].Section.PlatformId==IS_EFI_SYSTEM_PARTITION&&
					TempCatalog[i+1].Boot.Indicator==ELTORITO_ID_SECTION_BOOTABLE ){
					*BootStartAddr=TempCatalog[i+1].Boot.Lba*RAM_DISK_BLOCK_SIZE;
					*BootSize	 =TempCatalog[i+1].Boot.SectorCount*RAM_DISK_BLOCK_SIZE;
					return EFI_SUCCESS;
					}
				
				}				
			
				
				
				
				
				
				
				
				
				
				
				
		return 	EFI_NOT_FOUND;		
		}