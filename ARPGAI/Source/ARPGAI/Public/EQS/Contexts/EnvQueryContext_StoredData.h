// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_StoredData.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARPGAI_API UEnvQueryContext_StoredData : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DataTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bConsumeAfterReading = false;
};
