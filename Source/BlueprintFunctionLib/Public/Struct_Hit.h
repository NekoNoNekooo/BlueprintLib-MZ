// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Struct_Hit.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHitParameter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> HitSource = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SourceActionTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SourceAttackMarkTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer DamageTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHitResult HitResult;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct FCHTHitMontageInput
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer CharacterTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHitParameter HitParameter;
};

USTRUCT(BlueprintType)
struct FCHTHitMontageOutput
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HitTag;
};