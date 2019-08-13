#include "MyRamDisk.h"
///返回制定image句柄的目录的DP，如果FileName不为空，则将FileName添加到当前父的DP并返回

EFI_DEVICE_PATH_PROTOCOL* 
	GetCurrDirDP(
		IN	EFI_HANDLE 			FileHandle,
		IN 	const 	CHAR16		*FileName
		)
		{
			EFI_STATUS						Status;
			EFI_DEVICE_PATH_PROTOCOL		*ThisFileDP;
			EFI_DEVICE_PATH_PROTOCOL		*TempDP=NULL;
			EFI_DEVICE_PATH_PROTOCOL		*TempDPToFree=NULL;
			EFI_DEVICE_PATH_PROTOCOL		*TempDPDirPathNode=NULL;
			EFI_DEVICE_PATH_PROTOCOL		*TempDPFilePathNode=NULL;
			EFI_DEVICE_PATH_PROTOCOL		*TempDPNode;
			INTN							StrIndex;
			CHAR16							*NodeFileName;
			CHAR16							*NodeDirName;	
			//打开自己的映像DP信息
			Status=gBS->HandleProtocol(FileHandle,&gEfiLoadedImageDevicePathProtocolGuid,(VOID**)&ThisFileDP);
			if(EFI_ERROR (Status)){
				Print(L"LoadedImageDevicePathProtocol not found.Error=[%r]\n",Status);
				return NULL;
				}
			//复制一个dp	
			TempDP=DuplicateDevicePath(ThisFileDP);
			TempDPFilePathNode=TempDP;
			TempDPDirPathNode=TempDPFilePathNode;
			//只有一个endpath退出
			if(NULL==TempDP||IsDevicePathEnd(TempDP))return NULL;
			//移动dir和file指向倒数两个节点	
			while(!IsDevicePathEnd(NextDevicePathNode(TempDPFilePathNode))){	
				TempDPDirPathNode=TempDPFilePathNode;
				TempDPFilePathNode=NextDevicePathNode(TempDPFilePathNode);
				}
			//path不正常，不是文件path退出
			if( MEDIA_DEVICE_PATH!=TempDPFilePathNode->Type||
				MEDIA_FILEPATH_DP!=TempDPFilePathNode->SubType)return NULL;
			//将文件节点复制一份	
			TempDPNode=AppendDevicePathNode(NULL,TempDPFilePathNode);
			//删去原来的文件节点
			SetDevicePathEndNode(TempDPFilePathNode);
			
			//给空的根目录添加一个斜杠
			//Print(L"DirDP len before repair is %d\n",TempDPDirPathNode->Length[0]);	
			//如果有两个文件节点并且第一个为空
			if(MEDIA_DEVICE_PATH==TempDPDirPathNode->Type&&
				MEDIA_FILEPATH_DP==TempDPDirPathNode->SubType&&
				TempDPDirPathNode->Length[0]<8){
				Print(L"Repair first dir node\n");
				SetDevicePathEndNode(TempDPDirPathNode);
				TempDPDirPathNode=CreateDeviceNode(MEDIA_DEVICE_PATH,	MEDIA_FILEPATH_DP,8);
				((FILEPATH_DEVICE_PATH*)TempDPDirPathNode)->PathName[0]=L'\\';
				((FILEPATH_DEVICE_PATH*)TempDPDirPathNode)->PathName[1]=L'\0';
				TempDPToFree=TempDP;
				TempDP=AppendDevicePathNode(TempDP,TempDPDirPathNode);
				FreePool(TempDPToFree);
				}
			//Print(L"DirDP len after repair is %d\n",TempDPDirPathNode->Length[0]);
			
			//将文件名字符串复制一份
			NodeFileName=AllocateCopyPool(2*(StrLen(((FILEPATH_DEVICE_PATH*)TempDPNode)->PathName)+1),
				((FILEPATH_DEVICE_PATH*)TempDPNode)->PathName);
			FreePool(TempDPNode);	
			//检查字符串的分隔符
			NodeDirName=AllocateCopyPool(2*(StrLen(FileName)+1),FileName);			
			for(StrIndex=StrLen(NodeFileName)-1;StrIndex>=0;StrIndex--){
				if(NodeFileName[StrIndex]==L'\\'||NodeFileName[StrIndex]==L'/'){
					//将pathname去掉文件名留下路径名复制到dirname
					FreePool(NodeDirName);
					NodeDirName=AllocateCopyPool(2*(StrLen(FileName)+1)+2*(StrIndex+2),NodeFileName);
					NodeDirName[StrIndex+1]=L'\0';
					StrCat(NodeDirName,	FileName);	
					break;				
					}
				}
			FreePool(NodeFileName);	
			///将入口参数给出的文件名加到路径后面	
			//创建节点
			TempDPNode=CreateDeviceNode(MEDIA_DEVICE_PATH,	MEDIA_FILEPATH_DP,(UINT16)(4+2*(StrLen(NodeDirName)+1)));
			//将组合后的文件名写进去
			StrCat(((FILEPATH_DEVICE_PATH*)TempDPNode)->PathName,NodeDirName);
			FreePool(NodeDirName);
			TempDPToFree=TempDP;
			//将节点连接到前面分离出来的路径
			TempDP=AppendDevicePathNode(TempDP,TempDPNode);
			FreePool(TempDPToFree);
			FreePool(TempDPNode);
			//单节点	
			return TempDP;
		}