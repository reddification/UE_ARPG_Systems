// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/GameplayAbility_Stagger.h"

#include "AbilitySystemComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Interfaces/CombatAliveCreature.h"
#include "Interfaces/ICombatant.h"

class UBaseAttributeSet;

UGameplayAbility_Stagger::UGameplayAbility_Stagger()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Stagger_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Stagger::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// TODO stagger animation must be 3 phases (enter stagger, looped 'shaking-wobbling part, recover from stagger') in ABP and total playtime must correlate with the UCombatAttributeSet::StaggerDuration
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	auto OwnerCombatantAliveCreature = Cast<ICombatAliveCreature>(ActorInfo->AvatarActor.Get());
	if(OwnerCombatantAliveCreature->GetCombatantHealth() <= 0.f)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
		return;
	}
	
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->StartStagger();
	Combatant->PlayCombatSound(CombatGameplayTags::Combat_FX_Sound_Staggered);
	auto OwnerASC = ActorInfo->AbilitySystemComponent.Get();
	if (ensure(StaggerRecoverEffectClass))
	{
		auto StaggerEffectContext = OwnerASC->MakeEffectContext();
		auto StaggerEffectSpec = OwnerASC->MakeOutgoingSpec(StaggerRecoverEffectClass, 1.f, StaggerEffectContext);
		ActiveStaggerEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, StaggerEffectSpec);
	}
}

void UGameplayAbility_Stagger::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->FinishStagger();
	
	if (ActiveStaggerEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveStaggerEffectHandle);
		ActiveStaggerEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}