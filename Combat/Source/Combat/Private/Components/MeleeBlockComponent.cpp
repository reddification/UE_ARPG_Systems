// 


#include "Components/MeleeBlockComponent.h"
#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
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
	OwnerCombatant.SetObject(GetOwner());
	OwnerCombatant.SetInterface(Cast<ICombatant>(GetOwner()));
	CombatAnimInstance = OwnerCombatant->GetCombatAnimInstance();
	BlockStrengthDecayRate = CombatSettings->BlockStrengthDecayRate;
	DecayDelay = CombatSettings->BlockDecayDelay;
	BlockStrengthAccumulationScale = CombatSettings->BlockStrengthAccumulationScale;
	BlockStrengthToParry = CombatSettings->BlockStrengthToParry;
	AttackerStrengthScaleWhenHitBlock = CombatSettings->AttackerStrengthScaleWhenHitBlock;
}

void UMeleeBlockComponent::StartBlocking()
{
	bRegisteringBlock = true;
	AccumulatedBlock = FVector2d::ZeroVector;
	SetComponentTickEnabled(true);
	OwnerCombatant->SetBlocking(true);
	const UMeleeCombatSettings* MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	float StaminaRatio = OwnerCombatant->GetStaminaRatio();
	if (auto BlockStaminaAccumulationScaleDependency = MeleeCombatSettings->BlockStaminaAccumulationScaleDependency.GetRichCurveConst())
		BlockInputAccumulationScale = BlockStaminaAccumulationScaleDependency->Eval(StaminaRatio);
	else
		BlockInputAccumulationScale = 0.15f;

	BlockStrength = 1.f;
	CurrentDecayDelay = DecayDelay;
}

void UMeleeBlockComponent::StopBlocking()
{
	bRegisteringBlock = false;
	SetComponentTickEnabled(false);
	OwnerCombatant->SetBlocking(false);
}

EBlockResult UMeleeBlockComponent::BlockAttack(const FVector& AttackDirection, float AttackerStrength) const
{
	if (!bRegisteringBlock)
		return EBlockResult::None;

	// FVector2D AccumulatedBlockVector = GetAccumulatedBlockVector();
	FVector ProjectedDirection = FVector::VectorPlaneProject(-GetOwner()->GetActorForwardVector(), AttackDirection);
	FVector2D ProjectedDirection2D = FVector2D(ProjectedDirection.Y, ProjectedDirection.Z);
	bool bAttackAndBlockAreNonCollinear = (ProjectedDirection2D.GetSafeNormal() | AccumulatedBlock.GetSafeNormal()) < -0.5f;
	if (!bAttackAndBlockAreNonCollinear)
		return EBlockResult::None;
	
	if (BlockStrength >= BlockStrengthToParry)
		return EBlockResult::Parry;

	float BlockScaledOwnerStrength = OwnerCombatant->GetStrength() * BlockStrength;
	const float StrengthRatio = AttackerStrength * AttackerStrengthScaleWhenHitBlock / BlockScaledOwnerStrength;
	OnAttackBlockedEvent.ExecuteIfBound(StrengthRatio);
	return EBlockResult::Block;
}

void UMeleeBlockComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ensure(bRegisteringBlock))
		return;

	AddBlockInput(GetBlockInput(DeltaTime), DeltaTime);
	CombatAnimInstance->SetBlockPosition(AccumulatedBlock);

	GEngine->AddOnScreenDebugMessage(123, 5.f, FColor::Cyan, FString::Printf(TEXT("Block: %s [%.2f]"), *AccumulatedBlock.ToString(), BlockStrength));
	UE_VLOG(GetOwner(), LogCombat, VeryVerbose, TEXT("Current block position = %s"), *AccumulatedBlock.ToString());
}

void UMeleeBlockComponent::AddBlockInput(const FVector2D& BlockInput, float DeltaTime)
{
	FVector2D NewInputDirection = BlockInput.GetSafeNormal();
	FVector2D CurrentBlockDirection = AccumulatedBlock.GetSafeNormal();
	const float NewInputDotProduct = CurrentBlockDirection | NewInputDirection;
	bool bDirectionCollinear = !BlockInput.IsNearlyZero() && !CurrentBlockDirection.IsNearlyZero() && NewInputDotProduct > CollinearBlockInputsDotProductThreshold;
	float BlockStrengthScore = NewInputDotProduct - CollinearBlockInputsDotProductThreshold;
	FVector2D AccumulatedBlockPreChange = AccumulatedBlock;

	UE_LOG(LogCombat, Log, TEXT("Block ==========="))
	UE_LOG(LogCombat, Log, TEXT("Block input = %s"), *BlockInput.ToString());
	UE_LOG(LogCombat, Log, TEXT("Current block direction = %s"), *CurrentBlockDirection.ToString());
	UE_LOG(LogCombat, Log, TEXT("New input dot product = %.2f"), NewInputDotProduct);
	UE_LOG(LogCombat, Log, TEXT("Direction collinear = %s"), bDirectionCollinear ? TEXT("true") : TEXT("false"));
	UE_LOG(LogCombat, Log, TEXT("Block strength score = %.2f"), BlockStrengthScore);
	UE_LOG(LogCombat, Log, TEXT("Accumulated block pre change = %s"), *AccumulatedBlock.ToString());
	
	AccumulatedBlock = AccumulatedBlock + BlockInput;
	AccumulatedBlock = AccumulatedBlock.ClampAxes(-1.f, 1.f);

	UE_LOG(LogCombat, Log, TEXT("Accumulated block post change = %s"), *AccumulatedBlock.ToString());

	const float DeltaBlockStrength = (AccumulatedBlock - AccumulatedBlockPreChange).Size();
	UE_LOG(LogCombat, Log, TEXT("Delta block strength = %.2f"), DeltaBlockStrength);

	if (DeltaBlockStrength > KINDA_SMALL_NUMBER && bDirectionCollinear)
	{
		BlockStrength = BlockStrength + DeltaBlockStrength * DeltaTime * BlockStrengthScore * BlockStrengthAccumulationScale;
		CurrentDecayDelay = DecayDelay;
	}
	else if (NewInputDotProduct > 0.25f)
	{
		if (CurrentDecayDelay > 0.f)
			CurrentDecayDelay -= BlockStrengthDecayRate * DeltaTime;
		else 
			BlockStrength = FMath::Max(MinHeldBlockStrength, BlockStrength - BlockStrengthDecayRate * DeltaTime);
		
		UE_LOG(LogCombat, Log, TEXT("Block strength decaying"));
	}
	else
	{
		BlockStrength = 0.f;
	}

#if WITH_EDITOR
	if (BlockStrength > 0.5f)
	{
		DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.f, 25, 16, FColor::Cyan, false);
	}
#endif
	
	UE_LOG(LogCombat, Log, TEXT("New block strength = %.2f"), BlockStrength);
	
	// UE_LOG(LogCombat, Log, TEXT("Accumulated block = %s; Strength = %.2f"), *AccumulatedBlock.ToString(), BlockStrength);
}

FVector UMeleeBlockComponent::GetIncomingAttackDirection(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType)
{
	// all these 35.f, 15.f, 50.f corrections are just from the top of my head, don't consider them something well calculated
	const FVector AttackerLocation = AttackingActor->GetActorLocation() + FVector::UpVector * 35.f;
	const FVector AttackerUpVector = AttackingActor->GetActorUpVector();
	const FVector AttackerForwardVector = AttackingActor->GetActorForwardVector();
	const FVector AttackerRightVector = AttackingActor->GetActorRightVector();
	auto AttackerCombatant = Cast<ICombatant>(AttackingActor);
	const float AttackRange = AttackerCombatant->GetAttackRange();
	const FVector AttackLocation = AttackerLocation + FVector::UpVector * 15.f + AttackerForwardVector * AttackRange;
	FVector AttackOrigin = FVector::ZeroVector;
	switch (IncomingAttackType)
	{
		case EMeleeAttackType::None:
			ensure(false);
			break;
		case EMeleeAttackType::LeftUnterhauw:
			AttackOrigin = AttackerLocation - AttackerUpVector * 50.f - AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::LeftMittelhauw:
			AttackOrigin = AttackerLocation - AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::LeftOberhauw:
			AttackOrigin = AttackerLocation + AttackerUpVector * 50.f - AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::Thrust:
			AttackOrigin = AttackerLocation + AttackerForwardVector * AttackRange;
			break;
		case EMeleeAttackType::RightUnterhauw:
			AttackOrigin = AttackerLocation - AttackerUpVector * 50.f + AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::RightMittelhauw:
			AttackOrigin = AttackerLocation + AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::RightOberhauw:
			AttackOrigin = AttackerLocation + AttackerUpVector * 50.f + AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::VerticalSlash:
			AttackOrigin = AttackerLocation + AttackerUpVector * 50.f;
			break;
		case EMeleeAttackType::SpinLeftMittelhauw:
			AttackOrigin = AttackerLocation - AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinLeftOberhauw:
			AttackOrigin = AttackerLocation + AttackerUpVector * 50.f - AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinRightMittelhauw:
			AttackOrigin = AttackerLocation + AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::SpinRightOberhauw:
			AttackOrigin = AttackerLocation + AttackerUpVector * 50.f + AttackerRightVector * 50.f;
			break;
		case EMeleeAttackType::Max:
			ensure(false);
			break;
		default:
			ensure(false);
			return FMath::VRand();
			break;
	}

	return (AttackLocation - AttackOrigin).GetSafeNormal();
}

FVector2D UMeleeBlockComponent::GetDesiredBlockVector(EMeleeAttackType IncomingAttackType) const
{
	switch (IncomingAttackType)
	{
		case EMeleeAttackType::None:
			ensure(false);
			break;
		case EMeleeAttackType::LeftUnterhauw:
			return FVector2D(1.f, -1.f);
		case EMeleeAttackType::LeftMittelhauw:
		case EMeleeAttackType::SpinLeftMittelhauw:
			return FVector2D(1.f, 0.f);
		case EMeleeAttackType::LeftOberhauw:
		case EMeleeAttackType::SpinLeftOberhauw:
			return FVector2D(1.f, 1.f);
		case EMeleeAttackType::Thrust:
			return FVector2D(0.f, 0.f);
		case EMeleeAttackType::RightUnterhauw:
			return FVector2D(-1.f, -1.f);
		case EMeleeAttackType::SpinRightMittelhauw:
		case EMeleeAttackType::RightMittelhauw:
			return FVector2D(-1.f, 0.f);
		case EMeleeAttackType::RightOberhauw:
		case EMeleeAttackType::SpinRightOberhauw:
			return FVector2D(-1.f, 1.f);
		case EMeleeAttackType::VerticalSlash:
			return FVector2D(0.f, 1.f);
		default:
			ensure(false); // for light attacks. this is not a solution at all. I just don't know how to handle it yet
			break;
	}

	return FVector2D(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1)); // should be integers.
}
