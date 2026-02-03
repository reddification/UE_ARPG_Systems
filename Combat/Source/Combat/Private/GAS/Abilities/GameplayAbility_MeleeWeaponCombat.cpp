// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/GameplayAbility_MeleeWeaponCombat.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/PlayerSwingControlCombatComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "Engine/DamageEvents.h"
#include "GAS/Data/GameplayAbilityTargetData_Clash.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Interfaces/CombatAliveCreature.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_MeleeWeaponCombat::UGameplayAbility_MeleeWeaponCombat()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		AbilityTriggers.Reset();
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CombatGameplayTags::Combat_Ability_Attack_Event_Activate;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_MeleeWeaponCombat::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	GetMeleeCombatComponent();
}

void UGameplayAbility_MeleeWeaponCombat::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::ActivateAbility"));

	WaitForAbortEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, CombatGameplayTags::Combat_Ability_Attack_Event_Abort);
	WaitForAbortEvent->EventReceived.AddDynamic(this, &UGameplayAbility_MeleeWeaponCombat::OnAbilityAborted);
	WaitForAbortEvent->ReadyForActivation();
	
	SubscribeToLifecycleDelegates();
}

void UGameplayAbility_MeleeWeaponCombat::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    bool bReplicateEndAbility, bool bWasCancelled)
{
	UnsubscribeFromLifecycleDelegates();
	if (auto CombatComponent = GetMeleeCombatComponent())
	{
		if (bWasCancelled)
			CombatComponent->CancelAttack();
		else
			CombatComponent->ResetAttackState();
	}

	if (auto Combatant = Cast<ICombatant>(ActorInfo->AvatarActor.Get()))
	{
		if (bWasCancelled)
			Combatant->OnAttackCanceled();
		else
			Combatant->OnAttackEnded();
	}
	
	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::EndAbility"));

	if (WaitForAbortEvent)
	{
		WaitForAbortEvent->EventReceived.RemoveAll(this);
		WaitForAbortEvent = nullptr;
	}

	if (ActiveAttackEffectHandle.IsValid())
	{
		if (auto ASC = GetAbilitySystemComponentFromActorInfo())
			ASC->RemoveActiveGameplayEffect(ActiveAttackEffectHandle);
		
		ActiveAttackEffectHandle.Invalidate();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_MeleeWeaponCombat::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::CancelAbility"));
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

UMeleeCombatComponent* UGameplayAbility_MeleeWeaponCombat::GetMeleeCombatComponent()
{
	if (!CombatComponentCached.IsValid())
	{
		CombatComponentCached = GetAvatarActorFromActorInfo()->FindComponentByClass<UMeleeCombatComponent>();
		CombatComponentCached->OnAttackFeintedEvent.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnAttackFeinted);
		CombatComponentCached->OnAttackActivePhaseChanged.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnAttackActivePhaseChanged);
		CombatComponentCached->OnWeaponHitEvent.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnWeaponHit);
		CombatComponentCached->OnAttackWhiffedEvent.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnAttackWhiffed);
		CombatComponentCached->OnAttackCommitedEvent.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnAttackCommited);
	}

	return CombatComponentCached.Get();
}

void UGameplayAbility_MeleeWeaponCombat::SubscribeToLifecycleDelegates()
{
	if (CombatComponentCached.IsValid())
		if (ensure(!CombatComponentCached->OnAttackEndedEvent.IsBoundToObject(this)))
			CombatComponentCached->OnAttackEndedEvent.AddUObject(this, &UGameplayAbility_MeleeWeaponCombat::OnAttackEnded);
}

void UGameplayAbility_MeleeWeaponCombat::UnsubscribeFromLifecycleDelegates()
{
	if (CombatComponentCached.IsValid())
		CombatComponentCached->OnAttackEndedEvent.RemoveAll(this);
}

void UGameplayAbility_MeleeWeaponCombat::OnAttackCommited()
{
	if (!IsActive())
		return;
	
	auto SpecHandle = GetCurrentAbilitySpecHandle();
	auto ActorInfo = GetCurrentActorInfo();

	UE_VLOG(ActorInfo->AvatarActor.Get(), LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::OnAttackCommited"));
	
	bool bCommited = CommitAbility(SpecHandle, ActorInfo, CurrentActivationInfo);
	if (!bCommited)
		EndAbility(SpecHandle, ActorInfo, CurrentActivationInfo, true, true);
}

void UGameplayAbility_MeleeWeaponCombat::OnAttackWhiffed()
{
	if (!IsActive())
		return;

	auto AvatarActor = CurrentActorInfo->AvatarActor.Get();
	UE_VLOG(AvatarActor, LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::OnAttackWhiffed"));
	
	auto Combatant = Cast<ICombatant>(AvatarActor);
	Combatant->OnAttackWhiffed();
	
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	if (Debug_IgnoreAttackCost)
		return;
#endif
	
	if (ensure(HitWhiffedEffectClass))
	{
		auto ASC = GetAbilitySystemComponentFromActorInfo();
		auto GEContext = ASC->MakeEffectContext();
		const int WhiffedAttacksCount = CombatComponentCached->GetCurrentComboTotalAttackCount() - CombatComponentCached->GetCurrentComboHitAttackCount();
		auto Spec = ASC->MakeOutgoingSpec(HitWhiffedEffectClass, WhiffedAttacksCount, GEContext);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

void UGameplayAbility_MeleeWeaponCombat::OnAttackEnded()
{
	auto ActorInfo = GetActorInfo();
	EndAbility(GetCurrentAbilitySpecHandle(), &ActorInfo, GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_MeleeWeaponCombat::OnAttackFeinted()
{
	OnAttackEnded();
}

void UGameplayAbility_MeleeWeaponCombat::OnAttackActivePhaseChanged(EMeleeAttackPhase OldAttackPhase, EMeleeAttackPhase NewAttackPhase)
{
	if (!IsActive())
		return;

	UE_VLOG(GetCurrentActorInfo()->AvatarActor.Get(), LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::OnAttackActivePhaseChanged from %d to %d"),
		(uint8)OldAttackPhase, (uint8)NewAttackPhase);
	
	if (!ensure(IsValid(CharacterStateDuringAttackEffectClass)))
		return;
	
	if (OldAttackPhase == EMeleeAttackPhase::None && NewAttackPhase != EMeleeAttackPhase::None)
	{
		auto ASC = GetAbilitySystemComponentFromActorInfo();
		auto GEContext = ASC->MakeEffectContext();
		auto GESpec = ASC->MakeOutgoingSpec(CharacterStateDuringAttackEffectClass, GetMeleeCombatComponent()->GetCurrentComboTotalAttackCount(), GEContext);
		ActiveAttackEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*GESpec.Data);
	}
	else if (ActiveAttackEffectHandle.IsValid() && OldAttackPhase != EMeleeAttackPhase::None && NewAttackPhase == EMeleeAttackPhase::None)
	{
		auto ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(ActiveAttackEffectHandle);
		ActiveAttackEffectHandle.Invalidate();
	}
}

void UGameplayAbility_MeleeWeaponCombat::ResetEventTask(UAbilityTask_WaitGameplayEvent*& AbilityTask)
{
	if (IsValid(AbilityTask))
	{
		AbilityTask->EventReceived.RemoveAll(this);
		AbilityTask->ExternalCancel();
		AbilityTask = nullptr;
	}
}

void UGameplayAbility_MeleeWeaponCombat::OnWeaponHit(UPrimitiveComponent* OtherActorComponent, const FHitResult& HitResult,
                                                     EWeaponHitSituation WeaponHitSituation, const FVector& SweepDirection)
{
	if (!IsActive())
		return;
	
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	auto OwnerActor = GetCurrentActorInfo()->AvatarActor.Get();
	auto EnemyActor = OtherActorComponent->GetOwner();
	auto CombatantOwner = Cast<ICombatant>(OwnerActor);
	auto CombatantEnemy = Cast<ICombatant>(EnemyActor);

	UE_VLOG(OwnerActor, LogCombat, VeryVerbose, TEXT("UGameplayAbility_MeleeWeaponCombat::OnWeaponHit"));

	// if (WeaponHitSituation != EWeaponHitSituation::Body)
	// 	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	
	switch (WeaponHitSituation)
	{
		case EWeaponHitSituation::WeaponClash:
		// TODO handle non-combatant enemies (boxes, doors, etc)
			HandleWeaponsCollide(CombatSettings, OwnerActor, EnemyActor, CombatantOwner, CombatantEnemy, SweepDirection);
		break;
		case EWeaponHitSituation::AttackBlocked:
			HandleAttackBlocked(EnemyActor, SweepDirection, HitResult);
			CombatComponentCached->OnAttackBlocked();
			break;
		case EWeaponHitSituation::AttackParried:
			HandleAttackParried(OwnerActor);
			break;
		case EWeaponHitSituation::Body:
			HandleEnemyHit(CombatSettings, OwnerActor, EnemyActor, CombatantOwner, CombatantEnemy, HitResult, SweepDirection);
			break;
		case EWeaponHitSituation::ImmovableObject:
			TriggerClashAbility(OwnerActor, EClashSource::AttacksCollide, SweepDirection);
			break;
		case EWeaponHitSituation::None:
		default: break;
	}	
}

void UGameplayAbility_MeleeWeaponCombat::HandleWeaponsCollide(const UMeleeCombatSettings* CombatSettings, AActor* OwnerActor, AActor* EnemyActor,
                                                              ICombatant* CombatantOwner, ICombatant* CombatantEnemy, const FVector& SweepDirection)
{
	auto EnemyCombatComponent = EnemyActor->FindComponentByClass<UMeleeCombatComponent>();

	// TODO consider attack directions. Oberhau should be the most 'outstriking' attack compared to others
	auto EnemyActiveAttack = EnemyCombatComponent->GetActiveAttackType();
	auto OwnerActiveAttack = GetMeleeCombatComponent()->GetActiveAttackType();
	
	bool bOwnerOberhau = OwnerActiveAttack == EMeleeAttackType::LeftOberhauw || OwnerActiveAttack == EMeleeAttackType::RightOberhauw;
	bool bEnemyOberhau = EnemyActiveAttack == EMeleeAttackType::LeftOberhauw || EnemyActiveAttack == EMeleeAttackType::RightOberhauw;

	float OwnerStrength = CombatantOwner->GetStrength() * (bOwnerOberhau ? CombatSettings->OberhauAttackStrengthFactor : 0.f);
	float EnemyStrength = CombatantEnemy->GetStrength() * (bEnemyOberhau ? CombatSettings->OberhauAttackStrengthFactor : 0.f);
	
	bool OwnerOutstrikeEnemy = EnemyStrength - OwnerStrength > CombatSettings->OutstrikeStrengthDelta;
	bool EnemyOutstrikeOwner = OwnerStrength - EnemyStrength > CombatSettings->OutstrikeStrengthDelta;

	if (!OwnerOutstrikeEnemy && !EnemyOutstrikeOwner)
	{
		TriggerClashAbility(OwnerActor, EClashSource::AttacksCollide, SweepDirection);
		TriggerClashAbility(EnemyActor, EClashSource::AttacksCollide, SweepDirection);
	}
	else
	{
		auto OutstrikedActor = OwnerOutstrikeEnemy ? EnemyActor : OwnerActor;
		TriggerClashAbility(OutstrikedActor, EClashSource::Outstriked, SweepDirection);
	}
}

void UGameplayAbility_MeleeWeaponCombat::HandleAttackBlocked(AActor* EnemyActor, const FVector& SweepDirection, const FHitResult& HitResult)
{
	ApplyEffect(EffectForOwnerWhenItsAttackBlockedClass, 1.f);
	auto EnemyCombatant = Cast<ICombatant>(EnemyActor);
	FGameplayEventData OwnerPayload; // TODO strike data
	FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
	OwnerData->HealthDamage = 0.f;
	OwnerData->PoiseDamage = 0.f;
	OwnerData->HitResult = HitResult;
	OwnerData->HitLocation = HitResult.ImpactPoint;
	OwnerData->SetCauser(EnemyActor);
	OwnerData->HitDirectionTag = GetHitDirectionTag(EnemyActor, SweepDirection, HitResult.ImpactPoint);
	OwnerPayload.TargetData.Add(OwnerData);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CurrentActorInfo->AvatarActor.Get(), CombatGameplayTags::Combat_Ability_HitReact_Event_Activate, OwnerPayload);
	// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EnemyActor, CombatGameplayTags::Combat_Ability_HitReact_Event_Activate, OwnerPayload);
}

void UGameplayAbility_MeleeWeaponCombat::HandleAttackParried(AActor* OwnerActor)
{
	FGameplayEventData OwnerPayload;
	FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
	OwnerData->HitDirectionTag = CombatGameplayTags::Combat_HitDirection_Front;
	OwnerData->HealthDamage = 0;
	OwnerData->PoiseDamage = 0;
	OwnerPayload.TargetData.Add(OwnerData);

	FGameplayTag HitReactAbilityTag = CombatGameplayTags::Combat_Ability_ReceiveGuardBreak_Event_Activate;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, HitReactAbilityTag, OwnerPayload);
}

void UGameplayAbility_MeleeWeaponCombat::ApplyEffect(const TSubclassOf<UGameplayEffect>& EffectClass, float EffectLevel)
{
	if (EffectClass)
	{
		auto ASC = GetActorInfo().AbilitySystemComponent.Get();
#if WITH_EDITOR
		auto EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		ensure(EffectCDO->DurationPolicy != EGameplayEffectDurationType::Infinite);
#endif
		
		auto EffectContext = ASC->MakeEffectContext();
		auto EffectSpec = ASC->MakeOutgoingSpec(EffectClass, EffectLevel, EffectContext);
		ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
	}
}

void UGameplayAbility_MeleeWeaponCombat::TriggerClashAbility(AActor* OwnerActor, EClashSource ClashSource, const FVector& SweepDirection)
{
	FGameplayEventData OwnerPayload; // TODO strike data
	FGameplayAbilityTargetData_Clash* OwnerData = new FGameplayAbilityTargetData_Clash();
	OwnerData->Direction = -OwnerActor->GetActorForwardVector();
	OwnerData->ClashSource = ClashSource;
	OwnerPayload.TargetData.Add(OwnerData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, CombatGameplayTags::Combat_Ability_Clash_Event_Activate, OwnerPayload);
}

bool UGameplayAbility_MeleeWeaponCombat::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	auto CostGE = GetCostGameplayEffect(); // TODO find requested attack type (light, heavy, thrust, special, etc)
	auto CombatComponent = CombatComponentCached.Get();

	// If CanActivateAbility is executed not from the ability itself, (like when we check if this ability can be activated for NPC before actually requesting the execution of the BT task)
	// then this object will be CDO, and hence to CombatComponentCached will be available
	// However we can also get Non-replicated instances from the GameplayAbilitySpec in that case
	// auto CombatComponent = ActorInfo->AvatarActor->FindComponentByClass<UMeleeCombatComponent>();

	if (CostGE && ensure(CombatComponent))
	{
		UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
		auto EffectContext = MakeEffectContext(Handle, ActorInfo);
		if (!AbilitySystemComponent->CanApplyAttributeModifiers(CostGE, CombatComponent->GetCurrentComboTotalAttackCount(), EffectContext))
		{
			const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;

			if (OptionalRelevantTags)
			{
				if (CostTag.IsValid())
					OptionalRelevantTags->AddTag(CostTag);
				
				OptionalRelevantTags->AddTag(CombatGameplayTags::Combat_Ability_Attack_Event_CostUnaffordable);
			}

			return false;
		}
	}
	
	return true;
}

void UGameplayAbility_MeleeWeaponCombat::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	if (Debug_IgnoreAttackCost)
		return;
#endif
	
	if (UGameplayEffect* CostGE = GetCostGameplayEffect())
	{
		// assumption is that the effect is instant
		auto AppliedEffectHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CostGE,
			CombatComponentCached->GetCurrentComboTotalAttackCount());
	}
}

void UGameplayAbility_MeleeWeaponCombat::HandleEnemyHit(const UMeleeCombatSettings* CombatSettings, AActor* OwnerActor, AActor* EnemyActor,
                                                        ICombatant* CombatantOwner, ICombatant* CombatantEnemy, const FHitResult& HitResult, const FVector& SweepDirection)
{
	if (auto EnemyAliveCreature = Cast<ICombatAliveCreature>(EnemyActor))
		if (EnemyAliveCreature->GetCombatantHealth() <= 0.f)
			return;
		
	FAttackDamageEvaluationData AttackerDamageEvaluationData;
	AttackerDamageEvaluationData.WeaponDamageData = CombatantOwner->GetWeaponDamageData();
	AttackerDamageEvaluationData.Strength = CombatantOwner->GetStrength();
	AttackerDamageEvaluationData.StaminaRatio = CombatantOwner->GetStaminaRatio();
	AttackerDamageEvaluationData.Dexterity = CombatantOwner->GetDexterity();
	AttackerDamageEvaluationData.WeaponMastery = CombatantOwner->GetActiveWeaponMasteryLevel();
	AttackerDamageEvaluationData.Direction = OwnerActor->GetActorForwardVector();
	
	FAttackDamageEvaluationData EnemyDamageEvaluationData;
	EnemyDamageEvaluationData.Strength = CombatantEnemy->GetStrength();
	EnemyDamageEvaluationData.StaminaRatio = CombatantEnemy->GetStaminaRatio();
	EnemyDamageEvaluationData.Dexterity = CombatantEnemy->GetDexterity();
	EnemyDamageEvaluationData.Poise = CombatantEnemy->GetPoise();
	EnemyDamageEvaluationData.Direction = EnemyActor->GetActorForwardVector();
	
	float ResultingDamage = CombatComponentCached->GetAttackDamage(AttackerDamageEvaluationData, EnemyDamageEvaluationData,
		CombatantOwner->GetActiveWeaponTypeTag(), HitResult);
	float PoiseReduction = CombatComponentCached->GetPoiseDamage(AttackerDamageEvaluationData, EnemyDamageEvaluationData, HitResult);
	
	auto DamageEffect = GetDamageGameplayEffect();
	if (auto EnemyASCInterface = Cast<IAbilitySystemInterface>(EnemyActor))
	{
		auto EnemyASC = EnemyASCInterface->GetAbilitySystemComponent();
		auto OwnerASC = GetAbilitySystemComponentFromActorInfo();
		auto GEContext = OwnerASC->MakeEffectContext();

		float DamageProtection = 0.f;
		if (AttackerDamageEvaluationData.WeaponDamageData.DamageType.IsValid())
		{
			const auto* ProtectionAttribute = CombatSettings->DamageTypeToProtectionAttribute.Find(AttackerDamageEvaluationData.WeaponDamageData.DamageType);
			if (ProtectionAttribute)
				DamageProtection = EnemyASC->GetNumericAttribute(*ProtectionAttribute);
		}
		
		UE_VLOG(OwnerActor, LogCombat, Verbose, TEXT("Dealing damage to %s.\nRaw damage = %.2f\nDamage protection = %.2f\nRaw poise reduction = %.2f"), *EnemyActor->GetName(),
			ResultingDamage, DamageProtection, PoiseReduction);
		ResultingDamage = ResultingDamage * FMath::Exp (-DamageProtection / CombatSettings->ProtectionEffectivenessScale); // chat gpt suggested this
		PoiseReduction *= CombatantEnemy->GetPoiseDamageScale();
		PoiseReduction *= CombatSettings->GlobalPoiseDamageScale;
		auto DamageEffectSpec = OwnerASC->MakeOutgoingSpec(DamageEffect, 1.f, GEContext);
		DamageEffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Damage_Health, -ResultingDamage);
		DamageEffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Damage_Poise, -PoiseReduction);
		OwnerASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec.Data, EnemyASC);

		if (HitRewardEffectClass)
		{
			auto HitRewardEffectSpec = OwnerASC->MakeOutgoingSpec(HitRewardEffectClass, GetMeleeCombatComponent()->GetCurrentComboHitAttackCount(), GEContext);
			OwnerASC->ApplyGameplayEffectSpecToSelf(*HitRewardEffectSpec.Data);
		}

		FGameplayEventData OwnerPayload;
		FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
		OwnerData->HitDirectionTag = GetHitDirectionTag(EnemyActor, SweepDirection, HitResult.ImpactPoint);
		OwnerData->HitLocation = HitResult.ImpactPoint;
		OwnerData->HealthDamage = ResultingDamage;
		OwnerData->SetCauser(EnemyActor);
		OwnerData->PoiseDamage = PoiseReduction;
		OwnerData->HitResult = HitResult;
		OwnerPayload.TargetData.Add(OwnerData);

		UE_VLOG(OwnerActor, LogCombat, Verbose, TEXT("Actual damage = %.2f\nActual poise reduction = %.2f"), ResultingDamage, PoiseReduction);
		
		bool bStaggeringHit = CombatantEnemy->GetPoise() < CombatantEnemy->GetStaggerPoiseThreshold()
			&& !EnemyASC->HasMatchingGameplayTag(CombatGameplayTags::Combat_State_Stagger);
		
		FGameplayTag HitReactAbilityTag = bStaggeringHit
			? CombatGameplayTags::Combat_Ability_Stagger_Event_Activate
			: CombatGameplayTags::Combat_Ability_HitReact_Event_Activate;

		if (bStaggeringHit)
			CombatantOwner->OnStaggeredActor(EnemyActor);
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EnemyActor, HitReactAbilityTag, OwnerPayload);
	}
	else
	{
		// TODO
		auto Controller = Cast<APawn>(OwnerActor)->GetController();
		EnemyActor->TakeDamage(ResultingDamage, FDamageEvent(), Controller, OwnerActor);	
	}
}

TSubclassOf<UGameplayEffect> UGameplayAbility_MeleeWeaponCombat::GetDamageGameplayEffect() const
{
	if (auto CombatantOwner = Cast<ICombatant>(GetCurrentActorInfo()->AvatarActor.Get()))
		return CombatantOwner->GetDamageEffect();

	ensure(false);
	return TSubclassOf<UGameplayEffect>();
}

void UGameplayAbility_MeleeWeaponCombat::OnAbilityAborted(FGameplayEventData Payload)
{
	if (CombatComponentCached->CanCancel())
	{
		CombatComponentCached->CancelAttack();
	}
	else
	{
		UE_VLOG(GetAvatarActorFromActorInfo(), LogCombat, Verbose, TEXT("Can't abort current attack, it is in phase = %s"), 
			*StaticEnum<EMeleeAttackPhase>()->GetDisplayValueAsText(CombatComponentCached->GetActiveAttackPhase()).ToString());
	}
	
	// EndAbility(GetCurrentAbilitySpecHandle(),  GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

FGameplayTag UGameplayAbility_MeleeWeaponCombat::GetHitDirectionTag(const AActor* HitActor, const FVector& HitDirection, const FVector& HitLocation) const
{
	const FVector ToOriginDirection2D = (HitActor->GetActorLocation() - HitLocation).GetSafeNormal2D();
	const FVector OwnerForwardVector = HitActor->GetActorForwardVector();
	const FVector OwnerRightVector = HitActor->GetActorRightVector();

	float HitForwardVectorDotProduct = ToOriginDirection2D | OwnerForwardVector;
	float HitRightVectorDotProduct = ToOriginDirection2D | OwnerRightVector;
	
	if (HitForwardVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Back;
	else if (HitForwardVectorDotProduct < -DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Front;
	else if (HitRightVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Left;
	else
		return CombatGameplayTags::Combat_HitDirection_Right;	
}