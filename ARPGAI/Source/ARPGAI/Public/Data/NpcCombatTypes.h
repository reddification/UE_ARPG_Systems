#pragma once

#include "AiDataTypes.h"
#include "GameplayTagContainer.h"
#include "CombatEvaluationData.h"
#include "CommonTypes.h"
#include "NpcCombatTypes.generated.h"

class UNpcReactionEvaluatorBase;
class IThreat;
class UGameplayEffect;

UENUM()
enum class ENpcDefensiveAction : uint8
{
	None,
	StepOut,
	Parry,
	Dodge,
	CounterAttack,
};

UENUM()
enum class ENpcAttackResult : uint8
{
	None,
	Whiffed,
	Parried,
	Hit
};

USTRUCT(BlueprintType)
struct FTagFloatPair
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="AI.Threat"))
	FGameplayTag Tag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FFloatRange ValueRange;
};

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
	TMap<TSubclassOf<AActor>, FActorAwarenessData> ActorTypeAwarenesses;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	bool bIgnoreTeamDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float DamageScoreFactor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float TeammateTargetScoreFactor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float MobCoordinatorScoreFactor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility")
	TMap<FGameplayTag, FBehaviorUtilityParameters> CombatBehaviorUtilityParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FTagFloatPair> ThreatLevels;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Threat"))
	TMap<FGameplayTag, float> PerceivedThreatToMinAnxiety;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve IntelligenceAttackRangeDeviationDependency;

	// on incoming attack from enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve BackstepReactionProbabilityDependency;

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

	// Key - attitude to hit causer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, int> ForgivableCountOfReceivedHits;
};

UCLASS()
class UNpcStatesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="AI.State,G2VS2.Character.State"))
	TMap<FGameplayTag, FGameplayEffectsWrapper> StateEffects;
};


struct FNpcThreatData
{
	float RetreatUtilityScore = 0.f;
	float ThreatScore = 0.f;
	float AttackRange = 0.f;
};

struct FAttackingThreatData
{
	TWeakObjectPtr<AActor> Actor = nullptr;
	const IThreat* Threat = nullptr;
	float BiasedAttackDistance = 0.f;
	float BiasedDistanceToThreat = 0.f;
	FVector ForwardVector = FVector::ZeroVector;
};

typedef TMap<TWeakObjectPtr<AActor>, FNpcThreatData> FNpcActiveThreatsContainer;

struct FNpcActiveTargetData
{
	TWeakObjectPtr<AActor> ActiveTarget;
	FGameplayTag ActiveBehaviorTypeTag;
	NpcCombatEvaluation::FNpcCombatPerceptionData NpcCombatPerceptionData;	
	void Reset();
};

UENUM()
enum class ENpcTargetDistanceEvaluation : uint8
{
	TargetIsStationary,
	TargetIsApproaching,
	TargetIsGettingAway,
};