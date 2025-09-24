// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_Clash.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/CombatGameplayTags.h"
#include "GAS/Data/GameplayAbilityTargetData_Clash.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_Clash::UGameplayAbility_Clash()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Clash_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}

	// AbortTag = CombatGameplayTags::Combat_Ability_Clash_Event_Abort;
}

void UGameplayAbility_Clash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FGameplayAbilityTargetData_Clash* ActivationData = GetActivationData<FGameplayAbilityTargetData_Clash>(TriggerEventData->TargetData);
	if (!ensure(ActivationData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	FGameplayTagContainer OwnerTags;
	auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	if (ActivationData->ClashSource == EClashSource::Parry)
		OwnerTags.AddTagFast(CombatGameplayTags::Combat_Ability_Clash_Cause_Parried);

	// OwnerTags.AddTagFast(ActivationData->HitDirectionTag);
	UAnimMontage* HitReactMontage = nullptr; 
	for (const auto& HitReactMontageOption : HitReacts)
	{
		if (HitReactMontageOption.ContextTags.Matches(OwnerTags))
		{
			HitReactMontage = HitReactMontageOption.Montages_Deprecated[FMath::RandRange(0, HitReactMontageOption.Montages_Deprecated.Num() - 1)];
			break;				
		}
	}
	
	if (ensure(HitReactMontage))
	{
		HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Clash"), HitReactMontage);
		HitReactMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Clash::OnHitReactMontageCompleted);
		HitReactMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Clash::OnHitReactMontageInterrupted);
		HitReactMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Clash::OnHitReactMontageCancelled);
		
		HitReactMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		return;
	}
	
	if (auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get()))
	{
		Combatant->StartStagger();
	}
}

void UGameplayAbility_Clash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (HitReactMontageTask)
	{
		// HitReactMontageTask->ExternalCancel();
		HitReactMontageTask = nullptr;
	}
	
	if (auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get()))
	{
		Combatant->FinishStagger();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Clash::OnHitReactMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Clash::OnHitReactMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Clash::OnHitReactMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
