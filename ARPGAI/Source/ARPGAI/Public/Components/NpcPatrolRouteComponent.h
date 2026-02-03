// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/Decorators/BTDecorator_IsAtLocationSimple.h"

#include "NpcPatrolRouteComponent.generated.h"

USTRUCT(BlueprintType)
struct ARPGAI_API FPatrolPointMetadata
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer PointTags;
};

struct ARPGAI_API FPatrolPointData
{
	FVector Location = FAISystem::InvalidLocation;
	FPatrolPointMetadata Metadata;
};


class AAIController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcPatrolRouteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcPatrolRouteComponent();
	
	FORCEINLINE const FGameplayTag& GetRouteId() const { return RouteId; }
	FORCEINLINE int GetRoutePointsCount() const { return PatrolPoints.Num(); }
	// world locations expected
	FORCEINLINE void SetPatrolPoints(const TArray<FPatrolPointData>& InPoints) { PatrolPoints = InPoints; };

	FVector GetRoutePointLocation(int Index) const;

	struct FNpcPatrolRouteData GetClosestRoutePointDistanceSq(const FVector& Location) const; 
	FNpcPatrolRouteData GetClosestRoutePointDistancePathfinding(AAIController* AIController, const FVector& Location) const;

protected:
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// world locations expected
	TArray<FPatrolPointData> PatrolPoints;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bIsPathLooped = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FGameplayTag RouteId;
};
