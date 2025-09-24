// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_PredictAppearance.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_PredictAppearance : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInversePath = false;

	// hierarchical PF is cheaper, but less accurate
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseHierarchicalPathfinding = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PredictionTargetParameterTag;
};
