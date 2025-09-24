// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SetPlayerState.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SetPlayerState::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	if (bAdd)
		Context.Player->AddQuestState(StateTag, SetByCallerParams);
	else
		Context.Player->RemoveQuestState(StateTag);

	return EQuestActionExecuteResult::Success;
}
