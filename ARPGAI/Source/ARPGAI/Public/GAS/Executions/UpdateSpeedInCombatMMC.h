// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UpdateSpeedInCombatMMC.generated.h"

/**
 *  This MMC calculates scale for MoveSpeed attribute to match required speed based on data
 */
UCLASS()
class ARPGAI_API UUpdateSpeedInCombatMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UUpdateSpeedInCombatMMC();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition DistanceToTargetDef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReturnScale = false;;
};
