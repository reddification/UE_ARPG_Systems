#pragma once
#include "GameplayTagContainer.h"
#include "CombatDataTypes.generated.h"

struct FMontageData;
class UNiagaraSystem;

// This enums items order is important since it combines both attack types and attack trajectories
// so don't change order without necessity
UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	None = 0,

	Trajectory = 1,
	
	LeftUnterhauw = 2,
	LeftMittelhauw = 3,
	LeftOberhauw = 4,
	Thrust = 5,
	RightUnterhauw = 6,
	RightMittelhauw = 7,
	RightOberhauw = 8,
	VerticalSlash = 9,
	SpinLeftMittelhauw = 10,
	SpinLeftOberhauw = 11,
	SpinRightMittelhauw = 12,
	SpinRightOberhauw = 13,

	Type = 14,
	
	LightAttack = 15,
	HeavyAttack = 16,
	SpecialAttack = 17,
	Max
};

UENUM(BlueprintType)
enum class EMeleeAttackPhase : uint8
{
	None,
	WindUp,
	Release,
	Recover
};

UENUM(BlueprintType)
enum class EWeaponHitSituation : uint8
{
	None,
	WeaponClash,
	AttackBlocked,
	AttackParried,
	Body,
	ImmovableObject
};

UENUM(BlueprintType)
enum class EAttackStepDirection : uint8
{
	None,
	Forward,
	Left,
	Right,
	Back
};

USTRUCT(BlueprintType)
struct FReceivedHitData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag HitDirectionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HealthDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PoiseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FHitResult HitResult;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHardHit = false;	
};

UENUM(BlueprintType)
enum class EParryDirection : uint8
{
	None,
	Left,
	Right,
};

enum class EBlockResult : uint8
{
	None,
	Block,
	Parry,
};

struct FWeaponDamageData
{
	float BaseDamageOutput = 0.f;
	float MaxDamageOutput = 0.f;
	float RequiredWeaponStrength = 0.f; 
	float RequiredWeaponDexterity = 0.f;
	FGameplayTag DamageType;
};

struct FAttackDamageEvaluationData
{
	FWeaponDamageData WeaponDamageData;
	
	float Strength = 0.f;
	float Dexterity = 0.f;
	float StaminaRatio = 0.f;
	float Poise = 0.f;
	float WeaponMastery = 0.f;
	FVector Direction = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FMontageData
{
	GENERATED_BODY()

	// Optional. Used for gore-able skeletal meshes upon death
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRestoreInitialSkeletalMesh = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontage;	
};

USTRUCT(BlueprintType)
struct FContextMontages
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery ContextTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UAnimMontage*> Montages_Deprecated;

	// 20.06.2025 @AK: TODO move all montages from Montages_Deprecated to Montages 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMontageData> MontagesOptions;
};

struct FCombatCollisionShapeData
{
	enum ECombatCollisionShapeType : uint8
	{
		None,
		Cylinder,
		Box,
		Convex
	};
	
	ECombatCollisionShapeType Type = ECombatCollisionShapeType::None;
	FVector Center = FVector::ZeroVector; // local-space, i guess?
	float Radius = 0.f;
	float HalfHeight = 0.f;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector Extent;
	
	float GetRange() const
	{
		if (Type == ECombatCollisionShapeType::Cylinder)
			return HalfHeight * 2.f;
		else
			return Extent.Z * 2.f;
	}
};

USTRUCT(BlueprintType)
struct COMBAT_API FWeaponFX
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<class UNiagaraSystem> VFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<class USoundCue> SFX;
};

enum class ECollisionComponentWeaponType : uint8
{
	None,
	MeleeWeapon,
	Shield
};

UENUM()
enum class EPlayerCombatControl : uint8
{
	None = 0,
	Moveset = 1,
	Manual = 2
};