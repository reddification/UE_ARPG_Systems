// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_InitiateDialogueWithNpc.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_InitiateDialogueWithNpc::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	auto Npc = Context.NpcSubsystem->FindNpc(NpcId, Context.Player->GetCharacterLocation());
	if (!ensure(Npc.GetObject() != nullptr))
		return EQuestActionExecuteResult::Failure;
	
	Context.Player->StartQuestDialogueWithNpc(Cast<AActor>(Npc.GetObject()), OptionalDialogueId);
	return EQuestActionExecuteResult::Success;
}
