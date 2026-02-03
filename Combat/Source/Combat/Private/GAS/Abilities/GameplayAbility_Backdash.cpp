// 


#include "GAS/Abilities/GameplayAbility_Backdash.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/CombatantInterfaceComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_Backdash::UGameplayAbility_Backdash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Backdash_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Backdash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAnimMontage* BackstepMontage = nullptr;
	FGameplayTagContainer OwnerTags;
	auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	for (const auto& BackstepMontageOption : MontageOptions)
	{
		if (BackstepMontageOption.ContextTags.Matches(OwnerTags))
		{
			if (!BackstepMontageOption.MontagesOptions.IsEmpty())
			{
				const int Index = FMath::RandRange(0, BackstepMontageOption.MontagesOptions.Num() - 1);
				BackstepMontage = BackstepMontageOption.MontagesOptions[Index].AnimMontage.LoadSynchronous();
			}
			else if (!BackstepMontageOption.Montages_Deprecated.IsEmpty())
			{
				const int Index = FMath::RandRange(0, BackstepMontageOption.Montages_Deprecated.Num() - 1);
				BackstepMontage = BackstepMontageOption.Montages_Deprecated[Index];
			}
				
			break;				
		}
	}

	if (BackstepMontage)
	{
		UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_Backdash Decided to backdash"));
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Backdash"), BackstepMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Backdash::OnBackdashMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Backdash::OnBackdashMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Backdash::OnBackdashMontageCancelled);
		MontageTask->ReadyForActivation();

		if (auto CombatantInterface = ActorInfo->AvatarActor.Get()->FindComponentByClass<UCombatantInterfaceComponent>())
			CombatantInterface->OnBackdashStarted();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGameplayAbility_Backdash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	MontageTask = nullptr;
	if (auto CombatantInterface = ActorInfo->AvatarActor.Get()->FindComponentByClass<UCombatantInterfaceComponent>())
		CombatantInterface->OnBackdashFinished(!bWasCancelled);
}


void UGameplayAbility_Backdash::OnBackdashMontageEnded()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_Backdash::OnBackstepMontageEnded"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Backdash::OnBackdashMontageInterrupted()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_Backdash::OnBackstepMontageInterrupted"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Backdash::OnBackdashMontageCancelled()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_Backdash::OnBackstepMontageCancelled"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
