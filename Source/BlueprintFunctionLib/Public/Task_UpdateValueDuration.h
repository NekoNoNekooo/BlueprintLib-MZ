

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Task_UpdateValueDuration.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateDelegate, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishDelegate);

/**
 * 
 */
UCLASS(BlueprintType)
class BLUEPRINTFUNCTIONLIB_API UTask_UpdateValueDuration : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Async|Task")
	static UTask_UpdateValueDuration* UpdateValueDuration(UObject* WorldContextObject, bool bIsInfinite, float Duration, float Rate, float TickInterval, UTask_UpdateValueDuration*& AsyncTask);

	UPROPERTY(BlueprintAssignable, Category = "Async|Task")
	FOnUpdateDelegate OnUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Async|Task")
	FOnFinishDelegate OnFinish;

	virtual void Activate() override;

	void StopTask(); // Allow stopping the task manually

private:

	UObject* WorldContextObject = nullptr;
	bool bIsInfinite = false;
	float Duration = 0.0f;
	float Rate = 0.0f;
	float TickInterval = 0.033333f;
	float ElapsedTime = 0.0f;

	UWorld* CachedWorld = nullptr;

	FTimerHandle TimerHandle_Tick;
	FTimerHandle TimerHandle_Duration;


	void HandleOnUpdate();
	void HandleFinishTask();
};
