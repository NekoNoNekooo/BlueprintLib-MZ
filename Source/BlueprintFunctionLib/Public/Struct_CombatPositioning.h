

#pragma once

#include "CoreMinimal.h"
#include "Struct_CombatPositioning.generated.h"
/**
 * 
 */

USTRUCT(Blueprintable)
struct FOctantSlot
{
	GENERATED_BODY()

public:

	// 方位槽的索引, 0代表拥有者正前方的八分之一区域, 顺时针递增
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
	int32 OctantIndex = -1;

	// 方位槽的起始点，实际上就是拥有者的位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
	FVector Location = FVector::ZeroVector;

	// 已经占用这个槽的敌人数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
	int32 Occupancy = 0;

	// 向前向量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
	FVector FacingVector = FVector::ForwardVector;
};