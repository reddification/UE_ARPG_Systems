// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "UObject/Object.h"
#include "UpdateSpeedToBeAtPlaceInTimeMMC.generated.h"

UCLASS()
class ARPGAI_API UUpdateSpeedToBeAtPlaceInTimeMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UUpdateSpeedToBeAtPlaceInTimeMMC();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition DistanceToTargetDef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMIn = 0.f, ClampMin = 0.f))
	float MinSpeedDefault = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(UIMIn = 0.f, ClampMin = 0.f))
	float MaxSpeedDefault = 1000.f;
};
