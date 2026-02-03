// 


#include "GAS/Abilities/GameplayAbility_AttackByMontageShapeSweep.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/MeleeCombatSettings.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Interfaces/ICombatant.h"

UGameplayAbility_AttackByMontageShapeSweep::UGameplayAbility_AttackByMontageShapeSweep()
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

void UGameplayAbility_AttackByMontageShapeSweep::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ActiveAttackIndex = Attacks.Num() > 1 ? FMath::RandRange(0, Attacks.Num() - 1) : 0;
	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Attack"),
		Attacks[ActiveAttackIndex].AttackMontage.LoadSynchronous());
	ActiveMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_AttackByMontageShapeSweep::OnMontageCompleted);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_AttackByMontageShapeSweep::OnMontageCancelled);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_AttackByMontageShapeSweep::OnMontageInterrupted);
	ActiveMontageTask->ReadyForActivation();

	WaitOverlapTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, OverlapCollisionEventTag);
	WaitOverlapTask->EventReceived.AddDynamic(this, &UGameplayAbility_AttackByMontageShapeSweep::DoOverlap);
	WaitOverlapTask->ReadyForActivation();
}

bool UGameplayAbility_AttackByMontageShapeSweep::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& Attacks.Num() > 0 && IsValid(DamageGE);
}

void UGameplayAbility_AttackByMontageShapeSweep::DoOverlap(FGameplayEventData Payload)
{
	auto OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	const FVector StartLocation = OwnerCharacter->GetMesh()->GetSocketLocation(Attacks[ActiveAttackIndex].DamageOriginSocketName);
	// CollisionObjectQueryParams.AddObjectTypesToQuery();
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(OwnerCharacter);
	const UMeleeCombatSettings* MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	FHitResult HitResult;
	//@AK 19.09.2024: TODO implement virtual GetAttackDirection(OwnerCharacter) and make UGameplayAbility_NpcAttackByMontageShapeSweep
	FVector AttackDirection = OwnerCharacter->GetActorForwardVector();
	auto OwnerCombatant = Cast<ICombatant>(OwnerCharacter);
	const float AttackRange = OwnerCombatant->GetAttackRange();
	if (auto AIController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		if (auto Target = AIController->GetFocusActor())
		{
			ensure(Target != OwnerCharacter); // temp ensure to debug AI bug
			AttackDirection = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
		}
	}
	auto CollisionShape = FCollisionShape::MakeSphere(Attacks[ActiveAttackIndex].DamageSphereRadius);
	bool bHit = GetWorld()->SweepSingleByProfile(HitResult, StartLocation, StartLocation + AttackDirection * AttackRange,
		FQuat::Identity, MeleeCombatSettings->WeaponCollisionProfileName, CollisionShape, CollisionQueryParams);

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	if (bDrawSweepDebug)
	{
		FVector EndLocation = StartLocation + AttackDirection * AttackRange;
		FVector CapsuleCenter = (StartLocation + EndLocation) * 0.5f;
		FVector SweepVector = EndLocation - StartLocation;
		float CapsuleHalfHeight = SweepVector.Size() * 0.5f;
		FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(SweepVector).ToQuat();
		DrawDebugCapsule(GetWorld(), CapsuleCenter, CapsuleHalfHeight, Attacks[ActiveAttackIndex].DamageSphereRadius, CapsuleRotation,
			bHit ? FColor::Red : FColor::White, false, 2.0f, 0, 1.0f
		);
	}
#endif
	
	if (bHit && HitResult.GetActor() != nullptr) 
	{
		auto DamagedActor = HitResult.GetActor();
		ensure(DamagedActor != OwnerCharacter);
		auto TargetCombatant = Cast<ICombatant>(DamagedActor);
		if (TargetCombatant)
			if (HitResult.Component->GetCollisionObjectType() == TargetCombatant->GetWeaponCollisionObjectType() && TargetCombatant->IsBlocking())
				return;
		
		auto TargetASCInterface = Cast<IAbilitySystemInterface>(DamagedActor);
		if (!ensure(TargetASCInterface))
		{
			FPointDamageEvent PointDamageEvent(FallbackRawDamage * MeleeCombatSettings->GlobalHealthDamageScale, HitResult, AttackDirection, TSubclassOf<UDamageType>());
			HitResult.GetActor()->TakeDamage(FallbackRawDamage, PointDamageEvent, OwnerCharacter->GetController(), OwnerCharacter);
			return;
		}
		
		auto OwnerASC = GetAbilitySystemComponentFromActorInfo();
		auto TargetASC = TargetASCInterface->GetAbilitySystemComponent();
		auto EffectContext = OwnerASC->MakeEffectContext();
		auto EffectSpec = OwnerASC->MakeOutgoingSpec(DamageGE, 1.f, EffectContext);
		float HealthDamage = 0.f;
		float PoiseDamage = Attacks[ActiveAttackIndex].PoiseDamage;
		for (const auto& AttackDamage : Attacks[ActiveAttackIndex].Damages)
		{
			float TargetDamageTypeProtection = 0.f;
			const auto* ProtectionAttribute = MeleeCombatSettings->DamageTypeToProtectionAttribute.Find(AttackDamage.Key);
			if (ensure(ProtectionAttribute))
				TargetDamageTypeProtection = TargetASC->GetNumericAttribute(*ProtectionAttribute);

			HealthDamage = HealthDamage + AttackDamage.Value * FMath::Exp (-TargetDamageTypeProtection / MeleeCombatSettings->ProtectionEffectivenessScale); // chat gpt suggested this
		}
		
		if (TargetCombatant)
			PoiseDamage *= TargetCombatant->GetPoiseDamageScale();
		
		PoiseDamage *= MeleeCombatSettings->GlobalPoiseDamageScale;
		HealthDamage *= MeleeCombatSettings->GlobalHealthDamageScale;
		EffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Damage_Health, -HealthDamage);
		EffectSpec.Data->SetByCallerTagMagnitudes.Add(CombatGameplayTags::Combat_SetByCaller_Damage_Poise, -PoiseDamage);
		OwnerASC->ApplyGameplayEffectSpecToTarget(*EffectSpec.Data, TargetASC);

		FGameplayEventData OwnerPayload;
		FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
		OwnerData->HitDirectionTag = GetHitDirectionTag(HitResult.GetActor(), HitResult.ImpactPoint);
		OwnerData->HitLocation = HitResult.ImpactPoint;
		OwnerData->HealthDamage = HealthDamage;
		OwnerData->PoiseDamage = PoiseDamage;
		OwnerData->HitResult = HitResult;
		OwnerPayload.TargetData.Add(OwnerData);

		bool bStaggeringHit = TargetCombatant->GetPoise() < TargetCombatant->GetStaggerPoiseThreshold()
			&& !TargetASC->HasMatchingGameplayTag(CombatGameplayTags::Combat_State_Stagger);
		
		FGameplayTag HitReactAbilityTag = bStaggeringHit
			? CombatGameplayTags::Combat_Ability_Stagger_Event_Activate
			: CombatGameplayTags::Combat_Ability_HitReact_Event_Activate;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitResult.GetActor(), HitReactAbilityTag, OwnerPayload);
	}
}

void UGameplayAbility_AttackByMontageShapeSweep::OnMontageCompleted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_AttackByMontageShapeSweep::OnMontageCancelled()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGameplayAbility_AttackByMontageShapeSweep::OnMontageInterrupted()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

FGameplayTag UGameplayAbility_AttackByMontageShapeSweep::GetHitDirectionTag(const AActor* HitActor, const FVector& HitLocation) const
{
	const FVector ToOriginDirection2D = (HitActor->GetActorLocation() - HitLocation).GetSafeNormal2D();
	const FVector OwnerForwardVector = HitActor->GetActorForwardVector();
	const FVector OwnerRightVector = HitActor->GetActorRightVector();

	float HitForwardVectorDotProduct = ToOriginDirection2D | OwnerForwardVector;
	float HitRightVectorDotProduct = ToOriginDirection2D | OwnerRightVector;

	const float DirectionDotProductThreshold = 0.75f;
	if (HitForwardVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Back;
	else if (HitForwardVectorDotProduct < -DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Front;
	else if (HitRightVectorDotProduct > DirectionDotProductThreshold)
		return CombatGameplayTags::Combat_HitDirection_Left;
	else
		return CombatGameplayTags::Combat_HitDirection_Right;
}
