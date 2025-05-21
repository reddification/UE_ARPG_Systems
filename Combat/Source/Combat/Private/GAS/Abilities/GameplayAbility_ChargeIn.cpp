// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_ChargeIn.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Data/GameplayAbilityTargetData_ChargeIn.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_ChargeIn::UGameplayAbility_ChargeIn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_ChargeIn_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}

	AbortTag = CombatGameplayTags::Combat_Ability_ChargeIn_Event_Abort;
}

void UGameplayAbility_ChargeIn::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const FGameplayAbilityTargetData_ChargeIn* ActivationData = GetActivationData<FGameplayAbilityTargetData_ChargeIn>(TriggerEventData->TargetData);
	if (!ensure(ActivationData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	FGameplayTagContainer OwnerTags;
	auto Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(Character);
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	UAnimMontage* HitReactMontage = nullptr; 
	for (const auto& ChargeInMontageOption : ChargeInMontageOptions)
	{
		if (ChargeInMontageOption.ContextTags.Matches(OwnerTags))
		{
			HitReactMontage = ChargeInMontageOption.Montages[FMath::RandRange(0, ChargeInMontageOption.Montages.Num() - 1)];
			break;				
		}
	}
	
	if (ensure(HitReactMontage))
	{
		ChargeInMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("ChargeIn"), HitReactMontage);
		ChargeInMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageCompleted);
		ChargeInMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageInterrupted);
		ChargeInMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageCancelled);
		ChargeInMontageTask->ReadyForActivation();
	}
	else
	{
		UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Warning, TEXT("ChargeIn montage not found, Immediately ending ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	// Launching/adding impulse is useless here, because the montage has root motion.
	// Ideally, the montage should get rid of root motion and launch the character at anim notify
	// Or perhaps add 2 impulses: 1 at start, 1 at some animation moment when the character is making a leap
	// FVector Impulse(ActivationData->Direction.X * ActivationData->ForwardImpulse, ActivationData->Direction.Y * ActivationData->ForwardImpulse, ActivationData->VerticalImpulse);
	// Character->LaunchCharacter(Impulse, true, true);
	auto Combatant = Cast<ICombatant>(Character);
	Combatant->ChargeInStarted();
	Combatant->PlayCombatSound(CombatGameplayTags::Combat_FX_Sound_Grunt);
	
	if (ensure(ChargeInEffectClass))
	{
		auto OwnerASC = ActorInfo->AbilitySystemComponent.Get();
		auto StaggerEffectContext = OwnerASC->MakeEffectContext();
		auto StaggerEffectSpec = OwnerASC->MakeOutgoingSpec(ChargeInEffectClass, 1.f, StaggerEffectContext);
		ActiveChargeEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, StaggerEffectSpec);
	}
}

void UGameplayAbility_ChargeIn::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	Combatant->ChargeInFinished();
	
	if (ActiveChargeEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveChargeEffectHandle);
		ActiveChargeEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_ChargeIn::OnChargeInMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_ChargeIn::OnChargeInMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_ChargeIn::OnChargeInMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}