// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UObject/Object.h"
#include "BTService_PrepareAttack.generated.h"

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
		FDelegateHandle OnAttackRangeChangedDelegateHandle;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ResetPreparedAttackDistanceThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NextTickDelayAfterRequestToAttack = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LittleExtraAttackRange = 33.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float TooCloseRangeDistanceCoefficient = 0.4f;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShowDebugInfo = false;
#endif
	
private:
	EBlackboardNotificationResult OnAttackRangeChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);

};
