

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrenadeBounceNoiseReportComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UGrenadeBounceNoiseReportComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	float BounceNoiseRange = 300.f;
	uint8 MaxBouncesCountToReport = 3;

	uint8 BouncesCountReported = 0;
	
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};
