#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Data/NpcCombatTypes.h"
#include "Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "BehaviorEvaluator_Combat.generated.h"

UCLASS(DisplayName="Combat")
class ARPGAI_API UBehaviorEvaluatorConfig_Combat : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()

public:
	UBehaviorEvaluatorConfig_Combat();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutCombatTargetBBKey;

	// only updated when the behavior is active
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutAccumulatedDamageBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve HostileActorDistanceDesireDependencyCurve;

	// assumed normalized damage
	// x: total perceived accumulated damage / max health, y: factor to multiply total combat desire 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AccumulatedDamageToDesireScaleDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve StrengthAdvantageDesireDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DamageToDesireScaleDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve ProtectionAdvantageDesireDependencyCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGameplayTagFilterScalarValue> TagBasedScoreScales;
	
	// How much to scale combat utility if the perception of a potential target is coming from ally only
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AllyOnlyPerceptionScale = 0.5f;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Combat : public FBehaviorEvaluator_Base
{
	using Super = FBehaviorEvaluator_Base;
	
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
	FBehaviorEvaluator_Combat(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);

	virtual void Update(const float DeltaTime) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	float UpdatePerception();
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Combat> CombatConfig;
	TMap<TWeakObjectPtr<AActor>, FNpcImmediateThreatData> NpcThreatData;
};