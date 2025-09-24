// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_CompleteQuest.h"

#include "FlowAsset.h"
#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_CompleteQuest::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	auto FlowAsset = GetFlowAsset();
	const FName* QuestId = Context.QuestSubsystem->GetFlowQuestId(FlowAsset->GetTemplateAsset());
	if (ensure(QuestId))
		Context.QuestSubsystem->CompleteFlowQuest(*QuestId, FinalQuestState);

	FlowAsset->FinishFlow(EFlowFinishPolicy::Keep, true);
	return EQuestActionExecuteResult::Success;
}
