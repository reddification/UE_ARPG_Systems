#pragma once
#include "GameplayTagContainer.h"
#include "Data/NpcCombatTypes.h"
#include "Data/AiDataTypes.h"
#include "NpcCombatParametersDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FActorAwarenessData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseScore = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve DistanceAwarenessScaleDependencyCurve;
};

USTRUCT(BlueprintType)
struct FBehaviorUtilityParameters
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve AccumulatedDamageReactionDependency;

	// // a player can weight 1.f, a grenade can weight 4.f
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve PerceivedThreatsWeightReactionDependency;

	// a player can weight 1.f, a grenade can weight 4.f
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FActorAwarenessData> ActorTypeAwarenesses;
};

USTRUCT(BlueprintType)
struct FNpcFeintParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float Probability = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float RelativeAttackWindUpDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float NextAttackCooldown = 1.5f;
};

USTRUCT(BlueprintType)
struct FNpcCombatEvaluationParameters
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve IntelligenceAttackRangeDeviationDependency;

	// on incoming attack from enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve BackstepAggressionProbabilityDependency;

	// on attack whiff from NPC owner
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve BackstepStaminaProbabilityDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve ActiveThreatsCountReactionModifierDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve RelativeHealthToAnxietyScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FCombatMoveSpeedsData> MoveSpeedDependencies;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StepOutDistance = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PathfindingAvoidThreatsScoreFactor = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PathfindingDesiredAvoidThreatsDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcFeintParameters FeintParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Taunt")
	float TauntChanceAfterAttack = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Taunt")
	float NpcToEnemyHealthRatioToTauntStaggeredEnemy = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Taunt")
	float SafeDistanceForTauntFactor = 0.15f;

	// used like bTaunt = Rand(0.f, ChanceToTauntEnemyInBadScenario) > Intelligence, so not really a "raw" chance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Taunt")
	float ChanceToTauntEnemyInBadScenario = 0.4f;
};

UCLASS()
class UNpcCombatParametersDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FNpcCombatEvaluationParameters NpcCombatEvaluationParameters;

	// X = count, Y = "I want to retaliate" SCORE (starting point)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	FRuntimeFloatCurve BlockRetaliationDesire_ReactionDependency;
	
	// X = count, Y = retaliation SCALE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	FRuntimeFloatCurve BlockRetaliationDesireScale_AggressionDependency;
	
	// normalized stamina expected
	// X = count, Y = retaliation SCALE
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	FRuntimeFloatCurve BlockRetaliationDesireScale_StaminaDependency;
	
	// X = count, Y = "I don't want to retaliate" score. Later multiplied by intelligence and subtracted from retaliation desire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	FRuntimeFloatCurve BlockRetaliationDesire_EnemiesAroundCountDependency;
	
	// X = count, Y = "I don't want to retaliate" score. Later multiplied by intelligence and subtracted from retaliation desire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	FRuntimeFloatCurve BlockRetaliationDesire_EnemiesAttackingMeDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Block retalization")
	TMap<ENpcBlockResult, float> BlockResultRetaliationDesireScales;	
	
	// Key - attitude to hit causer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attitudes")
	TMap<FGameplayTag, int> ForgivableCountOfReceivedHits;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attitudes")
	TMap<FGameplayTag, float> RememberHitsFromCharactersDurationsGameTimeHours;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReacts")
	float BackdashChanceScaleOnHit = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReacts")
	FGameplayTagContainer RegularHitTypes;

	// this time is average - it will be taken in a random iterval +- 50%
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bait attack", meta=(ClampMin = 0.f, UIMIn = 0.f))
	float BaitAttackCooldownAvg = 3.f;

	// this time is average - it will be taken in a random iterval +- 50%
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bait attack", meta=(ClampMin = 0.f, UIMIn = 0.f))
	float BaitAttackDurationAvg = 2.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bait attack", meta=(ClampMin = 0.f, UIMIn = 0.f))
	float BaitAttackProbabilityBase = 1.f;
	
	// this value will be SUBTRACTED FROM calculated desire, not added to.  
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bait attack")
	FRuntimeFloatCurve BaitAttackDesire_AggressionDependency;

};
