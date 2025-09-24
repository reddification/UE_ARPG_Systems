// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_ChangeTagsOnNpcs.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_ChangeTagsOnNpcs::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	Context.NpcSubsystem->ChangeTagsForNpcs(CharacterId, Tags, bAdd, &CharacterFilter);
	return EQuestActionExecuteResult::Success;
}
