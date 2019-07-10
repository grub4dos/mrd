#include "MyRamDisk.h"
///等待
EFI_STATUS
	DidoWaitSec(
		UINTN 		Sec
		)
		///等待三次
		{
			EFI_STATUS  Status;
			EFI_EVENT 	myEvent;
			UINTN       index=0;
			UINTN		repeats;

			
			Status = gBS->CreateEvent(EVT_TIMER  , TPL_CALLBACK, (EFI_EVENT_NOTIFY)NULL, (VOID*)NULL, &myEvent);
			if(EFI_ERROR(Status)){
				Print(L"Fail EVT_TIMER | EVT_RUNTIME %r", Status);
				return Status;
			}
			Status = gBS->SetTimer(myEvent,TimerPeriodic , 10000000);
			gBS->RestoreTPL(TPL_APPLICATION);
			for(repeats=0;repeats<Sec;repeats++){
				Status = gBS->WaitForEvent(1, &myEvent, &index);
				Print(L"Wait for %d second to boot.\r",Sec-repeats-1);
				}
			Status = gBS->CloseEvent(myEvent);
			return Status;
		}
		
		
		