// 


#include "BehaviorTree/Tasks/BTTask_TriggerArbitraryNpcEvent.h"

#include "AIController.h"
#include "Interfaces/Npc.h"

UBTTask_TriggerArbitraryNpcEvent::UBTTask_TriggerArbitraryNpcEvent()
{
	NodeName = "Trigger arbitrary npc event";
}

EBTNodeResult::Type UBTTask_TriggerArbitraryNpcEvent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!EventTag.IsValid())
		return EBTNodeResult::Failed;
	
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Npc)
		return EBTNodeResult::Failed;
	
	return Npc->TriggerArbitraryNpcEvent(EventTag) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_TriggerArbitraryNpcEvent::GetStaticDescription() const
{
	return EventTag.ToString();
}
