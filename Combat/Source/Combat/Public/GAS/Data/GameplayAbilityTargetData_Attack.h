#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Data/CombatDataTypes.h"
#include "GameplayAbilityTargetData_Attack.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Attack : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_Attack();
	
	UPROPERTY()
	EMeleeAttackType AttackType;

	UPROPERTY()
	AActor* Attacker = nullptr;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Attack::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		Ar << AttackType;//.NetSerialize(Ar, Map, bOutSuccess);
		Ar << Attacker;
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_Attack::FGameplayAbilityTargetData_Attack()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Attack> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Attack>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};