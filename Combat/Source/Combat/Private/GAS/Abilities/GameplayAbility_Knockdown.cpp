// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/GameplayAbility_Knockdown.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_Knockdown::UGameplayAbility_Knockdown()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Knockdown_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Knockdown::TimerFinish()
{
	if (WakeUpMontageMontage) 
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "PlayDeathMontage", WakeUpMontageMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Knockdown::OnWakeUpMontageCompleted);
		MontageTask->ReadyForActivation();
	}
}

void UGameplayAbility_Knockdown::OnWakeUpMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UGameplayAbility_Knockdown::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                 const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->StartKnockdown();
	
	auto AnimInstance = ActorInfo->AnimInstance.IsValid() ? ActorInfo->AnimInstance.Get() : ActorInfo->GetAnimInstance();
	AnimInstance->StopAllMontages(0);
	AnimInstance->Montage_Play(KnockdownMontage);
	
	WaitTask = UAbilityTask_WaitDelay::WaitDelay(this,KnockdownRecoverTime);
	WaitTask->OnFinish.AddDynamic(this, &ThisClass::TimerFinish);
	WaitTask->ReadyForActivation();
}

void UGameplayAbility_Knockdown::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo))
		return;

	if (!ActorInfo->AvatarActor.IsValid() || !IsValid(ActorInfo->AvatarActor.Get()))
		return;
	
	auto AnimInstance = ActorInfo->AnimInstance.IsValid() ? ActorInfo->AnimInstance.Get() : ActorInfo->GetAnimInstance();
	AnimInstance->StopAllMontages(0);
	
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->FinishKnockdown();
	
	for (TSubclassOf<UGameplayEffect> RecoverEffect : RecoverEffects)
	{
		if(RecoverEffect != nullptr && (RecoverEffect.GetDefaultObject()->DurationPolicy == EGameplayEffectDurationType::Instant || RecoverEffect.GetDefaultObject()->DurationPolicy == EGameplayEffectDurationType::HasDuration))
		{
			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectToSelf(RecoverEffect.GetDefaultObject(), 1.0f, GetAbilitySystemComponentFromActorInfo()->MakeEffectContext());
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}