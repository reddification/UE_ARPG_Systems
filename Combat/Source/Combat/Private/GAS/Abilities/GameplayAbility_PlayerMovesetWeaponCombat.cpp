// 


#include "GAS/Abilities/GameplayAbility_PlayerMovesetWeaponCombat.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeleeCombatComponent.h"
#include "GAS/Data/GameplayAbilityTargetData_Attack.h"
#include "Helpers/GASHelpers.h"

void UGameplayAbility_PlayerMovesetWeaponCombat::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                                 const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FGameplayAbilityTargetData_Attack* ActivationData = GetActivationData<FGameplayAbilityTargetData_Attack>(TriggerEventData->TargetData);
	if (!ensure(ActivationData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	WaitForReactivateAttackRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Reactivate);
	WaitForReactivateAttackRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackReactivateRequested);
	WaitForReactivateAttackRequest->ReadyForActivation();

	WaitForFeintRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Feint);
	WaitForFeintRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackFeintRequested);
	WaitForFeintRequest->ReadyForActivation();
	
	GetMeleeCombatComponent()->RequestAttack(ActivationData->AttackType);
}

void UGameplayAbility_PlayerMovesetWeaponCombat::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// WaitForReactivateAttackRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Reactivate);
	// WaitForReactivateAttackRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackReactivateRequested);
	//
	// WaitForFeintRequest = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Feint);
	// WaitForFeintRequest->EventReceived.AddDynamic(this, &UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackFeintRequested);
}

void UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackReactivateRequested(FGameplayEventData Payload)
{
	const FGameplayAbilityTargetData_Attack* ActivationData = GetActivationData<FGameplayAbilityTargetData_Attack>(Payload.TargetData);
	if (!ensure(ActivationData))
		return;

	FGameplayTagContainer OptionalTags;
	if (CheckCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), &OptionalTags))
		GetMeleeCombatComponent()->RequestAttack(ActivationData->AttackType);
}

void UGameplayAbility_PlayerMovesetWeaponCombat::OnAttackFeintRequested(FGameplayEventData Payload)
{
	GetMeleeCombatComponent()->Feint();
}
