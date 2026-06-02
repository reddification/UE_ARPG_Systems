// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_PrepareAttack.generated.h"

class UNpcAttitudesComponent;
class UNpcCombatLogicComponent;
/**
 * 
 */
UCLASS(Category="Combat")
class ARPGAI_API UBTService_PrepareAttack : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTMemory_PrepareAttack : public FBTAuxiliaryMemory
	{
		float Intelligence = 0.f;
		float AttackRange = 0.f;
		float TooCloseDistance = 0.f;
		TWeakObjectPtr<UNpcCombatLogicComponent> NpcCombatComponent = nullptr;
		TWeakObjectPtr<UNpcAttitudesComponent> AttitudesComponent = nullptr;
		TWeakObjectPtr<AActor> TargetActor;
		ECollisionChannel AttackTraceChannel;
	};
	
public:
	UBTService_PrepareAttack();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_PrepareAttack); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutRequestAttackBBKey;

	// TODO probably not needed anymore
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AttackActiveBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector IntelligenceBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AttackRangeBBKey;

	// when target is out of attack range, if NPC is already attacking, add this threshold to prevent frequent toggling on/off "want to attack" state if target hasn't got too far away
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ResetPreparedAttackDistanceThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NextTickDelayAfterRequestToAttack = 2.f;

	// added to attack range. use it to control how far or close you want NPC to approach enemies before it decides to attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LittleExtraAttackRange = 33.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float TooCloseRangeDistanceCoefficient = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float AttackRelevantDotProductThreshold = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector NpcToTargetDotProductBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetToNpcDotProductBBKey;
	
private:
	EBlackboardNotificationResult OnAttackRangeChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	EBlackboardNotificationResult OnTargetChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);

	bool IsAnythingBlocksAttack(const FBTMemory_PrepareAttack* BTMemory, const AActor* Target, const APawn* Pawn,
								const FVector& NpcLocation, const FVector& TargetLocation) const;
};
