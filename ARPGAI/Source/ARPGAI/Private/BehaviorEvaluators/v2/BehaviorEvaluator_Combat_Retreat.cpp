#include "BehaviorEvaluators/v2/BehaviorEvaluator_Combat_Retreat.h"

#include "Components/NpcCombatLogicComponent.h"

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Combat_Retreat::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Combat_Retreat>(*BTComponent, this);
}

FBehaviorEvaluator_Combat_Retreat::FBehaviorEvaluator_Combat_Retreat(UBehaviorTreeComponent& BTComponent, const UBehaviorEvaluatorConfig_Base* Config)
	: Super(BTComponent, Config)
{
	RetreatConfig = Cast<UBehaviorEvaluatorConfig_Combat_Retreat>(Config);
}