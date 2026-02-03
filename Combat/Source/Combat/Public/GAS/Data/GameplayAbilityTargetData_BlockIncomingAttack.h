#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_BlockIncomingAttack.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_BlockIncomingAttack : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_BlockIncomingAttack();
	
	UPROPERTY()
	EMeleeAttackType IncomingAttackType;

	UPROPERTY()
	EMeleeAttackType IncomingAttackTrajectory;

	UPROPERTY()
	AActor* Attacker = nullptr;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_BlockIncomingAttack::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		Ar << IncomingAttackType;
		Ar << IncomingAttackTrajectory;
		Ar << Attacker;
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_BlockIncomingAttack::FGameplayAbilityTargetData_BlockIncomingAttack()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_BlockIncomingAttack> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_BlockIncomingAttack>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};