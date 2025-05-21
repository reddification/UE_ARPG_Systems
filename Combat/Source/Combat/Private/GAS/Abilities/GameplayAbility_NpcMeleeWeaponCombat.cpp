// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/GameplayAbility_NpcMeleeWeaponCombat.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeleeCombatComponent.h"
#include "Components/NpcMeleeCombatComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "Interfaces/NpcCombatant.h"

void UGameplayAbility_NpcMeleeWeaponCombat::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// TODO apply effects to reduce reaction while attacking!!!
	bool bAttackRequested = GetMeleeCombatComponent()->RequestAttack(EMeleeAttackType::LightAttack);
	if (!bAttackRequested)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!IsValid(WaitForContinueAttackRequestEvent))
	{
		WaitForContinueAttackRequestEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_RequestNextAttack);
		WaitForContinueAttackRequestEvent->EventReceived.AddDynamic(this, &UGameplayAbility_NpcMeleeWeaponCombat::RequestNextAttack);
		WaitForContinueAttackRequestEvent->ReadyForActivation();
	}
}

void UGameplayAbility_NpcMeleeWeaponCombat::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	if (Combatant)
	{
		if (bWasCancelled)
			Combatant->OnAttackCanceled();
		else
			Combatant->OnAttackEnded();
	}
	
	if (WaitForContinueAttackRequestEvent)
	{
		WaitForContinueAttackRequestEvent->EventReceived.RemoveAll(this);
		WaitForContinueAttackRequestEvent = nullptr;
	}
	
	// SendMessageToAIController(ActorInfo->AvatarActor.Get(), bWasCancelled ? AbilityAbortedAIMessage : AbilityEndedAIMessage, !bWasCancelled);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_NpcMeleeWeaponCombat::OnAttackActivePhaseChanged(EMeleeAttackPhase OldAttackPhase,
	EMeleeAttackPhase NewAttackPhase)
{
	Super::OnAttackActivePhaseChanged(OldAttackPhase, NewAttackPhase);
	if (!IsActive())
	{
		// ensure(NewAttackPhase == EMeleeAttackPhase::None);
		return;
	}
	
	if (NewAttackPhase == EMeleeAttackPhase::Recover)
	{
		auto CombatComponent = GetMeleeCombatComponent();
		INpcCombatant* NpcCombatant = Cast<INpcCombatant>(GetOwningActorFromActorInfo());
		if (CombatComponent->GetEnemiesHitThisAttack() > 0)
			NpcCombatant->OnAttackRecoveryAfterHitTarget();
	}
}

void UGameplayAbility_NpcMeleeWeaponCombat::OnAbilityAborted(FGameplayEventData Payload)
{
	if (!IsActive())
		return;

	Super::OnAbilityAborted(Payload);
}

void UGameplayAbility_NpcMeleeWeaponCombat::RequestNextAttack(FGameplayEventData Payload)
{
	auto AbilityOwner = GetCurrentActorInfo()->AvatarActor.Get();
	if (!IsActive())
	{
		UE_VLOG(AbilityOwner, LogCombat, Warning, TEXT("Requesting next attack on ability that is not active [%s]"), *GetName());
		return;
	}
	
	auto NpcCombatComponent = Cast<UNpcMeleeCombatComponent>(GetMeleeCombatComponent());
	bool bAttackContinued = false;
	FGameplayTagContainer TagContainer;
	if (CheckCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), &TagContainer))
	{
		bAttackContinued = NpcCombatComponent->RequestNextAttack();
	}

	if (!bAttackContinued)
	{
		UE_VLOG(AbilityOwner, LogCombat, Log, TEXT("Ending attack because request next attack failed"));
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}
