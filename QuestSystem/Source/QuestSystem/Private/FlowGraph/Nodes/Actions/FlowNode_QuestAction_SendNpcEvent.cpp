// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SendNpcEvent.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SendNpcEvent::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	auto Npcs = Context.NpcSubsystem->GetNpcs(NpcId, &NpcFilter);
	for (auto& Npc : Npcs)
		Npc->ReceiveQuestEvent(Event);

	return Success;
}
