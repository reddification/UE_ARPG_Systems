// 


#include "BehaviorEvaluators/BehaviorEvaluatorBase.h"

#include "AIController.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Data/AIGameplayTags.h"

UBehaviorEvaluatorBase::UBehaviorEvaluatorBase()
{
	UtilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorBase, UtilityBBKey));
}

void UBehaviorEvaluatorBase::Activate(AAIController* InAIController)
{
	AIController = InAIController;
	NpcAttitudesComponent = AIController->GetPawn()->FindComponentByClass<UNpcAttitudesComponent>();
	BlackboardComponent = AIController->GetBlackboardComponent();
	bActive = true;
}

void UBehaviorEvaluatorBase::Deactivate()
{
	AIController.Reset();
	BlackboardComponent.Reset();
	bActive = false;
}

void UBehaviorEvaluatorBase::ChangeUtility(const float DeltaUtility)
{
	float CurrentUtility = BlackboardComponent->GetValueAsFloat(UtilityBBKey.SelectedKeyName);
	BlackboardComponent->SetValueAsFloat(UtilityBBKey.SelectedKeyName, CurrentUtility + DeltaUtility);
}

bool UBehaviorEvaluatorBase::IsHostile(AActor* Actor)
{
	const FGameplayTag& Attitude = NpcAttitudesComponent->GetAttitude(Actor);
	return Attitude != AIGameplayTags::AI_Attitude_Friendly
		&& Attitude != AIGameplayTags::AI_Attitude_Neutral; 
}

UBlackboardData* UBehaviorEvaluatorBase::GetBlackboardAsset() const
{
	return !BlackboardAsset.IsNull() ?  BlackboardAsset.LoadSynchronous() : nullptr;
}

void UBehaviorEvaluatorBase::Tick(float DeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;
	if (TimeSinceLastUpdate >= TickInterval)
	{
		Evaluate();
		TimeSinceLastUpdate = 0.f;
	}
}


TStatId UBehaviorEvaluatorBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UBehaviorEvaluatorBase, STATGROUP_Tickables);
}

void UBehaviorEvaluatorBase::Evaluate()
{
	
}
