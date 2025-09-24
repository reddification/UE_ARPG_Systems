// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_RunNpcBehavior.h"

#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_RunNpcBehavior::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	bool bSuccess = Context.NpcSubsystem->TryRunQuestBehavior(QuestActionNpcRunBehavior, ActionId, Context);
	return bSuccess ? EQuestActionExecuteResult::Success : EQuestActionExecuteResult::Failure;
}

#if WITH_EDITOR

FString UFlowNode_QuestAction_RunNpcBehavior::GetQuestActionDescription() const
{
	auto Base = Super::GetQuestActionDescription();
	Base += FString::Printf(TEXT("For NPCs:\n[%s]\nRun activity:\n[%s]"),
		*QuestActionNpcRunBehavior.NpcIdsTags.ToStringSimple(),
		*QuestActionNpcRunBehavior.NpcQuestBehaviorDescriptor.RequestedBehaviorIdTag.ToString());

	if (!QuestActionNpcRunBehavior.RequiredTags.IsEmpty())
		Base += FString::Printf(TEXT("\nNPC filter:\n%s"), *QuestActionNpcRunBehavior.RequiredTags.GetDescription());

	if (!QuestActionNpcRunBehavior.NpcQuestBehaviorDescriptor.QuestBehaviorEndConditions.IsEmpty())
		Base += TEXT("\nHas end conditions");
	
	return Base;
}

EDataValidationResult UFlowNode_QuestAction_RunNpcBehavior::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;

	return !QuestActionNpcRunBehavior.NpcIdsTags.IsEmpty()
		&& QuestActionNpcRunBehavior.NpcQuestBehaviorDescriptor.RequestedBehaviorIdTag.IsValid()
	? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

#endif
