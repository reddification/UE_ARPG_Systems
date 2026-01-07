// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatDataTypes.h"
#include "GameplayTagContainer.h"
#include "Components/MeleeBlockComponent.h"
#include "MeleeCombatSettings.generated.h"

struct FGameplayTag;
UENUM(BlueprintType)
enum class EAttackFocusType : uint8
{
	None,
	Actor,
	Location
};

USTRUCT(BlueprintType)
struct FMultipleAttackMappingStep
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -180.f, ClampMin = -180.f, ClampMax = 180.f, UIMax = 180.f))
	float Angle = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, ClampMax = 180.f, UIMax = 180.f))
	float AngleThreshold = 15.f;
};

USTRUCT(BlueprintType)
struct FMultipleAttackInputsMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMultipleAttackMappingStep InitialStep;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -180.f, ClampMin = -180.f, ClampMax = 180.f, UIMax = 180.f))
	float ExpectedResultAngleDegree = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -180.f, ClampMin = -180.f, ClampMax = 180.f, UIMax = 180.f))
	float AngleThreshold = 15.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MinVectors = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMeleeAttackType ResultAttackType;
};

USTRUCT(BlueprintType)
struct FMeleeAttackTypeWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EMeleeAttackType> AttackTypes;	
};

USTRUCT(BlueprintType)
struct FChainableAttacksWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EMeleeAttackType, FMeleeAttackTypeWrapper> ChainableAttacks;
};

USTRUCT(BlueprintType)
struct FChainableAttacksAgainstWeaponWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FChainableAttacksWrapper> AttacksAgainstWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FChainableAttacksWrapper DefaultAttacks;
};


USTRUCT(BlueprintType)
struct FWeaponCombinationsData
{
	GENERATED_BODY()

	// key - weapon mastery level
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int, FChainableAttacksAgainstWeaponWrapper> MasteryLevelChainableAttacks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FChainableAttacksWrapper DefaultAttacks;
};

USTRUCT(BlueprintType)
struct FMeleeAttackPhaseSpeedModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EMeleeAttackPhase, float> AttackPhaseSpeed;
};

USTRUCT(BlueprintType)
struct FWeaponComboSequenceWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EMeleeAttackType> LightAttackCombos;
};

USTRUCT(BlueprintType)
struct FWeaponMasteryMoveset
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int, FWeaponComboSequenceWrapper> WeaponMasteryMovesets;
};

USTRUCT(BlueprintType)
struct FMeleeCombatDirectionalInputParameters
{
	GENERATED_BODY()

	// basically how many frames it takes for each attack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int InputBufferSize = 15;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int FramesWithoutInputToResetSwing = 5;

	// Lateral mouse movement (on X axis) has more range than upside-down movement just due to human's physiology.
	// So I introduce "balancing parameter" to easier detect upside down motions
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -5.f, ClampMin = -5.f, UIMax = 5.f, ClampMax = 5.f))
	float VerticalInputFactor = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinConsiderableDifferenceInInputScales = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BestTargetSearchRadius = 250.f;

	// attack direction acceleration significance is supposed to be [0..1]
	// so this value shouldn't be too big as it means how much of significance decays each second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackDirectionAccelerationSignificanceDecayRate = 0.25f;

	// if dot product less than this value - then it's a new acceleration (or movement input) and it should replace previous
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float ConsiderableAccelerationDifferenceDotProduct = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float TargetEvaluationAccelerationSignificanceBase = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float FocusInterpSpeed = 5.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAbsorbNewerAttacksByOlder = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAttackFocusType AttackFocusType = EAttackFocusType::Location;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShowDebug = false;

	// How many frames of attack motion are accumulated into a single "attack" input
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int AttackFrameAccumulation = 5;

	// key - weapon mastery, value - how many inputs in accumulated vector are required for activation 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<int, int> WeaponMasteryToMinAttackInputs;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int MinInputsToAttack = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	float TickRate = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float MaxCameraRotationAngleDuringAttack = 45.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float InitialLookRatioDuringAttack = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float AccelerationSignificanceThreshold = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float AccelerationToMoveDirectionMatchDotProductThreshold = 0.75f;
};

USTRUCT(BlueprintType)
struct FNpcCombatSituationKey
{
	GENERATED_BODY()

	FNpcCombatSituationKey()
	{
		NpcActiveWeaponType = FGameplayTag::EmptyTag;
		EnemyActiveWeaponType = FGameplayTag::EmptyTag;
		WeaponMasteryLevel = 0;
	}

	FNpcCombatSituationKey(const FGameplayTag& NpcActiveWeaponType, const FGameplayTag& EnemyActiveWeaponType,
		int WeaponMasteryLevel)
		: NpcActiveWeaponType(NpcActiveWeaponType),
		  EnemyActiveWeaponType(EnemyActiveWeaponType),
		  WeaponMasteryLevel(WeaponMasteryLevel)
	{
	}

	FORCEINLINE bool operator==(const FNpcCombatSituationKey& Other) const
	{
		return Equals(Other);
	}

	FORCEINLINE	bool Equals(const FNpcCombatSituationKey& Other) const
	{
		return (NpcActiveWeaponType == Other.NpcActiveWeaponType && EnemyActiveWeaponType == Other.EnemyActiveWeaponType && WeaponMasteryLevel == Other.WeaponMasteryLevel);
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcActiveWeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EnemyActiveWeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int WeaponMasteryLevel = 1;
};

FORCEINLINE uint32 GetTypeHash(const FNpcCombatSituationKey& This)
{
	return HashCombine(HashCombine(GetTypeHash(This.NpcActiveWeaponType), GetTypeHash(This.EnemyActiveWeaponType)), GetTypeHash(This.WeaponMasteryLevel));
}

USTRUCT(BlueprintType)
struct FWeaponDamageAttributeDependency
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve StrengthDeltaDamageOutputDependency;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve DexterityDeltaDamageOutputDependency;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve StaminaDamageOutputDependency;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve WeaponMasteryDamageOutputDependency;
};

USTRUCT(BlueprintType)
struct FCombatStyleMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer EquippedItemTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CombatStyle;
};

// This class is getting out of control with the amount of anim montages stored in it.
// TODO leave all data here, move moveset-related stuff to separate data assets and "SoftObjectPtr" it
UCLASS(Config=Game, defaultconfig, DisplayName="Combat")
class COMBAT_API UMeleeCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FMeleeCombatDirectionalInputParameters MeleeCombatDirectionalInputParameters;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMultipleAttackInputsMapping> MultipleAttacksToAttackCombination;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	float OutstrikeStrengthDelta = 15.f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ConsequitiveComboAttackWindUpSpeedScale = 1.5f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	TMap<FGameplayTag, FWeaponCombinationsData> AIWeaponTypeChainableAttacks;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	FRuntimeFloatCurve AIAttackRangeIntelligenceMisperceptionFactor;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	FRuntimeFloatCurve AITargetDistanceIntelligenceMisperceptionFactor;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	FRuntimeFloatCurve AIBlockReactionDelayDependency;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	FRuntimeFloatCurve AIBlockStaminaDelayDependency;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	float AIRangeDiffForLongAttack = 25.f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TArray<FCombatStyleMapping> CombatStyles;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Protection")
	TMap<FGameplayTag, FGameplayAttribute> DamageTypeToProtectionAttribute;

	// This is an empiric balancing parameter that defines how effective protection is
	// The final damage calculation equation is final damage = raw damage * e^(-protection / ProtectionEffectivenessScale)
	//Smaller scale = higher resistance impact.
	//Larger scale = softer curve, resistance is less effective.
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Protection", meta=(ClampMin = 0.1, UIMin = 0.1f))
	double ProtectionEffectivenessScale = 50.f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve BlockStaminaAccumulationScaleDependency;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Poise")
	FRuntimeFloatCurve StrengthDiffPoiseDamage;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Poise")
	FRuntimeFloatCurve StaminaRatioPoiseDamageScale;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category="Poise")
	FRuntimeFloatCurve DexterityPoiseDamageReductionScale;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve TargetPoiseToReceivedDamagedScaleDependency;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve CurrentAttackEnemiesHitDamageDependency;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FRuntimeFloatCurve TargetToAttackerDotProductPoiseDamageDependency;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FWeaponDamageAttributeDependency> WeaponDamageAttributesDependencies;

	// if bone is not present in the list then 1.f is taken
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, float> HitBonesDamageScore;

	// if bone is not present in the list then 1.f is taken
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, float> HitBonesPoiseScore;

	// Sweeps are done only in weapon release phase
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	int WeaponCollisionSweepsPerSeconds = 100;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponCollisionProfileName = FName("Weapon");
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	bool bDrawDebugSweeps = true;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<int, FMeleeAttackPhaseSpeedModifier> PlayerWeaponMasteryAttackPhaseSpeedScales;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<int, FMeleeAttackPhaseSpeedModifier> NpcWeaponMasteryAttackPhaseSpeedScales;

	// It is expected that skeletal meshes physics asset primitives have a box primitive with this name that must be used as the combat collision (or just 1 box primitive with any name)
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FName CombatCollisionName = FName("CombatCollision");

	// Socket for the center of the blade. Used for locations of shape sweeps. Most important for spears where not all weapon is a damaging part
	// Sockets Z axis must be oriented along the blade
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FName CombatCollisionCenterSocketName = FName("CombatCollisionCenter");
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float OberhauAttackStrengthFactor = 1.4f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float HeavyAttackWindupSpeedModifier = 0.7f;
	
	// UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(Category="Block"))
	// float BreakBlockStrengthDelta = 15.f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float AttackerStrengthScaleWhenHitBlock = 0.5f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1, Category="Block"))
	int MaxAccumulatedBlockInputs = 50;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, ClampMax = 1.f, UIMax = 1.f, Category="Block"))
	float CollinearBlockInputsDotProductThreshold = 0.75f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float StrongBlockActivationThreshold = 0.75f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float PlayerBlockInputAccumulationScale = 2.5f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float BlockStrengthDecayRate = 3.f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float BlockDecayDelay = 0.5;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Block"))
	float BlockStrengthAccumulationScale = 2.f;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f, Category="Block"))
	float BlockStrengthToParry = 0.5f;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(Category="TargetLock"))
	TArray<TEnumAsByte<ECollisionChannel>> TargetLockObjectChannels;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, Category="Anim stance"))
	float KeepWeaponReadyAfterAttackDelay = 3.f;
};
