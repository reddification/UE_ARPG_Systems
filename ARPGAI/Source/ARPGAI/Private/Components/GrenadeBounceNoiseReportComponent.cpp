


#include "Components/GrenadeBounceNoiseReportComponent.h"

#include "Data/AIGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Settings/NpcCombatSettings.h"

void UGrenadeBounceNoiseReportComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (UProjectileMovementComponent* ProjectileMovementComponent = GetOwner()->FindComponentByClass<UProjectileMovementComponent>())
		{
			ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &UGrenadeBounceNoiseReportComponent::OnBounce);
			if (const UNpcCombatSettings* MobSettings = GetDefault<UNpcCombatSettings>())
			{
				// BounceNoiseRange = MobSettings->GrenadeBounceNoiseRange;
				// MaxBouncesCountToReport = MobSettings->MaxGrenadeBouncesCountToReport;
			}
		}	
	}
}

void UGrenadeBounceNoiseReportComponent::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BouncesCountReported < MaxBouncesCountToReport && GetOwnerRole() == ROLE_Authority)
	{
		BouncesCountReported++;
		AActor* OwnerOwner = GetOwner()->GetOwner();
		UAISense_Hearing::ReportNoiseEvent(this, ImpactResult.Location, 1.f,
			OwnerOwner ? OwnerOwner : GetOwner(), BounceNoiseRange, AIGameplayTags::AI_Noise_Throwable_Bounce.GetTag().GetTagName());	
	}

	if (BouncesCountReported >= MaxBouncesCountToReport)
	{
		if (UProjectileMovementComponent* ProjectileMovementComponent = GetOwner()->FindComponentByClass<UProjectileMovementComponent>())
		{
			ProjectileMovementComponent->OnProjectileBounce.RemoveDynamic(this, &UGrenadeBounceNoiseReportComponent::OnBounce);
		}
	}
}
