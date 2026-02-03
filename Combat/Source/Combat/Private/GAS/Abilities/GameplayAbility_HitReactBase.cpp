// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_HitReactBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatLogChannels.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/CombatAliveCreature.h"
#include "Interfaces/ICombatant.h"

void UGameplayAbility_HitReactBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const FGameplayAbilityTargetData_ReceivedHit* ActivationData = GetActivationData<FGameplayAbilityTargetData_ReceivedHit>(TriggerEventData->TargetData);
	if (!ensure(ActivationData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	auto AliveCreature = Cast<ICombatAliveCreature>(ActorInfo->AvatarActor.Get());
	if(AliveCreature->GetCombatantHealth() <= 0.f )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	auto CombatantOwner = Cast<ICombatant>(ActorInfo->AvatarActor);

	FGameplayTagContainer OwnerTags;
	auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	OwnerTags.AddTagFast(ActivationData->HitDirectionTag);
	UAnimMontage* HitReactMontage = nullptr; 
	for (int i = 0; i < HitReacts.Num(); i++)
	{
		const auto& HitReactMontageOption = HitReacts[i];
		if (HitReactMontageOption.ContextTags.Matches(OwnerTags) || HitReactMontageOption.ContextTags.IsEmpty() && i == HitReacts.Num() - 1)
		{
			if (!HitReactMontageOption.MontagesOptions.IsEmpty())
				HitReactMontage = HitReactMontageOption.MontagesOptions[FMath::RandRange(0, HitReactMontageOption.MontagesOptions.Num() - 1)].AnimMontage.LoadSynchronous();
			else
				HitReactMontage = HitReactMontageOption.Montages_Deprecated[FMath::RandRange(0, HitReactMontageOption.Montages_Deprecated.Num() - 1)];
				
			break;				
		}
	}
	
	ActiveHitReactTags = OwnerTags;
	if (ensure(HitReactMontage))
	{
		HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("HitReact"), HitReactMontage);
		HitReactMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_HitReactBase::OnHitReactMontageCompleted);
		HitReactMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_HitReactBase::OnHitReactMontageInterrupted);
		HitReactMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_HitReactBase::OnHitReactMontageCancelled);
		HitReactMontageTask->ReadyForActivation();
		UE_VLOG(ActorInfo->OwnerActor.Get(), LogCombat, Verbose, TEXT("%s montage ready for activation"), *GetName());
	}
	else
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		return;
	}

	FReceivedHitData ReceivedHitData;
	ReceivedHitData.HitDirectionTag = ActivationData->HitDirectionTag;
	ReceivedHitData.HealthDamage = ActivationData->HealthDamage;
	ReceivedHitData.PoiseDamage = ActivationData->PoiseDamage;
	ReceivedHitData.HitResult = ActivationData->HitResult;
	ReceivedHitData.HitTypeTag = HitTypeTag;
	ReceivedHitData.Causer = ActivationData->Causer.Get();
	ReceivedHitData.CauserId = ActivationData->CauserId;
	CombatantOwner->TakeHit(ReceivedHitData);
}

void UGameplayAbility_HitReactBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_VLOG(GetCurrentActorInfo()->OwnerActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_HitReactBase::EndAbility %s completed"), *GetName());
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_HitReactBase::OnHitReactMontageCompleted()
{
	UE_VLOG(GetCurrentActorInfo()->OwnerActor.Get(), LogCombat, Verbose, TEXT("%s montage completed"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_HitReactBase::OnHitReactMontageInterrupted()
{
	UE_VLOG(GetCurrentActorInfo()->OwnerActor.Get(), LogCombat, Verbose, TEXT("%s montage interrupted"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_HitReactBase::OnHitReactMontageCancelled()
{
	UE_VLOG(GetCurrentActorInfo()->OwnerActor.Get(), LogCombat, Verbose, TEXT("%s montage canceled"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
