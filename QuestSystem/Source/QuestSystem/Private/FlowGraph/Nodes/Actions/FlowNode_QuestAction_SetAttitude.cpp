// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SetAttitude.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SetAttitude::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	Context.NpcSubsystem->AddCustomAttitude(SourceCharacterIds, TargetCharacterIds, NewAttitude, GameHoursDuration);
	return EQuestActionExecuteResult::Success;
}
