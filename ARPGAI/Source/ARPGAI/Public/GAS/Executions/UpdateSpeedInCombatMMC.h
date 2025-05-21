// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UpdateSpeedInCombatMMC.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UUpdateSpeedInCombatMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
