

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EWeightCalculationMethod : uint8
{
    HighestScore   UMETA(DisplayName = "Highest Score"),
    RandomByScoreWeight   UMETA(DisplayName = "ScoreWeight"),
};
