// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_BroadcastBehaviorTag.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_BroadcastBehaviorTag : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_BroadcastBehaviorTag();
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BehaviorTag;
};
