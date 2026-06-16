#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_Hunt.generated.h"

UCLASS(meta=(DisplayName = "NPC Goal: Hunt"))
class ARPGAI_API UFlowNode_NpcGoal_Hunt : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_Hunt() { NpcGoalType = ENpcGoalType::Hunt; }
};
