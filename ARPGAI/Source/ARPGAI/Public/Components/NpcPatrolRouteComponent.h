// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "NpcPatrolRouteComponent.generated.h"

class AAIController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcPatrolRouteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE const FGameplayTag& GetRouteId() const { return RouteId; }
	FORCEINLINE int GetRoutePointsCount() const { return PatrolPoints.Num(); }
	FORCEINLINE void SetPatrolPoints(const TArray<FVector>& InPoints) { PatrolPoints = InPoints; };

	FVector GetRoutePointLocation(int Index) const;

	struct FNpcPatrolRouteData GetClosestRoutePointDistanceSq(const FVector& Location) const; 
	FNpcPatrolRouteData GetClosestRoutePointDistancePathfinding(AAIController* AIController, const FVector& Location) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FGameplayTag RouteId;
};
