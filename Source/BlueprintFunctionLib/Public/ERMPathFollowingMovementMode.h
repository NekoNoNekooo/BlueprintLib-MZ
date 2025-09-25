#pragma once

#include "CoreMinimal.h"
#include "ERMPathFollowingMovementMode.generated.h"

/**
 * Movement modes for root-motion driven path following.
 */
UENUM(BlueprintType)
enum class ERMPathFollowingMovementMode : uint8
{
        FacePath UMETA(DisplayName = "Face Path"),
        MaintainWorldRotation UMETA(DisplayName = "Maintain World Rotation")
};

