// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_NpcDisplayReaction.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_NpcDisplayReaction::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	if (!ensure(PhraseId.IsValid()))
		return EQuestActionExecuteResult::Failure;
	
	auto Npc = Context.NpcSubsystem->FindNpc(CharacterId, Context.Player->GetCharacterLocation());
	if (!ensure(Npc.GetObject() != nullptr))
		return EQuestActionExecuteResult::Failure;

	Npc->SayQuestPhrase(PhraseId);
	return EQuestActionExecuteResult::Success;
}
