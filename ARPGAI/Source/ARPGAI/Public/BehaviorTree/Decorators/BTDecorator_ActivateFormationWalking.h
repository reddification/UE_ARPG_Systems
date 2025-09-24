// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ActivateFormationWalking.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_ActivateFormationWalking : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_ActivateFormationWalking();
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
};
