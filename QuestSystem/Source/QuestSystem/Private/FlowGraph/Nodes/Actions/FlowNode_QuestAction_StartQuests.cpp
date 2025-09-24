// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_StartQuests.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_StartQuests::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	for (const auto& QuestToStart : QuestsToStart)
		Context.QuestSubsystem->StartQuest(QuestToStart);

	return EQuestActionExecuteResult::Success;
}
