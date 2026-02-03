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
	bool bCanRequest = Super::RequestAttack(RequestedAttackType);
	// kind of bullshit to get it everytime, 
	// but the alternative is to provide SetWeaponMasteryLevel to be called to owner when NPC takes another weapon or improves its skill
	// TODO 16.01.2026 actually do that one day
	WeaponMasteryLevel = OwnerCombatant->GetActiveWeaponMasteryLevel();

	return bCanRequest && RequestNextAttack(RequestedAttackType);
}

EMeleeAttackType UNpcMeleeCombatComponent::GetNextAttack(const ICombatant* TargetCombatant,
                                                         const FGameplayTag& OwnerCombatStyle, const FGameplayTag& EnemyCombatStyle, const UMeleeCombatSettings* CombatSettings) const
{
	EMeleeAttackType NewAttack = EMeleeAttackType::None;
	FNpcCombatSituationKey Key(OwnerCombatStyle, EnemyCombatStyle, WeaponMasteryLevel);
	const auto WeaponCombinationsForNpcWeapon = CombatSettings->AIWeaponTypeChainableAttacks.Find(OwnerCombatStyle);
	if (!WeaponCombinationsForNpcWeapon)
	{
		ensure(false);
		return NewAttack;
	}
		
	const FChainableAttacksWrapper* ChainableAttacks = &WeaponCombinationsForNpcWeapon->DefaultAttacks;
	if (auto WeaponCombinationsForWeaponMastery = WeaponCombinationsForNpcWeapon->MasteryLevelChainableAttacks.Find(WeaponMasteryLevel))
	{
		ChainableAttacks = &WeaponCombinationsForWeaponMastery->DefaultAttacks;
		if (EnemyCombatStyle.IsValid())
			if (auto WeaponCombinationsAgainstWeapon = WeaponCombinationsForWeaponMastery->AttacksAgainstWeapon.Find(EnemyCombatStyle))
				ChainableAttacks = WeaponCombinationsAgainstWeapon;
	}
		
	if (!ensure(ChainableAttacks))
		return NewAttack;
		
	// TODO include active enemy attack into search for best moveset?
	EMeleeAttackType ActiveEnemyAttack = TargetCombatant != nullptr ? TargetCombatant->GetActiveAttack() : EMeleeAttackType::None;
	EMeleeAttackType ActiveEnemyAttackTrajectory = TargetCombatant != nullptr ? TargetCombatant->GetActiveAttackTrajectory() : EMeleeAttackType::None;

	// TODO figure out some smarter way for starting attack. Ideally AI should analyze active stance
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
	
	return NewAttack;
}

bool UNpcMeleeCombatComponent::RequestNextAttack(EMeleeAttackType NewAttack)
{
	// if (AttackPhase != EMeleeAttackPhase::None && !bComboWindowActive)
	// 	return true; // not really the best way to return "you requested next attack in incorrect timing", but the alternative will lead to aborting attack

	// TODO bullshit, it shouldn't be a problem, NPC should be able to just swing ahead for whatever reason (i.e. player is in invisibility or in the dark)
	auto Target = AIController->GetFocusActor();
	ICombatant* TargetCombatant = Target != nullptr ? Cast<ICombatant>(Target) : nullptr;
	FGameplayTag OwnerCombatStyle = OwnerCombatant->GetActiveCombatStyleTag();
	FGameplayTag EnemyCombatStyle = TargetCombatant != nullptr ? TargetCombatant->GetActiveCombatStyleTag() : FGameplayTag::EmptyTag;
	ensure(OwnerCombatStyle.IsValid());
	
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	const float Intelligence = OwnerNPCCombatant->GetIntelligence();
	const float TrueTargetDistance = Target != nullptr ? (GetOwner()->GetActorLocation() - Target->GetActorLocation()).Size() : 200.f;
	float TargetDistance = AddIntelligenceMisperception(TrueTargetDistance, Intelligence, CombatSettings->AITargetDistanceIntelligenceMisperceptionFactor);
	const float RawAttackRange = OwnerCombatant->GetAttackRange();
	float AttackRange = AddIntelligenceMisperception(RawAttackRange, Intelligence, CombatSettings->AIAttackRangeIntelligenceMisperceptionFactor);
	if (NewAttack == EMeleeAttackType::None)
		NewAttack = GetNextAttack(TargetCombatant, OwnerCombatStyle, EnemyCombatStyle, CombatSettings);
	
	bool bNeedLongRangeAttack = ShouldMakeLongRangeAttack(Target, TargetDistance, AttackRange, CombatSettings);
	
	UE_VLOG(GetOwner(), LogCombat, VeryVerbose, TEXT("UNpcMeleeCombatComponent::RequestNextAttack:"));
	UE_VLOG(GetOwner(), LogCombat, VeryVerbose, TEXT("Raw attack range = %.2f\nAssumed attack range = %.2f"), RawAttackRange, AttackRange);
	UE_VLOG(GetOwner(), LogCombat, VeryVerbose, TEXT("Raw distance to target = %.2f\nAssumed distance to target = %.2f"), TrueTargetDistance, TargetDistance);
	UE_VLOG(GetOwner(), LogCombat, VeryVerbose, TEXT("%s"), bNeedLongRangeAttack ? TEXT("Need long range attack") : TEXT("Don't need long range attack"));
	
	const FVector AttackVector = Target != nullptr ? Target->GetActorLocation() - GetOwner()->GetActorLocation() : GetOwner()->GetActorForwardVector();
	const FVector Direction = AttackVector.GetSafeNormal();
	const float ApproachTime = 0.25f; // TODO parametrize
	FVector Acceleration = bNeedLongRangeAttack ? AttackVector / ApproachTime : FVector::ZeroVector;
	OwnerCombatant->OnAttackRequested(NewAttack, Direction, Acceleration);
	EAttackStepDirection AttackStepDirection = bNeedLongRangeAttack ? EAttackStepDirection::Forward : EAttackStepDirection::None;
	UE_VLOG(AIController.Get(), LogCombat, Verbose, TEXT("Npc starting attack %s"), *UEnum::GetValueAsString(NewAttack));
	RequestedAttacksCount++;
	
#if WITH_EDITOR
	// debug 01.02.2026 REMOVE ASAP
	if (!Debug_ForcedAttacksSequence.IsEmpty())
	{
		NewAttack = Debug_ForcedAttacksSequence[ForcedAttackSequenceIndex];
		ForcedAttackSequenceIndex = (ForcedAttackSequenceIndex + 1) % Debug_ForcedAttacksSequence.Num();
	}
#endif

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
	FTimerHandle Timer;
	auto Lambdos = [this]()
	{
		if (IsValid(this))
			PreviousAttack = EMeleeAttackType::None;
	};
	
	GetWorld()->GetTimerManager().SetTimer(Timer, Lambdos, 0.25f, false);
}

bool UNpcMeleeCombatComponent::ShouldMakeLongRangeAttack(AActor* Target, float TargetDistance, float AttackRange, const UMeleeCombatSettings* Settings) const
{
	return TargetDistance - AttackRange > Settings->AIRangeDiffForLongAttack;
}