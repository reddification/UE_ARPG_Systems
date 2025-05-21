// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "UObject/Object.h"
#include "BTDecorator_GameplayTagCondition.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_GameplayTagCondition : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTDecorator_GameplayTagCondition();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer GameplayTagContainer;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery GameplayTagQuery;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseTagQuery = false;
};
