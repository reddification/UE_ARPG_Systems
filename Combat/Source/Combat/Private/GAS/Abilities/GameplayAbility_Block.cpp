// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_Block.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeleeBlockComponent.h"
#include "Data/CombatGameplayTags.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Interfaces/ICombatant.h"

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
	
	auto MeleeBlockComponent = ActorInfo->AvatarActor->FindComponentByClass<UMeleeBlockComponent>();
	if (ensure(MeleeBlockComponent))
		MeleeBlockComponent->StopBlocking();

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
	OwnerBlockComponent->OnAttackParriedEvent.BindUObject(this, &UGameplayAbility_Block::OnAttackParried);
	OwnerBlockComponent->OnAttackBlockedEvent.BindUObject(this, &UGameplayAbility_Block::OnAttackBlocked);
}

void UGameplayAbility_Block::OnAttackParried(UActorComponent* ActorComponent, const FHitResult& HitResult,
	const FVector& Vector)
{
	if (AttackParriedEffect)
	{
		auto Combatant = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get());
		auto ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
		auto EffectContext = ASC->MakeEffectContext();
		auto EffectSpec = ASC->MakeOutgoingSpec(AttackParriedEffect, Combatant->GetActiveWeaponMasteryLevel(), EffectContext);
		ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
	}
	
	auto EnemyActor = ActorComponent->GetOwner();
	FGameplayEventData OwnerPayload;
	FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
	OwnerData->HitDirectionTag = CombatGameplayTags::Combat_HitDirection_Front;
	OwnerData->HitLocation = HitResult.ImpactPoint;
	OwnerData->HealthDamage = 0;
	OwnerData->PoiseDamage = 0;
	OwnerData->HitResult = HitResult;
	OwnerPayload.TargetData.Add(OwnerData);

	FGameplayTag HitReactAbilityTag = CombatGameplayTags::Combat_Ability_Stagger_Event_Activate;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EnemyActor, HitReactAbilityTag, OwnerPayload);
}

void UGameplayAbility_Block::ApplyInstantEffect(const TSubclassOf<UGameplayEffect>& EffectClass)
{
	auto ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get();
	auto Combatant = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get());
	auto EffectContext = ASC->MakeEffectContext();
	auto EffectSpec = ASC->MakeOutgoingSpec(EffectClass, Combatant->GetActiveWeaponMasteryLevel(), EffectContext);
	ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
}

void UGameplayAbility_Block::OnAttackBlocked(float ConsumptionScale)
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
	auto Combatant = Cast<ICombatant>(AbilityOwner);
	const float CurrentPoise = Combatant->GetPoise();
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
				HitReactMontage = HitReactMontageOption.Montages[FMath::RandRange(0, HitReactMontageOption.Montages.Num() - 1)];
				break;				
			}
		}
	
		if (ensure(HitReactMontage))
		{
			auto AnimInstance = ActorInfo->AnimInstance.IsValid() ? ActorInfo->AnimInstance.Get() : ActorInfo->GetAnimInstance();
			AnimInstance->Montage_Play(HitReactMontage);
		}
	}
}

void UGameplayAbility_Block::OnAbort(FGameplayEventData Payload)
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
