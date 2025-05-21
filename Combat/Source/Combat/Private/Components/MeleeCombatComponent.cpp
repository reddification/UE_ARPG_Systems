// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MeleeCombatComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/MeleeBlockComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Helpers/CombatCommonHelpers.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UMeleeCombatComponent::UMeleeCombatComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;	
}

void UMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	auto CombatantInterface = Cast<ICombatant>(GetOwner());
	if (ensure(CombatantInterface))
	{
		OwnerCombatant.SetObject(GetOwner());
		OwnerCombatant.SetInterface(CombatantInterface);

		CombatAnimInstance = OwnerCombatant->GetCombatAnimInstance();
		ensure(IsValid(CombatAnimInstance.GetObject()));
	}

	if (auto CombatSettings = GetDefault<UMeleeCombatSettings>())
	{
		WeaponCollisionSweepsPerSecond = CombatSettings->WeaponCollisionSweepsPerSeconds;
		WeaponCollisionProfileName = CombatSettings->WeaponCollisionProfileName;
		CombatCollisionName = CombatSettings->CombatCollisionName;
		CombatCollisionCenterSocketName = CombatSettings->CombatCollisionCenterSocketName;
		ConsequitiveComboAttackWindUpSpeedScale = CombatSettings->ConsequitiveComboAttackWindUpSpeedScale;
		HeavyAttackWindupSpeedModifier = CombatSettings->HeavyAttackWindupSpeedModifier;
		KeepWeaponReadyAfterAttackDelay = CombatSettings->KeepWeaponReadyAfterAttackDelay;
	}
}

void UMeleeCombatComponent::SetDamageCollisionsEnabled(bool bEnabled)
{
	for (const auto WeaponCollision : WeaponCollisionsComponents)
		if (IsValid(WeaponCollision))
			WeaponCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

float UMeleeCombatComponent::GetAttackDamage(const FAttackDamageEvaluationData& AttackDamageEvaluationData, const FAttackDamageEvaluationData& EnemyDamageEvaluationData,
                                             const FGameplayTag& WeaponTypeTag, const FHitResult& HitResult)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeCombatComponent::GetAttackDamage)
	// TODO utilize weapon types (rapiers are more dexterity dependent than zweihanders, etc
	float WeaponMasteryDamageScale = 1.f;
	float StrengthDeltaDamageScale = 1.f;
	float DexterityDeltaDamageScale = 1.f;
	float StaminaRatioDamageScale = 1.f;
	float TargetPoiseDamageScale = 1.f;
	float CurrentAttackEnemiesHitDamageScale = 1.f;
	
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	auto WeaponTypeDependencies = CombatSettings->WeaponDamageAttributesDependencies.Find(WeaponTypeTag);
	if (!ensure(WeaponTypeDependencies))
		return AttackDamageEvaluationData.WeaponDamageData.BaseDamageOutput;
	
	if (auto WeaponMasteryDamageOutputDependency = WeaponTypeDependencies->WeaponMasteryDamageOutputDependency.GetRichCurveConst())
	{
		WeaponMasteryDamageScale = WeaponMasteryDamageOutputDependency->Eval(AttackDamageEvaluationData.WeaponMastery);
	}

	if (auto StrengthDeltaDamageScaleDependency = WeaponTypeDependencies->StrengthDeltaDamageOutputDependency.GetRichCurveConst())
	{
		const float DeltaStrength = AttackDamageEvaluationData.Strength - AttackDamageEvaluationData.WeaponDamageData.RequiredWeaponStrength;
		StrengthDeltaDamageScale = StrengthDeltaDamageScaleDependency->Eval(DeltaStrength);
	}

	if (auto DexterityDeltaDamageDependency = WeaponTypeDependencies->DexterityDeltaDamageOutputDependency.GetRichCurveConst())
	{
		const float DeltaDexterity = AttackDamageEvaluationData.Dexterity - AttackDamageEvaluationData.WeaponDamageData.RequiredWeaponDexterity;
		DexterityDeltaDamageScale = DexterityDeltaDamageDependency->Eval(DeltaDexterity);
	}

	if (auto StaminaRatioDamageDependency = WeaponTypeDependencies->StaminaDamageOutputDependency.GetRichCurveConst())
	{
		StaminaRatioDamageScale = StaminaRatioDamageDependency->Eval(AttackDamageEvaluationData.StaminaRatio);
	}

	if (auto TargetPoiseDamageDependency = CombatSettings->TargetPoiseToReceivedDamagedScaleDependency.GetRichCurveConst())
	{
		TargetPoiseDamageScale = TargetPoiseDamageDependency->Eval(EnemyDamageEvaluationData.Poise);
	}

	if (auto CurrentAttackEnemiesHitDamageDependency = CombatSettings->CurrentAttackEnemiesHitDamageDependency.GetRichCurveConst())
	{
		CurrentAttackEnemiesHitDamageScale = CurrentAttackEnemiesHitDamageDependency->Eval(CurrentAttackActorsHit.Num());
	}

	// TODO I guess it can happen that if a hit went through wrist and head it might be that only wrist hit would considered
	// so maybe add a 0.1s delay to consider the most significant hit bone
	const float* HitBoneSignificanceScorePtr = CombatSettings->HitBonesDamageScore.Find(HitResult.BoneName);
	const float AttackSignificanceScore = HitBoneSignificanceScorePtr ? *HitBoneSignificanceScorePtr : 1.f;
	
	const float StatsAffectedDamage = AttackDamageEvaluationData.WeaponDamageData.BaseDamageOutput
		* WeaponMasteryDamageScale
		* StrengthDeltaDamageScale
		* DexterityDeltaDamageScale
		* StaminaRatioDamageScale
		* TargetPoiseDamageScale
		* AttackSignificanceScore
		* CurrentAttackEnemiesHitDamageScale;

	return FMath::Min(StatsAffectedDamage, AttackDamageEvaluationData.WeaponDamageData.MaxDamageOutput);
}

float UMeleeCombatComponent::GetPoiseDamage(const FAttackDamageEvaluationData& AttackDamageEvaluationData,
                                            const FAttackDamageEvaluationData& EnemyDamageEvaluationData, const FHitResult& HitResult)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeCombatComponent::GetPoiseDamage)
	
	float ResultPoiseDamange = 0.f;
	float StrengthDiff = AttackDamageEvaluationData.Strength - EnemyDamageEvaluationData.Strength;
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	float DotProductPoiseDamageScale = 1.f;
	
	if (auto StrengthDiffPoiseDamange = CombatSettings->StrengthDiffPoiseDamage.GetRichCurveConst())
		ResultPoiseDamange = StrengthDiffPoiseDamange->Eval(StrengthDiff);

	if (auto DexterityPoiseDamageReductionScale = CombatSettings->DexterityPoiseDamageReductionScale.GetRichCurveConst())
		ResultPoiseDamange *= DexterityPoiseDamageReductionScale->Eval(EnemyDamageEvaluationData.Dexterity);

	if (auto StaminaRatioPoiseDamageScale = CombatSettings->StaminaRatioPoiseDamageScale.GetRichCurveConst())
		ResultPoiseDamange *= StaminaRatioPoiseDamageScale->Eval(EnemyDamageEvaluationData.StaminaRatio);
	
	// TODO I guess it can happen that if a hit went through wrist and head it might be that only wrist hit would considered
	// so maybe add a 0.1s delay to consider the most significant hit bone
	const float* HitBoneSignificanceScorePtr = CombatSettings->HitBonesPoiseScore.Find(HitResult.BoneName);
	const float AttackSignificanceScore = HitBoneSignificanceScorePtr ? *HitBoneSignificanceScorePtr : 1.f;
	
	if (auto TargetToAttackerDotProductStaggerDependency = CombatSettings->TargetToAttackerDotProductPoiseDamageDependency.GetRichCurveConst())
	{
		const float DotProduct = AttackDamageEvaluationData.Direction | EnemyDamageEvaluationData.Direction;
		DotProductPoiseDamageScale = TargetToAttackerDotProductStaggerDependency->Eval(DotProduct);
	}

	return ResultPoiseDamange * AttackSignificanceScore * DotProductPoiseDamageScale;
}

FVector UMeleeCombatComponent::GetActiveAttackDirection() const
{
	// all these 35.f, 15.f, 50.f corrections are just from the top of my head, don't consider them something well calculated
	FVector OwnerLocation = GetOwner()->GetActorLocation() + FVector::UpVector * 35.f;
	FVector OwnerUpVector = GetOwner()->GetActorUpVector();
	FVector OwnerRightVector = GetOwner()->GetActorRightVector();
	FVector AttackLocation = OwnerLocation + FVector::UpVector * 15.f + GetOwner()->GetActorForwardVector() * OwnerCombatant->GetAttackRange();
	FVector AttackOrigin = FVector::ZeroVector;
	switch (ActiveAttack)
	{
		case EMeleeAttackType::None:
			ensure(false);
			break;
		case EMeleeAttackType::LeftUnterhauw:
			AttackOrigin = OwnerLocation - OwnerUpVector * 50.f - OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::LeftMittelhauw:
			AttackOrigin = OwnerLocation - OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::LeftOberhauw:
			AttackOrigin = OwnerLocation + OwnerUpVector * 50.f - OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::Thrust:
			AttackOrigin = OwnerLocation;
			break;
		case EMeleeAttackType::RightUnterhauw:
			AttackOrigin = OwnerLocation - OwnerUpVector * 50.f + OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::RightMittelhauw:
			AttackOrigin = OwnerLocation + OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::RightOberhauw:
			AttackOrigin = OwnerLocation + OwnerUpVector * 50.f + OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::VerticalSlash:
			AttackOrigin = OwnerLocation + OwnerUpVector * 50.f;
			break;
		case EMeleeAttackType::SpinLeftMittelhauw:
			AttackOrigin = OwnerLocation - OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinLeftOberhauw:
			AttackOrigin = OwnerLocation + OwnerUpVector * 50.f - OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinRightMittelhauw:
			AttackOrigin = OwnerLocation + OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinRightOberhauw:
			AttackOrigin = OwnerLocation + OwnerUpVector * 50.f + OwnerRightVector * 50.f;
			break;
		case EMeleeAttackType::Max:
			ensure(false);
			break;
		default:
			ensure(false);
			break;
	}

	return (AttackLocation - AttackOrigin).GetSafeNormal();
}

bool UMeleeCombatComponent::RequestAttack(EMeleeAttackType RequestedAttackType)
{
	return true;
}

void UMeleeCombatComponent::FinalizeAttack()
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::FinalizeAttack"));
	
	if (ActiveAttack == EMeleeAttackType::None && AttackPhase == EMeleeAttackPhase::None)
		return;
	
	// ResetAttackState();
	OnAttackEndedEvent.Broadcast();
}

void UMeleeCombatComponent::ResetAttackState()
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::ResetAttackState"));
	GetWorld()->GetTimerManager().ClearTimer(WeaponCollisionSweepsTimer);
	OwnerCombatant->SetMovementEnabled(CombatGameplayTags::Combat_Movement_Lock_Attack,true);
	CombatAnimInstance->OnAttackFinished();
	CurrentComboAttacksHitCount = 0;
	CurrentComboTotalAttacksCount = 0;
	ActiveAttack = EMeleeAttackType::None;
	ActiveAttackTrajectory = EMeleeAttackType::None;
	SetAttackPhase(EMeleeAttackPhase::None, 0);
	ActiveAnimationId = 0;
}

float UMeleeCombatComponent::GetAttackPhasePlayRate(EMeleeAttackPhase NewAttackPhase) const
{
	const int WeaponMastery = OwnerCombatant->GetActiveWeaponMasteryLevel();
	float Result = 1.f;
	if (auto AttackPhasesSpeedScales = GetDefault<UMeleeCombatSettings>()->WeaponMasteryAttackPhaseSpeedScales.Find(WeaponMastery))
	{
		if (const float* SpeedScalePtr = AttackPhasesSpeedScales->AttackPhaseSpeed.Find(NewAttackPhase))
		{
			const float ComboScale = CurrentComboTotalAttacksCount > 0 && NewAttackPhase == EMeleeAttackPhase::WindUp
				? ConsequitiveComboAttackWindUpSpeedScale
				: 1.f;
			
			Result = (*SpeedScalePtr) * ComboScale;
		}
	}

	// making heavy attacks look heavy. currently only for player
	if (NewAttackPhase == EMeleeAttackPhase::WindUp && ActiveAttack == EMeleeAttackType::HeavyAttack)
	{
		Result *= HeavyAttackWindupSpeedModifier;
	}

	return Result * OwnerCombatant->GetWeaponAttackPhaseSpeedScale(NewAttackPhase);
}

void UMeleeCombatComponent::SetAttackPhase(EMeleeAttackPhase NewAttackPhase, float TotalDuration)
{
	// if it's a new attack or a next attack in combo (or attack morph)
	if (NewAttackPhase == EMeleeAttackPhase::WindUp || (NewAttackPhase == EMeleeAttackPhase::Release && AttackPhase != EMeleeAttackPhase::WindUp))
		OwnerCombatant->OnAttackStarted();
	
	auto OldAttackPhase = AttackPhase;
	AttackPhase = NewAttackPhase;
	PhaseEndsAt = GetWorld()->GetTimeSeconds() + TotalDuration;
	
	CombatAnimInstance->SetAttackPhase(AttackPhase);
	float PhasePlayRate = GetAttackPhasePlayRate(NewAttackPhase);
	CombatAnimInstance->SetAttackPlayRate(PhasePlayRate);

	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::SetAttackPhase Changing attack phase from %d to %d for %.2fs until %.2fs world time"),
		(uint8)OldAttackPhase, (uint8)NewAttackPhase, TotalDuration, PhaseEndsAt);
	
	OnAttackActivePhaseChanged.Broadcast(OldAttackPhase, NewAttackPhase);
	OwnerCombatant->SetAttackPhase(NewAttackPhase);

}

void UMeleeCombatComponent::CancelAttack()
{
	// FinalizeAttack();
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::CancelAttack"));
	ResetAttackState();
}

bool UMeleeCombatComponent::Feint()
{
	if (AttackPhase != EMeleeAttackPhase::WindUp)
	{
		UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("Attempted to feint not in wind up phase"));
		return false;
	}
	
	// FinalizeAttack();
	OnAttackFeintedEvent.Broadcast();
	OwnerCombatant->OnAttackFeinted();
	return true;
}

void UMeleeCombatComponent::BeginWindUp(float TotalDuration, const uint32 AnimationId, EMeleeAttackType WindupAttackTrajectoryType)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("Begin wind up. Animation id = %d"), AnimationId);
	SetAttackPhase(EMeleeAttackPhase::WindUp, TotalDuration);
	ActiveAnimationId = AnimationId;
	OwnerCombatant->SetMovementEnabled(CombatGameplayTags::Combat_Movement_Lock_Attack,false);
	CombatAnimInstance->OnAttackWindUpBegin();
	ActiveAttackTrajectory = WindupAttackTrajectoryType;
}

void UMeleeCombatComponent::BeginRelease(float TotalDuration, const uint32 AnimationId)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("Begin release. Animation id = %d"), AnimationId);
	// Ok if we check this on BeginRelease - it's important that each consequitive attack starts within Windup phase in animation!!!!
	// TODO enforce this in CombatAnimInstance->SetAttack
	if (ActiveAnimationId != AnimationId || ActiveAttack == EMeleeAttackType::None)
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("Begin release: Active animation id != updated animation id: %d != %d OR active attack is not set"), ActiveAnimationId, AnimationId);
		return;
	}
	
	// ActiveAnimationId = AnimationId; // I moved it to BeginWindUp but i don't remember why I placed it in BeginRelease in the first place...
	OwnerCombatant->PlayCombatSound(CombatGameplayTags::Combat_FX_Sound_Grunt);
	
	SetAttackPhase(EMeleeAttackPhase::Release, TotalDuration);
	CurrentAttackActorsHit.Reset();
	AliveActorsHit = 0;
	CurrentComboTotalAttacksCount++;
	// SetDamageCollisionsEnabled(true);
	OnAttackCommitedEvent.Broadcast();

	int i = 0;
	for (const auto* WeaponCollision : WeaponCollisionsComponents)
	{
		PreviousWeaponCollisionTransform[i++] = WeaponCollision->GetComponentTransform();
	}
	
	GetWorld()->GetTimerManager().SetTimer(WeaponCollisionSweepsTimer, this, &UMeleeCombatComponent::SweepWeaponCollisions,
		1.f / WeaponCollisionSweepsPerSecond, true);

	const FWeaponFX* WeaponFX = OwnerCombatant->GetWeaponFX(CombatGameplayTags::Combat_FX_Sound_Whoosh);
	if (WeaponFX && !WeaponFX->SFX.IsNull())
	{
		if (auto SFX = WeaponFX->SFX.LoadSynchronous())
			UGameplayStatics::PlaySoundAtLocation(this, SFX, GetOwner()->GetActorLocation());
	}
}

void UMeleeCombatComponent::BeginRecover(float TotalDuration, const uint32 AnimationId)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("Begin recover. Animation id = %d"), AnimationId);
	// this happens sometimes when feinting. My guess - ABP doesnt instantly stop active animation hence sometimes next attack phase can start even thought the attack has been feinted
	if (AnimationId != ActiveAnimationId)
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("Begin recover: Active animation id != updated animation id: %d != %d"), ActiveAnimationId, AnimationId);
		return;
	}
	else if (ActiveAttack == EMeleeAttackType::None)
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("Begin recover: Active attack is none. Active animation id = %d"), ActiveAnimationId);
		return;
	}
	
	// ActiveAnimationId = AnimationId; // huh? why did I put it here? it breaks logic in one other place (starting next attack before previous ended)
	SetAttackPhase(EMeleeAttackPhase::Recover, TotalDuration);
}

void UMeleeCombatComponent::EndWindUp(const uint32 AnimationId)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("End windup. Animation id = %d"), AnimationId);
}

void UMeleeCombatComponent::EndRelease(const uint32 AnimationId)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("End release. Animation id = %d"), AnimationId);
	if (ActiveAnimationId != AnimationId)
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("End release: Active animation id != updated animation id: %d != %d"), ActiveAnimationId, AnimationId);
		return;
	}
	
	// SetDamageCollisionsEnabled(false);
	GetWorld()->GetTimerManager().ClearTimer(WeaponCollisionSweepsTimer);
	if (CurrentAttackActorsHit.IsEmpty())
		ReportAttackWhiffed();
}

void UMeleeCombatComponent::EndRecover(const uint32 AnimationId)
{
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("End recover. Animation id = %d"), AnimationId);
	if (ActiveAnimationId != AnimationId)
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("End recover: Active animation id != updated animation id: %d != %d"), ActiveAnimationId, AnimationId);
		return;
	}
	
	if (AttackPhase == EMeleeAttackPhase::Recover)
	{
		UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("End recover: signalizing attack ended"));
		OnAttackEndedEvent.Broadcast();
		CombatAnimInstance->KeepWeaponReady(KeepWeaponReadyAfterAttackDelay);
	}
	else
	{
		UE_VLOG(GetOwner(), LogCombat, Warning, TEXT("End recover: Attack phase is not recover. Active animation id = %d"), ActiveAnimationId);
	}
}

void UMeleeCombatComponent::CacheCollisionShapes()
{
	WeaponCollisionShapes = GetCombatCollisionShapes(CombatCollisionName, WeaponCollisionsComponents, GetOwner());
}

void UMeleeCombatComponent::UpdateDamageCollisions()
{
	if (!OwnerCombatant)
	{
		OwnerCombatant.SetObject(GetOwner());
		OwnerCombatant.SetInterface(Cast<ICombatant>(GetOwner()));
	}
	
	WeaponCollisionsComponents = OwnerCombatant->GetDamageCollisionsComponents();
	WeaponCollisionShapes.Reset();
	
	CacheCollisionShapes();

	PreviousWeaponCollisionTransform.SetNumZeroed(WeaponCollisionsComponents.Num());
}

void UMeleeCombatComponent::StartComboWindow(const uint32 AttackAnimationId)
{
	ActiveComboWindowId = AttackAnimationId;
	bComboWindowActive = true;
}

void UMeleeCombatComponent::EndComboWindow(const uint32 AttackAnimationId)
{
	// this check is required, because when attacks are chained, StartComboWindow of new attack can appear before EndComboWindow of previous attack
	// And hence EndComboWindow of previous attack gets triggered and closes combo window of the new attack
	if (AttackAnimationId != ActiveComboWindowId)
		return;
	
	bComboWindowActive = false;
}

void UMeleeCombatComponent::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool BFromSweep, const FHitResult& SweepResult)
{
	OnWeaponOverlap(OtherActor, OtherComp, SweepResult, -SweepResult.ImpactNormal);
}

void UMeleeCombatComponent::OnWeaponOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult, const FVector& SweepDirection)
{
	if (CurrentAttackActorsHit.Contains(OtherActor))
		return;

	FGameplayTag FXSourceTag;
	if (auto OtherCombatant = Cast<ICombatant>(OtherActor))
	{
		auto OtherActorMeleeComponent = OtherActor->FindComponentByClass<UMeleeCombatComponent>();
		if (OtherComp->GetCollisionObjectType() == OtherCombatant->GetWeaponCollisionObjectType())
		{
			// ok here are the possible scenarios
			// 1. Hit weapon (any equipment slot but not shield) in any phase but release - ideally continue collision detection
			// 2. Hit weapon in release phase - clash
			// 3. Hit item is used for blocking and block is held - either parry or block, inflict stamina and poise damage
			// 4. Hit shield and character is not blocking - currently just ignore the shield (continue collision detection), alternatively - inflict some substantial poise damage

			ECollisionComponentWeaponType WeaponType = OtherCombatant->GetCollisionWeaponType(OtherComp);
			if (WeaponType == ECollisionComponentWeaponType::MeleeWeapon && OtherActorMeleeComponent->GetCurrentAttackPhase() == EMeleeAttackPhase::Release)
			{
				OnWeaponHitEvent.Broadcast(OtherComp, SweepResult, EWeaponHitSituation::WeaponClash, SweepDirection);
			}
			else if (WeaponType == ECollisionComponentWeaponType::MeleeWeapon && !OtherCombatant->IsUsingShield() || WeaponType == ECollisionComponentWeaponType::Shield)
			{
				auto EnemyBlockComponent = OtherActor->FindComponentByClass<UMeleeBlockComponent>();
				EBlockResult BlockResult = EnemyBlockComponent->BlockAttack(SweepDirection,OtherCombatant->GetStrength());
				if (BlockResult == EBlockResult::Block)
					OnWeaponHitEvent.Broadcast(OtherComp, SweepResult, EWeaponHitSituation::AttackBlocked, SweepDirection);
				else if (BlockResult == EBlockResult::Parry)
					OnWeaponHitEvent.Broadcast(OtherComp, SweepResult, EWeaponHitSituation::AttackParried, SweepDirection);
			}
			
			FXSourceTag = CombatGameplayTags::Combat_FX_Hit_Steel;
			UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::OnWeaponOverlap: hit weapon"));
		}
		else if (OtherComp->GetCollisionObjectType() == OtherCombatant->GetBodyCollisionObjectType())
		{
			CurrentAttackActorsHit.Add(OtherActor); // order is important here
			AliveActorsHit++;
			CurrentComboAttacksHitCount++;
			OnWeaponHitEvent.Broadcast(OtherComp, SweepResult, EWeaponHitSituation::Body, SweepDirection);
			FXSourceTag = CombatGameplayTags::Combat_FX_Hit_Body;
			UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::OnWeaponOverlap: hit actor %s"), *OtherActor->GetName());
		}
	}
	else if (AliveActorsHit <= 0 && (OtherComp->GetCollisionObjectType() == ECC_WorldStatic || OtherComp->GetCollisionObjectType() == ECC_WorldDynamic))
	{
		CurrentAttackActorsHit.Add(OtherActor);

		FVector ViewLocation;
		FRotator ViewRotation;
		GetOwner()->GetActorEyesViewPoint(ViewLocation, ViewRotation);

		const bool bOwnerLookingDown = ViewRotation.Pitch < -50.f;
		
		float HitLocationToFeetDistance = FMath::Abs(GetOwner()->GetActorLocation().Z - SweepResult.ImpactPoint.Z);
		float constexpr ActorRootToFloorDistanceThreshold = 70.f;
		bool bHitFloor = HitLocationToFeetDistance > ActorRootToFloorDistanceThreshold;		

		float SwingDirectionToHitNormalDotProduct = (-SweepDirection) | SweepResult.ImpactNormal;
		const bool bNonTangentialHit = SwingDirectionToHitNormalDotProduct > 0.5f;
		
		if (!bHitFloor || bNonTangentialHit && bOwnerLookingDown)
			OnWeaponHitEvent.Broadcast(OtherComp, SweepResult, EWeaponHitSituation::ImmovableObject, SweepDirection);
		
		FXSourceTag = CombatGameplayTags::Combat_FX_Hit_Environment;
		UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("UMeleeCombatComponent::OnWeaponOverlap: hit environment"));
	}

	if (const FWeaponFX* WeaponCollideFX = OwnerCombatant->GetWeaponFX(FXSourceTag))
	{
		if (!WeaponCollideFX->VFX.IsNull())
		{
			if (auto VFX = WeaponCollideFX->VFX.LoadSynchronous())
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFX, SweepResult.ImpactPoint, SweepResult.Normal.Rotation());
		}

		if (!WeaponCollideFX->SFX.IsNull())
		{
			if (auto SFX = WeaponCollideFX->SFX.LoadSynchronous())
				UGameplayStatics::SpawnSoundAttached(SFX, OtherComp);
		}
	}
}

void UMeleeCombatComponent::SweepWeaponCollisions()
{
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeCombatComponent::SweepWeaponCollisions)
	for (int i = 0; i < WeaponCollisionsComponents.Num(); i++)
	{
		const FTransform& PreviousTransform = PreviousWeaponCollisionTransform[i];
		const FTransform& CurrentTransform = WeaponCollisionsComponents[i]->GetSocketTransform(CombatCollisionCenterSocketName);// GetComponentTransform();
		FVector SweepDirection = (CurrentTransform.GetLocation() - PreviousTransform.GetLocation()).GetSafeNormal();
		FHitResult HitResult;
		FQuat Rotation = FQuat::Slerp(PreviousTransform.GetRotation(), CurrentTransform.GetRotation(), 0.5f);
		// FQuat Rotation = CurrentTransform.GetRotation();
		// radius = 5 is out of my head value. should be enough and that'd be enough
		auto CollisionShape = FCollisionShape::MakeCapsule(5.f, WeaponCollisionShapes[i].HalfHeight);

		// box sweep causes too much hitting the floor
		// auto CollisionShape = FCollisionShape::MakeBox(WeaponCollisionShapes[i].Extent);
		// FVector TrueStart = PreviousTransform.TransformPosition(WeaponCollisionShapes[i].Center);
		// FVector TrueEnd = CurrentTransform.TransformPosition(WeaponCollisionShapes[i].Center);

		const float HandAdjustment = 15.f;
		FVector TrueStart = PreviousTransform.GetLocation() + PreviousTransform.GetUnitAxis(EAxis::Type::Z) * (WeaponCollisionShapes[i].HalfHeight + HandAdjustment);
		FVector TrueEnd = CurrentTransform.GetLocation() + CurrentTransform.GetUnitAxis(EAxis::Type::Z) * (WeaponCollisionShapes[i].HalfHeight + HandAdjustment);
		
		bool bHit = GetWorld()->SweepSingleByProfile(HitResult, TrueStart, TrueEnd, Rotation,
			WeaponCollisionProfileName, CollisionShape, CollisionQueryParams);
		if (bHit)
		{
			OnWeaponOverlap(HitResult.GetActor(), HitResult.GetComponent(), HitResult, SweepDirection);
		}

#if WITH_EDITOR
		if (GetDefault<UMeleeCombatSettings>()->bDrawDebugSweeps)
		{
			if (CollisionShape.IsBox())
			{
				DrawDebugBox(GetWorld(), CurrentTransform.GetLocation(), CollisionShape.GetExtent(), Rotation, bHit ? FColor::Red : FColor::Green,
					false, 5);
			}
			else if (CollisionShape.IsCapsule())
			{
				// DrawDebugCapsule(GetWorld(), CurrentTransform.GetLocation(), CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(),
				// 	Rotation, bHit ? FColor::Red : FColor::Green, false, 5);
				DrawDebugCapsule(GetWorld(), TrueEnd, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(),
					Rotation, bHit ? FColor::Red : FColor::Green, false, 5);
			}
			else
			{
				ensure(false);
			}
		}
#endif
		
		PreviousWeaponCollisionTransform[i] = CurrentTransform;
	}
}

void UMeleeCombatComponent::ReportAttackWhiffed()
{
	OnAttackWhiffedEvent.Broadcast();
}
