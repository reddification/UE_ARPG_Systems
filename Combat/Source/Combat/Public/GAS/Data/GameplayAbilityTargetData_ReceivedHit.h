// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_ReceivedHit.generated.h"

//@AK 19.09.2024:
// Exposed all UPROPERTYies to be EditAnywhere and BlueprintReadWrite to try to call ReceiveHit ability from blueprints
// Alas couldn't find a way to pass this fucking struct to the TargetData in BP
// So actually no reason to keep EditAnywhere and BlueprintReadWrite anymore. TODO remove these accessors later
USTRUCT(BlueprintType)
struct COMBAT_API FGameplayAbilityTargetData_ReceivedHit : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

public:
	FGameplayAbilityTargetData_ReceivedHit() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HitLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HitDirectionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoiseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient)
	FHitResult HitResult;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_ReceivedHit::StaticStruct();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// The engine already defined NetSerialize for FName & FPredictionKey, thanks Epic!
		HitLocation.NetSerialize(Ar, Map, bOutSuccess);
		HitDirectionTag.NetSerialize(Ar, Map, bOutSuccess);
		Ar << PoiseDamage;
		Ar << HealthDamage;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ReceivedHit> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ReceivedHit>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};