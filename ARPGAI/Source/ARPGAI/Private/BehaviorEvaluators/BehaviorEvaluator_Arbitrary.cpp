#include "BehaviorEvaluators/BehaviorEvaluator_Arbitrary.h"

UBehaviorEvaluatorConfig_Arbitrary::UBehaviorEvaluatorConfig_Arbitrary()
{
	bTickable = false;
	bUpdateWhenActivated = false;
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Arbitrary::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Arbitrary>(*BTComponent, this);
}

FBehaviorEvaluator_Arbitrary::FBehaviorEvaluator_Arbitrary(UBehaviorTreeComponent& OwnerComp,
                                                                       const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	ArbitraryConfig = Cast<UBehaviorEvaluatorConfig_Arbitrary>(Config);
}
