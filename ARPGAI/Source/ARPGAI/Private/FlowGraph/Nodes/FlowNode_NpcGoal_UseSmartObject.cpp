#include "FlowGraph/Nodes/FlowNode_NpcGoal_UseSmartObject.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Data/NpcSettings.h"
#include "EnvironmentQuery/EnvQuery.h"

ENpcGoalStartResult UFlowNode_NpcGoal_UseSmartObject::Start()
{
	auto Result = Super::Start();
	return Result;
}

ENpcGoalStartResult UFlowNode_NpcGoal_UseSmartObject::Restore(bool bInitialStart)
{
	auto SmartObjectEqsSoftPtr = GetDefault<UNpcSettings>()->SmartObjectNpcGoalEqs;
	auto Parameters = GetParameters();
	if (ensure(!SmartObjectEqsSoftPtr.IsNull()))
	{
		auto SmartObjectEqs = SmartObjectEqsSoftPtr.LoadSynchronous();
		if (IsValid(SmartObjectEqs))
		{
			BlackboardComponent->SetValueAsObject(BlackboardKeys->EqsToRunBBKey.SelectedKeyName, SmartObjectEqs);
			BlackboardComponent->SetValueAsEnum(BlackboardKeys->EQSRunModeBBKey.SelectedKeyName, Parameters->EqsRunMode);
		}
	}

	if (CurrentGoalState != EGoalState::Running && Parameters->LocationIdTag.IsValid())
		NpcFlowComponent->SetGoalTagParameter(AIGameplayTags::Activity_Goal_Parameter_LocationId, Parameters->LocationIdTag);
	
	return Super::Restore(bInitialStart);
}

ENpcGoalAdvanceResult UFlowNode_NpcGoal_UseSmartObject::Advance(const FGameplayTagContainer& GoalExecutionResultTags)
{
	auto Parameters = GetParameters();
	if (GoalExecutionResultTags.HasTagExact(AIGameplayTags::Activity_Goal_Result_Execution_Success) && Parameters->bRepeatUntilNoInteractableActorsLeft)
	{
		return GoalExecutionResultTags.HasTag(AIGameplayTags::AI_Activity_Goal_Result_SmartObject_NotFound)
			? ENpcGoalAdvanceResult::Completed : ENpcGoalAdvanceResult::InProgress; 
	}
	else
	{
		return Super::Advance(GoalExecutionResultTags);
	}
}

void UFlowNode_NpcGoal_UseSmartObject::Suspend()
{
	Super::Suspend();
	NpcFlowComponent->RemoveGoalTagParameter(AIGameplayTags::Activity_Goal_Parameter_LocationId);
}

void UFlowNode_NpcGoal_UseSmartObject::ClearBlackboard()
{
	Super::ClearBlackboard();
	// this one is tricky. it makes sense to clear blackboard because it could be that some other goal that also has interaction actor was running before resuming this goal
	// but at the same time it can be that this goal was actually running, then suspended (because of combat, for example), and now it's resuming again
	// but a valid interaction actor will be overwritten. so idk
	// 10.08.2025 (aki) TODO perhaps I should pass a parameter like "was this goal running before resuming"
	BlackboardComponent->ClearValue(BlackboardKeys->InteractionActorBBKey.SelectedKeyName);
}

const FNpcGoalParameters_UseSmartObject* UFlowNode_NpcGoal_UseSmartObject::GetParameters() const
{
	if (NpcGoalDataParameterId.IsValid())
	{
		const FNpcGoalParameters_UseSmartObject* Result = NpcComponent->GetNpcGoalParameters<FNpcGoalParameters_UseSmartObject>(NpcGoalDataParameterId);
		return Result;
	}

	return &DefaultParameters;
}

#if WITH_EDITOR

EDataValidationResult UFlowNode_NpcGoal_UseSmartObject::ValidateNode()
{
	auto Base = Super::ValidateNode();
	if (Base == EDataValidationResult::Invalid)
		return Base;

	if (NpcGoalDataParameterId.IsValid())
		return EDataValidationResult::NotValidated;

	return !DefaultParameters.IntentionFilter.IsEmpty() ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}

FString UFlowNode_NpcGoal_UseSmartObject::GetGoalDescription() const
{
	auto Base = Super::GetGoalDescription();
	if (NpcGoalDataParameterId.IsValid())
	{
		Base += FString::Printf(TEXT("\nUse parameters from NPC by key: %s)"), *NpcGoalDataParameterId.ToString());
	}
	else
	{
		if (DefaultParameters.LocationIdTag.IsValid())
			Base += FString::Printf(TEXT("\nAt location: %s\n---------\n"), *DefaultParameters.LocationIdTag.ToString());
		else 
			Base += TEXT("\nAround self\n---------\n");

		if (!DefaultParameters.IntentionFilter.IsEmpty())
			Base += FString::Printf(TEXT("Intention:\n%s\n---------\n"), *DefaultParameters.IntentionFilter.GetDescription());

		if (!DefaultParameters.SmartObjectActorFilter.IsEmpty())
			Base += FString::Printf(TEXT("Actor filter:\n%s\n---------\n"), *DefaultParameters.SmartObjectActorFilter.GetDescription());

		if (DefaultParameters.bRepeatUntilNoInteractableActorsLeft)
			Base += TEXT("Repeat until no interactable actors left\n");
		else if (DefaultParameters.RequiredInteractionsCount > 1)
			Base += FString::Printf(TEXT("Use %d objects\n"), DefaultParameters.RequiredInteractionsCount);

		FString EQSRunModeString;
		switch (DefaultParameters.EqsRunMode)
		{
			case EEnvQueryRunMode::Type::SingleResult:
			case EEnvQueryRunMode::AllMatching:
				EQSRunModeString = TEXT("Use best");
			break;
			case EEnvQueryRunMode::RandomBest5Pct:
				EQSRunModeString = TEXT("Use random best 5%");
			break;
			case EEnvQueryRunMode::RandomBest25Pct:
				EQSRunModeString = TEXT("Use random best 25%");
			break;
			default:
				ensure(false);
			break;
		}
		
		Base += FString::Printf(TEXT("EQS run mode: %s"), *EQSRunModeString);
	}
	
	return Base;
}

#endif

struct FConstStructView UFlowNode_NpcGoal_UseSmartObject::GetParametersView()
{
	FConstStructView ParametersConstStructView = FConstStructView(FNpcGoalParameters_UseSmartObject::StaticStruct(), reinterpret_cast<const uint8*>(&DefaultParameters));
	return ParametersConstStructView;
}
