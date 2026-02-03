// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Interfaces/ICombatant.h"
#include "GameplayAbilityTargetData_ReceivedHit.generated.h"

USTRUCT(BlueprintType)
struct COMBAT_API FGameplayAbilityTargetData_ReceivedHit : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

public:
	FGameplayAbilityTargetData_ReceivedHit() {};

	FVector HitLocation = FVector::ZeroVector;
	FGameplayTag HitDirectionTag;
	float PoiseDamage = 0.f;
	float HealthDamage = 0.f;
	FHitResult HitResult;
	TWeakObjectPtr<AActor> Causer;
	FGuid CauserId;
	
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
		Ar << CauserId;
		bOutSuccess = true;
		return true;
	}

	void SetCauser(AActor* Actor)
	{
		Causer = Actor;
		if (auto Combatant = Cast<ICombatant>(Actor))
			CauserId = Combatant->GetCombatantId();
	};
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ReceivedHit> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ReceivedHit>
{
	enum
	{
		WithNetSerializer = true // This is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};