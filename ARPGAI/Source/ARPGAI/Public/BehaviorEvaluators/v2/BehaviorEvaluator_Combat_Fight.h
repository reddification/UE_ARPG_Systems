#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"
#include "BehaviorEvaluators/BehaviorEvaluator_CombatBase.h"
#include "UObject/Object.h"
#include "BehaviorEvaluator_Combat_Fight.generated.h"

UCLASS(DisplayName = "Combat | Fight")
class ARPGAI_API UBehaviorEvaluatorConfig_Combat_Fight : public UBehaviorEvaluatorConfig_CombatBase
{
	GENERATED_BODY()
	
public:
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Combat_Fight : public FBehaviorEvaluator_CombatBase
{

	private:
	using Super = FBehaviorEvaluator_CombatBase;

public:
	FBehaviorEvaluator_Combat_Fight(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);

private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Combat_Fight> FightConfig;
};