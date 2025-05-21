// 

#pragma once

#include "CoreMinimal.h"
#include "BTTask_HandleGameplayAbility.h"
#include "BTTask_StartDialogueWithPlayer.generated.h"

UENUM(BlueprintType)
enum class ENpcStartDialogueWithPlayerReason : uint8
{
	NpcGoal = 0,
	Reaction = 1
};

UCLASS()
class ARPGAI_API UBTTask_StartDialogueWithPlayer : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_StartDialogueWithPlayer();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetCharacterBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector bOutDialogueActiveBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ENpcStartDialogueWithPlayerReason Reason = ENpcStartDialogueWithPlayerReason::NpcGoal; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxAcceptableDistance = 300.f;

private:
	EBTNodeResult::Type StartDialogueFromNpcGoal(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent* Blackboard,
		uint8* NodeMemory, APawn* Pawn);
	EBTNodeResult::Type StartDialogueFromReaction(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, APawn* Pawn);
};
