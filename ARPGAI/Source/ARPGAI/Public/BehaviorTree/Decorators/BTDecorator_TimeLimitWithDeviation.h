// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTDecorator_TimeLimitWithDeviation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_TimeLimitWithDeviation : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_TimeLimitWithDeviation
	{
		bool bElapsed = false;
		float ActualTimeLimit = 0.f;
	};

	
public:
	UBTDecorator_TimeLimitWithDeviation();
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnNodeProcessed(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

	UPROPERTY(Category=Decorator, EditAnywhere)
	FValueOrBBKey_Float TimeLimit = 10.f;
	
	UPROPERTY(EditAnywhere)
	float DeviationTime = 5.f;

	UPROPERTY(EditAnywhere)
	bool bUseDeviationFraction = false;

	UPROPERTY(EditAnywhere)
	bool bFinishWithSuccessOnTimeLimit = false; 
};
