// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Enum_CombatPositioning.h"
#include "ERMPathFollowingMovementMode.h"
#include "CommonBlueprintFunctionLib.generated.h"

/**
 * 
 */

class UChooserTable;
class UAnimMontage;
struct FAttributeFloat;
class UTask_UpdateValueDuration;
struct FOctantSlot;
class UNavigationPath;

UCLASS()
class BLUEPRINTFUNCTIONLIB_API UCommonBlueprintFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	// ???????ChooserTable, ???CHTHitMontageInput??????????Hit???????Tag
	UFUNCTION(BlueprintCallable)
	static bool EvaluateHitAnimMontage(const FCHTHitMontageInput InputStruct, const UChooserTable* ChooserTable, FCHTHitMontageOutput& OutStruct);

	// ???Curve????????float?
	UFUNCTION(BlueprintPure)
	static float MapFloatByCurve(float InputValue, UCurveFloat* Curve, float CurveXValue);

	// ???AttributeMap?????Tag???
	UFUNCTION(BlueprintCallable)
	static void SetAttributeMapMember(UPARAM(ref) TMap<FGameplayTag, FAttributeFloat>& TargetMap, FGameplayTag InAttributeTag, FAttributeFloat InValue, bool& found);

	UFUNCTION(BlueprintCallable)
	static void StopAndReleaseUpdateValueDuration(UPARAM(ref) UTask_UpdateValueDuration*& AsyncTask);

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Calculates the unsigned angle in degrees between the Origin actor's forward vector (projected onto the XY plane) and the look-at vector from Origin to Target (also projected onto the XY plane). Returns a value in the range [0, 180]."))
	static float GetDeltaAngleRotationAndLookAtTarget(const AActor* OriginActor, const AActor* TargetActor);

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Calculates the signed angle in degrees between the Origin actor's forward vector (projected onto the XY plane) and the look-at vector from Origin to Target (also projected onto the XY plane). Returns a value in the range [-180, 180]."))
	static float GetDeltaAngleRotationAndLookAtTargetWithRotInfo(const AActor* OriginActor, const AActor* TargetActor, bool& bIsClockwise);

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Checks if the specified Actor is of the given class or a subclass of it."))
	static bool IsActorSubclassOf(const AActor* Actor, TSubclassOf<AActor> ParentClass);

	/**
	 * Generate evenly spaced points along an arc around StartLocation (XY plane),
	 * facing toward QuerierLocation. Optionally project each point to NavMesh.
	 *
	 * @param WorldContextObject Any object that can provide a UWorld (e.g., this, AIController, etc.)
	 * @param StartLocation      Arc center (usually the target).
	 * @param QuerierLocation    Direction reference (arc faces the querier).
	 * @param ArcDegrees         Total arc span in degrees [0..360].
	 * @param DistanceBetweenPoints  Desired arc-length spacing between points (cm).
	 * @param bProjectToNav      If true, project to NavMesh.
	 * @param NavProjectExtent   XY extent used for nav projection (radius in cm).
	 * @param bDiscardIfNotNavigable  If true and projection fails, the point is discarded; otherwise keep original.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static TArray<FVector> GenerateArcEQSPoints(const UObject* WorldContextObject, FVector StartLocation, FVector QuerierLocation, float ArcDegrees, float DistanceBetweenPoints, bool bProjectToNav = true,float NavProjectExtent = 50.f, bool bDiscardIfNotNavigable = true, bool bIsLeft = false);

	// ========================= ?????????????????????? ===================================
	UFUNCTION(BlueprintCallable, Category = "Combat Positioning")
	static FOctantSlot EvaluateSuitableOctantSlot(const TArray<FOctantSlot>& OctantSlots, AActor* Querier, float DistanceWeight, float OccupancyWeight, TMap<int, float> CustomOctantWeight, EWeightCalculationMethod Method);

	UFUNCTION(BlueprintPure, Category = "Combat Positioning")
	static TArray<FOctantSlot> GetOctantSlots(AActor* CPMOwner, AActor* QuerierActor, TArray<AActor*> Actors, TMap<AActor*, FVector> Reservation, float MaxSlotRadius);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"), Category = "Combat Positioning")
	static TArray<FVector> SpawnPositioningPointsInOctantSlot(const UObject* WorldContextObject, const FOctantSlot& OctantSlot, float MinRadius, float MaxRadius, int PointsNum, bool bProjectToNav = true, float NavProjectExtent = 50.f, bool bDiscardIfNotNavigable = true);

	UFUNCTION(BlueprintPure, Category = "Combat Positioning")
	static int GetOctantIndex(AActor* CPMOwner, AActor* Querier);

	UFUNCTION(BlueprintPure, Category = "Combat Positioning")
	static int GetOctantIndexFromSlot(FOctantSlot InSlot, AActor* Querier);
	// ============================================================

	UFUNCTION(BlueprintPure)
	static float GetSignedAngleBetween(const FVector& OriginalForward, const FVector& TargetForward);

	// =========================== ??????? ================================

        UFUNCTION(BlueprintPure)
        static float CalculateAttackScore(AActor* Attacker, AActor* Target, float distanceWeight, float angleWeight, float MaxAtkDist);

        /**
         * Helper for root-motion driven AI locomotion. Consumes a pre-generated navigation path
         * and outputs the planar movement direction (suitable for 8-way blend spaces) as well as
         * an interpolated facing rotation.
         *
         * @param Pawn                               Pawn that should follow the path.
         * @param NavigationPath                     Path instance created elsewhere (e.g. via UNavigationSystemV1).
         * @param AcceptanceRadius                   Distance threshold used to advance to the next point (cm).
         * @param DeltaTime                          Frame delta time used for rotation interpolation.
         * @param RotationInterpSpeed                Interp speed for facing rotation. <= 0 means snap instantly.
         * @param MovementMode                       Determines whether the pawn turns to face the path or keeps a fixed world rotation.
         * @param FixedWorldRotation                 Only used when MovementMode is MaintainWorldRotation.
         * @param InOutPathPointIndex                Index of the current target point along the path (will be advanced as needed).
         * @param OutBlendspaceDirection             Normalized planar direction expressed in the pawn's local space.
         * @param OutFacingRotation                  Interpolated rotation that should be applied to the pawn.
         * @param bOutReachedPathEnd                 True once the final path point has been reached.
         */
        UFUNCTION(BlueprintCallable, Category = "AI|RootMotion")
        static bool UpdateRootMotionPathFollowing(APawn* Pawn, UNavigationPath* NavigationPath, float AcceptanceRadius, float DeltaTime, float RotationInterpSpeed, ERMPathFollowingMovementMode MovementMode, FRotator FixedWorldRotation, int32& InOutPathPointIndex, FVector2D& OutBlendspaceDirection, FRotator& OutFacingRotation, bool& bOutReachedPathEnd);
};
