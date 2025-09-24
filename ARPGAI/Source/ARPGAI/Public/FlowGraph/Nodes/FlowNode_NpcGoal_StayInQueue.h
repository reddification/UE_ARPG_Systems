// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_NpcGoal.h"
#include "FlowNode_NpcGoal_StayInQueue.generated.h"

struct FNpcQueueMemberPosition;
/**
 * 
 */
UCLASS(meta=(DisplayName = "NPC Goal: Stay in queue"))
class ARPGAI_API UFlowNode_NpcGoal_StayInQueue : public UFlowNode_NpcGoal
{
	GENERATED_BODY()

public:
	UFlowNode_NpcGoal_StayInQueue(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { NpcGoalType = ENpcGoalType::StayInQueue; }
 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcGoalParameters_StayInQueue NpcGoalParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag QueueGoalParametersId;

	virtual ENpcGoalStartResult Start() override;
	virtual ENpcGoalAdvanceResult Advance(const FGameplayTagContainer& GoalExecutionResultTags) override;
	virtual ENpcGoalStartResult Restore(bool bInitialStart) override;
	virtual void Finish() override;
	
	void OnNpcQueueMemberAdvanced(AActor* NpcActor, const FNpcQueueMemberPosition& NpcQueueMemberPosition);
	
	void UpdateQueuePosition(const FNpcQueueMemberPosition& NpcQueueMemberPosition, const FNpcGoalParameters_StayInQueue* ActualGoalParameters) const;

	void UpdateQueuePosition(const FNpcQueueMemberPosition& NpcQueueMemberPosition) const;

private:
	const FNpcGoalParameters_StayInQueue* GetGoalParameters() const;
	
};
