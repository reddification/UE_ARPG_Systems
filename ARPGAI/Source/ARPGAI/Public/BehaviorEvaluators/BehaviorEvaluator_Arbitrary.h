#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "UObject/Object.h"
#include "BehaviorEvaluator_Arbitrary.generated.h"

UCLASS(DisplayName="Arbitrary")
class ARPGAI_API UBehaviorEvaluatorConfig_Arbitrary : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Arbitrary();	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Arbitrary : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;
	
public:
	FBehaviorEvaluator_Arbitrary(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Arbitrary> ArbitraryConfig;
};
