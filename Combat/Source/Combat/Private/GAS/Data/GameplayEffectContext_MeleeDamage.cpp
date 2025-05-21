// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/GameplayEffectContext_MeleeDamage.h"

FGameplayEffectContext_MeleeDamage* FGameplayEffectContext_MeleeDamage::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FGameplayEffectContext_MeleeDamage::StaticStruct()))
	{
		return (FGameplayEffectContext_MeleeDamage*)BaseEffectContext;
	}

	return nullptr;
}

bool FGameplayEffectContext_MeleeDamage::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}
