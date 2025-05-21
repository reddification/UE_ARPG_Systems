// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_HandleGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_HandleGameplayAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_HandleGameplayAbility();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CompletedMessageTag;

private:
	FGameplayTag ActivationFailedCantAffordTag;
	FGameplayTag ActivationFailedConditionsNotMetTag;
};
