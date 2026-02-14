#pragma once

#include "GameplayTagContainer.h"
#include "CombatEvaluationData.h"
#include "NpcCombatTypes.generated.h"

class UNpcReactionEvaluatorBase;
class IThreat;
class UGameplayEffect;

UENUM()
enum class ENpcDefensiveAction : uint8
{
	None,
	Backdash,
	Parry,
	Dodge,
	CounterAttack,
};

UENUM()
enum class ENpcBlockResult : uint8
{
	None,
	Blocked,
	Parried,
};

UENUM()
enum class ENpcAttackResult : uint8
{
	None,
	Whiffed,
	Parried,
	Hit
};

UENUM(BlueprintType)
enum class ENpcCombatRole : uint8
{
	None = 0,
	Attacker = 1,
	Surrounder = 2,
	Idle = 3,
	Max = Idle UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FTagFloatPair
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FFloatRange ValueRange;
};

struct FNpcThreatData
{
	float RetreatUtilityScore = 0.f;
	float ThreatScore = 0.f;
	float AttackRange = 0.f;
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