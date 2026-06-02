#pragma once

#include "CoreMinimal.h"
#include "Data/NpcCombatTypes.h"
#include "v2/BehaviorEvaluator_OperationBased.h"
#include "BehaviorEvaluator_CombatBase.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBehaviorEvaluatorConfig_CombatBase : public UBehaviorEvaluatorConfig_OperationBased
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_CombatBase();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector CombatTargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector LastSeenTargetLocationBBKey;
};

class FBehaviorEvaluator_CombatBase : public FBehaviorEvaluator_OperationBased
{
	using Super = FBehaviorEvaluator_OperationBased;

public:
	FBehaviorEvaluator_CombatBase(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);

protected:
	virtual void PreEvaluate() override;
	virtual float Evaluate() override;
	
	virtual void OnActivated() override;
	virtual void Cleanup() override;

	virtual bool IsCharacterRelevant(const FCharacterPerceptionData& CharacterPerceptionData, 
		const FEntityOperationEvaluationParameters& EvaluationParameters) const override;
	virtual void OnIndividualScoreCalculated(AActor* Actor, const FCharacterPerceptionData& Value, float IndividualScore) override;

	virtual void UpdateAiBehaviorState(const FActorScoresContainer& EnemiesData) const;
	virtual float CalculateCombatUtility(float EnemyPressure, float StatePressure);
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_CombatBase> CombatConfig;
	TMap<TWeakObjectPtr<AActor>, FNpcImmediateThreatData> NpcImmediateThreatData;
	TMap<TWeakObjectPtr<AActor>, FNpcEnemyCombatMemory> EnemyMemoryData;
	
private:
#if WITH_EDITOR
	void LogTargetChange(AActor* BestTarget, const FNpcActiveTargetData& CurrentPrimaryTargetData) const;
#endif
};