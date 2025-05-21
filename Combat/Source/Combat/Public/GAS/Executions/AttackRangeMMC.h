// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UObject/Object.h"
#include "AttackRangeMMC.generated.h"

/**
 *  Not very reliable... i'd say this MMC is experimental
 *  There are cases like spears when an actual attack range is longer than the size of the damaging part
 *  So perhaps just have a separate gameplay effect each with it's own range
 */
UCLASS()
class COMBAT_API UAttackRangeMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
