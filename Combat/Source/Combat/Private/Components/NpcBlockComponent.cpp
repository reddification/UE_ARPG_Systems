#include "Components/NpcBlockComponent.h"

#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UNpcBlockComponent::BeginPlay()
{
	Super::BeginPlay();
	NpcCombatant.SetObject(GetOwner());
	NpcCombatant.SetInterface(Cast<INpcCombatant>(GetOwner()));
}

void UNpcBlockComponent::StartBlocking(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType)
{
	StartBlocking();
	FVector IncomingAttackDirection = GetIncomingAttackDirection(AttackingActor, IncomingAttackType);
	if (IncomingAttackDirection.IsNearlyZero())
	{
		OnNpcFinishedBlockingEvent.ExecuteIfBound();
		return;
	}
	
	PendingBlockInputs.Reset();
	CurrentPendingBlockIndex = 0;
	
#if WITH_EDITOR
	FVector DebugDrawAttackStart = AttackingActor->GetActorLocation() + FVector::UpVector * 35.f;
	UE_VLOG_ARROW(GetOwner(), LogCombat_Block, Log, DebugDrawAttackStart, DebugDrawAttackStart + IncomingAttackDirection * 100.f, FColor::Red, TEXT("Incoming attack"));
#endif

	// thrust is a special case
	if ((AttackingActor->GetActorForwardVector() | IncomingAttackDirection) > 0.95f)
	{
		PendingBlockInputs.Add(FVector2D(-0.5f, 0.f));
		PendingBlockInputs.Add(FVector2D(0.5f, 0.f));
	}
	else
	{
		// is it even correct? 
		// TODO 16.01.2026: check if it's a proper direction 
		FVector Projection = FVector::VectorPlaneProject(IncomingAttackDirection, AttackingActor->GetActorForwardVector());
		PendingBlockInputs.Add(-FVector2D(Projection.Y, Projection.Z).GetSafeNormal());
	}

	bNpcStartedBlocking = false;
	SetBlockReactionDelay();
}

void UNpcBlockComponent::StartBlocking(float Angle)
{
	StartBlocking();
	const float Radians = FMath::DegreesToRadians(Angle);
	FVector2D NewBlockInput(FMath::Cos(Radians), FMath::Sin(Radians));
#if WITH_EDITOR
	ensure(NewBlockInput.Size() == 1.f);
#endif
	
	bool bAddZero = IsBlocking() && PendingBlockInputs.Num() > 0 && (PendingBlockInputs.Last() | NewBlockInput) > 0.5f;
	PendingBlockInputs.Reset();
	CurrentPendingBlockIndex = 0;
	if (bAddZero)
		PendingBlockInputs.Add(FVector2D(0.f, 0.f));
	
	PendingBlockInputs.Add(NewBlockInput);

	bNpcStartedBlocking = false;
	SetBlockReactionDelay();
}

void UNpcBlockComponent::StartBlocking()
{
	Super::StartBlocking();
}

void UNpcBlockComponent::SetBlockReactionDelay()
{
	CurrentReactionDelay = 0.f;
	const UMeleeCombatSettings* MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	if (auto BlockReactionDelayDependency = MeleeCombatSettings->AIBlockReactionDelayDependency.GetRichCurveConst())
		CurrentReactionDelay += BlockReactionDelayDependency->Eval(NpcCombatant->GetReaction());

	float StaminaRatio = OwnerCombatant->GetStaminaRatio();
	if (auto BlockStaminaDelayDependency = MeleeCombatSettings->AIBlockStaminaDelayDependency.GetRichCurveConst())
		CurrentReactionDelay += BlockStaminaDelayDependency->Eval(StaminaRatio);
}

FVector2D UNpcBlockComponent::GetBlockInput(float DeltaTime) const
{
	return PendingBlockInputs[CurrentPendingBlockIndex] * BlockInputAccumulationScale * DeltaTime;
}

void UNpcBlockComponent::AddBlockInput(const FVector2D& BlockDirectionInput, float DeltaTime)
{
	if (CurrentReactionDelay > 0.f)
	{
		CurrentReactionDelay -= DeltaTime;
		return;
	}
	else if (!bNpcStartedBlocking)
	{
		OwnerCombatant->OnBlockSet();
		bNpcStartedBlocking = true;
	}
	
	Super::AddBlockInput(BlockDirectionInput, DeltaTime);

	// FVector2D AccumulatedBlock = GetAccumulatedBlockVector();
	if (AccumulatedBlock.SizeSquared() >= PendingBlockInputs[CurrentPendingBlockIndex].SizeSquared())
	{
		if (CurrentPendingBlockIndex == PendingBlockInputs.Num() - 1)
			OnNpcFinishedBlockingEvent.ExecuteIfBound();
		else
			CurrentPendingBlockIndex++;
	}
}