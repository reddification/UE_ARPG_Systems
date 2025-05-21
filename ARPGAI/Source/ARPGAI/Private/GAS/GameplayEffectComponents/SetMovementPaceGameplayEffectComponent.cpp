// 


#include "GAS/GameplayEffectComponents/SetMovementPaceGameplayEffectComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Components/NpcComponent.h"

bool USetMovementPaceGameplayEffectComponent::OnActiveGameplayEffectAdded(
	FActiveGameplayEffectsContainer& ActiveGEContainer, FActiveGameplayEffect& ActiveGE) const
{
	// We don't allow prediction of expiration (on removed) effects
	if (ActiveGEContainer.IsNetAuthority())
	{
		// When this ActiveGE gets removed, so will our events so no need to unbind this.
		const FGameplayTag& CurrentPace = ActiveGEContainer.Owner->GetOwner()->FindComponentByClass<UNpcComponent>()->GetMovementPaceType();
		ActiveGE.EventSet.OnEffectRemoved.AddUObject(this, &USetMovementPaceGameplayEffectComponent::OnActiveGameplayEffectRemoved, &ActiveGEContainer, CurrentPace);
	}

	return true;
}

void USetMovementPaceGameplayEffectComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec,
	FPredictionKey& PredictionKey) const
{
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
	auto NpcComponent = ActiveGEContainer.Owner->GetOwner()->FindComponentByClass<UNpcComponent>();
	NpcComponent->SetMovementPaceType(PaceTag);
}

void USetMovementPaceGameplayEffectComponent::OnActiveGameplayEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo, FActiveGameplayEffectsContainer* ActiveGEContainer, FGameplayTag InitialPace) const
{
	auto NpcComponent = ActiveGEContainer->Owner->GetOwner()->FindComponentByClass<UNpcComponent>();
	if (NpcComponent->GetMovementPaceType() == PaceTag) // tbh this doesn't 100% guarantee correct behavior, if there's another GE that also has SetMovementPaceGEC with the same tag
		NpcComponent->SetMovementPaceType(InitialPace);
}
