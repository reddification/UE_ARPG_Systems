#include "Components/MeleeBlockComponent.h"
#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Helpers/CombatCommonHelpers.h"
#include "Interfaces/ICombatant.h"

UMeleeBlockComponent::UMeleeBlockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMeleeBlockComponent::BeginPlay()
{
	Super::BeginPlay();
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	CollinearBlockInputsDotProductThreshold = CombatSettings->CollinearBlockInputsDotProductThreshold;
	StrongBlockActivationThreshold = CombatSettings->StrongBlockActivationThreshold;
	auto OwnerLocal = GetOwner();
	OwnerCombatant.SetObject(OwnerLocal);
	OwnerCombatant.SetInterface(Cast<ICombatant>(OwnerLocal));
	
	CombatAnimInstance = OwnerCombatant->GetCombatAnimInstance();
	BlockStrengthDecayRate = CombatSettings->BlockStrengthDecayRate;
	DecayDelay = CombatSettings->BlockDecayDelay;
	BlockStrengthAccumulationScale = CombatSettings->BlockStrengthAccumulationScale;
	BlockStrengthToParry = CombatSettings->BlockStrengthToParry;
	AttackerStrengthScaleWhenHitBlock = CombatSettings->AttackerStrengthScaleWhenHitBlock;
}

void UMeleeBlockComponent::StartBlocking()
{
	UE_VLOG(GetOwner(), LogCombat_Block, Verbose, TEXT("UMeleeBlockComponent::StartBlocking"));
	bRegisteringBlock = true;
	bBlockPeakNotified = false;
	AccumulatedBlock = FVector2d::ZeroVector;
	SetComponentTickEnabled(true);
	OwnerCombatant->SetBlocking(true);
	OnBlockActiveChangedEvent.Broadcast(true);
	const UMeleeCombatSettings* MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	float StaminaRatio = OwnerCombatant->GetStaminaRatio();
	if (auto BlockStaminaAccumulationScaleDependency = MeleeCombatSettings->BlockStaminaAccumulationScaleDependency.GetRichCurveConst())
		BlockInputAccumulationScale = BlockStaminaAccumulationScaleDependency->Eval(StaminaRatio);
	else
		BlockInputAccumulationScale = 0.15f;

	bool bUsingShield = OwnerCombatant->IsUsingShield();
	BlockStrength = bUsingShield ? BlockStrengthToParry + 0.1f : BlockStrengthToParry * 0.25f;
	OnBlockAccumulationChangedEvent.Broadcast(AccumulatedBlock, BlockStrength);
	if (bUsingShield)
		OnParryWindowActiveChangedEvent.Broadcast(true);
	
	CurrentDecayDelay = DecayDelay;
}

void UMeleeBlockComponent::StopBlocking()
{
	UE_VLOG(GetOwner(), LogCombat_Block, Log, TEXT("UMeleeBlockComponent::StopBlocking"));
	bRegisteringBlock = false;
	BlockStrength = 0.f;
	SetComponentTickEnabled(false);
	OwnerCombatant->SetBlocking(false);
	OnBlockActiveChangedEvent.Broadcast(false);
}

EBlockResult UMeleeBlockComponent::BlockAttack(const FVector& AttackDirection, float AttackerStrength, const FHitResult& HitResult,
	AActor* Attacker, const FMeleeAttackDebugInfo& AttackDebugInfo) const
{
	if (!bRegisteringBlock)
		return EBlockResult::None;

	FVector AccumulatedBlock3DRelative = FVector(1.f, AccumulatedBlock.X, AccumulatedBlock.Y);
	FVector AccumulatedBlock3D = GetOwner()->GetTransform().TransformVectorNoScale(AccumulatedBlock3DRelative).GetSafeNormal();
	const float dp = HitResult.ImpactNormal | AccumulatedBlock3D;
	bool bBlockDirectionBlocksAttack = dp >= -0.5f;

	auto OwnerLocal = GetOwner();
	
#if WITH_EDITOR
	UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("Attempting block:\nBlock strength = %.2f\nAttack direction: %s\nImpact normal: %s\nBlock input 3D: %s\ndp3=%.2f"),
		BlockStrength, *AttackDirection.ToString(), *HitResult.ImpactNormal.ToString(),
		*AccumulatedBlock3D.ToString(), dp);
	
	auto BlockCollisionSkelMesh = OwnerCombatant->GetBlockCollisionsComponent();
	TArray<USkeletalMeshComponent*> BlockCollisionMeshes = { BlockCollisionSkelMesh };
	auto CollisionShapeInfos = GetCombatCollisionShapes(FName("CombatCollision"), BlockCollisionMeshes, OwnerLocal);
	UE_VLOG_CAPSULE(OwnerLocal, LogCombat_Block, Verbose, BlockCollisionSkelMesh->GetComponentLocation(), CollisionShapeInfos[0].HalfHeight, CollisionShapeInfos[0].Radius,
		BlockCollisionSkelMesh->GetComponentQuat(), FColor::Cyan, TEXT("Block collision"));
	// UE_VLOG_CAPSULE(OwnerLocal, LogCombat_Block, Verbose, AttackDebugInfo.HitResult.TraceStart + AttackDebugInfo.Rotation.Vector() * AttackDebugInfo.HalfHeight, AttackDebugInfo.HalfHeight, AttackDebugInfo.Radius,
	// 	AttackDebugInfo.Rotation, FColor::Orange, TEXT("Sweep start"));
	// UE_VLOG_CAPSULE(OwnerLocal, LogCombat_Block, Verbose, AttackDebugInfo.HitResult.TraceEnd + AttackDebugInfo.Rotation.Vector() * AttackDebugInfo.HalfHeight, AttackDebugInfo.HalfHeight, AttackDebugInfo.Radius,
	// 	AttackDebugInfo.Rotation, FColor::Orange, TEXT("Sweep end"));
	UE_VLOG_LOCATION(OwnerLocal, LogCombat_Block, Verbose, HitResult.TraceStart, AttackDebugInfo.Radius, FColor::Orange, TEXT("Sweep start"));
	UE_VLOG_LOCATION(OwnerLocal, LogCombat_Block, Verbose, HitResult.TraceEnd, AttackDebugInfo.Radius, FColor::Orange, TEXT("Sweep end"));
	UE_VLOG_LOCATION(OwnerLocal, LogCombat_Block, Verbose, HitResult.ImpactPoint, 5.f, FColor::Red, TEXT("Hit impact point"));
	UE_VLOG_CAPSULE(OwnerLocal, LogCombat_Block, Verbose, Attacker->GetActorLocation() - FVector::UpVector * 90.f, 90.f, 25.f,
		FQuat::Identity, FColor::Black, TEXT("Attacker"));
	UE_VLOG_CAPSULE(OwnerLocal, LogCombat_Block, Verbose, OwnerLocal->GetActorLocation() - FVector::UpVector * 90.f, 90.f, 25.f,
		FQuat::Identity, FColor::White, TEXT("Defender"));
	UE_VLOG_ARROW(OwnerLocal, LogCombat_Block, Verbose, Attacker->GetActorLocation(), Attacker->GetActorLocation() + Attacker->GetActorForwardVector() * 100.f,
		FColor::Black, TEXT("Attacker FV"));
	UE_VLOG_ARROW(OwnerLocal, LogCombat_Block, Verbose, OwnerLocal->GetActorLocation(), OwnerLocal->GetActorLocation() + OwnerLocal->GetActorForwardVector() * 100.f,
		FColor::White, TEXT("Defender FV"));
	UE_VLOG_ARROW(OwnerLocal, LogCombat_Block, Verbose, HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.ImpactNormal * 100.f,
		FColor::White, TEXT("Hit impact normal"));
	// UE_VLOG_ARROW(OwnerLocal, LogCombat_Block, Verbose, HitResult.ImpactPoint, HitResult.ImpactPoint + HitResult.Normal * 100.f,
	// 	FColor::White, TEXT("Hit Normal"));
	UE_VLOG_ARROW(OwnerLocal, LogCombat_Block, Verbose, BlockCollisionSkelMesh->GetComponentLocation(), BlockCollisionSkelMesh->GetComponentLocation() + AccumulatedBlock3D * 100.f,
		FColor::White, TEXT("Block input"));
	
#endif
	
	if (!bBlockDirectionBlocksAttack)
	{
		UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("Attack not blocked"));
		return EBlockResult::None;
	}
	
	if (BlockStrength >= BlockStrengthToParry)
	{
		UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("Attack parried, [%.2f >= %.2f]"), BlockStrength, BlockStrengthToParry);
		OnAttackParriedEvent.Broadcast(Attacker);
		return EBlockResult::Parry;
	}
	
	float BlockScaledOwnerStrength = OwnerCombatant->GetStrength() * FMath::Max(BlockStrength, MinHeldBlockStrength);
	const float StrengthRatio = AttackerStrength * AttackerStrengthScaleWhenHitBlock / BlockScaledOwnerStrength;
	UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("Attack just blocked, [%.2f < %.2f]"), BlockStrength, BlockStrengthToParry);
	OnAttackBlockedEvent.Broadcast(StrengthRatio, Attacker);
	return EBlockResult::Block;
}

void UMeleeBlockComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ensure(bRegisteringBlock))
		return;

	float PreviousBlockStrength = BlockStrength;
	AddBlockInput(GetBlockInput(DeltaTime), DeltaTime);
	OnBlockAccumulationChangedEvent.Broadcast(AccumulatedBlock, BlockStrength);
	if (BlockStrength >= BlockStrengthToParry && PreviousBlockStrength < BlockStrengthToParry)
		OnParryWindowActiveChangedEvent.Broadcast(true);
	else if (BlockStrength < BlockStrengthToParry && PreviousBlockStrength >= BlockStrengthToParry)
		OnParryWindowActiveChangedEvent.Broadcast(false);
	
	CombatAnimInstance->SetBlockPosition(AccumulatedBlock);
	if (!bBlockPeakNotified && AccumulatedBlock.SizeSquared() >= 1)
	{
		bBlockPeakNotified = true;
		OwnerCombatant->OnBlockPeakReached(AccumulatedBlock);
	}
	
	UE_VLOG(GetOwner(), LogCombat_Block, VeryVerbose, TEXT("Current block position = %s"), *AccumulatedBlock.ToString());
	UE_VLOG(GetOwner(), LogCombat_Block, VeryVerbose, TEXT("Current block strength = %.2f"), BlockStrength);
}

void UMeleeBlockComponent::AddBlockInput(const FVector2D& BlockInput, float DeltaTime)
{
	FVector2D NewInputDirection = BlockInput.GetSafeNormal();
	FVector2D CurrentBlockDirection = AccumulatedBlock.GetSafeNormal();
	const float NewInputDotProduct =  CurrentBlockDirection | NewInputDirection;
	bool bDirectionCollinear = !BlockInput.IsNearlyZero() && !CurrentBlockDirection.IsNearlyZero() && NewInputDotProduct > CollinearBlockInputsDotProductThreshold;
	float BlockStrengthScore = NewInputDotProduct - CollinearBlockInputsDotProductThreshold;
	FVector2D AccumulatedBlockPreChange = AccumulatedBlock;

	auto OwnerLocal = GetOwner();
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("=========== Block ==========="));
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Block input = %s"), *BlockInput.ToString());
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Current block direction = %s"), *CurrentBlockDirection.ToString());
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("New input dot product = %.2f"), NewInputDotProduct);
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Direction collinear = %s"), bDirectionCollinear ? TEXT("true") : TEXT("false"));
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Block strength score = %.2f"), BlockStrengthScore);
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Accumulated block pre change = %s"), *AccumulatedBlock.ToString());
	
	AccumulatedBlock = AccumulatedBlock + BlockInput;
	AccumulatedBlock = AccumulatedBlock.ClampAxes(-1.f, 1.f);

	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Accumulated block post change = %s"), *AccumulatedBlock.ToString());

	const float DeltaBlockStrength = (AccumulatedBlock - AccumulatedBlockPreChange).Size();
	UE_VLOG(OwnerLocal, LogCombat_Block, VeryVerbose, TEXT("Delta block strength = %.2f"), DeltaBlockStrength);

	if (DeltaBlockStrength > KINDA_SMALL_NUMBER && bDirectionCollinear)
	{
		BlockStrength = BlockStrength + DeltaBlockStrength * DeltaTime * BlockStrengthScore * BlockStrengthAccumulationScale;
		CurrentDecayDelay = DecayDelay;
	}
	else if (NewInputDotProduct < -0.1)
	{
		UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("Resetting block strength"));
		BlockStrength = 0.f;
	}
	else
	{
		DecayBlock(DeltaTime);
	}

	UE_VLOG(OwnerLocal, LogCombat_Block, Verbose, TEXT("New block strength = %.2f"), BlockStrength);
}

void UMeleeBlockComponent::DecayBlock(float DeltaTime)
{
	if (CurrentDecayDelay > 0.f)
		CurrentDecayDelay -= BlockStrengthDecayRate * DeltaTime;
	else 
		BlockStrength = FMath::Max(MinHeldBlockStrength, BlockStrength - BlockStrengthDecayRate * DeltaTime);
		
	UE_VLOG(GetOwner(), LogCombat_Block, Verbose, TEXT("Block strength decaying"));
}

// experimental. doesn't really work good. (aki): currently unused as of 12 Feb 2026
FVector UMeleeBlockComponent::GetIncomingAttackDirection(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType)
{
	// all these 35.f, 15.f, 50.f corrections are just from the top of my head, don't consider them something well calculated
	const FVector AttackerLocation = AttackingActor->GetActorLocation() + FVector::UpVector * 35.f;
	const FVector AttackerUpVector = AttackingActor->GetActorUpVector();
	const FVector AttackerForwardVector = AttackingActor->GetActorForwardVector();
	const FVector AttackerRightVector = AttackingActor->GetActorRightVector();
	auto AttackerCombatant = Cast<ICombatant>(AttackingActor);
	const float AttackRange = AttackerCombatant->GetAttackRange();
	const float AdjustedAttackRange = AttackRange * 0.8f;
	const FVector ExpectedHitLocation = AttackerLocation + FVector::UpVector * 15.f + AttackerForwardVector * AttackRange;
	FVector AttackingPointOrigin = FVector::ZeroVector;
	switch (IncomingAttackType)
	{
		case EMeleeAttackType::None:
			ensure(false);
			break;
		case EMeleeAttackType::LeftUnterhauw:
			AttackingPointOrigin = AttackerLocation - AttackerUpVector * AdjustedAttackRange - AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::LeftMittelhauw:
			AttackingPointOrigin = AttackerLocation - AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::LeftOberhauw:
			AttackingPointOrigin = AttackerLocation + AttackerUpVector * AdjustedAttackRange - AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::Thrust:
			AttackingPointOrigin = AttackerLocation + AttackerForwardVector * AttackRange;
			break;
		case EMeleeAttackType::RightUnterhauw:
			AttackingPointOrigin = AttackerLocation - AttackerUpVector * AdjustedAttackRange + AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::RightMittelhauw:
			AttackingPointOrigin = AttackerLocation + AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::RightOberhauw:
			AttackingPointOrigin = AttackerLocation + AttackerUpVector * AdjustedAttackRange + AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::VerticalSlash:
			AttackingPointOrigin = AttackerLocation + AttackerUpVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::SpinLeftMittelhauw:
			AttackingPointOrigin = AttackerLocation - AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::SpinLeftOberhauw:
			AttackingPointOrigin = AttackerLocation + AttackerUpVector * AdjustedAttackRange - AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::SpinRightMittelhauw:
			AttackingPointOrigin = AttackerLocation + AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::SpinRightOberhauw:
			AttackingPointOrigin = AttackerLocation + AttackerUpVector * AdjustedAttackRange + AttackerRightVector * AdjustedAttackRange;
			break;
		case EMeleeAttackType::Max:
			ensure(false);
			break;
		default:
			ensure(false);
			return FMath::VRand();
			break;
	}

	return (ExpectedHitLocation - AttackingPointOrigin).GetSafeNormal();
}

FVector2D UMeleeBlockComponent::GetDesiredBlockVector(EMeleeAttackType IncomingAttackType) const
{
	bool bWithShield = OwnerCombatant->IsUsingShield();
	switch (IncomingAttackType)
	{
		case EMeleeAttackType::None:
			UE_VLOG(GetOwner(), LogCombat_Block, Error, TEXT("Something is wrong. incoming attack type is none"));
			break;
		case EMeleeAttackType::LeftUnterhauw:
			return FVector2D(1.f, -1.f);
		case EMeleeAttackType::LeftMittelhauw:
		case EMeleeAttackType::SpinLeftMittelhauw:
			return FVector2D(1.f, 0.f);
		case EMeleeAttackType::LeftOberhauw:
		case EMeleeAttackType::SpinLeftOberhauw:
			return bWithShield ? FVector2D(1.f, 1.f) : FVector2D(-1.f, 1.f);
		case EMeleeAttackType::Thrust:
			return FVector2D(0.f, 0.f);
		case EMeleeAttackType::RightUnterhauw:
			return FVector2D(-1.f, -1.f);
		case EMeleeAttackType::SpinRightMittelhauw:
		case EMeleeAttackType::RightMittelhauw:
			return FVector2D(-1.f, 0.f);
		case EMeleeAttackType::RightOberhauw:
		case EMeleeAttackType::SpinRightOberhauw:
			return bWithShield ? FVector2D(-1.f, 1.f) : FVector2D(1.f, 1.f);
		case EMeleeAttackType::VerticalSlash:
			// 12 Feb 2026 (aki): deliberately returning [+-1.f, 1.f] and not [0.f, 1.f] when NOT with shield,
			// because with my current aim offset based animations (poses), [+-1.f, 1.f] covers both [+-1.f. 1.f] and [0.f, 1.f],
			// while aim offset at [0.f, 1.f] either requires a separate pose (which would look almost like [+-1.f, 1.f] 
			// or would look and behave bad if it is just interpolated between [-1.f, 1.f] and [1.f, 1.f]
			return bWithShield ? FVector2D(0.f, 1.f) :  FVector2D(FMath::RandBool() ? 1.f : -1.f, 1.f);
		default:
			ensure(false); // for light attacks. this is not a solution at all. I just don't know how to handle it yet
			break;
	}

	return FVector2D(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1)); // should be integers.
}
