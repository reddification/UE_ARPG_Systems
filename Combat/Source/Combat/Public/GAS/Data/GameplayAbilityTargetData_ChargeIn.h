#pragma once
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_ChargeIn.generated.h"

USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_ChargeIn : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
public:
	FGameplayAbilityTargetData_ChargeIn();
	
	UPROPERTY()
	FVector ToLocation;
	
	UPROPERTY()
	float VerticalImpulse = 0.f;

	UPROPERTY()
	float ForwardImpulse = 0.f;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_ChargeIn::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		ToLocation.NetSerialize(Ar, Map, bOutSuccess);
		Ar << ToLocation;
		Ar << VerticalImpulse;
		Ar << ForwardImpulse;
		
		bOutSuccess = true;
		return true;
	}
};

inline FGameplayAbilityTargetData_ChargeIn::FGameplayAbilityTargetData_ChargeIn()
{
	
}

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ChargeIn> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ChargeIn>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};