#include "MyRamDisk.h"
///等待
EFI_STATUS
	DidoWaitSec(
		UINTN 		Sec
		)
		///等待三次
		{
			EFI_STATUS  Status;
			EFI_EVENT 	myEvent[2];
			UINTN       index=0;
			UINTN		repeats;

			
			Status = gBS->CreateEvent(EVT_TIMER  , TPL_CALLBACK, (EFI_EVENT_NOTIFY)NULL, (VOID*)NULL, myEvent);
			if(EFI_ERROR(Status)){
				Print(L"Fail EVT_TIMER | EVT_RUNTIME %r", Status);
				return Status;
			}
			myEvent[1]=gST->ConIn->WaitForKey;
			Status = gBS->SetTimer(myEvent[0],TimerPeriodic , 10000000);
			//必须降低运行级别才能等待，并且避免实机进入pe的logo后死机重启
			gBS->RestoreTPL(TPL_APPLICATION);
			Print(L"Press any key to skip countdown.\n");
			for(repeats=0;repeats<Sec;repeats++){
				Status = gBS->WaitForEvent(2, myEvent, &index);
				if(index==1)break;
				Print(L"Wait for %d second to boot.\r",Sec-repeats-1);
				}
			Status = gBS->CloseEvent(myEvent[0]);
			Status = gBS->CloseEvent(myEvent[1]);
			return Status;
		}
		
		
		