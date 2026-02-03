// 


#include "FlowGraph/Nodes/FlowNode_NpcGoal_Wander.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"

ENpcGoalStartResult UFlowNode_NpcGoal_Wander::Restore(bool bInitialStart)
{
	auto RestoreResult = Super::Restore(bInitialStart);
	if (RestoreResult == ENpcGoalStartResult::Failed)
		return RestoreResult;

	BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, AreaEqs_Obsolete);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->GestureToPlayBBKey.SelectedKeyName, GestureOptionsTags);
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(BlackboardKeys->ActivityPhrasesBBKey.SelectedKeyName, SpeechOptionsTags);

	return ENpcGoalStartResult::InProgress;
}

#if WITH_EDITOR

EDataValidationResult UFlowNode_NpcGoal_Wander::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;

	return IsValid(AreaEqs_Obsolete) ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

FString UFlowNode_NpcGoal_Wander::GetGoalDescription() const
{
	FString Base = Super::GetGoalDescription();
	Base = Base + (IsValid(AreaEqs_Obsolete) ? FString::Printf(TEXT("\nRun EQS: %s"), *AreaEqs_Obsolete->GetName()) : TEXT("\nInvalid Area Eqs"));
	if (GestureOptionsTags.IsValid())
		Base += FString::Printf(TEXT("\nOn site do gestures:\n%s"), *GestureOptionsTags.ToStringSimple());

	if (SpeechOptionsTags.IsValid())
		Base += FString::Printf(TEXT("\nOn site say:\n%s"), *SpeechOptionsTags.ToStringSimple());
	
	return Base;
}

#endif
