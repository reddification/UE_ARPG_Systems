// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_EvaluateDistanceToTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_EvaluateDistanceToTarget : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_EvaluateDistanceToTarget : public FBTAuxiliaryMemory
	{
		FDelegateHandle DelegateHandle;
		float AccumulatedTargetDeltaDistance = 0.f;
		int AccumulatedTargetDeltasCount = 0;
		FVector PreviousTargetLocation = FVector::ZeroVector;
		TWeakObjectPtr<class UNpcCombatLogicComponent> NpcCombatLogicComponent;
	};
	
public:
	UBTService_EvaluateDistanceToTarget();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_EvaluateDistanceToTarget); }
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUsePathfinding = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseSquareDistance = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseUpdateCondition = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float UpdateDistanceThreshold = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bUseUpdateCondition"))
	FBlackboardKeySelector UpdateConditionBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutDistanceBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutEvaluatedTargetMoveDirectionBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int AccumulatedTargetDeltasCapacity = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NonStationaryAccumulatedTargetDeltaDistance = 300.f;

private:
	EBlackboardNotificationResult OnCheckConditionUpdated(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};