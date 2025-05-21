// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "CoreMinimal.h"
#include "GameplayAbilityTargetData_Dodge.generated.h"

USTRUCT(BlueprintType)
struct COMBAT_API FGameplayAbilityTargetData_Dodge : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

public:
	FGameplayAbilityTargetData_Dodge() {};
	
	UPROPERTY()
	FVector DodgeLocation = FVector::ZeroVector;
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Dodge::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		DodgeLocation.NetSerialize(Ar, Map, bOutSuccess);
		
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Dodge> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Dodge>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};