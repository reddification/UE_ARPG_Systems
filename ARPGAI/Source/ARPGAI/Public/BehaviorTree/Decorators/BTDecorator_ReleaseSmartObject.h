// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ReleaseSmartObject.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_ReleaseSmartObject : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_ReleaseSmartObject();
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ClaimedSmartObjectClaimHandleBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActiveSmartObjectClaimHandleBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector InteractionActorBBKey;
};
