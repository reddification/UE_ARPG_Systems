// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UObject/Object.h"
#include "BTDecorator_BlackboardGate.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_BlackboardGate : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_BlackboardGate : public FBTAuxiliaryMemory
	{
		float OldGateValue = 0.f;
		bool bNodeActive = false;
	};
	
public:
	UBTDecorator_BlackboardGate();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_BlackboardGate); };
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Open = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Close = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector GateParameterBBKey;

private:
	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};