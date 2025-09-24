// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Decorators/BTDecorator_LoadDynamicTrees.h"
#include "EnhancedBehaviorTreeComponent.generated.h"

// TODO service with observable messages support?
UCLASS()
class ARPGAI_API UEnhancedBehaviorTreeComponent : public UBehaviorTreeComponent
{
	GENERATED_BODY()

public:
	virtual void HandleMessage(const FAIMessage& Message) override;
	void HandleMessageImmediately(const FAIMessage& Message);
	void LoadDynamicTrees(const FGameplayTagContainer& BehaviorTags, UBTCompositeNode* StartingNode);
};
