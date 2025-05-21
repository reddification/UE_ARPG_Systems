#pragma once

#include "CommonTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FGameplayEffectsWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)	
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FShapeComponentsWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UShapeComponent*> Shapes;
};
