// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_PlayerManualControlMeleeWeaponCombat.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeleeCombatComponent.h"
#include "Components/PlayerSwingControlCombatComponent.h"
#include "Data/CombatGameplayTags.h"

void UGameplayAbility_PlayerManualControlMeleeWeaponCombat::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	GetMeleeCombatComponent()->RequestAttack(EMeleeAttackType::LightAttack);
	
	WaitForAttackReleaseRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Release);
	WaitForAttackReleaseRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackReleaseRequested);
	WaitForAttackReleaseRequest->ReadyForActivation();

	WaitForReactivateAttackRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Reactivate);
	WaitForReactivateAttackRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackReactivateRequested);
	WaitForReactivateAttackRequest->ReadyForActivation();

	WaitForFeintRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Feint);
	WaitForFeintRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackFeintRequested);
	WaitForFeintRequest->ReadyForActivation();
}

void UGameplayAbility_PlayerManualControlMeleeWeaponCombat::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetEventTask(WaitForAttackReleaseRequest);
	ResetEventTask(WaitForReactivateAttackRequest);
	ResetEventTask(WaitForFeintRequest);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackReleaseRequested(FGameplayEventData Payload)
{
	auto PlayerMeleeCombatComponent = Cast<UPlayerSwingControlCombatComponent>(GetMeleeCombatComponent());
	PlayerMeleeCombatComponent->RequestReleaseAttack();
}

void UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackFeintRequested(FGameplayEventData Payload)
{
	GetMeleeCombatComponent()->Feint();	
}

void UGameplayAbility_PlayerManualControlMeleeWeaponCombat::OnAttackReactivateRequested(FGameplayEventData Payload)
{
	GetMeleeCombatComponent()->RequestReactivateAttack();
}
