

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "WeaponNoiseReportComponent.generated.h"

class URangedWeaponInstance;
UCLASS(meta=(BlueprintSpawnableComponent))
class ARPGAI_API UWeaponNoiseReportComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void OnShot(const FVector& ShotLocation, const URangedWeaponInstance* RangeWeaponInstance);
	
	float NextReportTime = 0.f;
	float NoiseReportInterval = 0.25f;
	float DefaultLoudness = 1.f;
	float DefaultShotRange = 5000.f;
};
