#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_Clash.generated.h"

UENUM()
enum class EClashSource
{
	None,
	Parry,
	AttacksCollide,
	Outstriked
};

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Clash : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_Clash();
	
	UPROPERTY()
	FVector Direction;

	UPROPERTY()
	EClashSource ClashSource;
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Clash::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		Direction.NetSerialize(Ar, Map, bOutSuccess);
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_Clash::FGameplayAbilityTargetData_Clash()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Clash> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Clash>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
