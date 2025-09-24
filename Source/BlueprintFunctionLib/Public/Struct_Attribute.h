// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Struct_Attribute.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAttributeFloat
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 0.0f;
};