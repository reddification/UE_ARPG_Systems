// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_DecayBlackboardValueOverTime.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_DecayBlackboardValueOverTime : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_DecayBlackboardValueOverTime
	{
		FDelegateHandle BlackboardKeyChangedDelegateHandle;
		FDelegateHandle GateChangedDelegateHandle;
	};

public:
	UBTService_DecayBlackboardValueOverTime();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_DecayBlackboardValueOverTime); };

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
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
