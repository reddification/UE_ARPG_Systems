#include "BehaviorTree/Tasks/Combat/BTTask_Backdash.h"

#include "AIController.h"
#include "Components/NpcInterfaceComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_Backdash::UBTTask_Backdash()
{
	NodeName = "Backdash";
}

EBTNodeResult::Type UBTTask_Backdash::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	auto Npc = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcInterfaceComponent>();
	if (!Npc)
		return EBTNodeResult::Failed;
	
	bool bStarted = Npc->Backdash();
	return bStarted ? EBTNodeResult::InProgress : EBTNodeResult::Failed; 
}

void UBTTask_Backdash::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Backdash_Completed;
	
}
