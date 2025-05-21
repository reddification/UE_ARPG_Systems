// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CombatDamageExecutionCalculation.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UCombatDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UCombatDamageExecutionCalculation();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
