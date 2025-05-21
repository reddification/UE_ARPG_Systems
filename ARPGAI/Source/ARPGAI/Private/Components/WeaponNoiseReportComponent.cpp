


#include "Components/WeaponNoiseReportComponent.h"

#include "Data/AIGameplayTags.h"
#include "Perception/AISense_Hearing.h"
#include "Settings/NpcCombatSettings.h"

void UWeaponNoiseReportComponent::BeginPlay()
{
	Super::BeginPlay();
	// if (AWeaponActor* WeaponActor = Cast<AWeaponActor>(GetOwner()))
	// {
	// 	WeaponActor->OnShot.AddUObject(this, &UWeaponNoiseReportComponent::OnShot);
	// }

	if (const UNpcCombatSettings* MobSettings = GetDefault<UNpcCombatSettings>())
	{
		// NoiseReportInterval = MobSettings->NoiseReportInterval;
		// DefaultLoudness = MobSettings->DefaultLoudness;
		// DefaultShotRange = MobSettings->DefaultShotRange;
	}
}

void UWeaponNoiseReportComponent::OnShot(const FVector& ShotLocation, const URangedWeaponInstance* RangeWeaponInstance)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		const float WorldTime = GetWorld()->GetTimeSeconds();
		float Loudness = DefaultLoudness;
		float NoiseRange = DefaultShotRange;
		if (RangeWeaponInstance)
		{
			// Loudness = RangeWeaponInstance->GetShotNoiseLoudness();
			// NoiseRange = RangeWeaponInstance->GetShotNoiseRange();
		}
		
		if (WorldTime > NextReportTime)
		{
			AActor* OwnerOwner = GetOwner()->GetOwner();
			UAISense_Hearing::ReportNoiseEvent(this, ShotLocation, Loudness, GetOwner()->GetOwner(), NoiseRange,
				AIGameplayTags::AI_Noise_Shot.GetTag().GetTagName());
			NextReportTime = WorldTime + NoiseReportInterval;
		}
	}
}
