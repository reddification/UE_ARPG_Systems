// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_Death.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_Death::UGameplayAbility_Death()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Death_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	if (!ensure(Combatant))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	if (!DeathSkeletonOverride.IsEmpty())
	{
		auto SkMesh = Combatant->GetCombatantMeshComponent();
		if (!IsValid(SkMesh))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
			return;
		}
		TArray<TSoftObjectPtr<USkeletalMesh>> Skeletals;
		DeathSkeletonOverride.GetKeys(Skeletals);
		auto NewSkeletonSoft = Skeletals[FMath::RandRange(0, Skeletals.Num() - 1)];
		// @todo maybe do this in vitals component or etc
		if (!NewSkeletonSoft.IsValid())
		{
			NewSkeletonSoft.LoadSynchronous();
		}
		if (NewSkeletonSoft.IsValid() && SkMesh->GetSkeletalMeshAsset() != NewSkeletonSoft.Get())
		{
			SkMesh->SetSkinnedAssetAndUpdate(NewSkeletonSoft.Get(), false);
		}
		SkMesh->SetAnimationMode(EAnimationMode::Type::AnimationSingleNode);
		auto AnimToPlay = DeathSkeletonOverride[NewSkeletonSoft];
		if (IsValid(AnimToPlay))
		{
			SkMesh->PlayAnimation(AnimToPlay, false);
		}
		Combatant->StartDeath();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
	else
	{
		FGameplayTagContainer OwnerTags;
		auto GameplayTagsActor = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
		GameplayTagsActor->GetOwnedGameplayTags(OwnerTags);
		UAnimMontage* CurrentDeathMontage = nullptr;
		FReceivedHitData LastHit = Combatant->GetLastHitData();
		OwnerTags.AddTagFast(LastHit.HitDirectionTag);
		for (const auto& MontageOption : DeathMontages)
		{
			if (MontageOption.ContextTags.Matches(OwnerTags))
			{
				CurrentDeathMontage = MontageOption.Montages[FMath::RandRange(0, MontageOption.Montages.Num() - 1)];
				break;
			}
		}

		if (!CurrentDeathMontage)
			CurrentDeathMontage = DefaultDeathMontage;
	
		if (CurrentDeathMontage) 
		{
			MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, "PlayDeathMontage", CurrentDeathMontage);
			MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Death::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Death::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Death::OnMontageInterrupted);
			MontageTask->ReadyForActivation();
		}

		Combatant->StartDeath();
		Combatant->PlayCombatSound(CombatGameplayTags::Combat_FX_Sound_Dying);
	
		if (CurrentDeathMontage == nullptr)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}
}

void UGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->FinishDeath();
}

void UGameplayAbility_Death::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UGameplayAbility_Death::OnMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void UGameplayAbility_Death::OnMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}
