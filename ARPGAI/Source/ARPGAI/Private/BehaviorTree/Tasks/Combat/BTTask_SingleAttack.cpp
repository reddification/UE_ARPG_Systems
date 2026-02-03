// 


#include "BehaviorTree/Tasks/Combat/BTTask_SingleAttack.h"

#include "AIController.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_SingleAttack::UBTTask_SingleAttack()
{
	NodeName = "Single attack";
}

EBTNodeResult::Type UBTTask_SingleAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;
	
	Super::ExecuteTask(OwnerComp, NodeMemory);
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Attack_Canceled.GetTag().GetTagName());
	Npc->StartAttack();
	return EBTNodeResult::InProgress;
}

void UBTTask_SingleAttack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Attack_Completed;
}
