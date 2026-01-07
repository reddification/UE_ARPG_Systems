#pragma once

#include "GameplayTagContainer.h"
#include "CommonWrapperTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FParametrizedGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="SetByCaller"))
	TMap<FGameplayTag, float> SetByCallerParameters;	
};

USTRUCT(BlueprintType)
struct FGameplayEffectsWrapper
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)	
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects_Obsolete;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)	
	TArray<FParametrizedGameplayEffect> ParametrizedGameplayEffects;
};

USTRUCT(BlueprintType)
struct ARPGAI_API FShapeComponentsWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UShapeComponent*> Shapes;
};
