#include "BehaviorEvaluators/v2/BehaviorEvaluator_Combat_Fight.h"

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Combat_Fight::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Combat_Fight>(*BTComponent, this);
}

FBehaviorEvaluator_Combat_Fight::FBehaviorEvaluator_Combat_Fight(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config)
	: Super(OwnerComp, Config)
{
	FightConfig = Cast<UBehaviorEvaluatorConfig_Combat_Fight>(Config);
}
