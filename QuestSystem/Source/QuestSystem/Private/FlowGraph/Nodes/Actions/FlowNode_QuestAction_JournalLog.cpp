// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_JournalLog.h"

#include "FlowAsset.h"
#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_JournalLog::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	const FName* QuestId = Context.QuestSubsystem->GetFlowQuestId(GetFlowAsset()->GetTemplateAsset());
	if (ensure(QuestId))
	{
		Context.QuestSubsystem->AddJournalLog(*QuestId, JournalEntry, JournalEntryTags);
		return EQuestActionExecuteResult::Success;
	}

	return EQuestActionExecuteResult::Failure;
}

#if WITH_EDITOR

FString UFlowNode_QuestAction_JournalLog::GetQuestActionDescription() const
{
	return Super::GetQuestActionDescription() + JournalEntry.ToString();
}

#endif