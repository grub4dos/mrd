//处理内存盘的协议安装

#include "MyRamDisk.h"






EFI_STATUS
	MyFileDiskInstall(
		IN 	EFI_FILE_HANDLE 	FileDiskFileHandle,
		IN	BOOLEAN				DiskInRam
	)
	{
		EFI_STATUS 							Status=EFI_SUCCESS;
		EFI_DEVICE_PATH_PROTOCOL 			*TempPath1;
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*PartitionSFSProtocol=NULL;				//用于测试是否已安装fat协议,没用了
		//初始化全局指针pridata,分配3个私有数据结构，第一个用于整个光盘，第二个用于传统启动分区，第三个用于efi启动分区
		pridata=AllocatePool(3*sizeof(DIDO_DISK_PRIVATE_DATA)+8);
		///初始化不同部分
		//获取文件大小
		Status=FileHandleGetSize(FileDiskFileHandle,&pridata[0].Size);
		//获取分区信息并填写各Present,StartAddr,Size
		Status=FindPartitionInFile(FileDiskFileHandle,&pridata[1].StartAddr,&pridata[1].Size,&pridata[2].StartAddr,&pridata[2].Size);
		if(EFI_ERROR (Status)) {
			Print(L"No bootable partition on the virtual disk!\n");
			return EFI_NOT_FOUND;
			}		
		if(DiskInRam){				//是否需要载入内存
			//分配内存
			Status = gBS->AllocatePool (EfiBootServicesData,pridata[0].Size+8,(VOID**)&pridata[0].StartAddr); 
			if(EFI_ERROR (Status)) {
				Print(L"Allocate Memory failed!\n");
				return EFI_NOT_FOUND;
				}
			Print(L"Creating ramdisk.Please wait.\n");
			//全读iso文件
			FileHandleSetPosition(FileDiskFileHandle,0);	
			Status=FileHandleRead(FileDiskFileHandle,&pridata[0].Size,(VOID*)pridata[0].StartAddr);
			if(EFI_ERROR (Status)){
				Print(L"ReadFile failed.Error=[%r]\n",Status);
				Status=gBS->FreePool((VOID*)pridata[0].StartAddr);
				return EFI_NOT_FOUND;
				}
			//不载入内存则StartAddr代表文件偏移量
			}else{
				pridata[0].StartAddr=0;
				
				}

		pridata[1].StartAddr=pridata[1].StartAddr+pridata[0].StartAddr;
		pridata[2].StartAddr=pridata[2].StartAddr+pridata[0].StartAddr;		

		

				

		
		
		//填写DP
		//整个光盘镜像的DP
		TempPath1=CreateDeviceNode  ( HARDWARE_DEVICE_PATH ,HW_VENDOR_DP ,  sizeof(VENDOR_DEVICE_PATH));
		((VENDOR_DEVICE_PATH*)TempPath1)->Guid=MyGuid;
		pridata[0].VirDiskDevicePath=AppendDevicePathNode(NULL,TempPath1);
		FreePool(TempPath1);
		//传统启动映像，多数是引导代码
		TempPath1=CreateDeviceNode  ( MEDIA_DEVICE_PATH ,MEDIA_CDROM_DP ,  sizeof(CDROM_DEVICE_PATH));		
		((CDROM_DEVICE_PATH*)TempPath1)->BootEntry=0;
		((CDROM_DEVICE_PATH*)TempPath1)->PartitionStart=(pridata[1].StartAddr-pridata[0].StartAddr)/RAM_DISK_BLOCK_SIZE;
		((CDROM_DEVICE_PATH*)TempPath1)->PartitionSize=pridata[1].Size/RAM_DISK_BLOCK_SIZE;
		pridata[1].VirDiskDevicePath=AppendDevicePathNode(pridata[0].VirDiskDevicePath,TempPath1);
		FreePool(TempPath1);
		//efi启动映像，一般是一个包含/efi/boot/bootx64.efi的软盘
		TempPath1=CreateDeviceNode  ( MEDIA_DEVICE_PATH ,MEDIA_CDROM_DP ,  sizeof(CDROM_DEVICE_PATH));		
		((CDROM_DEVICE_PATH*)TempPath1)->BootEntry=1;
		((CDROM_DEVICE_PATH*)TempPath1)->PartitionStart=(pridata[2].StartAddr-pridata[0].StartAddr)/RAM_DISK_BLOCK_SIZE;
		((CDROM_DEVICE_PATH*)TempPath1)->PartitionSize=pridata[2].Size/RAM_DISK_BLOCK_SIZE;
		pridata[2].VirDiskDevicePath=AppendDevicePathNode(pridata[0].VirDiskDevicePath,TempPath1);
		FreePool(TempPath1);

		///初始化相同部分
		for(INTN	i=0;i<3;i++){
			//pridata[i].Present
			pridata[i].VirDiskHandle=NULL;
			//pridata[i].VirDiskDevicePath	
			pridata[i].VirDiskFileHandle=FileDiskFileHandle;
			pridata[i].InRam=DiskInRam;
			//pridata[i].StartAddr
			//pridata[i].Size
			
			//blockio的初始化
			CopyMem (&pridata[i].BlockIo, &mFileDiskBlockIoTemplate, sizeof (EFI_BLOCK_IO_PROTOCOL));
			CopyMem (&pridata[i].BlockIo2, &mFileDiskBlockIo2Template, sizeof (EFI_BLOCK_IO2_PROTOCOL));

			pridata[i].BlockIo.Media          		= &pridata[i].Media;
			pridata[i].BlockIo2.Media      	 		= &pridata[i].Media;
			pridata[i].Media.MediaId		  		= 0x1;	
			pridata[i].Media.RemovableMedia   		= TRUE;				
			pridata[i].Media.MediaPresent     		= TRUE;
			pridata[i].Media.LogicalPartition 		= TRUE;					//不同	
			pridata[i].Media.ReadOnly         		= TRUE;								//被我修改以适应虚拟光驱原来是FALSE
			pridata[i].Media.WriteCaching     		= FALSE;
			pridata[i].Media.IoAlign 		  		= RAM_DISK_BLOCK_SIZE;				//不添加这个参数不能被diskio识别
			pridata[i].Media.BlockSize        		= RAM_DISK_BLOCK_SIZE;
			pridata[i].Media.LastBlock        		= DivU64x32 (
														  pridata[i].Size + RAM_DISK_BLOCK_SIZE - 1,
														  RAM_DISK_BLOCK_SIZE
														  ) - 1;
			

			}
		pridata[0].Media.LogicalPartition 		= FALSE;

				


		//安装启动映像的块协议		
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
			Print(L"%r\n",Status);
			Exit(Status);
			}
		
		Print(L"Virtual partition handle is : %X\n",pridata[2].VirDiskHandle);	

		gBS->ConnectController (pridata[2].VirDiskHandle, NULL, NULL, TRUE);	
		//将磁盘放到分区后面来安装是因为virtualbox的固件下，生成的分区有时候有错误
		//安装磁盘的块协议		
		Print(L"installing disk blockio protocol\n");
		gBS->InstallMultipleProtocolInterfaces (
			&pridata[0].VirDiskHandle,
			&gEfiDevicePathProtocolGuid,
			pridata[0].VirDiskDevicePath,
			&gEfiBlockIoProtocolGuid,
			&pridata[0].BlockIo,
	  
			&gEfiBlockIo2ProtocolGuid,
			&pridata[0].BlockIo2,

			NULL
			);		
		Print(L"Virtual disk handle is : %X\n",pridata[0].VirDiskHandle);
		gBS->ConnectController (pridata[0].VirDiskHandle, NULL, NULL, TRUE);
				
		//实验表明执行完ConnectController后大部分固件会自动为pridata[2].VirDiskHandle安装好fat协议		
		//但是由bios转uefi见此贴http://bbs.wuyou.net/forum.php?mod=viewthread&tid=378197&extra=的固件会错误的
		//给pridata[2].VirDiskHandle再安装一个partition协议，而不是fat协议，导致在此条件下不能正常启动。

		Status=gBS->HandleProtocol(pridata[2].VirDiskHandle,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&PartitionSFSProtocol);
		if(!EFI_ERROR(Status)){
			return EFI_SUCCESS;
			}
		if(FindBootPartitionHandle()){
			return EFI_ALREADY_STARTED;
			}			
		//下面的语句在系统中搜索fat驱动，然后为pridata[2].VirDiskHandle强制安装fat协议
		//如果固件没有自动为pridata[2].VirDiskHandle安装fat协议，则找到fat驱动句柄并强制安装fat协议		
		{

			EFI_HANDLE							FatDriverHandle=NULL;
			EFI_HANDLE							*Buffer;
			UINTN								BufferCount;
			UINTN								BufferIndex;
			EFI_COMPONENT_NAME_PROTOCOL			*DriverNameProtocol;
			CHAR16								*DriverName;
			
			
			//列出所有的支持驱动名字的句柄
			Status=gBS->LocateHandleBuffer(ByProtocol,&gEfiComponentNameProtocolGuid,NULL,&BufferCount,&Buffer);
			if(EFI_ERROR (Status)){
				Print(L"ComponentNameProtocol not found.Error=[%r]\n",Status);
				//return Status;
				}
			//循环缓冲区中的所有句柄	
			for(BufferIndex=0;BufferIndex<BufferCount;BufferIndex++){
				gBS->HandleProtocol(Buffer[BufferIndex],&gEfiComponentNameProtocolGuid,(VOID**)&DriverNameProtocol);
				DriverNameProtocol->GetDriverName(DriverNameProtocol,"eng",&DriverName);
				//是否fat驱动
				if(NULL!=StrStr(DriverName,L"FAT File System Driver")){
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