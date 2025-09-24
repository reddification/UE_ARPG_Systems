// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NpcZone.generated.h"

class UBoxComponent;
// This class does not need to be modified.
UINTERFACE()
class UNpcZone : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API INpcZone
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<UShapeComponent*> GetNpcZoneVolumes() const;

	virtual bool IsLocationWithinNpcArea(const FVector& TestLocation, float AreaExtent) = 0;
	virtual TArray<FNavLocation> ProvideEqsPoints(const float Density, const float ExtentScale) = 0;
	virtual FVector GetNpcNavigationLocation(const FVector& QuerierLocation) const = 0;
};
