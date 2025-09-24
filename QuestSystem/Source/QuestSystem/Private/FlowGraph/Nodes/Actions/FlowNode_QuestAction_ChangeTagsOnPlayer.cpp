// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_ChangeTagsOnPlayer.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"

EQuestActionExecuteResult UFlowNode_QuestAction_ChangeTagsOnPlayer::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	if (bAdd)
		Context.Player->AddQuestTags(Tags);
	else
		Context.Player->RemoveQuestTags(Tags);

	return EQuestActionExecuteResult::Success;
}
