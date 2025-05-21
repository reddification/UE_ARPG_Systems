// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerMovesetCombatComponent.h"

#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/LockableTarget.h"
#include "Interfaces/PlayerCombat.h"

void UPlayerMovesetCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	// OwnerCombatant->OnCombatantWeaponChanged.BindUObject(this, &UPlayerMovesetCombatComponent::OnWeaponChanged);
	// CacheMoveset();

	auto PlayerCombat = Cast<IPlayerCombatant>(GetOwner());
	if (!ensure(PlayerCombat))
		return;
	
	OwnerPlayerCombat.SetInterface(PlayerCombat);
	OwnerPlayerCombat.SetObject(GetOwner());
	CombatAnimInstance->OnLastComboAttackEvent.BindUObject(this, &UPlayerMovesetCombatComponent::OnLastComboAttack);
	SetComponentTickEnabled(false);
}

void UPlayerMovesetCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ActiveAttack == EMeleeAttackType::None) // can happen when knocked down
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	// if (!ensure(Target.IsValid()))
	// 	return;
	
	// TODO adjust attacker positioning to target IF there is no root motion
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FRotator TargetRotation;

	FRotator CurrentRotation = GetOwner()->GetActorForwardVector().Rotation();
	
	// Player rotation priority:
	// 1. Manual target (from target lock component, for example)
	// 2. Manual player input
	// 3. Auto target (from overlap sphere)
	// 4. Player camera view direction
	UE_VLOG_CAPSULE(GetOwner(), LogCombatMovesetComponent, VeryVerbose, GetOwner()->GetActorLocation() - FVector::UpVector * 45.f, 90, 30, FQuat::Identity, FColor::White, TEXT("Owner"));

	if (ManualTarget.IsValid())
	{
		UE_VLOG_CAPSULE(GetOwner(), LogCombatMovesetComponent, VeryVerbose, ManualTarget->GetActorLocation() - FVector::UpVector * 45.f, 90, 30, FQuat::Identity, FColor::Red, TEXT("Manual target"));
		TargetRotation = (ManualTarget->GetActorLocation() - OwnerLocation).Rotation();
	}
	else
	{
		FVector RequestedInputDirection = OwnerPlayerCombat->ConsumeCombatMovementRawInput();
		if (!RequestedInputDirection.IsNearlyZero())
		{
			LastValidPlayerInputMovementRequestRotator = InitialCombatantViewDirection.RotateAngleAxis(RequestedInputDirection.Rotation().Yaw, GetOwner()->GetActorUpVector()).ToOrientationRotator();
			TargetRotation = LastValidPlayerInputMovementRequestRotator;
			UE_VLOG_ARROW(GetOwner(), LogCombatMovesetComponent, VeryVerbose, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + TargetRotation.Vector() * 150.f, FColor::Orange, TEXT("RID attack direction"));
		}
		else if (!LastValidPlayerInputMovementRequestRotator.IsNearlyZero())
		{
			TargetRotation = LastValidPlayerInputMovementRequestRotator;
			UE_VLOG_ARROW(GetOwner(), LogCombatMovesetComponent, VeryVerbose, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + TargetRotation.Vector() * 150.f, FColor::Orange, TEXT("RID attack cached direction"));
		}
		else if (AutoTarget.IsValid())
		{
			TargetRotation = (AutoTarget->GetActorLocation() - OwnerLocation).Rotation();
			UE_VLOG_ARROW(GetOwner(), LogCombatMovesetComponent, VeryVerbose, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + TargetRotation.Vector() * 150.f, FColor::Orange, TEXT("Auto target attack direction"));
		}
		else
		{
			TargetRotation = OwnerPlayerCombat->GetPlayerCombatantViewDirection();
			UE_VLOG_ARROW(GetOwner(), LogCombatMovesetComponent, VeryVerbose, GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation() + TargetRotation.Vector() * 150.f, FColor::Orange, TEXT("View attack direction"));
		}
	}

	TargetRotation.Pitch = CurrentRotation.Pitch;
	TargetRotation.Roll = CurrentRotation.Roll;
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationRate);
	GetOwner()->SetActorRotation(NewRotation);
}

bool UPlayerMovesetCombatComponent::RequestAttack(EMeleeAttackType RequestedAttackType)
{
	bool bCanRequest = Super::RequestAttack(RequestedAttackType);
	if (!bCanRequest)
		return false;

	// TODO if requested next move in attack sequence more than N times - block the combo continuation? 
	if (ActiveAttack != EMeleeAttackType::None && !bComboWindowActive)
	{
		CurrentMisinputs++;
		return false;
	}

	if (CurrentMisinputs >= MisinputsOutsideOfComboWindowToResetAttack)
		return false;

	if (RequestedAttackType == EMeleeAttackType::LightAttack && bLastComboAttack)
		return false;

	// only light attack can skip each other (and even light attack is protected when it's the last in combo)
	if (RequestedAttackType == ActiveAttack && ActiveAttack != EMeleeAttackType::LightAttack)
		return false;
	
	OwnerPlayerCombat->ConsumeCombatMovementRawInput();
	
	ActiveAttack = RequestedAttackType;
	// if (RequestedAttackType == EMeleeAttackType::LightAttack)
	// @AK 03.05.2025 so i thought it was a good idea to only increment attack counter for light attacks, but now i think the opposite, whatever the action is - it should increment the attack counter 
	MovesetAttackIndex++;
	
	ManualTarget = OwnerCombatant->GetTarget();
	
	auto OwnerLocal = GetOwner();
	FVector OwnerLocation = OwnerLocal->GetActorLocation();

	FVector CurrentAcceleration = OwnerPlayerCombat->GetCombatMovementDirection().GetSafeNormal();
	FVector TargetDirection = CurrentAcceleration.IsNearlyZero() ? OwnerPlayerCombat->GetPlayerCombatantViewDirection().Vector() : CurrentAcceleration;

	if (!ManualTarget.IsValid())
		FindAutoTarget(OwnerLocal, OwnerLocation, TargetDirection);

	EAttackStepDirection AttackStepDirection = EAttackStepDirection::Forward;
	auto ActualTarget = ManualTarget.IsValid() ? ManualTarget.Get() : AutoTarget.Get();
	if (ActualTarget)
	{
		TargetDirection = (ActualTarget->GetActorLocation() - OwnerLocation).GetSafeNormal();	
		const float AttackRange = OwnerCombatant->GetAttackRange();
		const float DistanceToTargetSq = (ActualTarget->GetActorLocation() - OwnerLocation).SizeSquared();
		AttackStepDirection = DistanceToTargetSq < AttackRange * AttackRange ? EAttackStepDirection::None : EAttackStepDirection::Forward;
	}

	InitialCombatantViewDirection = OwnerPlayerCombat->GetPlayerCombatantViewDirection().Vector();
	InitialCombatantViewDirection.Z = 0;
	UE_VLOG(GetOwner(), LogCombat, Verbose, TEXT("Player starting attack %s"), *UEnum::GetValueAsString(RequestedAttackType));
	CombatAnimInstance->SetAttack(ActiveAttack, TargetDirection, AttackStepDirection, MovesetAttackIndex, FMath::RandRange(0, 10));

	return true;
}
void UPlayerMovesetCombatComponent::ResetAttackState()
{
	Super::ResetAttackState();
	MovesetAttackIndex = 0;
	CurrentMisinputs = 0;
	LastValidPlayerInputMovementRequestRotator = FRotator::ZeroRotator;
	InitialCombatantViewDirection = FVector::ZeroVector;
	OwnerPlayerCombat->ConsumeCombatMovementRawInput(); // might be redundant since it's called in RequestAttack
	OwnerPlayerCombat->SetOrientationFollowsAttack(false);
	SetComponentTickEnabled(false);
	AutoTarget.Reset();
	ManualTarget.Reset();
	bLastComboAttack = false;
}

void UPlayerMovesetCombatComponent::BeginWindUp(float TotalDuration, const uint32 AnimationId, EMeleeAttackType WindupAttackType)
{
	Super::BeginWindUp(TotalDuration, AnimationId, WindupAttackType);
	SetComponentTickEnabled(true);
	// Ok the problem is in combinations some attacks might completely skip wind up phase, hence no possibility of movement
	// this might be interesting though. Like you as a player have to actually control your actions more
	OwnerPlayerCombat->SetOrientationFollowsAttack(true);
}

void UPlayerMovesetCombatComponent::BeginRelease(float TotalDuration, const uint32 AnimationId)
{
	if (AttackPhase != EMeleeAttackPhase::WindUp)
		OwnerPlayerCombat->SetOrientationFollowsAttack(true);
		
	Super::BeginRelease(TotalDuration, AnimationId);
	LastValidPlayerInputMovementRequestRotator = FRotator::ZeroRotator;
	SetComponentTickEnabled(false);
}

void UPlayerMovesetCombatComponent::BeginRecover(float TotalDuration, const uint32 AnimationId)
{
	Super::BeginRecover(TotalDuration, AnimationId);
}

void UPlayerMovesetCombatComponent::EndRecover(const uint32 AnimationId)
{
	if (AttackPhase == EMeleeAttackPhase::Recover)
		OwnerPlayerCombat->SetOrientationFollowsAttack(false);
	
	Super::EndRecover(AnimationId);
}

void UPlayerMovesetCombatComponent::OnLastComboAttack()
{
	bLastComboAttack = true;
}

void UPlayerMovesetCombatComponent::FindAutoTarget(AActor* OwnerLocal, const FVector& OwnerLocation, const FVector& TargetDirection)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AutoTargetRadius);
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	auto World = GetWorld();
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	auto MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	for (const auto& TargetLockObjectChannel : MeleeCombatSettings->TargetLockObjectChannels)
		CollisionObjectQueryParams.AddObjectTypesToQuery(TargetLockObjectChannel.GetValue());
	
	bool bOverlapped = World->OverlapMultiByObjectType(Overlaps, OwnerLocation, FQuat::Identity, CollisionObjectQueryParams, CollisionShape,
		CollisionQueryParams);
		
	if (!bOverlapped)
		return;

	float BestDP = -FLT_MAX;
	AActor* BestTarget = nullptr;
	FCollisionShape SweepShape = FCollisionShape::MakeCapsule(OwnerCombatant->GetCombatantCapsuleRadius(), OwnerCombatant->GetCombatantCapsuleHalfHeight() * 0.85f);
	for (const auto& Overlap : Overlaps)
	{
		auto Actor = Overlap.GetActor();
		auto Targetable = Cast<ITargetable>(Actor);
		if (Targetable == nullptr || !Targetable->CanTarget(OwnerLocal))
			continue;

		FHitResult HitResult;
		bool bSwepped = World->SweepSingleByChannel(HitResult, OwnerLocation, Actor->GetActorLocation(),
													OwnerLocal->GetActorTransform().GetRotation(), ECC_Visibility, SweepShape, CollisionQueryParams);;
		if (bSwepped && HitResult.GetActor() == Actor)
		{
			float DotProduct = TargetDirection | ((Actor->GetActorLocation() - OwnerLocation).GetSafeNormal());
			if (DotProduct > BestDP)
			{
				BestDP = DotProduct;
				BestTarget = Actor;
			}
		}
	}
		
	AutoTarget = BestTarget;
}
