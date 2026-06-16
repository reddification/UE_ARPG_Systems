#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_OperationBased.h"
#include "BehaviorEvaluator_Combat_Heal.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Combat | Heal")
class ARPGAI_API UBehaviorEvaluatorConfig_Combat_Heal : public UBehaviorEvaluatorConfig_OperationBased
{
	GENERATED_BODY()
	
public:
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;	
};

class FBehaviorEvaluator_Combat_Heal : public FBehaviorEvaluator_OperationBased
{
	using Super = FBehaviorEvaluator_OperationBased;

public:
	FBehaviorEvaluator_Combat_Heal(UBehaviorTreeComponent& BTComponent, const UBehaviorEvaluatorConfig_Base* Config);
	
protected:
	virtual float Evaluate() override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Combat_Heal> HealConfig;
};
