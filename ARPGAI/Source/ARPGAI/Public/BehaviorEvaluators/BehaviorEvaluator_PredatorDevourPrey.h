#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "BehaviorEvaluator_PredatorDevourPrey.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Beast | Post fight")
class ARPGAI_API UBehaviorEvaluatorConfig_PredatorDevourPrey : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_PredatorDevourPrey();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToDeadPreyDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToAliveEnemyDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery PreyFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector DevourTargetBBKey;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_PredatorDevourPrey : public FBehaviorEvaluator_Base
{
	
private:
	using Super = FBehaviorEvaluator_Base;
	
public:
	FBehaviorEvaluator_PredatorDevourPrey(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	float UpdatePerception() const;
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_PredatorDevourPrey> Config;
};