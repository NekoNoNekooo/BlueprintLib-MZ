#include "RootMotionPathFollowingAsyncAction.h"

#include "CommonBlueprintFunctionLib.h"
#include "NavigationPath.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

URootMotionPathFollowingAsyncAction* URootMotionPathFollowingAsyncAction::FollowPathWithRootMotion(
        UObject* InWorldContextObject,
        APawn* Pawn,
        UNavigationPath* NavigationPathObject,
        float InAcceptanceRadius,
        float InRotationInterpSpeed,
        ERMPathFollowingMovementMode InMovementMode,
        FRotator InFixedWorldRotation)
{
        URootMotionPathFollowingAsyncAction* Action = NewObject<URootMotionPathFollowingAsyncAction>();
        Action->WorldContextObject = InWorldContextObject;
        Action->ControlledPawn = Pawn;
        Action->NavigationPath = NavigationPathObject;
        Action->AcceptanceRadius = FMath::Max(0.f, InAcceptanceRadius);
        Action->RotationInterpSpeed = InRotationInterpSpeed;
        Action->MovementMode = InMovementMode;
        Action->FixedWorldRotation = InFixedWorldRotation;
        Action->PathPointIndex = 1;
        Action->ConsecutiveStalledFrames = 0;

        UObject* RegisterContext = InWorldContextObject ? InWorldContextObject : Pawn;
        if (RegisterContext)
        {
                Action->RegisterWithGameInstance(RegisterContext);
        }

        return Action;
}

void URootMotionPathFollowingAsyncAction::Activate()
{
        if (bHasFinished)
        {
                return;
        }

        APawn* Pawn = ControlledPawn.Get();
        UNavigationPath* Path = NavigationPath.Get();

        if (!Pawn || !Path)
        {
                FinishTask(false);
                return;
        }

        if (!Pawn->GetWorld())
        {
                FinishTask(false);
                return;
        }

        if (Path->PathPoints.Num() < 2)
        {
                FinishTask(true);
                return;
        }

        PathPointIndex = FMath::Clamp(PathPointIndex, 1, Path->PathPoints.Num() - 1);
        ConsecutiveStalledFrames = 0;
        bIsActive = true;

        if (!TickerHandle.IsValid())
        {
                TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &URootMotionPathFollowingAsyncAction::TickTask));
        }
}

void URootMotionPathFollowingAsyncAction::BeginDestroy()
{
        Cleanup();
        Super::BeginDestroy();
}

void URootMotionPathFollowingAsyncAction::Cancel()
{
        FinishTask(false);
}

bool URootMotionPathFollowingAsyncAction::TickTask(float DeltaTime)
{
        if (!bIsActive)
        {
                return false;
        }

        if (DeltaTime <= KINDA_SMALL_NUMBER)
        {
                return true;
        }

        APawn* Pawn = ControlledPawn.Get();
        UNavigationPath* Path = NavigationPath.Get();
        if (!Pawn || !Path)
        {
                FinishTask(false);
                return false;
        }

        FVector2D BlendspaceDirection = FVector2D::ZeroVector;
        FRotator FacingRotation = Pawn->GetActorRotation();
        bool bReachedPathEnd = false;

        const bool bProducedUpdate = UCommonBlueprintFunctionLib::UpdateRootMotionPathFollowing(
                Pawn,
                Path,
                AcceptanceRadius,
                DeltaTime,
                RotationInterpSpeed,
                MovementMode,
                FixedWorldRotation,
                PathPointIndex,
                BlendspaceDirection,
                FacingRotation,
                bReachedPathEnd);

        if (bReachedPathEnd)
        {
                if (bProducedUpdate)
                {
                        OnPathUpdated.Broadcast(BlendspaceDirection, FacingRotation);
                }

                FinishTask(true);
                return false;
        }

        if (bProducedUpdate)
        {
                ConsecutiveStalledFrames = 0;
                OnPathUpdated.Broadcast(BlendspaceDirection, FacingRotation);
                return true;
        }

        ++ConsecutiveStalledFrames;
        if (ConsecutiveStalledFrames > MaxAllowedStallFrames)
        {
                FinishTask(false);
                return false;
        }

        return true;
}

void URootMotionPathFollowingAsyncAction::FinishTask(bool bReachedDestination)
{
        if (bHasFinished)
        {
                return;
        }

        bHasFinished = true;
        bIsActive = false;
        Cleanup();
        OnFinished.Broadcast(bReachedDestination);
        SetReadyToDestroy();
}

void URootMotionPathFollowingAsyncAction::Cleanup()
{
        if (TickerHandle.IsValid())
        {
                FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
                TickerHandle = FTSTicker::FDelegateHandle();
        }

}

