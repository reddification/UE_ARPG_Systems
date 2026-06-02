// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "BehaviorEvaluator_Idle.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Idle")
class ARPGAI_API UBehaviorEvaluatorConfig_Idle : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Idle();
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Idle : public FBehaviorEvaluator_Base
{
	using Super = FBehaviorEvaluator_Base;
	
public:
	FBehaviorEvaluator_Idle(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* const Config);
	
protected:
	virtual void SetState(EBehaviorEvaluatorState NewState) override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Idle> IdleConfig;
};