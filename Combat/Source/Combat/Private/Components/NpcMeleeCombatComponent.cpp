// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NpcMeleeCombatComponent.h"

#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UNpcMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	auto PawnOwner = Cast<APawn>(GetOwner());
	if (!ensure(PawnOwner))
		return;

	AIController = Cast<AAIController>(PawnOwner->GetController());
	OwnerNPCCombatant.SetObject(GetOwner());
	OwnerNPCCombatant.SetInterface(Cast<INpcCombatant>(GetOwner()));
}

void UNpcMeleeCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	auto World = GetWorld();
	if (World)
		World->GetTimerManager().ClearTimer(ResetPreviousAttackTimer);
	
	Super::EndPlay(EndPlayReason);
}

void UNpcMeleeCombatComponent::EndRecover(const uint32 AnimationId)
{
	Super::EndRecover(AnimationId);
	// if (ActiveAnimationId == AnimationId)
	// 	GetWorld()->GetTimerManager().SetTimer(ResetPreviousAttackTimer, this, &UNpcMeleeCombatComponent::ResetPreviousAttack, 0.25f, false);
	
	// PreviousAttack = EMeleeAttackType::None;
}

const TMap<int, FMeleeAttackPhaseSpeedModifier>& UNpcMeleeCombatComponent::GetAttackPhasePlayRates() const
{
	return CachedMeleeCombatSettings->NpcWeaponMasteryAttackPhaseSpeedScales;
}

void UNpcMeleeCombatComponent::ResetPreviousAttack()
{
	PreviousAttack = EMeleeAttackType::None;
}

float UNpcMeleeCombatComponent::AddIntelligenceMisperception(float BaseValue, float Intelligence,
                                                             const FRuntimeFloatCurve& RuntimeFloatCurve)
{
	if (auto Curve = RuntimeFloatCurve.GetRichCurveConst())
	{
		float Misperception = Curve->Eval(Intelligence);
		ensure(Misperception <= 1.f && Misperception >= 0.f);
		float Deviation = BaseValue * Misperception;
		return FMath::RandRange(BaseValue - Deviation, BaseValue + Deviation);
	}

	return BaseValue;
}

bool UNpcMeleeCombatComponent::RequestAttack(EMeleeAttackType RequestedAttackType)
{
	bool bCanRequest = Super::RequestAttack(EMeleeAttackType::LightAttack);
	WeaponMasteryLevel = OwnerCombatant->GetActiveWeaponMasteryLevel();
	// PreviousAttack = EMeleeAttackType::None;
	return bCanRequest && RequestNextAttack();
}

bool UNpcMeleeCombatComponent::RequestNextAttack()
{
	// if (AttackPhase != EMeleeAttackPhase::None && !bComboWindowActive)
	// 	return true; // not really the best way to return "you requested next attack in incorrect timing", but the alternative will lead to aborting attack

	// TODO bullshit, it shouldn't be a problem, NPC should be able to just swing ahead for whatever reason (i.e. player is in invisibility or in the dark)
	auto Target = AIController->GetFocusActor();
	if (!ensure(Target))
	{
		UE_VLOG(AIController.Get(), LogCombat, Warning, TEXT("UNpcMeleeCombatComponent::RequestNextAttack - no target"));
		OnAttackEndedEvent.Broadcast();
		return false;
	}

	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	auto TargetCombatant = Cast<ICombatant>(Target);
	FGameplayTag OwnerCombatStyle = OwnerCombatant->GetActiveCombatStyleTag();
	FGameplayTag EnemyCombatStyle = TargetCombatant->GetActiveCombatStyleTag();
	ensure(OwnerCombatStyle.IsValid());
	
	float TargetDistance = (GetOwner()->GetActorLocation() - Target->GetActorLocation()).Size();
	const float Intelligence = OwnerNPCCombatant->GetIntelligence();
	TargetDistance = AddIntelligenceMisperception(TargetDistance, Intelligence, CombatSettings->AITargetDistanceIntelligenceMisperceptionFactor);
	FNpcCombatSituationKey Key(OwnerCombatStyle, EnemyCombatStyle, WeaponMasteryLevel);
	const auto WeaponCombinationsForNpcWeapon = CombatSettings->AIWeaponTypeChainableAttacks.Find(OwnerCombatStyle);
	if (!WeaponCombinationsForNpcWeapon)
	{
		ensure(false);
		return false;
	}
	
	const FChainableAttacksWrapper* ChainableAttacks = &WeaponCombinationsForNpcWeapon->DefaultAttacks;
	if (auto WeaponCombinationsForWeaponMastery = WeaponCombinationsForNpcWeapon->MasteryLevelChainableAttacks.Find(WeaponMasteryLevel))
	{
		ChainableAttacks = &WeaponCombinationsForWeaponMastery->DefaultAttacks;
		if (TargetCombatant)
			if (auto WeaponCombinationsAgainstWeapon = WeaponCombinationsForWeaponMastery->AttacksAgainstWeapon.Find(EnemyCombatStyle))
				ChainableAttacks = WeaponCombinationsAgainstWeapon;
	}
	
	if (!ensure(ChainableAttacks))
		return false;
	
	// criterias:
	// 1. distance
	// 2. enemy active weapon type
	// 3. active enemy attack at the moment (if any)
	// 4. enemy stamina and health
	// pizdec

	const float RawAttackRange = OwnerCombatant->GetAttackRange();
	float AttackRange = AddIntelligenceMisperception(RawAttackRange, Intelligence, CombatSettings->AIAttackRangeIntelligenceMisperceptionFactor);
	bool bNeedLongRangeAttack = TargetDistance - AttackRange > CombatSettings->AIRangeDiffForLongAttack;
	// TODO include active enemy attack into search for best moveset?
	EMeleeAttackType ActiveEnemyAttack = TargetCombatant->GetActiveAttack();
	EMeleeAttackType ActiveEnemyAttackTrajectory = TargetCombatant->GetActiveAttackTrajectory();

	// TODO figure out some smarter way for starting attack. Ideally AI should analyze active stance
	EMeleeAttackType NewAttack = EMeleeAttackType::None;
	TArray<EMeleeAttackType> PossibleAttacks;
	
	if (PreviousAttack != EMeleeAttackType::None)
	{
		auto PossibleContinuations = ChainableAttacks->ChainableAttacks.Find(PreviousAttack);
		if (PossibleContinuations && PossibleContinuations->AttackTypes.Num() > 0)
			PossibleAttacks = PossibleContinuations->AttackTypes;		
	}

	if (PossibleAttacks.Num() == 0)
	{
		PossibleAttacks.Reserve(ChainableAttacks->ChainableAttacks.Num());
		for (const auto& ChainableAttack : ChainableAttacks->ChainableAttacks)
			PossibleAttacks.Add(ChainableAttack.Key);
	}

	NewAttack = PossibleAttacks[FMath::RandRange(0, PossibleAttacks.Num() - 1)];		

	// currently attack animations are in states of an ABP
	// however, there is no way to handle a situation when you need to chain a horizontal swing into the same horizontal swing - the ABP will just think that it is already
	// in the required state and hence NPC won't continue attack but AI controller will stuck because it waits for calls from the NPC attack gameplay ability
	// which awaits a call from this component, and this component waits for anim notifies, which will never occur in this case, to call the component state functions
	if (PreviousAttack < EMeleeAttackType::Type && NewAttack == PreviousAttack)
	{
		do NewAttack = PossibleAttacks[FMath::RandRange(0, PossibleAttacks.Num() - 1)];
		while (NewAttack == PreviousAttack);
	}
	
	const FVector AttackVector = Target->GetActorLocation() - GetOwner()->GetActorLocation();
	const FVector Direction = AttackVector.GetSafeNormal();
	const float ApproachTime = 0.25f; // TODO parametrize
	FVector Acceleration = bNeedLongRangeAttack ? AttackVector / ApproachTime : FVector::ZeroVector;
	OwnerCombatant->OnAttackRequested(NewAttack, Direction, Acceleration);
	EAttackStepDirection AttackStepDirection = bNeedLongRangeAttack ? EAttackStepDirection::Forward : EAttackStepDirection::None;
	UE_VLOG(AIController.Get(), LogCombat, Verbose, TEXT("Npc starting attack %s"), *UEnum::GetValueAsString(NewAttack));
	RequestedAttacksCount++;
	CombatAnimInstance->SetAttack(NewAttack, Acceleration, AttackStepDirection, RequestedAttacksCount);
	OnAttackStartedEvent.Broadcast(NewAttack);
	if (NewAttack <= EMeleeAttackType::SpinRightOberhauw)
		ActiveAttackTrajectory = NewAttack;
	
	ActiveAttack = NewAttack;
	PreviousAttack = NewAttack;
	return true;
}

void UNpcMeleeCombatComponent::ResetAttackState()
{
	Super::ResetAttackState();
	RequestedAttacksCount = 0;
	PreviousAttack = EMeleeAttackType::None;
}
