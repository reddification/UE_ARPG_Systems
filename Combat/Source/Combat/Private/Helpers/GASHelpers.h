#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Data/CombatGameplayTags.h"

template<typename TGameplayAbilityTargetDataStruct>
const TGameplayAbilityTargetDataStruct* GetActivationData(const FGameplayAbilityTargetDataHandle& TargetData)
{
	const auto Data = TargetData.Get(0);
	if (Data)
	{
		if (ensure(Data->GetScriptStruct() == TGameplayAbilityTargetDataStruct::StaticStruct()))
		{
			const TGameplayAbilityTargetDataStruct* ActivationData = static_cast<const TGameplayAbilityTargetDataStruct*>(Data);
			return ActivationData;
		}
	}

	return nullptr;
}

inline FGameplayTag GetActorRelativeDirectionTag(const AActor* Actor, const FVector& WorldDirection, const float DirectionDotProductThreshold)
{
	if (!ensure(IsValid(Actor)))
		return FGameplayTag::EmptyTag;
	
	FVector OwnerForwardVector = Actor->GetActorForwardVector();
	FVector OwnerRightVector = Actor->GetActorRightVector();
	float HitForwardVectorDotProduct = WorldDirection | OwnerForwardVector;
	float HitRightVectorDotProduct = WorldDirection | OwnerRightVector;
		
	if (HitForwardVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Front;
	else if (HitForwardVectorDotProduct < -DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Back;
	else if (HitRightVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Right;
	else
		return CombatGameplayTags::Combat_HitDirection_Left;
}