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
	
	FGameplayTagContainer OwnerTags;
	auto GameplayTagsActor = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
	GameplayTagsActor->GetOwnedGameplayTags(OwnerTags);
	UAnimMontage* CurrentDeathMontage = nullptr;
	FReceivedHitData LastHit = Combatant->GetLastHitData();
	OwnerTags.AddTagFast(LastHit.HitDirectionTag);

	auto OwnerSkeletalMeshComponent = Combatant->GetCombatantSkeletalMeshComponent();
	
	USkeletalMesh* SkeletalMeshOverride = nullptr;
	for (const auto& MontageOption : DeathMontages)
	{
		if (MontageOption.ContextTags.IsEmpty() || MontageOption.ContextTags.Matches(OwnerTags))
		{
			if (!MontageOption.MontagesOptions.IsEmpty())
			{
				const auto& MontageData = MontageOption.MontagesOptions[FMath::RandRange(0, MontageOption.MontagesOptions.Num() - 1)];
				CurrentDeathMontage = MontageData.AnimMontage.LoadSynchronous();
				if (!MontageData.SkeletalMeshOverride.IsNull())
				{
					SkeletalMeshOverride = MontageData.SkeletalMeshOverride.LoadSynchronous();
					if (MontageData.bRestoreInitialSkeletalMesh)
						InitialSkeletalMesh = OwnerSkeletalMeshComponent->GetSkeletalMeshAsset();
				}
			}
			else if (!MontageOption.Montages_Deprecated.IsEmpty())
			{
				CurrentDeathMontage = MontageOption.Montages_Deprecated[FMath::RandRange(0, MontageOption.Montages_Deprecated.Num() - 1)];
			}
			
			break;
		}
	}

	if (CurrentDeathMontage == nullptr)
	{
		if (!DefaultDeathMontages.IsEmpty())
		{
			const auto& MontageData = DefaultDeathMontages[FMath::RandRange(0, DefaultDeathMontages.Num() - 1)];
			CurrentDeathMontage = MontageData.AnimMontage.LoadSynchronous();
			if (!MontageData.SkeletalMeshOverride.IsNull())
			{
				SkeletalMeshOverride = MontageData.SkeletalMeshOverride.LoadSynchronous();
				if (MontageData.bRestoreInitialSkeletalMesh)
					InitialSkeletalMesh = OwnerSkeletalMeshComponent->GetSkeletalMeshAsset();
			}
		}
		else if (!DefaultDeathMontageOptions_Deprecated.IsEmpty())
		{
			CurrentDeathMontage = DefaultDeathMontageOptions_Deprecated[FMath::RandRange(0, DefaultDeathMontageOptions_Deprecated.Num() - 1)].LoadSynchronous();
		}
	}

	if (SkeletalMeshOverride != nullptr)
		OwnerSkeletalMeshComponent->SetSkinnedAssetAndUpdate(SkeletalMeshOverride, false);
	
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
		Combatant->ActivateCombatantRagdoll();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
}

void UGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->FinishDeath();
	if (!InitialSkeletalMesh.IsNull())
	{
		auto OwnerSkeletalMeshComponent = Combatant->GetCombatantSkeletalMeshComponent();
		OwnerSkeletalMeshComponent->SetSkinnedAssetAndUpdate(InitialSkeletalMesh.LoadSynchronous(), false);
		InitialSkeletalMesh = nullptr;
	}
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
