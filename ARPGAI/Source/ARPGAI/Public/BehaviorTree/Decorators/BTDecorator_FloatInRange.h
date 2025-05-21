// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_FloatInRange.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_FloatInRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_FloatInRange();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Min = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Max = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector FloatBBKey;
	
private:
	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
