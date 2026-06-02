// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_PathSafety.generated.h"

class UEnvQueryContext_ThreatEnemies;
/**
 * 
 */
UCLASS()
class ARPGAI_API UEnvQueryTest_PathSafety : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_PathSafety();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UEnvQueryContext> ThreatsActorsContext;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UEnvQueryContext> ThreatsLocationsContext;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNavigationQueryFilter> FilterClass;

	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue DistanceStep;

	// consider path always safe if its length is less than this value
	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue ConsiderShortPathSafeLengthThreshold;

	// If path segment is dangerous, scale its length by multiplying it by ConsiderSafeDistanceFromThreat / DistanceToThreat
	// so if segment path is 150, enemy can see NPC, safe distance is 1000, and distance to threat is 1500, then actual scored path length would be
	// 150 * 1000/1500 = 100
	// Currently equation is linear but some form of exponent/hyperbole is required 
	UPROPERTY(EditDefaultsOnly)
	FAIDataProviderFloatValue ConsiderSafeDistanceFromThreat;

	
	UPROPERTY(EditDefaultsOnly)
	float VerticalOffset = 150.f;
	
	// Dot product is taken between threat forward vector and threat to test location
	UPROPERTY(EditDefaultsOnly)
	float PointSafeDotProductThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	UPROPERTY(EditDefaultsOnly)
	bool bUseHierarchicalPathFinding = false;

private:
	void TestPointAgainstThreats(APawn* QueryOwner, bool bContextActors, const FVector& TestLocation, const TArray<FVector>& ContextLocations, const TArray<AActor*>& ContextActors,
	                             const FVector& QuerierLocation, float& SafePathDistance, float& UnsafePathDistance, bool& bPreviousPointSafe, const float SegmentDistance,
	                             const float ConsideredSafeDistanceToTarget) const;
	bool IsPointSafe(const FVector& TestLocation, const FVector& ThreatLocation, const FVector& ThreatForwardVector, const APawn* Querier) const;

};
