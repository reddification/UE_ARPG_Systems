// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerSwingControlCombatComponent.h"

#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/PlayerCombat.h"

UPlayerSwingControlCombatComponent::UPlayerSwingControlCombatComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Called when the game starts
void UPlayerSwingControlCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	auto PlayerCombat = Cast<IPlayerCombatant>(GetOwner());
	if (!ensure(PlayerCombat))
		return;
	
	OwnerPlayerCombat.SetInterface(PlayerCombat);
	OwnerPlayerCombat.SetObject(GetOwner());

	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	MeleeCombatParameters = CombatSettings->MeleeCombatDirectionalInputParameters;
	CurrentPlayerInputs.SetNumZeroed(MeleeCombatParameters.InputBufferSize);

	SetComponentTickInterval(1.f / MeleeCombatParameters.TickRate);

	RadiansToAttackMapping =
	{
		FAttackDirectionToAttackTypeMapping { 0.f, EMeleeAttackType::RightMittelhauw },
		FAttackDirectionToAttackTypeMapping { 2.f * PI, EMeleeAttackType::RightMittelhauw },
		FAttackDirectionToAttackTypeMapping { PI / 4.f, EMeleeAttackType::RightOberhauw },
		FAttackDirectionToAttackTypeMapping { PI / 2.f, EMeleeAttackType::Thrust },
		FAttackDirectionToAttackTypeMapping { 3.f * PI / 4.f, EMeleeAttackType::LeftOberhauw },
		FAttackDirectionToAttackTypeMapping { PI, EMeleeAttackType::LeftMittelhauw },
		FAttackDirectionToAttackTypeMapping {  5.f * PI / 4.f, EMeleeAttackType::LeftUnterhauw },
		FAttackDirectionToAttackTypeMapping {  3.f * PI / 2.f, EMeleeAttackType::VerticalSlash },
		FAttackDirectionToAttackTypeMapping {  7.f * PI / 4.f, EMeleeAttackType::RightUnterhauw },
	};
}

void UPlayerSwingControlCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	auto NewMovementInput = OwnerPlayerCombat->GetCombatMovementDirection();
	if (!NewMovementInput.IsNearlyZero())
	{
		if (CurrentAccelerationSignificance > 0.f)
			CurrentAccelerationSignificance -= DeltaTime * MeleeCombatParameters.AttackDirectionAccelerationSignificanceDecayRate;
	
		FVector NewAccelerationNormalized = NewMovementInput.GetSafeNormal();
		if ((NewAccelerationNormalized | CurrentAcceleration) < MeleeCombatParameters.ConsiderableAccelerationDifferenceDotProduct)
		{
			CurrentAcceleration = NewAccelerationNormalized;
			CurrentAccelerationSignificance = MeleeCombatParameters.TargetEvaluationAccelerationSignificanceBase;
		}
	}
	else
	{
		if (CurrentAccelerationSignificance > 0.f)
			CurrentAccelerationSignificance -= DeltaTime * MeleeCombatParameters.AttackDirectionAccelerationSignificanceDecayRate * 5.f;

		CurrentAcceleration = FVector::ZeroVector;
	}
	
	if (bRegisteringAttack)
	{
		// UpdateFocus(DeltaTime);
		auto NewAttackDirectionInput = OwnerPlayerCombat->ConsumeCurrentAttackInput();
		if (NewAttackDirectionInput.IsNearlyZero())
		{
			if (!bComboWindowActive)
				IncreaseNoInputFrames();
		}
		else
		{
			RegisterNewInput(NewAttackDirectionInput);
		}
	}
	
	EAttackStepDirection CurrentAttackStepDirection = GetAttackStepDirection(CurrentAcceleration.GetSafeNormal2D(), GetOwner()->GetActorForwardVector());
	AttackStepDirectionUpdatedEvent.Broadcast(CurrentAttackStepDirection);
	
	TArray<FAccumulatedAttackVector> Dump;
	EMeleeAttackType CurrentlyAccumulatedAttack = GetBasicAccumulatedAttack(Dump);
	float x = (GetAttackActivationInputsCount() + MeleeCombatParameters.InputBufferSize) / 2.f;
	float CurrentAccumulationProgress = static_cast<float>(CurrentAttackFrames) / x;
	AttackInputProgressUpdatedEvent.Broadcast(CurrentlyAccumulatedAttack, CurrentAccumulationProgress);
}

// Not used for now
void UPlayerSwingControlCombatComponent::UpdateFocus(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPlayerSwingControlCombatComponent::UpdateFocus)
	TSet<AActor*> DangerousEnemies = OwnerCombatant->GetDangerousEnemies();
	if (DangerousEnemies.Num() <= 0)
		return;
	
	const double DistSqBetweenFocusedEnemyAndOwner = FocusTarget.IsValid()
		? FVector::DistSquared(FocusTarget->GetActorLocation(), GetOwner()->GetActorLocation())
		: DBL_MAX;
	
	const bool bUpdateFocus = !FocusTarget.IsValid() || !DangerousEnemies.Contains(FocusTarget.Get())
		|| (DangerousEnemies.Num() > 1 && FMath::Abs(DistSqBetweenFocusedEnemyAndOwner - LastDistSqBetweenFocusedEnemyAndOwner) > DistSqToUpdateFocusedEnemy);

	AActor* BestTarget = nullptr;
	if (bUpdateFocus)
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		GetOwner()->GetActorEyesViewPoint(ViewLocation, ViewRotation);
		FVector OwnerFV = GetOwner()->GetActorForwardVector();
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(GetOwner());

		float BestScore = -FLT_MAX;
		
		for (auto* DangerousEnemy : DangerousEnemies)
		{
			if (!ensure(DangerousEnemy))
				continue;
		
			FHitResult TraceResult;
			FVector EnemyLocation = DangerousEnemy->GetActorLocation();
			bool bTraced = GetWorld()->LineTraceSingleByChannel(TraceResult, ViewLocation, EnemyLocation, ECC_Visibility, CollisionQueryParams);
			if (bTraced && TraceResult.GetActor() == DangerousEnemy)
			{
				const float DotProduct = OwnerFV | (EnemyLocation - ViewLocation).GetSafeNormal();
				if (DotProduct > 0.5f) // if in -60; +60 degrees range
				{
					const float IdealDistance = 100.f; // TODO should it be attack range? 75% of attack range?
					const float Score = DotProduct + IdealDistance / TraceResult.Distance;
					if (Score > BestScore)
					{
						BestScore = Score;
						BestTarget = DangerousEnemy;
					}
				}
			}
		}
	}
	
	if (BestTarget && BestTarget != FocusTarget)
	{
		OwnerPlayerCombat->SetCombatFocus(FocusTarget.Get());
		FocusTarget = BestTarget;
		LastDistSqBetweenFocusedEnemyAndOwner = DistSqBetweenFocusedEnemyAndOwner;
	}
	else if (BestTarget == nullptr && FocusTarget.IsValid())
	{
		OwnerPlayerCombat->ResetCombatFocus();
		FocusTarget.Reset();
	}
}

bool UPlayerSwingControlCombatComponent::RequestAttack(EMeleeAttackType RequestedAttackType)
{
	if (!Super::RequestAttack(EMeleeAttackType::LightAttack))
		return false;
	
	// SetFocusOnBestTarget();
	ensure(!bPlayerRequestsAttack);
	bPlayerRequestsAttack = true;
	ensure(AttackPhase == EMeleeAttackPhase::None);
	OnAttackStarted();
	return true;
}

void UPlayerSwingControlCombatComponent::RequestReleaseAttack()
{
	if (bPlayerRequestsAttack)
	{
		bPlayerRequestsAttack = false;
		OnAttackReleased();
	}
}

void UPlayerSwingControlCombatComponent::RequestReactivateAttack()
{
	Super::RequestReactivateAttack();
	if (!ensure(!bPlayerRequestsAttack))
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
			TEXT("UPlayerSwingControlCombatComponent::RequestReactivateAttack: bPlayerRequestsAttack == true when expected false"));
	}
	// ensure(!bPlayerRequestsAttack);
	bPlayerRequestsAttack = true;
	if (bComboWindowActive)
		SetRegisteringAttack(true);
}

void UPlayerSwingControlCombatComponent::StartComboWindow(const uint32 AttackAnimationId)
{
	Super::StartComboWindow(AttackAnimationId);
	UE_LOG(LogCombat, Log, TEXT("Combo window started"));
	if (bPlayerRequestsAttack)
		SetRegisteringAttack(true);
}

void UPlayerSwingControlCombatComponent::EndComboWindow(const uint32 AttackAnimationId)
{
	// this check is required, because when attacks are chained, StartComboWindow of new attack can appear before EndComboWindow of previous attack
	// And hence EndComboWindow of previous attack gets triggered and closes combo window of the new attack
	if (AttackAnimationId != ActiveComboWindowId)
		return;
	
	Super::EndComboWindow(AttackAnimationId);

	UE_LOG(LogCombat, Log, TEXT("Combo window ended"));
	if (bRegisteringAttack && bPlayerRequestsAttack)
	{
		Attack();
		bPlayerRequestsAttack = false;
	}
}

void UPlayerSwingControlCombatComponent::BeginWindUp(float TotalDuration, const uint32 AttackAnimationId, EMeleeAttackType WindupAttackType)
{
	// Since some animations can have the WindupAttackType not set (just because somebody forgot to do it) SwingControl component already has it when it requests attack
	// so just route it through
	Super::BeginWindUp(TotalDuration, AttackAnimationId, ActiveAttack);
	OwnerPlayerCombat->SetLookEnabled(true);
	// OwnerPlayerCombat->SetOrientationFollowsAttack(true);
	OwnerPlayerCombat->EnableAttackCameraDampering(MeleeCombatParameters.MaxCameraRotationAngleDuringAttack, MeleeCombatParameters.InitialLookRatioDuringAttack);
}

void UPlayerSwingControlCombatComponent::BeginRelease(float TotalDuration, const uint32 AttackAnimationId)
{
	// if (AttackPhase != EMeleeAttackPhase::WindUp)
	// 	OwnerPlayerCombat->SetOrientationFollowsAttack(true);
	
	Super::BeginRelease(TotalDuration, AttackAnimationId);
	// OwnerPlayerCombat->SetLookEnabled(true);
	// OwnerPlayerCombat->SetLookDampering(MaxCameraRotationAngleDuringAttack, LookDamperingRatio);
}

void UPlayerSwingControlCombatComponent::BeginRecover(float TotalDuration, const uint32 AttackAnimationId)
{
	Super::BeginRecover(TotalDuration, AttackAnimationId);
	// if (bPlayerRequestsAttack)
	// 	SetRegisteringAttack(true);

	OwnerPlayerCombat->SetLookEnabled(false);
	OwnerPlayerCombat->DisableAttackCameraDampering();
}

void UPlayerSwingControlCombatComponent::EndRecover(const uint32 AnimationId)
{
	// if (AttackPhase == EMeleeAttackPhase::Recover)
	// 	OwnerPlayerCombat->SetOrientationFollowsAttack(false);
	
	Super::EndRecover(AnimationId);
}

void UPlayerSwingControlCombatComponent::ResetAttackState()
{
	Super::ResetAttackState();
	SetRegisteringAttack(false);
	OwnerPlayerCombat->SetLookEnabled(true);
	// OwnerPlayerCombat->SetOrientationFollowsAttack(false);
	OwnerPlayerCombat->DisableAttackCameraDampering();
	bPlayerRequestsAttack = false;
	LastDistSqBetweenFocusedEnemyAndOwner = 0.f;
}

bool UPlayerSwingControlCombatComponent::Feint()
{
	bool bFeinted = Super::Feint();
	return bFeinted;
}

void UPlayerSwingControlCombatComponent::OnAttackStarted()
{
	SetRegisteringAttack(true);
}

void UPlayerSwingControlCombatComponent::OnAttackReleased()
{
	if (bRegisteringAttack)
		Attack();
}

void UPlayerSwingControlCombatComponent::IncreaseNoInputFrames()
{
	NoInputFrames++;
	if (NoInputFrames > MeleeCombatParameters.FramesWithoutInputToResetSwing && AttackPhase == EMeleeAttackPhase::None)
	{
		ResetAttackAccumulationData();
		ResetAttackState();
		OnAttackEndedEvent.Broadcast();
	}

	if (!bRegisteringAttack)
	{
		// SetComponentTickEnabled(false);
	}
}

void UPlayerSwingControlCombatComponent::ResetAttackAccumulationData()
{
	CurrentPlayerInputs.Reset();
	CurrentPlayerInputs.SetNumZeroed(MeleeCombatParameters.InputBufferSize);
	CurrentAttackFrames = 0;
	CurrentRegisteredInputIndex = 0;
	NoInputFrames = 0;
}

void UPlayerSwingControlCombatComponent::RegisterNewInput(const FVector2d& Input)
{
	CurrentPlayerInputs[CurrentRegisteredInputIndex] = Input;
	CurrentPlayerInputs[CurrentRegisteredInputIndex].Y *= MeleeCombatParameters.VerticalInputFactor;
	CurrentRegisteredInputIndex = (CurrentRegisteredInputIndex + 1) % MeleeCombatParameters.InputBufferSize;
	CurrentAttackFrames++;
	if (CurrentAttackFrames >= MeleeCombatParameters.InputBufferSize)
	{
		Attack();
	}
}

EMeleeAttackType UPlayerSwingControlCombatComponent::GetBasicAccumulatedAttack(TArray<FAccumulatedAttackVector>& AccumulatedVectors) const
{
	int a = 0;
	while (a < MeleeCombatParameters.InputBufferSize)
	{
		FAccumulatedAttackVector AttackVector;
		for (int i = 0; i < MeleeCombatParameters.AttackFrameAccumulation && a < MeleeCombatParameters.InputBufferSize; i++, a++)
		{
			int index = CurrentRegisteredInputIndex - a < 0
				? CurrentRegisteredInputIndex - a + (MeleeCombatParameters.InputBufferSize - 1)
				: CurrentRegisteredInputIndex - a;
			AttackVector.RawInput += CurrentPlayerInputs[index];
		}

		if (AttackVector.RawInput.IsNearlyZero())
			break;

		AttackVector.NormalizedDirection = AttackVector.RawInput.GetSafeNormal();
		AttackVector.AngleRadians = FMath::Acos(AttackVector.NormalizedDirection | FVector2d(1.f, 0.f));
		if (AttackVector.RawInput.Y < 0)
			AttackVector.AngleRadians = 2.f * PI - AttackVector.AngleRadians;
		
		AttackVector.AngleDegrees = FMath::RadiansToDegrees(AttackVector.AngleRadians);
		AttackVector.RawInputSize = AttackVector.RawInput.Size();
		
		AccumulatedVectors.Add(AttackVector);
	}

	if (AccumulatedVectors.IsEmpty())
		return EMeleeAttackType::None;

	for (int i = 0; i < AccumulatedVectors.Num(); i++)
		AccumulatedVectors[i].AttackType = GetAttackType(AccumulatedVectors[i].AngleRadians);
	
	return AccumulatedVectors[0].AttackType;
}

EAttackStepDirection UPlayerSwingControlCombatComponent::GetAttackStepDirection(const FVector& NormalizedAcceleration, const FVector& ForwardVector) const
{
	EAttackStepDirection AttackStepDirection = EAttackStepDirection::None;
	if (CurrentAccelerationSignificance > MeleeCombatParameters.AccelerationSignificanceThreshold)
	{
		const float Acc2FVDotProduct = ForwardVector | NormalizedAcceleration;
		if (Acc2FVDotProduct > MeleeCombatParameters.AccelerationToMoveDirectionMatchDotProductThreshold)
			AttackStepDirection = EAttackStepDirection::Forward;
		else if (Acc2FVDotProduct < -MeleeCombatParameters.AccelerationToMoveDirectionMatchDotProductThreshold)
			AttackStepDirection = EAttackStepDirection::Back;
		else
		{
			FVector RightVector = GetOwner()->GetActorRightVector();
			float Acc2RVDotProduct = RightVector | NormalizedAcceleration;
			if (Acc2RVDotProduct > MeleeCombatParameters.AccelerationToMoveDirectionMatchDotProductThreshold)
				AttackStepDirection = EAttackStepDirection::Right;
			else if (Acc2RVDotProduct < -MeleeCombatParameters.AccelerationToMoveDirectionMatchDotProductThreshold)
				AttackStepDirection = EAttackStepDirection::Left;
		}
	}
	return AttackStepDirection;
}

int UPlayerSwingControlCombatComponent::GetAttackActivationInputsCount() const
{
	int AccumulatedVectorActivationInputsCount = 8;
	int WeaponMastery = OwnerCombatant->GetActiveWeaponMasteryLevel();
	if (MeleeCombatParameters.WeaponMasteryToMinAttackInputs.Contains(WeaponMastery))
		AccumulatedVectorActivationInputsCount = MeleeCombatParameters.WeaponMasteryToMinAttackInputs[WeaponMastery];
	
	return AccumulatedVectorActivationInputsCount;
}

void UPlayerSwingControlCombatComponent::Attack()
{
	if (!ensure(CurrentPlayerInputs.Num() == MeleeCombatParameters.InputBufferSize))
		return;

	const int AccumulatedVectorActivationInputsCount = GetAttackActivationInputsCount();
	if (CurrentAttackFrames < AccumulatedVectorActivationInputsCount)
	{
		if (AttackPhase == EMeleeAttackPhase::None)
			FinalizeAttack();
		
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("Not enough inputs to perform an attack"));
		return;
	}
	
	TArray<FAccumulatedAttackVector> AccumulatedVectors;
	EMeleeAttackType FinalAttackType = GetBasicAccumulatedAttack(AccumulatedVectors);
	if (FinalAttackType == EMeleeAttackType::None && AttackPhase == EMeleeAttackPhase::None)
	{
		FinalizeAttack();
		return;
	}
	
	DeduceComplexAttackInput(AccumulatedVectors, FinalAttackType);
	
	// OwnerCombatant->PerformAttack(FinalAttackType, AttackDirection, CurrentAcceleration * CurrentAccelerationSignificance);
	
	FVector NormalizedAcceleration = CurrentAcceleration.GetSafeNormal();
	FVector RelativeAttackAcceleration = CurrentAcceleration * CurrentAccelerationSignificance;
	FVector ForwardVector = GetOwner()->GetActorForwardVector();
	EAttackStepDirection AttackStepDirection = GetAttackStepDirection(NormalizedAcceleration, ForwardVector);

	ActiveAttack = FinalAttackType;
	ActiveAttackTrajectory = FinalAttackType;
	
	OwnerCombatant->OnAttackRequested(FinalAttackType, ForwardVector, RelativeAttackAcceleration);
	CombatAnimInstance->SetAttack(FinalAttackType, RelativeAttackAcceleration, AttackStepDirection, CurrentComboTotalAttacksCount);
	OnAttackStartedEvent.Broadcast(FinalAttackType);
	SetRegisteringAttack(false);
}

void UPlayerSwingControlCombatComponent::MergeAttacks(TArray<FAccumulatedAttackVector>& AccumulatedVectors) const
{
	const float ReverseMinConsiderableDifferenceInInputScales = 1.f / MeleeCombatParameters.MinConsiderableDifferenceInInputScales;
	for (int i = 0; i < AccumulatedVectors.Num() - 1; i++)
	{
		// one accumulated vector can absorb another one if one is much bigger that the other
		const float RawInputDifference = AccumulatedVectors[i].RawInputSize / AccumulatedVectors[i + 1].RawInputSize;
		if (RawInputDifference > MeleeCombatParameters.MinConsiderableDifferenceInInputScales)
		{
			AccumulatedVectors[i + 1] = AccumulatedVectors[i];
		}
		else if (MeleeCombatParameters.bAbsorbNewerAttacksByOlder && RawInputDifference < ReverseMinConsiderableDifferenceInInputScales)
		{
			AccumulatedVectors[i] = AccumulatedVectors[i + 1];
		}
	}
}

EMeleeAttackType UPlayerSwingControlCombatComponent::GetAttackType(float Radians) const
{
	const float HalfStep = (PI / 8.f);
	for (const auto& RadiansToAttack : RadiansToAttackMapping)
	{
		if (FMath::Abs(RadiansToAttack.Radians - Radians) < HalfStep)
			return RadiansToAttack.Attack;
	}

	ensure(false);
	return EMeleeAttackType::LeftMittelhauw;
}

void UPlayerSwingControlCombatComponent::DeduceComplexAttackInput(const TArray<FAccumulatedAttackVector>& AccumulatedVectors, EMeleeAttackType& FinalAttackType) const
{
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	for (const auto& MultiAttackMapping : CombatSettings->MultipleAttacksToAttackCombination)
	{
		if (AccumulatedVectors.Num() < MultiAttackMapping.MinVectors)
			continue;

		int av = AccumulatedVectors.Num() - 1;
		while (av > 1 && FMath::Abs(AccumulatedVectors[av].AngleDegrees - MultiAttackMapping.InitialStep.Angle) > MultiAttackMapping.InitialStep.AngleThreshold)
		{
			av--;
		}
		
		if (av <= 1)
			continue;
		
		FVector2D VectorSum = FVector2D::ZeroVector;
		for (; av >= 0; av--)
		{
			VectorSum += AccumulatedVectors[av].NormalizedDirection;
			VectorSum.Normalize(); // TODO do i really need to normalize each iteration?
		}

		float VectorAngle = FMath::RadiansToDegrees(FMath::Acos(VectorSum | FVector2D(1.f, 0.f)));
		if (VectorSum.Y < 0)
			VectorAngle = 360.f - VectorAngle;
		
		if (FMath::Abs(FMath::UnwindDegrees(VectorAngle - MultiAttackMapping.ExpectedResultAngleDegree)) < MultiAttackMapping.AngleThreshold)
		{
			FinalAttackType = MultiAttackMapping.ResultAttackType;
			return;
		}
	}
}

void UPlayerSwingControlCombatComponent::SetRegisteringAttack(bool bEnabled)
{
	bRegisteringAttack = bEnabled;
	// SetComponentTickEnabled(bEnabled);
	OwnerPlayerCombat->SetLookEnabled(false);
	if (!bEnabled)
		ResetAttackAccumulationData();
	
	RegisteringAttackStateChangedEvent.Broadcast(bEnabled);
}