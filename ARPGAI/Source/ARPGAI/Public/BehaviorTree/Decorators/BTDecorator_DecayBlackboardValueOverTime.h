// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_DecayBlackboardValueOverTime.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_DecayBlackboardValueOverTime : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_DecayBlackboardValueOverTime : public FBTAuxiliaryMemory
	{
		FDelegateHandle BlackboardKeyChangedDelegateHandle;
		FDelegateHandle GateChangedDelegateHandle;
	};
	
public:
	UBTDecorator_DecayBlackboardValueOverTime();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_DecayBlackboardValueOverTime); };
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector GateBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BlackboardKey;
	
	UPROPERTY(EditAnywhere)
	bool bGateInversed = false;
	
	UPROPERTY(EditAnywhere)
	float ResetDelayTime = 5.f;
	
	
private:
	EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);
	EBlackboardNotificationResult OnGateChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);

};
