// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Data/NpcCombatTypes.h"
#include "Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "BehaviorEvaluator_Retreat.generated.h"

UCLASS(DisplayName="Retreat")
class ARPGAI_API UBehaviorEvaluatorConfig_Retreat : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Retreat();
	
	// used only for sight-distance analysis
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StrengthDisadvantageActivation = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve ThreatDistanceScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AllyDistanceCalmnessDependency;

	// assumed normalized damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AccumulatedDamageScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve HealthFractionToFearScaleDependency;
	
	// Only consider actors if saw them for longer than this time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SightActivationThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGameplayTagFilterScalarValue> TagBasedScoreScales;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutPrimaryRetreatTargetBBKey;

	// only updated when the behavior is active
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAccumulatedDamageBBKey;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Retreat : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;
	
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
	FBehaviorEvaluator_Retreat(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	
	virtual void Update(const float DeltaTime) override;

protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	float UpdatePerception();
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Retreat> RetreatConfig;
	TMap<TWeakObjectPtr<AActor>, FNpcImmediateThreatData> NpcThreatData;
};