#pragma once
#include "GameplayTagContainer.h"
#include "AITypes.h"
#include "NpcMemoryDataTypes.generated.h"

UENUM()
enum class EDetectionSource : uint8
{
	None = 0 UMETA(Hidden),
	Damage = 1,
	VisualMemory = Damage << 1, // this means NPC saw actor and still remembers it (stimulus hasn't expired yet)
	Ally = VisualMemory << 1,
	Audio = Ally << 1,
	Assumption = Audio << 1,
	VisualActive = Assumption << 1 // this means NPC sees actor right now
};
ENUM_CLASS_FLAGS(EDetectionSource)

struct ARPGAI_API FCharacterShortTermMemory
{
	FGameplayTag CharacterId;
	FGameplayTag Attitude;
	FGameplayTagContainer CharacterTags;
	FGameplayTagContainer ProducedSounds;
	
	float Distance = 0.f;
	float DamageOutput = 0.f;
	float TimeSeen = 0.f;
	float NormalizedHealth = 0.f;
	float Health = 0.f;
	float MaxHealth = 0.f;
	
	// from this character. Active as long as damage sense is active
	float ShortTermAccumulatedDamage = 0.f;

	// accumulated throughout the fight. Can be more than max health if NPC healed
	float LongTermAccumulatedReceivedDamage = 0.f;
	
	// accumulated throughout the fight. Can be more than max health if NPC healed
	float LongTermAccumulatedDealtDamage = 0.f;
	
	// NPC FV dot character FV (-1 means looking at each other, 1 - away from each other)
	float ForwardVectorsDotProduct = 1.f;
	// 1 -> means that the character is facing the NPC, -1 means that the character is facing away from the NPC
	float DotProduct_ActorFV_ToOwner = 1.f;

	float DotProduct_OwnerFV_ToActor = 1.f;
	
	// normalized. independent of NPCs health, distance between target and npc, friendly/ally state. just based on strength/protection/etc
	float StaticThreatScore = 0.f;
	float Protection = 0.f;
	float AttackRange = 0.f;
	
	// 13 June 2026 (aki): at this point, it's time to replace these bools with ENpcPerceptionTraits
	bool bCharacterSeesNpc = false;
	bool bAlly = false;
	bool bHostile = false;
	bool bAlive = true;
	
	// 14 June 2026 (aki): TODO implement
	bool bCanOwnerHitEnemy = false;
	bool bCanEnemyHitOwner = false;
	
	EDetectionSource DetectionSource = EDetectionSource::None;

	FORCEINLINE bool IsAlive() const { return bAlive; }
	FORCEINLINE bool IsHostile() const { return bHostile; }
	FORCEINLINE bool HasVisualDetection() const { return (DetectionSource & EDetectionSource::VisualMemory) != EDetectionSource::None; }
	FORCEINLINE bool HasImmediateVisualDetection(float MinDuration = 0.f) const 
	{ return (DetectionSource & EDetectionSource::VisualActive) != EDetectionSource::None && TimeSeen >= MinDuration; }

	bool HasDetectionSource(EDetectionSource TestSource) const { return (DetectionSource & TestSource) != EDetectionSource::None; };
};

UENUM()
enum class EPerceivedSoundTrait
{
	None = 0 UMETA(Hidden),
	CanSee = 1,
	ByAlly = CanSee << 1,
	InFront = ByAlly << 1,
	OutsideTerritory = InFront << 1,
};
ENUM_CLASS_FLAGS(EPerceivedSoundTrait)

struct FHeardSoundMemory
{
	FVector Location = FAISystem::InvalidLocation;
	FGameplayTag SoundTag;
	float Distance = 0.f;
	float Age = 0.f;
	EPerceivedSoundTrait Traits = EPerceivedSoundTrait::None;
	float Loudness = 1.f;

	bool IsByAlly() const { return (Traits & EPerceivedSoundTrait::ByAlly) != EPerceivedSoundTrait::None; }
};

struct FHazardPerceptionData
{
	FVector Location = FAISystem::InvalidLocation;
	float Radius = 0.f;
};

struct ARPGAI_API FNpcValueableItemPerceptionData
{
	TWeakObjectPtr<AActor> Actor;
	FGameplayTag ItemId;
	FGameplayTagContainer ItemTags;
	
	float Distance = 0.f;
	float TimeSeen = 0.f;
	float Value = 1.f;
};

USTRUCT(BlueprintType)
struct FNpcLongTermMemoryReason
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcStateFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ActorFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AttitudesFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RememberedTraits;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	TOptional<float> ForDurationGTH;
};