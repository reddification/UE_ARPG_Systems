// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_LoadDynamicTrees.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_LoadDynamicTrees : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_LoadDynamicTrees();
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, meta=(Categories="AI.Behavior"))
	FGameplayTagContainer DynamicBehaviorTags;

	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
};
