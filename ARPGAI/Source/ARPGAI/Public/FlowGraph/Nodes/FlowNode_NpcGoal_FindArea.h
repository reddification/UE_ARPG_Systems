// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_FindArea.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Find area"))
class ARPGAI_API UFlowNode_NpcGoal_FindArea : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_FindArea(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::FindArea; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UEnvQuery* AreaEqs;
	
	virtual ENpcGoalStartResult Start() override;	
};
