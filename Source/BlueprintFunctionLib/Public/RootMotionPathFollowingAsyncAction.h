#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Containers/Ticker.h"
#include "ERMPathFollowingMovementMode.h"
#include "RootMotionPathFollowingAsyncAction.generated.h"

class APawn;
class UNavigationPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRootMotionPathFollowingUpdateSignature, FVector2D, BlendspaceDirection, FRotator, FacingRotation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRootMotionPathFollowingResultSignature, bool, bReachedDestination);

/**
 * Blueprint async action that drives root-motion navigation along a precomputed path.
 * Each tick it outputs a normalized 2D direction suitable for 8-way blend spaces and
 * an interpolated facing rotation for the controlled pawn.
 */
UCLASS()
class BLUEPRINTFUNCTIONLIB_API URootMotionPathFollowingAsyncAction : public UBlueprintAsyncActionBase
{
        GENERATED_BODY()

public:
        /**
         * Event fired every tick with the desired blend space direction and facing rotation.
         */
        UPROPERTY(BlueprintAssignable)
        FRootMotionPathFollowingUpdateSignature OnPathUpdated;

        /**
         * Event fired once the task finishes. bReachedDestination indicates whether the
         * end of the path was reached (true) or the task aborted/cancelled (false).
         */
        UPROPERTY(BlueprintAssignable)
        FRootMotionPathFollowingResultSignature OnFinished;

        /**
         * Start following the provided navigation path using root-motion. The task keeps
         * ticking until the pawn reaches the end of the path or it is cancelled/invalidated.
         */
        UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Follow Path With Root Motion", Category = "AI|Root Motion"))
        static URootMotionPathFollowingAsyncAction* FollowPathWithRootMotion(UObject* WorldContextObject, APawn* Pawn, UNavigationPath* NavigationPath, float AcceptanceRadius, float RotationInterpSpeed, ERMPathFollowingMovementMode MovementMode, FRotator FixedWorldRotation);

        /**
         * Abort the async task. OnFinished will be broadcast with bReachedDestination = false.
         */
        UFUNCTION(BlueprintCallable, Category = "AI|Root Motion")
        void Cancel();

        //~UBlueprintAsyncActionBase interface
        virtual void Activate() override;
        virtual void BeginDestroy() override;

private:
        bool TickTask(float DeltaTime);
        void FinishTask(bool bReachedDestination);
        void Cleanup();

private:
        TWeakObjectPtr<UObject> WorldContextObject;
        TWeakObjectPtr<APawn> ControlledPawn;
        TWeakObjectPtr<UNavigationPath> NavigationPath;

        float AcceptanceRadius = 0.f;
        float RotationInterpSpeed = 0.f;
        ERMPathFollowingMovementMode MovementMode = ERMPathFollowingMovementMode::FacePath;
        FRotator FixedWorldRotation = FRotator::ZeroRotator;

        int32 PathPointIndex = 1;
        int32 ConsecutiveStalledFrames = 0;
        static constexpr int32 MaxAllowedStallFrames = 5;

        bool bIsActive = false;
        bool bHasFinished = false;

        FTSTicker::FDelegateHandle TickerHandle;
};

