#pragma once

#include "CoreMinimal.h"
#include "Data/NpcCombatTypes.h"
#include "v2/BehaviorEvaluator_OperationBased.h"
#include "BehaviorEvaluator_CombatBase.generated.h"

class UNpcAreasComponent;
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
	
	// if old target is still relevant but there's a new target with a higher priority - delay target switch for this time
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetSwitchDelay = 2.f;
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

	virtual void OnIndividualScoreCalculated(AActor* Actor, const FCharacterShortTermMemory& Value, float IndividualScore) override;
	virtual void UpdateBehaviorStateData(const FActorScoresContainer& EnemiesData);
	virtual float CalculateCombatUtility(float EnemyPressure, float StatePressure);
	
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_CombatBase> CombatConfig;
	TMap<TWeakObjectPtr<AActor>, FNpcImmediateThreatData> NpcImmediateThreatData;
	TMap<TWeakObjectPtr<AActor>, FNpcEnemyCombatMemory> EnemyMemoryData;
	
private:
	// In world time
	float LastTargetSwitchTime = 0.f;
	bool IsTargetUpdateRedundant(AActor* BestTarget, const FNpcPrimaryCombatTargetData& CurrentPrimaryTargetData) const;
	
#if WITH_EDITOR
	void LogTargetChange(AActor* BestTarget, const FNpcPrimaryCombatTargetData& CurrentPrimaryTargetData) const;
#endif
};