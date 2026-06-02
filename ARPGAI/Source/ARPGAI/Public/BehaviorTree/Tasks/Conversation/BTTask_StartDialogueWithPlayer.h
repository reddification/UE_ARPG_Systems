#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_StartDialogueWithPlayer.generated.h"

UENUM(BlueprintType)
enum class ENpcStartDialogueWithPlayerReason : uint8
{
	NpcGoal = 0,
	Reaction = 1
};

UCLASS(Category="Conversation")
class ARPGAI_API UBTTask_StartDialogueWithPlayer : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_StartDialogueWithPlayer();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetCharacterBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ENpcStartDialogueWithPlayerReason Reason = ENpcStartDialogueWithPlayerReason::NpcGoal; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxAcceptableDistance = 300.f;

private:
	bool StartDialogueFromNpcGoal(
		APawn* NpcPawn, APawn* Collocutor);
	bool StartDialogueFromReaction(APawn* NpcPawn, APawn* TargetPawn);
};
