#include "Components/PlayerManualBlockComponent.h"

#include "Data/MeleeCombatSettings.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/PlayerCombat.h"

UPlayerManualBlockComponent::UPlayerManualBlockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerManualBlockComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayerCombat.SetObject(GetOwner());
	OwnerPlayerCombat.SetInterface(Cast<IPlayerCombatant>(GetOwner()));
	PlayerBlockInputAccumulationScale = GetDefault<UMeleeCombatSettings>()->PlayerBlockInputAccumulationScale;
}

FVector2D UPlayerManualBlockComponent::GetBlockInput(float DeltaTime) const
{
	FVector2D RawBlockInput = OwnerPlayerCombat->ConsumeCurrentBlockInput();
	return RawBlockInput * DeltaTime * BlockInputAccumulationScale * PlayerBlockInputAccumulationScale;
}

void UPlayerManualBlockComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UPlayerManualBlockComponent::StartBlocking()
{
	Super::StartBlocking();
	OwnerPlayerCombat->SetLookEnabled(false);
}

void UPlayerManualBlockComponent::StopBlocking()
{
	Super::StopBlocking();
	OwnerPlayerCombat->SetLookEnabled(true);
	OwnerPlayerCombat->ResetCombatFocus();
	FocusTarget.Reset();
}

void UPlayerManualBlockComponent::UpdateFocus()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UPlayerManualBlockComponent::UpdateFocus)
	TSet<AActor*> DangerousEnemies = OwnerCombatant->GetDangerousEnemies();
	if (DangerousEnemies.Num() <= 0)
		return;
	
	const double DistSqBetweenFocusedEnemyAndOwner = FocusTarget.IsValid()
		? FVector::DistSquared(FocusTarget->GetActorLocation(), GetOwner()->GetActorLocation())
		: DBL_MAX;
	
	const bool bUpdateFocus = !FocusTarget.IsValid() || !DangerousEnemies.Contains(FocusTarget.Get())
		|| (DangerousEnemies.Num() > 1 && FMath::Abs(DistSqBetweenFocusedEnemyAndOwner - LastDistSqBetweenFocusedEnemyAndOwner) > DistSqToUpdateFocusedEnemy);
	
	AActor* BestTarget = bUpdateFocus ? nullptr : FocusTarget.Get();
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
		FocusTarget = BestTarget;
		OwnerPlayerCombat->SetCombatFocus(FocusTarget.Get());
		LastDistSqBetweenFocusedEnemyAndOwner = DistSqBetweenFocusedEnemyAndOwner;
	}
	else if (BestTarget == nullptr && FocusTarget.IsValid())
	{
		OwnerPlayerCombat->ResetCombatFocus();
		FocusTarget.Reset();
	}
}
