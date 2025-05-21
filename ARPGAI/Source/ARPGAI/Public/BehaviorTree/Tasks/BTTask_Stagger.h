// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BTTask_HandleGameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Stagger.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_Stagger : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_Stagger();
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
