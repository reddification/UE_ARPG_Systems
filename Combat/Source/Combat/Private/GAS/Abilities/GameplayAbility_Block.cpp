// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_Block.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeleeBlockComponent.h"
#include "Components/MeleeCombatComponent.h"
#include "Data/CombatGameplayTags.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/PlayerCombat.h"

UGameplayAbility_Block::UGameplayAbility_Block()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Block_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bool bBlockingStarted = StartBlocking(ActorInfo, TriggerEventData);
	if (!ensure(bBlockingStarted))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if (bApplyPenaltyOnActivation)
		ApplyBlockActivationPenalty(ActorInfo->AbilitySystemComponent.Get());
	
	if (ActiveBlockEffect)
	{
		auto OwnerCombatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
		auto EffectContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		auto EffectSpec = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(ActiveBlockEffect, OwnerCombatant->GetActiveWeaponMasteryLevel(), EffectContext);
		ActiveEffectSpecHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
	}

	WaitAbortGameplayEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Block_Event_Stop);
	WaitAbortGameplayEvent->EventReceived.AddDynamic(this, &UGameplayAbility_Block::OnAbort);
	WaitAbortGameplayEvent->ReadyForActivation();
}

void UGameplayAbility_Block::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo == nullptr || !ActorInfo->AvatarActor.IsValid())
	{
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}
	
	if (ActiveEffectSpecHandle.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectSpecHandle);
		ActiveEffectSpecHandle.Invalidate();
	}

	if (WaitAbortGameplayEvent)
	{
		WaitAbortGameplayEvent->EventReceived.RemoveAll(this);
		WaitAbortGameplayEvent->ExternalCancel();
		WaitAbortGameplayEvent = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	auto MeleeBlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UMeleeBlockComponent>();
	if (ensure(MeleeBlockComponent))
		MeleeBlockComponent->StopBlocking();
}

bool UGameplayAbility_Block::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bCanActivateBase = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (!bCanActivateBase)
		return false;

	if (auto CombatComponentLocal = ActorInfo->AvatarActor.Get()->FindComponentByClass<UMeleeCombatComponent>())
	{
		bool bCanBlock = CombatComponentLocal->CanChangeAttackToBlock();
		if (!bCanBlock && OptionalRelevantTags)
			OptionalRelevantTags->AddTagFast(CombatGameplayTags::Combat_Ability_Block_ActivationFailed_AttackDoesntAllow);
		
		if (bCanBlock && CombatComponentLocal->GetActiveAttackPhase() != EMeleeAttackPhase::None)
			bApplyPenaltyOnActivation = true;
		
		return bCanBlock;
	}
	
	return true;
}

bool UGameplayAbility_Block::StartBlocking(const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	auto MeleeBlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UMeleeBlockComponent>();
	if (!ensure(MeleeBlockComponent))
		return false;
	
	MeleeBlockComponent->StartBlocking();
	return true;
}

void UGameplayAbility_Block::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	auto OwnerBlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UMeleeBlockComponent>();
	OwnerBlockComponent->OnAttackParriedEvent.AddUObject(this, &UGameplayAbility_Block::OnAttackParried);
	OwnerBlockComponent->OnAttackBlockedEvent.AddUObject(this, &UGameplayAbility_Block::OnAttackBlocked);
}

void UGameplayAbility_Block::OnAttackParried(AActor* Attacker)
{
	if (AttackParriedEffect)
	{
		auto Combatant = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get());
		auto ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
		auto EffectContext = ASC->MakeEffectContext();
		auto EffectSpec = ASC->MakeOutgoingSpec(AttackParriedEffect, Combatant->GetActiveWeaponMasteryLevel(), EffectContext);
		ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
	}
}

void UGameplayAbility_Block::ApplyInstantEffect(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	auto ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
	auto Combatant = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get());
	auto EffectContext = ASC->MakeEffectContext();
	auto EffectSpec = ASC->MakeOutgoingSpec(EffectClass, Combatant->GetActiveWeaponMasteryLevel(), EffectContext);
	ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
}

void UGameplayAbility_Block::ApplyBlockActivationPenalty(UAbilitySystemComponent* ASC)
{
	bApplyPenaltyOnActivation = false;
	if (!IsValid(StartedBlockingDuringAttackPenaltyEffect))
		return;
	
	auto Combatant = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get());
	auto EffectContext = ASC->MakeEffectContext();
	auto EffectSpec = ASC->MakeOutgoingSpec(StartedBlockingDuringAttackPenaltyEffect, Combatant->GetActiveWeaponMasteryLevel(), EffectContext);
	ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
}

void UGameplayAbility_Block::OnAttackBlocked(float ConsumptionScale, AActor* Attacker)
{
	if (AttackBlockedEffect)
	{
		auto ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
		auto EffectContext = ASC->MakeEffectContext();
		auto EffectSpec = ASC->MakeOutgoingSpec(AttackBlockedEffect, 1.f, EffectContext);
		EffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Block_Consumption_Stamina, -BaseBlockStaminaConsumption * ConsumptionScale);
		EffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Block_Consumption_Poise, -BaseBlockPoiseConsumption * ConsumptionScale);
		ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
	}
	
	// ApplyInstantEffect(AttackBlockedEffect);

	auto AbilityOwner = GetCurrentActorInfo()->AvatarActor.Get();
	auto CombatantOwner = Cast<ICombatant>(AbilityOwner);
	const float CurrentPoise = CombatantOwner->GetPoise();
	if (CurrentPoise <= 0.f)
	{
		// guard break

		FGameplayEventData OwnerPayload;
		FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
		OwnerData->HitDirectionTag = CombatGameplayTags::Combat_HitDirection_Front;
		OwnerPayload.TargetData.Add(OwnerData);

		FGameplayTag HitReactAbilityTag = CombatGameplayTags::Combat_Ability_ReceiveGuardBreak_Event_Activate;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AbilityOwner, HitReactAbilityTag, OwnerPayload);
	}
	else
	{
		auto ActorInfo = GetCurrentActorInfo();
		FGameplayTagContainer OwnerTags;
		auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(ActorInfo->AvatarActor.Get());
		OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
		UAnimMontage* HitReactMontage = nullptr; 
		for (const auto& HitReactMontageOption : HitReacts)
		{
			if (HitReactMontageOption.ContextTags.IsEmpty() || HitReactMontageOption.ContextTags.Matches(OwnerTags))
			{
				if (!HitReactMontageOption.MontagesOptions.IsEmpty())
					HitReactMontage = HitReactMontageOption.MontagesOptions[FMath::RandRange(0, HitReactMontageOption.MontagesOptions.Num() - 1)].AnimMontage.LoadSynchronous();
				else
					HitReactMontage = HitReactMontageOption.Montages_Deprecated[FMath::RandRange(0, HitReactMontageOption.Montages_Deprecated.Num() - 1)];
				
				break;				
			}
		}
	
		if (ensure(HitReactMontage))
		{
			auto AnimInstance = ActorInfo->AnimInstance.IsValid() ? ActorInfo->AnimInstance.Get() : ActorInfo->GetAnimInstance();
			AnimInstance->Montage_Play(HitReactMontage);
		}
		
		// TODO refactor: either separate GameplayAbility_Block_Player or call GA_HitReact with "is from blocking" parameter
		if (auto PlayerCombatant = Cast<IPlayerCombatant>(AbilityOwner))
			PlayerCombatant->PlayCameraShake_Combat(CombatGameplayTags::Combat_Ability_Block);
	}
}

void UGameplayAbility_Block::OnAbort(FGameplayEventData Payload)
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
