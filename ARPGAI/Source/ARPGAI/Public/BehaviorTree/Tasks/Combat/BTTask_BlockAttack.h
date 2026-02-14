// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "Data/NpcCombatTypes.h"
#include "UObject/Object.h"
#include "BTTask_BlockAttack.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_BlockAttack : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_BlockAttack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	void HandleBlockResult(UBehaviorTreeComponent& OwnerComp, ENpcBlockResult BlockResult);

	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BlockResultBBKey;
	
	UPROPERTY(EditAnywhere)
	float BackdashChanceCoefficient = 2.f;
};
