// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_Gesture.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcBlackboardDataAsset.h"

ENpcGoalStartResult UFlowNode_NpcGoal_Gesture::Start()
{
	auto Result = Super::Start();
	if (Result == ENpcGoalStartResult::InProgress)
	{
		if (GestureOptions.IsValid())
			BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, GestureOptions);
		else
			Result = ENpcGoalStartResult::Failed;
	}
	
	return Result;
}

ENpcGoalStartResult UFlowNode_NpcGoal_Gesture::Restore(bool bInitialStart)
{
	auto Result = Super::Restore(bInitialStart);
	if (Result == ENpcGoalStartResult::InProgress)
	{
		if (GestureOptions.IsValid())
			BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, GestureOptions);
		else
			Result = ENpcGoalStartResult::Failed;
	}
	
	return Result;
}

#if WITH_EDITOR

FString UFlowNode_NpcGoal_Gesture::GetGoalDescription() const
{
	return FString::Printf(TEXT("Gesture:%s\n%s"), *GestureOptions.ToStringSimple(), *Super::GetGoalDescription());
}

EDataValidationResult UFlowNode_NpcGoal_Gesture::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;

	return GestureOptions.IsValid() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

#endif
