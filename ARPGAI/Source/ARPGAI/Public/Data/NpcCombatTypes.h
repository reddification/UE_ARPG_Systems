#pragma once

#include "GameplayTagContainer.h"
#include "AITypes.h"
#include "NpcMemoryDataTypes.h"
#include "NpcCombatTypes.generated.h"

class UNpcReactionEvaluatorBase;
class INpcThreat;
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

struct FNpcImmediateThreatData
{
	FNpcImmediateThreatData() = default;
	FNpcImmediateThreatData(float InScore, float InAttackRange) : BehaviorEvaluatorScore(InScore), AttackRange(InAttackRange) {}
	
	float BehaviorEvaluatorScore = 0.f;
	float AttackRange = 0.f;
};

typedef TMap<TWeakObjectPtr<AActor>, FNpcImmediateThreatData> FNpcCurrentCombatThreatsContainer;

struct FNpcPrimaryCombatTargetData
{
	TWeakObjectPtr<AActor> Actor;
	FGameplayTag BehaviorType;
	
	void Reset();
	bool IsValid() const;
};

UENUM()
enum class ENpcTargetDistanceEvaluation : uint8
{
	TargetIsStationary,
	TargetIsApproaching,
	TargetIsGettingAway,
};

// unlike perception, this data is stored in UNpcCombatLogicComponent for more flexible logic
struct FNpcEnemyCombatMemory
{
	FNpcEnemyCombatMemory() {  }
	
	FNpcEnemyCombatMemory(const FNpcEnemyCombatMemory& Other)
		: LastSeenLocation(Other.LastSeenLocation),
		  LastUpdateTime(Other.LastUpdateTime),
		  CurrentDetectionSource(Other.CurrentDetectionSource),
		  bAlive(Other.bAlive)
	{
		AccumulatedDealtDamage += Other.AccumulatedDealtDamage;
		AccumulatedReceivedDamage += Other.AccumulatedReceivedDamage;
	}

	FNpcEnemyCombatMemory(FNpcEnemyCombatMemory&& Other) noexcept
		: LastSeenLocation(std::move(Other.LastSeenLocation)),
		  LastUpdateTime(Other.LastUpdateTime),
		  CurrentDetectionSource(Other.CurrentDetectionSource),
		  bAlive(Other.bAlive)
	{
		AccumulatedDealtDamage += Other.AccumulatedDealtDamage;
		AccumulatedReceivedDamage += Other.AccumulatedReceivedDamage;
	}

	FNpcEnemyCombatMemory& operator=(const FNpcEnemyCombatMemory& Other)
	{
		if (this == &Other)
			return *this;
			
		LastSeenLocation = Other.LastSeenLocation;
		LastUpdateTime = Other.LastUpdateTime;
		CurrentDetectionSource = Other.CurrentDetectionSource;
		bAlive = Other.bAlive;
		
		AccumulatedReceivedDamage += Other.AccumulatedReceivedDamage;
		AccumulatedDealtDamage += Other.AccumulatedDealtDamage;
		
		return *this;
	}

	FNpcEnemyCombatMemory& operator=(FNpcEnemyCombatMemory&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		
		LastSeenLocation = std::move(Other.LastSeenLocation);
		LastUpdateTime = Other.LastUpdateTime;
		CurrentDetectionSource = Other.CurrentDetectionSource;
		bAlive = Other.bAlive;
		
		AccumulatedReceivedDamage += Other.AccumulatedReceivedDamage;
		AccumulatedDealtDamage += Other.AccumulatedDealtDamage;
		
		return *this;
	}

	FVector LastSeenLocation = FAISystem::InvalidLocation;
	double LastUpdateTime = 0.f;
	float AccumulatedReceivedDamage = 0.f; // unlike short term memory, this is accumulated damage throughout the whole combat activity
	float AccumulatedDealtDamage = 0.f; // unlike short term memory, this is accumulated damage throughout the whole combat activity
	EDetectionSource CurrentDetectionSource = EDetectionSource::None;
	bool bAlive = true;
};