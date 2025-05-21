#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_BlockAttack.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_BlockAttack : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_BlockAttack();
	
	UPROPERTY()
	EMeleeAttackType IncomingAttackType;

	UPROPERTY()
	EMeleeAttackType IncomingAttackTrajectory;
	
	UPROPERTY()
	AActor* Attacker = nullptr;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_BlockAttack::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		Ar << IncomingAttackType;//.NetSerialize(Ar, Map, bOutSuccess);
		Ar << Attacker;
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_BlockAttack::FGameplayAbilityTargetData_BlockAttack()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_BlockAttack> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_BlockAttack>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};