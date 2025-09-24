// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "Data/AiDataTypes.h"
#include "FlowNode_NpcGoal_FollowPath.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: follow path"))
class ARPGAI_API UFlowNode_NpcGoal_FollowPath : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_FollowPath(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::Patrol; }

protected:
	const FNpcGoalParameters_FollowPath* GetParameters() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcGoalDataParameterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="NpcGoalDataParameterId.IsValid() == false"))
	FNpcGoalParameters_FollowPath Parameters;
	
	virtual ENpcGoalStartResult Start() override;
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;
	virtual ENpcGoalAdvanceResult Advance(const FGameplayTagContainer& GoalExecutionResultTags) override;
	virtual void Finish() override;

private:
	void UpdateStayAtPatrolPointTime() const;
};
