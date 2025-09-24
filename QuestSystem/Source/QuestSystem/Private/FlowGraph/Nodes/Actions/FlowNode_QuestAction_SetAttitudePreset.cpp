// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SetAttitudePreset.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SetAttitudePreset::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	Context.NpcSubsystem->SetCustomAttitudePreset(ForCharacterIds, NewAttitudePreset, GameHoursDuration);
	return EQuestActionExecuteResult::Success;
}
