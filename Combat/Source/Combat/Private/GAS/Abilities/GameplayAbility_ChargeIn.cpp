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
	auto CombatantOwner = Cast<ICombatant>(Character);
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	UAnimMontage* ChargeInMontage = nullptr; 
	for (const auto& ChargeInMontageOption : ChargeInMontageOptions)
	{
		if (ChargeInMontageOption.ContextTags.IsEmpty() || ChargeInMontageOption.ContextTags.Matches(OwnerTags))
		{
			if (!ChargeInMontageOption.MontagesOptions.IsEmpty())
				ChargeInMontage = ChargeInMontageOption.MontagesOptions[FMath::RandRange(0, ChargeInMontageOption.MontagesOptions.Num() - 1)].AnimMontage.LoadSynchronous();
			else if (!ChargeInMontageOption.Montages_Deprecated.IsEmpty())
				ChargeInMontage = ChargeInMontageOption.Montages_Deprecated[FMath::RandRange(0, ChargeInMontageOption.Montages_Deprecated.Num() - 1)];
			
			break;				
		}
	}
	
	if (ensure(ChargeInMontage))
	{
		float TranslationScale = 1.f;
		float Dexterity = 0.f;
		float MontageSpeedScale = 1.f;
		float DistanceToTarget = (Character->GetActorLocation() - ActivationData->ToLocation).Size();
		if (CombatantOwner)
			Dexterity = CombatantOwner->GetDexterity();
		
		if (auto RootMotionDistanceScaleDependency = RootMotionTranslationScaleDistanceDependency.GetRichCurveConst())
			if (RootMotionDistanceScaleDependency->GetNumKeys() > 0)
				TranslationScale = RootMotionDistanceScaleDependency->Eval(DistanceToTarget);

		// if (Dexterity > 0.f)
		// 	if (auto TranslationScaleFromDexterityDependency = RootMotionTranslationScaleDexterityDependency.GetRichCurveConst())
		// 		if (TranslationScaleFromDexterityDependency->GetNumKeys() > 0)
		// 			TranslationScale *= TranslationScaleFromDexterityDependency->Eval(Dexterity);
		
		if (Dexterity > 0)
			if (auto SpeedScaleDependency = MontageSpeedFromDexterityDependency.GetRichCurveConst())
				if (SpeedScaleDependency->GetNumKeys() > 0)
					MontageSpeedScale = SpeedScaleDependency->Eval(Dexterity);
		
		ChargeInMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("ChargeIn"),
			ChargeInMontage, MontageSpeedScale, NAME_None, true, TranslationScale);
		ChargeInMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageCompleted);
		ChargeInMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageInterrupted);
		ChargeInMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ChargeIn::OnChargeInMontageCancelled);
		ChargeInMontageTask->ReadyForActivation();
		
		bool bPushCapsule = ActivationData->ToLocation != FVector::ZeroVector
			&& (ActivationData->ForwardImpulse > 0.f || ActivationData->VerticalImpulse > 0.f);
		bPushCapsule = bPushCapsule && !ChargeInMontage->HasRootMotion();
		
		if (bPushCapsule)
		{
			if (auto CMC = Character->FindComponentByClass<UCharacterMovementComponent>())
			{
				// TODO 30.01.2026 apply translation scale?
				FVector Direction = (ActivationData->ToLocation - Character->GetActorLocation()).GetSafeNormal();
				FVector ImpulseVector = FVector(Direction.X * ActivationData->ForwardImpulse,
					Direction.Y * ActivationData->ForwardImpulse, ActivationData->VerticalImpulse);
				CMC->AddImpulse(ImpulseVector, true);
			}
		}
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
	if (ActiveChargeEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveChargeEffectHandle);
		ActiveChargeEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get()))
		Combatant->ChargeInFinished();
}

void UGameplayAbility_ChargeIn::OnChargeInMontageCompleted()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_ChargeIn::OnChargeInMontageCompleted"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_ChargeIn::OnChargeInMontageInterrupted()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_ChargeIn::OnChargeInMontageInterrupted"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_ChargeIn::OnChargeInMontageCancelled()
{
	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_ChargeIn::OnChargeInMontageCompleted"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}