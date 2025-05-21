// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "GameFramework/Character.h"
#include "GAS/Data/GameplayAbilityTargetData_Dodge.h"
#include "Helpers/GASHelpers.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_Dodge::UGameplayAbility_Dodge()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Dodge_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const FGameplayAbilityTargetData_Dodge* ActivationData = GetActivationData<FGameplayAbilityTargetData_Dodge>(TriggerEventData->TargetData);
	if (!ensure(ActivationData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	auto OwnerActor = Cast<AActor>(ActorInfo->AvatarActor.Get());
	FGameplayTag RelativeDirectionTag = GetActorRelativeDirectionTag(OwnerActor, (ActivationData->DodgeLocation - OwnerActor->GetActorLocation()).GetSafeNormal(),
		DirectionDotProductThreshold);
	FGameplayTagContainer OwnerTags;
	auto OwnerTagInterface = Cast<IGameplayTagAssetInterface>(OwnerActor);
	OwnerTagInterface->GetOwnedGameplayTags(OwnerTags);
	OwnerTags.AddTag(RelativeDirectionTag);
	UAnimMontage* DodgeMontage = nullptr; 
	for (const auto& DodgeMontageOption : DodgeMontageOptions)
	{
		if (DodgeMontageOption.ContextTags.Matches(OwnerTags))
		{
			DodgeMontage = DodgeMontageOption.Montages[FMath::RandRange(0, DodgeMontageOption.Montages.Num() - 1)];
			break;				
		}
	}
	
	if (ensure(DodgeMontage))
	{
		UE_VLOG(OwnerActor, LogCombat, Verbose, TEXT("Preparing dodge montage task"));
		const float Dexterity = Cast<ICombatant>(OwnerActor)->GetDexterity();
		float AnimationSpeed = 1.f;
		float RootMotionTranslationScale = 1.f;

		if (auto SpeedDependency = AnimationSpeedDexterityDependence.GetRichCurveConst())
			if (SpeedDependency->GetNumKeys() > 0)
				AnimationSpeed = SpeedDependency->Eval(Dexterity);

		if (auto RootMotionDistanceScaleDependency = RootMotionTranslationScaleDexterityDependence.GetRichCurveConst())
			if (RootMotionDistanceScaleDependency->GetNumKeys() > 0)
				RootMotionTranslationScale = RootMotionDistanceScaleDependency->Eval(Dexterity);
		
		DodgeMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Dodge"), DodgeMontage,
			AnimationSpeed, NAME_None, true, RootMotionTranslationScale);
		
		DodgeMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Dodge::OnDodgeMontageCompleted);
		DodgeMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Dodge::OnDodgeMontageInterrupted);
		DodgeMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Dodge::OnDodgeMontageCancelled);
		DodgeMontageTask->ReadyForActivation();
	}
	else
	{
		UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Warning, TEXT("Dodge montage not found, Immediately ending ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	auto Combatant = Cast<ICombatant>(OwnerActor);
	FGameplayTagContainer OptionalTags;
	CommitAbility(Handle, ActorInfo, ActivationInfo, &OptionalTags);
	Combatant->DodgeStarted();
	Combatant->PlayCombatSound(CombatGameplayTags::Combat_FX_Sound_Grunt);
	
	if (ensure(DodgeEffectClass))
	{
		auto OwnerASC = ActorInfo->AbilitySystemComponent.Get();
		auto StaggerEffectContext = OwnerASC->MakeEffectContext();
		auto StaggerEffectSpec = OwnerASC->MakeOutgoingSpec(DodgeEffectClass, 1.f, StaggerEffectContext);
		ActiveDodgeEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, StaggerEffectSpec);
	}
}

void UGameplayAbility_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, Verbose, TEXT("UGameplayAbility_Dodge::EndAbility"));

	auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get());
	if (bWasCancelled)
		Combatant->DodgeCanceled();
	else
		Combatant->DodgeFinished();
	
	if (ActiveDodgeEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveDodgeEffectHandle);
		ActiveDodgeEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Dodge::OnDodgeMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Dodge::OnDodgeMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_Dodge::OnDodgeMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
