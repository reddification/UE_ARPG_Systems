// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UObject/Object.h"
#include "UpdateSpeedToCatchUpWithTargetMMC.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UUpdateSpeedToCatchUpWithTargetMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
