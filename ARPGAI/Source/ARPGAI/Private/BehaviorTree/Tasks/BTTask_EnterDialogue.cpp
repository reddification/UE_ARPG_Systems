#include "BehaviorTree/Tasks/BTTask_EnterDialogue.h"

#include "Data/AIGameplayTags.h"

UBTTask_EnterDialogue::UBTTask_EnterDialogue()
{
	NodeName = "Enter dialogue";
}

EBTNodeResult::Type UBTTask_EnterDialogue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	return EBTNodeResult::InProgress;
}

FString UBTTask_EnterDialogue::GetStaticDescription() const
{
	return TEXT("Maintain dialogue");
}

void UBTTask_EnterDialogue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Dialogue_Npc_Completed;
}
