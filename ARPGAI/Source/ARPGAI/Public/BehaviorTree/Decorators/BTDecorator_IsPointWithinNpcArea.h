// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsPointWithinNpcArea.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_IsPointWithinNpcArea : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsPointWithinNpcArea();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector LocationBBKey;

	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.f, UIMin = 0.f))
	float Extent = 1.f;
};
