


#include "Task_UpdateValueDuration.h"

UTask_UpdateValueDuration* UTask_UpdateValueDuration::UpdateValueDuration(UObject* WorldContextObject, bool bIsInfinite, float Duration, float Rate, float TickInterval, UTask_UpdateValueDuration*& AsyncTask)
{
	if(!WorldContextObject)
	{
		return nullptr;
	}

	UTask_UpdateValueDuration* Task = NewObject<UTask_UpdateValueDuration>();
	Task->WorldContextObject = WorldContextObject;
	Task->bIsInfinite = bIsInfinite;
	Task->Duration = Duration;
	Task->Rate = Rate;
	Task->TickInterval = TickInterval;
	AsyncTask = Task;
	return Task;
}

void UTask_UpdateValueDuration::Activate()
{
	CachedWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!CachedWorld)
	{
		return;
	}
	if(TickInterval <= 0.0f && Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTask_UpdateValueDuration: TickInterval and Duration cannot both be zero or negative."));
		return;
	}

	if (bIsInfinite)
	{
		CachedWorld->GetTimerManager().SetTimer(TimerHandle_Tick, this, &UTask_UpdateValueDuration::HandleOnUpdate, TickInterval, true);
	}
	else 
	{
		CachedWorld->GetTimerManager().SetTimer(TimerHandle_Tick, this, &UTask_UpdateValueDuration::HandleOnUpdate, TickInterval, true);
		CachedWorld->GetTimerManager().SetTimer(TimerHandle_Duration, this, &UTask_UpdateValueDuration::HandleFinishTask, Duration, false);
	}
}

void UTask_UpdateValueDuration::HandleOnUpdate()
{
	ElapsedTime += TickInterval;
	OnUpdate.Broadcast(Rate * TickInterval);
}

void UTask_UpdateValueDuration::HandleFinishTask()
{
	if (CachedWorld)
	{
		FTimerManager& TM = CachedWorld->GetTimerManager();
		TM.ClearTimer(TimerHandle_Tick);
		TM.ClearTimer(TimerHandle_Duration);
	}
	else
	{
		return;
	}

	TimerHandle_Tick.Invalidate();
	TimerHandle_Duration.Invalidate();

	OnFinish.Broadcast();

	SetReadyToDestroy();
}

void UTask_UpdateValueDuration::StopTask()
{
	HandleFinishTask();
}