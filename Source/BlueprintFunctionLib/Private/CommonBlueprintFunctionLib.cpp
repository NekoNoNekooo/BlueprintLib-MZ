// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonBlueprintFunctionLib.h"
#include "Struct_Hit.h"
#include "Struct_Attribute.h"
#include "Chooser.h"
#include "Task_UpdateValueDuration.h"
#include "NavigationSystem.h"
#include "Animation/AnimMontage.h"
#include "Struct_CombatPositioning.h"

bool UCommonBlueprintFunctionLib::EvaluateHitAnimMontage(const FCHTHitMontageInput InputStruct, const UChooserTable* ChooserTable, FCHTHitMontageOutput& OutStruct)
{
	if (!ChooserTable)
	{
		return false;
	}

	// ��������������
	FChooserEvaluationContext Context;
	Context.AddStructParam(const_cast<FCHTHitMontageInput&>(InputStruct));

	// �������
	FCHTHitMontageOutput TempStructOutput;

	// ����������
	Context.Params.Add(FStructView::Make(TempStructOutput));

	// ��������
	UChooserTable::EvaluateChooser(Context, ChooserTable,
		FObjectChooserBase::FObjectChooserIteratorCallback::CreateLambda([](UObject*)
			{
				return FObjectChooserBase::EIteratorStatus::ContinueWithOutputs;
			}));

	// �����������
	for (const FStructView& View : Context.Params)
	{
		if (View.IsValid() && View.GetScriptStruct() == FCHTHitMontageOutput::StaticStruct())
		{
			const FCHTHitMontageOutput* Result = reinterpret_cast<const FCHTHitMontageOutput*>(View.GetMemory());
			if (Result)
			{
				OutStruct = *Result;

				// �������������Ƿ���Ч
				if (OutStruct.HitMontage && OutStruct.HitTag.IsValid())
				{
					FString DebugMontageName = OutStruct.HitMontage->GetName();
					FString DebugTagName = OutStruct.HitTag.ToString();
					UE_LOG(LogTemp, Log, TEXT("[BPC_HitEffect] EvaluateHitAnimMontage -> Montage: %s | HitTags: %s"),
						*DebugMontageName,
						*DebugTagName
					)
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("[BPC_HitEffect] EvaluateFailed, result is not valid."))
					return false;
				}
				return true;
			}
		}
	}

	return false;
}

float UCommonBlueprintFunctionLib::MapFloatByCurve(float InputValue, UCurveFloat* Curve, float CurveXValue)
{
	if (!Curve)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *MapFloatByCurve* Curve is null"));
		return InputValue;
	}

	// ���� X �����ȡ����ֵ��Yֵ0~1��
	float CurveValue = Curve->GetFloatValue(CurveXValue);

	// ӳ�䣺ԭֵ * ����Yֵ
	return InputValue * CurveValue;
}

void UCommonBlueprintFunctionLib::SetAttributeMapMember(UPARAM(ref) TMap<FGameplayTag, FAttributeFloat>& TargetMap, FGameplayTag InAttributeTag, FAttributeFloat InValue, bool& found)
{
	if (FAttributeFloat* FoundValue = TargetMap.Find(InAttributeTag))
	{
		*FoundValue = InValue; // Key ���� -> ����Ϊ�½ṹ
		found = true;
	}
	else
	{
		found = false; // Key�����ڣ������κβ���
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *SetAttributeMapMember* key '%s' not found, do nothing."), *InAttributeTag.ToString());
	}
}

float UCommonBlueprintFunctionLib::GetDeltaAngleRotationAndLookAtTarget(const AActor* OriginActor, const AActor* TargetActor)
{
	if (!OriginActor || !TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetDeltaAngleRotationAndLookAtTarget* OriginActor or TargetActor is null."));
		return 0.0f;
	}

	FVector OriginLocation = OriginActor->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	FVector OriginForwardVector2D = OriginActor->GetActorForwardVector().GetSafeNormal2D();
	FVector LookAtVector2D = (TargetLocation - OriginLocation).GetSafeNormal2D();

	float DeltaAngle = FMath::Acos(FVector::DotProduct(OriginForwardVector2D, LookAtVector2D));

	return FMath::RadiansToDegrees(DeltaAngle);
}

float UCommonBlueprintFunctionLib::GetDeltaAngleRotationAndLookAtTargetWithRotInfo(const AActor* OriginActor, const AActor* TargetActor, bool& bIsClockwise)
{
	if (!OriginActor || !TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetDeltaAngleRotationAndLookAtTarget* OriginActor or TargetActor is null."));
		return 0.0f;
	}

	FVector OriginLocation = OriginActor->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	FVector Forward2D = OriginActor->GetActorForwardVector().GetSafeNormal2D();
	FVector ToTarget2D = (TargetLocation - OriginLocation).GetSafeNormal2D();

	// ��ˣ�cos�ȣ�
	float Dot = FVector::DotProduct(Forward2D, ToTarget2D);
	// ��˵�Z������sin�ȣ�
	float CrossZ = FVector::CrossProduct(Forward2D, ToTarget2D).Z;

	if (CrossZ < 0.f)
	{
		// �����˵�Z����Ϊ����˵������ʱ����ת
		bIsClockwise = false;
	}
	else
	{
		// ������˳ʱ����ת
		bIsClockwise = true;
	}

	// atan2(y, x) = atan2(sin��, cos��)
	float AngleRad = FMath::Atan2(CrossZ, Dot);

	return FMath::RadiansToDegrees(AngleRad);
}

bool UCommonBlueprintFunctionLib::IsActorSubclassOf(const AActor* Actor, TSubclassOf<AActor> ParentClass)
{
	if (!Actor || !*ParentClass)
	{
		return false;
	}

	// IsA() returns true if Actor's class is the same as ParentClass or a subclass of it.
	return Actor->IsA(ParentClass);
}

static bool ProjectPointToNav(UWorld* World, const FVector& In, FVector& Out, float Extent)
{
	if (!World) return false;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys) return false;

	const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);

	FNavLocation NavLoc;
	const bool bOk = NavSys->ProjectPointToNavigation(In, NavLoc, FVector(Extent), NavData);
	if (bOk)
	{
		Out = NavLoc.Location;
	}
	return bOk;
}

TArray<FVector> UCommonBlueprintFunctionLib::GenerateArcEQSPoints(const UObject* WorldContextObject, FVector StartLocation, FVector QuerierLocation, float ArcDegrees, float DistanceBetweenPoints, bool bProjectToNav, float NavProjectExtent, bool bDiscardIfNotNavigable, bool bIsLeft)
{
	UE_LOG(LogTemp, Log, TEXT("Use function: GenerateArcEQSPoints"));

	TArray<FVector> Points;

	// ��ȫ���
	ArcDegrees = FMath::Clamp(ArcDegrees, 0.f, 360.f);
	if (ArcDegrees <= KINDA_SMALL_NUMBER || DistanceBetweenPoints <= KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	// �뾶 = ���ĵ� Querier �� XY ����
	FVector ToQuerier = QuerierLocation - StartLocation;
	ToQuerier.Z = 0.f;
	const float Radius = ToQuerier.Size();
	if (Radius <= KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	// ������ָ�� Querier
	const FVector ForwardDir = ToQuerier.GetSafeNormal();
	const float HalfArc = ArcDegrees * 0.5f;

	// ���ݻ�������ǲ���
	const float AngleStepDeg = (DistanceBetweenPoints / Radius) * (180.f / PI);
	if (AngleStepDeg <= KINDA_SMALL_NUMBER)
	{
		return Points;
	}

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	if (bIsLeft)
	{
		// ��QuerierΪ���ɿ�ʼλ�� ��������ɵ�
		for (float Angle = 0; Angle >= -HalfArc; Angle -= AngleStepDeg)
		{
			UE_LOG(LogTemp, Log, TEXT("Angle: %f"), Angle);

			const FVector Dir = ForwardDir.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();
			FVector P = StartLocation + Dir * Radius;

			if (bProjectToNav)
			{
				FVector Projected;
				const bool bOk = ProjectPointToNav(World, P, Projected, NavProjectExtent);
				if (bOk)
				{
					P = Projected;
				}
				else if (bDiscardIfNotNavigable)
				{
					continue; // ���������ߵĵ�
				}
				// ������ԭ�㣨δͶ�䣩
			}

			Points.Add(P);
		}
	}
	else
	{
		// ��QuerierΪ���ɿ�ʼλ�� ���Ҳ����ɵ�
		for (float Angle = 0; Angle <= HalfArc; Angle += AngleStepDeg)
		{
			UE_LOG(LogTemp, Log, TEXT("Angle: %f"), Angle);
			const FVector Dir = ForwardDir.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();
			FVector P = StartLocation + Dir * Radius;
			if (bProjectToNav)
			{
				FVector Projected;
				const bool bOk = ProjectPointToNav(World, P, Projected, NavProjectExtent);
				if (bOk)
				{
					P = Projected;
				}
				else if (bDiscardIfNotNavigable)
				{
					continue; // ���������ߵĵ�
				}
				// ������ԭ�㣨δͶ�䣩
			}
			Points.Add(P);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d points along arc."), Points.Num());

	return Points;
}


FOctantSlot UCommonBlueprintFunctionLib::EvaluateSuitableOctantSlot(const TArray<FOctantSlot>& OctantSlots, AActor* Querier, float DistanceWeight, float OccupancyWeight, TMap<int, float> CustomOctantWeight, EWeightCalculationMethod Method)
{
	if (!Querier)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *EvaluateSuitableOctantSlot* Querier is null."));
		return FOctantSlot();
	}
	if (OctantSlots.Num() != 8)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *EvaluateSuitableOctantSlot* OctantSlots array must contain exactly 8 slots."));
		return FOctantSlot();
	}

	int QuerierOctantIndex = GetOctantIndexFromSlot(OctantSlots[0], Querier);
	TArray<float> Scores;

	
	

	for(const FOctantSlot& Slot : OctantSlots)
	{
		float score = 0.f;

		//
		int DeltaIndex = FMath::Abs(Slot.OctantIndex - QuerierOctantIndex);

		if (DeltaIndex == 0) //
		{
			score += 100.f * DistanceWeight; //
		}
		else if (DeltaIndex == 1 || DeltaIndex == 7) //
		{
			score += 20.f * DistanceWeight; //
		}
		else if (DeltaIndex == 2 || DeltaIndex == 6) //
		{
			score += 10.f * DistanceWeight; //
		}
		else //
		{
			score += 0.f; //
		}
		
		//
		score -= Slot.Occupancy * 10.f * OccupancyWeight;
		score = FMath::Clamp(score, 0.f, 1000.f);

		//
		if (CustomOctantWeight.Contains(Slot.OctantIndex))
		{
			score *= CustomOctantWeight[Slot.OctantIndex];
		}

		/*UE_LOG(LogTemp, Log, TEXT("[CommonBlueprintFunctionLib] *EvaluateSuitableOctantSlot* Slot Index: %d | DistanceScore: %.2f | Occupancy: %d | TotalScore: %.2f"),
			Slot.OctantIndex,
			score - Slot.Occupancy * 10.f * OccupancyWeight,
			Slot.Occupancy,
			score
		);*/

		Scores.Add(score);
	}

	int BestIndex = 0;
	float Total = 0.f;

	switch (Method)
	{
	case EWeightCalculationMethod::HighestScore:
	{

		for (int i = 1; i < Scores.Num(); i++)
		{
			if (Scores[i] > Scores[BestIndex])
			{
				BestIndex = i;
			}
		}

		for (int i = 1; i < Scores.Num(); i++)
		{
			if (Scores[i] > Scores[BestIndex])
			{
				BestIndex = i;
			}
		}
	
		UE_LOG(LogTemp, Log, TEXT("[CommonBlueprintFunctionLib] *EvaluateSuitableOctantSlot* HighestScore: Best Slot Index: %d | Score: %.2f"), BestIndex, Scores[BestIndex]);


		break;
	}

	case EWeightCalculationMethod::RandomByScoreWeight:
	{

		for (float Score : Scores)
		{
			Total += Score;
		}

		float RandPoint = FMath::FRandRange(0.f, Total);
		float Accum = 0.f;

		for (int i = 0; i < Scores.Num(); i++)
		{
			Accum += Scores[i];
			if (RandPoint <= Accum)
			{
				BestIndex = i;

				break;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[CommonBlueprintFunctionLib] *EvaluateSuitableOctantSlot* RandomByScoreWeight: Best Slot Index: %d | Score: %.2f"), BestIndex, Scores[BestIndex]);

		break;
	}
	}

	return OctantSlots[BestIndex];
}

TArray<FOctantSlot> UCommonBlueprintFunctionLib::GetOctantSlots(AActor* CPMOwner, AActor* QuerierActor, TArray<AActor*> Actors, TMap<AActor*, FVector> Reservation, float MaxSlotRadius)
{
	if (!CPMOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetOctantSlots* CPMOwner is null."));
		return TArray<FOctantSlot>();
	}

	// Clamp MaxSlotRadius to a reasonable range
	MaxSlotRadius = FMath::Clamp(MaxSlotRadius, 100.f, 5000.f);

	// Get owner's forward vector 2D
	FVector OwnerForward2D = CPMOwner->GetActorForwardVector().GetSafeNormal2D();
	

	TArray<FOctantSlot> OctantSlots;

	// Initialize 8 octant slots
	for (int i = 0; i < 8; i++)
	{
		FOctantSlot NewSlot;
		NewSlot.OctantIndex = i;
		NewSlot.Location = CPMOwner->GetActorLocation(); // Start at owner location
		NewSlot.Occupancy = 0;
		NewSlot.FacingVector = OwnerForward2D.RotateAngleAxis(i * 45.f, FVector::UpVector); // ÿ���۵ķ���

		OctantSlots.Add(NewSlot);
	}

	// UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetOctantSlots* Reservation count: %d"), Reservation.Num());

	// Calculate occupancy
	for (auto Actor: Actors)
	{
		// filter out null or self
		if (!Actor || Actor == CPMOwner || Actor == QuerierActor) continue;

		if(Reservation.Contains(Actor))
		{
			// If the actor has a reservation, use that location instead
			FVector ReservedLocation = Reservation[Actor];
			float distance = FVector::Dist2D(ReservedLocation, CPMOwner->GetActorLocation());
			if (distance > MaxSlotRadius) continue;
			int OctantIndex = GetOctantIndex(CPMOwner, Actor);
			if (OctantSlots.IsValidIndex(OctantIndex))
			{
				OctantSlots[OctantIndex].Occupancy++;
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetOctantSlots* Calculated invalid OctantIndex from reservation: %d"), OctantIndex);
			}
			continue; // Skip further processing for this actor
		}

		// filter out actors that are too far
		float distance = FVector::Dist2D(Actor->GetActorLocation(), CPMOwner->GetActorLocation());

		if (distance > MaxSlotRadius) continue;

		int OctantIndex = GetOctantIndex(CPMOwner, Actor);

		if (OctantSlots.IsValidIndex(OctantIndex))
		{
			OctantSlots[OctantIndex].Occupancy++;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *GetOctantSlots* Calculated invalid OctantIndex: %d"), OctantIndex);
		}
	}

	return OctantSlots;
}

TArray<FVector> UCommonBlueprintFunctionLib::SpawnPositioningPointsInOctantSlot(const UObject* WorldContextObject, const FOctantSlot& OctantSlot, float MinRadius, float MaxRadius, int PointsNum, bool bProjectToNav, float NavProjectExtent, bool bDiscardIfNotNavigable)
{

	TArray<FVector> PointsLocation;

	for (size_t i = 0; i < PointsNum; i++)
	{


		float RandRadius = FMath::RandRange(MinRadius, MaxRadius);
		float RandAngle = FMath::RandRange(-22.5f, 22.5f);

		FVector Dir = OctantSlot.FacingVector.RotateAngleAxis(RandAngle, FVector::UpVector).GetSafeNormal();
		FVector PointLocation = OctantSlot.Location + Dir * RandRadius;

		// Project to NavMesh if needed
		if (bProjectToNav)
		{
			UWorld* World = GEngine ? GEngine->GetWorldFromContextObjectChecked(WorldContextObject) : nullptr;
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
			const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);

			FNavLocation NavLoc;
			const bool bOk = NavSys->ProjectPointToNavigation(PointLocation, NavLoc, FVector(NavProjectExtent), NavData);
			if (bOk)
			{
				PointLocation = NavLoc.Location;
			}
			else if (bDiscardIfNotNavigable)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *SpawnPositioningPointsInOctantSlot* Point projection to NavMesh failed, discarding point."));

				continue; // Discard this point
			}
		}

		PointsLocation.Add(PointLocation);
	}

	return PointsLocation;
}

int UCommonBlueprintFunctionLib::GetOctantIndex(AActor* CPMOwner, AActor* Querier)
{
	FVector OwnerForward2D = CPMOwner->GetActorForwardVector().GetSafeNormal2D();
	FVector ToActor2D = (Querier->GetActorLocation() - CPMOwner->GetActorLocation()).GetSafeNormal2D();

	float dot = FMath::Clamp(FVector::DotProduct(OwnerForward2D, ToActor2D), -1.f, 1.f); // get normalized dot product value

	float AngleDeg = FMath::Acos(dot) * (180.f / PI); // Convert to degrees

	// Determine if the angle is clockwise or counter-clockwise
	FVector Cross = FVector::CrossProduct(OwnerForward2D, ToActor2D);
	if (Cross.Z < 0.f)
	{
		AngleDeg = 360.f - AngleDeg; // Adjust for clockwise direction
	}

	// Determine octant index (0-7)
	int OctantIndex = FMath::FloorToInt((AngleDeg + 22.5f) / 45.f) % 8;

	return OctantIndex;
}

int UCommonBlueprintFunctionLib::GetOctantIndexFromSlot(FOctantSlot InSlot, AActor* Querier)
{
	FVector OwnerForward2D = InSlot.FacingVector;
	FVector ToActor2D = (Querier->GetActorLocation() - InSlot.Location).GetSafeNormal2D();
	float dot = FMath::Clamp(FVector::DotProduct(OwnerForward2D, ToActor2D), -1.f, 1.f); // get normalized dot product value
	float AngleDeg = FMath::Acos(dot) * (180.f / PI); // Convert to degrees
	// Determine if the angle is clockwise or counter-clockwise
	FVector Cross = FVector::CrossProduct(OwnerForward2D, ToActor2D);
	if (Cross.Z < 0.f)
	{
		AngleDeg = 360.f - AngleDeg; // Adjust for clockwise direction
	}
	// Determine octant index (0-7)
	int OctantIndex = FMath::FloorToInt((AngleDeg + 22.5f) / 45.f) % 8;
	return OctantIndex;
}

float UCommonBlueprintFunctionLib::GetSignedAngleBetween(const FVector& OriginalForward, const FVector& TargetForward)
{
	FVector A = OriginalForward.GetSafeNormal();
	FVector B = TargetForward.GetSafeNormal();

	// �����н�
	float Dot = FVector::DotProduct(A, B);
	Dot = FMath::Clamp(Dot, -1.0f, 1.0f);
	float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

	// ����ж�����
	FVector Cross = FVector::CrossProduct(A, B);
	float Sign = Cross.Z;

	return (Sign >= 0) ? AngleDeg : -AngleDeg;
}

float UCommonBlueprintFunctionLib::CalculateAttackScore(AActor* Attacker, AActor* Target, float distanceWeight, float angleWeight, float MaxAtkDist)
{
	const FVector ALoc = Attacker->GetActorLocation();
	const FVector TLoc = Target->GetActorLocation();
	const FVector ToTarget = TLoc - ALoc;

	// �������÷֣�Խ��Խ��
	float Dist = FMath::Clamp(ToTarget.Size(), 0.f, MaxAtkDist);
	float DistScore = 1.f - FMath::Clamp(Dist / MaxAtkDist, 0.f, 1.f);

	// ���㷽λ�Ƕȵ÷֣��෽�ͺ󷽸���
	const FVector TargetForward = Target->GetActorForwardVector().GetSafeNormal2D();
	const FVector ToTargetDir = ToTarget.GetSafeNormal2D();
	float Dot = FVector::DotProduct(TargetForward, ToTargetDir);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);
	
	float AngleScore = 1.f - ((Dot + 1.f) / 2.f); // ӳ�䵽 [0,1]���෽�ͺ󷽵÷ָ���
	
	// �ۺϵ÷� ӳ�䵽 0-100
	float TotalScore = DistScore * distanceWeight + AngleScore * angleWeight;
	TotalScore = FMath::GetMappedRangeValueClamped(FVector2f(0.f, 1.f),FVector2f(0.f, 100.f), TotalScore);

	return TotalScore;
}

void UCommonBlueprintFunctionLib::RM_MoveTo(UObject* WorldContextObject, struct FLatentActionInfo LatentInfo,
	class ACharacter* Character, FVector TargetLocation, FRMMoveToOptions Options, ERMMoveToResult& OutResult)
{
	
}

void UCommonBlueprintFunctionLib::StopAndReleaseUpdateValueDuration(UTask_UpdateValueDuration*& AsyncTask)
{
	if (AsyncTask)
	{
		AsyncTask->StopTask();
		AsyncTask = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommonBlueprintFunctionLib] *StopUpdateValueDuration* AsyncTask is null, do nothing."));
	}
}
