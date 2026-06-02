#include "BehaviorEvaluators/BehaviorEvaluator_Idle.h"

UBehaviorEvaluatorConfig_Idle::UBehaviorEvaluatorConfig_Idle()
{
	bTickable = false;
	bUpdateWhenActivated = false;
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Idle::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Idle>(*BTComponent, this);
}

FBehaviorEvaluator_Idle::FBehaviorEvaluator_Idle(UBehaviorTreeComponent& OwnerComp,
                                                 const UBehaviorEvaluatorConfig_Base* const Config) : Super(OwnerComp, Config)
{
	IdleConfig = Cast<UBehaviorEvaluatorConfig_Idle>(Config);
}

void FBehaviorEvaluator_Idle::SetState(EBehaviorEvaluatorState NewState)
{
	auto OldState = GetState();
	Super::SetState(NewState);
	if (NewState == EBehaviorEvaluatorState::Relevant && (OldState == EBehaviorEvaluatorState::NotRequested || OldState == EBehaviorEvaluatorState::Blocked))
	{
		SetMaxUtility();
	}
}
