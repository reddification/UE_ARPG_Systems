#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"
#include "BehaviorEvaluators/BehaviorEvaluator_CombatBase.h"
#include "BehaviorEvaluator_Combat_Retreat.generated.h"

UCLASS(DisplayName = "Combat | Retreat")
class ARPGAI_API UBehaviorEvaluatorConfig_Combat_Retreat : public UBehaviorEvaluatorConfig_CombatBase
{
	GENERATED_BODY()
	
public:
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Combat_Retreat : public FBehaviorEvaluator_CombatBase
{
private:
	using Super = FBehaviorEvaluator_CombatBase;
	
public:
	FBehaviorEvaluator_Combat_Retreat(UBehaviorTreeComponent& BTComponent, const UBehaviorEvaluatorConfig_Base* Config);
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Combat_Retreat> RetreatConfig;
};