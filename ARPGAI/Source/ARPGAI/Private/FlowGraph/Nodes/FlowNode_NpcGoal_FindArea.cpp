// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_FindArea.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"

ENpcGoalStartResult UFlowNode_NpcGoal_FindArea::Start()
{
	auto Result = Super::Start();
	BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, AreaEqs);
	return Result;
}
