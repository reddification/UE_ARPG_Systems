// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_Retreat.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_Retreat : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

private:
	struct FFearData
	{
		AActor* Actor = nullptr;
		float FearScore = 0.f;

		bool operator < (const FFearData& Other) const
		{
			return FearScore > Other.FearScore;
		}
	};
	
public:
	UBTService_BehaviorEvaluator_Retreat();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual FString GetStaticDescription() const override;

protected:
	// used only for sight-distance analysis
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StrengthDisadvantageActivation = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve ThreatDistanceScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AllyDistanceCalmnessDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AccumulatedDamageScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve HealthFractionToFearScaleDependency;
	
	// Only consider actors if saw them for longer than this time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SightActivationThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutPrimaryRetreatTargetBBKey;

	// only updated when the behavior is active
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAccumulatedDamageBBKey;

private:
	float UpdatePerception(UBehaviorTreeComponent& OwnerComp, const FBTMemory_BehaviorEvaluator_Base* BTMemory) const;
};
