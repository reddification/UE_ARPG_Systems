// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "EnhancedBehaviorTreeComponent.generated.h"

// TODO service with observable messages support?
UCLASS()
class ARPGAI_API UEnhancedBehaviorTreeComponent : public UBehaviorTreeComponent
{
	GENERATED_BODY()

public:
	void SetDynamicSubtreeAtIndex(FGameplayTag InjectTag, UBehaviorTree* BehaviorAsset, const int32 InstanceStackStartIndex);
	virtual void SetDynamicSubtree(FGameplayTag InjectTag, UBehaviorTree* BehaviorAsset) override;
	virtual void HandleMessage(const FAIMessage& Message) override;
	void HandleMessageImmediately(const FAIMessage& Message);
};
