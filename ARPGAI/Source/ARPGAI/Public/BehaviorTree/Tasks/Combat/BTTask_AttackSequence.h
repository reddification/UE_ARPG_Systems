

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Data/NpcCombatTypes.h"
#include "BTTask_AttackSequence.generated.h"

UCLASS(Category="Combat")
class ARPGAI_API UBTTask_AttackSequence : public UBTTaskNode
{
	GENERATED_BODY()

private:
	struct FBTMemory_Attack : public FBTTaskMemory
	{
		bool bAbortRequested = false;
		bool bAttacking = false;
		bool bPreparingNextAttack = false;
		float Intelligence = 0.5f;
		float Reaction = 0.5f;
		ENpcAttackResult AttackResult = ENpcAttackResult::None;
		int MorphCounters = 1;
	};
	
public:
	UBTTask_AttackSequence();
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAttackingBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAttackResultBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutDefenseActionBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutTauntRequestBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector DistanceToEnemyBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AggressionBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFloatRange TauntIfEnemyInRange = FFloatRange(250, 700);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector RequestAttackBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackActivationFailedCooldownTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AttackActivationFailedCooldownTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFloatRange NextAttackDelay = FFloatRange(0.1f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int> Debug_Options;
	
private:
	void FinalizeAttack(UBehaviorTreeComponent& OwnerComp, FBTMemory_Attack* AttackMemory, UBlackboardComponent* Blackboard, class INpc* Npc, bool
	                    bRequestFinishAttack) const;

	FGameplayTag AttackWhiffedMessageTag;
	FGameplayTag AttackParriedMessageTag;
	FGameplayTag AttackHitTargetMessageTag;
	FGameplayTag AttackStartedMessageTag;
	FGameplayTag AttackCommitedMessageTag;
	FGameplayTag AttackFinishedMessageTag;
	FGameplayTag AttackCancelledMessageTag;
	FGameplayTag EnemyBlockingAttackMessageTag;
	FGameplayTag AbilityActivationFailedCantAffordMessageTag;
	FGameplayTag AbilityActivationFailedConditionsNotMetMessageTag;
};
