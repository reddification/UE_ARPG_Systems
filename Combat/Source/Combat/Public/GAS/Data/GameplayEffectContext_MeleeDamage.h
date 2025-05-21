// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectContext_MeleeDamage.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct COMBAT_API FGameplayEffectContext_MeleeDamage : public FGameplayEffectContext
{
	GENERATED_BODY()
	
public:
	FGameplayEffectContext_MeleeDamage()
		: FGameplayEffectContext()
	{
	}

	FGameplayEffectContext_MeleeDamage(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	static FGameplayEffectContext_MeleeDamage* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGameplayEffectContext_MeleeDamage* NewContext = new FGameplayEffectContext_MeleeDamage();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayEffectContext_MeleeDamage::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

};


template<>
struct TStructOpsTypeTraits<FGameplayEffectContext_MeleeDamage> : public TStructOpsTypeTraitsBase2<FGameplayEffectContext_MeleeDamage>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};