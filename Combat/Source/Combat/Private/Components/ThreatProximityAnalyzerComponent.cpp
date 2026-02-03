#include "Components/ThreatProximityAnalyzerComponent.h"

#include "GameFramework/Character.h"
#include "Interfaces/CombatAliveCreature.h"
#include "Interfaces/CombatAnimInstance.h"
#include "Interfaces/ICombatant.h"

DEFINE_LOG_CATEGORY(LogCombat_ThreatProximity)

UThreatProximityAnalyzerComponent::UThreatProximityAnalyzerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.15f;
}

void UThreatProximityAnalyzerComponent::BeginPlay()
{
	Super::BeginPlay();
	ConsideredCloseCombatRangeSq = ConsideredCloseCombatRange * ConsideredCloseCombatRange;
	auto OwnerCharacter = Cast<ACharacter>(GetOwner());
	auto CombatantOwnerInterface = Cast<ICombatant>(OwnerCharacter);
	CombatantOwner.SetObject(OwnerCharacter);
	CombatantOwner.SetInterface(CombatantOwnerInterface);

	if (auto OwnerAliveCreature = Cast<ICombatAliveCreature>(OwnerCharacter))
		OwnerAliveCreature->OnCombatCreatureDeadEvent.AddUObject(this, &UThreatProximityAnalyzerComponent::OnOwnerDied);
	
	CombatAnimInstance = CombatantOwnerInterface->GetCombatAnimInstance();
}

void UThreatProximityAnalyzerComponent::OnOwnerDied(AActor* Actor)
{
	SetComponentTickEnabled(false);
}

void UThreatProximityAnalyzerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TRACE_CPUPROFILER_EVENT_SCOPE(UThreatProximityAnalyzerComponent::TickComponent)
	
	TSet<AActor*> EnemiesInSight = CombatantOwner->GetCombatObservedActors();
	
	const FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector OwnerViewLocation;
	FRotator OwnerViewRotation;
	GetOwner()->GetActorEyesViewPoint(OwnerViewLocation, OwnerViewRotation);
	const FVector OwnerViewDirection = OwnerViewRotation.Vector();
	bool bEnemyClose = false;
	for (const auto* Enemy : EnemiesInSight)
	{
		UE_VLOG_LOCATION(GetOwner(), LogCombat_ThreatProximity, Verbose, Enemy->GetActorLocation(), 25, FColor::Yellow, TEXT("Enemy %s"), *Enemy->GetName());
		
		const FVector EnemyLocation = Enemy->GetActorLocation();
		const float DotProduct = OwnerViewDirection | (EnemyLocation - OwnerLocation).GetSafeNormal();
		if (DotProduct >= DotProductThreshold)
		{
			UE_VLOG(GetOwner(), LogCombat_ThreatProximity, VeryVerbose, TEXT("dot product to %s passes. dp = %.2f"), *Enemy->GetName(), DotProduct);
			
			const float EnemyToOwnerDistSq = (EnemyLocation - OwnerLocation).SizeSquared();
			if (EnemyToOwnerDistSq < ConsideredCloseCombatRangeSq)
			{
				UE_VLOG(GetOwner(), LogCombat_ThreatProximity, VeryVerbose, TEXT("%s closer than threshold range. distance = %.2f"), *Enemy->GetName(), FMath::Sqrt(EnemyToOwnerDistSq));
				bEnemyClose = true;
				break;			
			}
			else if (auto Combatant = Cast<ICombatant>(Enemy))
			{
				if (Combatant->IsUsingRangeWeapon())
				{
					const float RangeWeaponDistance = Combatant->GetAttackRange();
					const float RangeWeaponDistanceSq = RangeWeaponDistance * RangeWeaponDistance;
					if (RangeWeaponDistanceSq * 1.1f < EnemyToOwnerDistSq)
					{
						bEnemyClose = true;
						break;
					}
				}
			}
		}
	}

	if (bEnemyClose)
	{
		UE_VLOG(GetOwner(), LogCombat_ThreatProximity, VeryVerbose, TEXT("Enemy is close. Ready up weapon"));
        CombatAnimInstance->SetReadyUpWeapon(true);
		RemainingDelaySwitchToThreatIsFar = SwitchToThreatIsFarDelay;
	}
	else
	{
		RemainingDelaySwitchToThreatIsFar -= DeltaTime;
		if (RemainingDelaySwitchToThreatIsFar <= 0.f)
		{
			UE_VLOG(GetOwner(), LogCombat_ThreatProximity, VeryVerbose, TEXT("No enemy nearby. Unready weapon"));
			CombatAnimInstance->SetReadyUpWeapon(false);
		}
	}
}
