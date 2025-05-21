// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BTTask_HandleGameplayAbility.h"
#include "UObject/Object.h"
#include "BTTask_GetParried.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_GetParried : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_GetParried();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
