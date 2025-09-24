// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_PredatorDevourPrey.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_PredatorDevourPrey : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

public:
	UBTService_BehaviorEvaluator_PredatorDevourPrey();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToDeadPreyDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceToAliveEnemyDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector DevourTargetBBKey;
	
private:
	float UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const;
};
