// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UObject/Object.h"
#include "BTDecorator_ResetBlackboardKeys.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_ResetBlackboardKeys : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_ResetBlackboardKeys();
	
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBlackboardKeySelector> BBKeys;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnActivation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnDeactivation;

private:
	void ResetBlackboardKeys(UBehaviorTreeComponent& BTComponent);
};
