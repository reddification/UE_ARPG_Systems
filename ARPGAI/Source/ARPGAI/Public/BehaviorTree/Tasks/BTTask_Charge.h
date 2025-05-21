// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BTTask_HandleGameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_Charge.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_Charge : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_Charge();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float VerticalImpulseStrength = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ForwardImpulseStrength = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bChargeToTarget = true;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;
};
