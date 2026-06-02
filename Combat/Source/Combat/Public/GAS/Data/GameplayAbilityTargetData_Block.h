#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_Block.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Block : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_Block();
	
	UPROPERTY()
	EMeleeAttackType IncomingAttackType;

	UPROPERTY()
	EMeleeAttackType IncomingAttackTrajectory;

	UPROPERTY()
	AActor* Attacker = nullptr;
	
	UPROPERTY()
	bool bUseGuidedBlocking = false;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Block::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		Ar << IncomingAttackType;
		Ar << IncomingAttackTrajectory;
		Ar << Attacker;
		Ar << bUseGuidedBlocking;
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_Block::FGameplayAbilityTargetData_Block()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Block> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Block>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};