// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_Combat.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_Combat : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

private:
	struct FCombatEvaluator_ActorPriority
	{
		AActor* Actor = nullptr;
		float Priority = 0.f;
		
		bool operator < (const FCombatEvaluator_ActorPriority& Other) const
		{
			return Priority > Other.Priority;
		}
	};
	
public:
	UBTService_BehaviorEvaluator_Combat();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutCombatTargetBBKey;

	// only updated when the behavior is active
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAccumulatedDamageBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve HostileActorDistanceCombatDesireDependencyCurve;

	// x: total perceived accumulated damage / max health, y: factor to multiply total combat desire 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AccumulatedDamageToCombatDesireScaleDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve StrengthAdvantageCombatDesireDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DamageToCombatDesireScaleDependencyCurve;

	// How much to scale combat utility if the perception of a potential target is coming from ally only
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AllyOnlyPerceptionScale = 0.5f;
	
private:
	float UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const;
};
