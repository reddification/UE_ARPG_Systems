// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BTTask_HandleGameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_Parry.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_Parry : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_Parry();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBlackboardKeySelector ParriedAttackBBKey;
};
