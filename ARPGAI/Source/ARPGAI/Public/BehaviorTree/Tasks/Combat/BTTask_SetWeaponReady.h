// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "UObject/Object.h"
#include "BTTask_SetWeaponReady.generated.h"

/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTTask_SetWeaponReady : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_SetWeaponReady();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSetReady = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAwaitCompletion = true;
};
