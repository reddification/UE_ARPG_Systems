// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_UpdateWorldState.h"

#include "Data/QuestTypes.h"
#include "Subsystems/WorldStateSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_UpdateWorldState::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	Context.WorldStateSubsystem->ChangeWorldState(WorldStateTagsChange, bAppend);
	return EQuestActionExecuteResult::Success;
}

#if WITH_EDITOR

FString UFlowNode_QuestAction_UpdateWorldState::GetQuestActionDescription() const
{
	FString Base = Super::GetQuestActionDescription();
	if (!WorldStateTagsChange.IsValid())
		Base += TEXT("Warning: no tags specified");
	else
		Base += FString::Printf(TEXT("%s: %s"), (bAppend ? TEXT("Append") : TEXT("Remove")), *WorldStateTagsChange.ToStringSimple());
	
	return Base;
}

EDataValidationResult UFlowNode_QuestAction_UpdateWorldState::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;
	
	return WorldStateTagsChange.IsValid() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

#endif