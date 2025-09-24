// 


#include "Components/NpcPatrolRouteComponent.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Subsystems/NpcPatrolRoutesSubsystem.h"

void UNpcPatrolRouteComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ensure(RouteId.IsValid()))
		return;
	
	UNpcPatrolRoutesSubsystem* PatrolRoutesSubsystem = UNpcPatrolRoutesSubsystem::Get(this);
	PatrolRoutesSubsystem->RegisterPatrolRoute(this);
}

void UNpcPatrolRouteComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UNpcPatrolRoutesSubsystem* PatrolRoutesSubsystem = UNpcPatrolRoutesSubsystem::Get(this))
		PatrolRoutesSubsystem->UnregisterPatrolRoute(this);
	
	Super::EndPlay(EndPlayReason);
}

FVector UNpcPatrolRouteComponent::GetRoutePointLocation(int Index) const
{
	return PatrolPoints[Index % PatrolPoints.Num()].Location;	
}

FNpcPatrolRouteData UNpcPatrolRouteComponent::GetClosestRoutePointDistanceSq(const FVector& Location) const
{
	FNpcPatrolRouteData Result;
	Result.Distance = FLT_MAX;
	for (int i = 0; i < PatrolPoints.Num(); i++)
	{
		FVector PatrolPointWorldLocation = PatrolPoints[i].Location;
		float DistSq = (Location - PatrolPointWorldLocation).SizeSquared();
		if (DistSq < Result.Distance)
		{
			Result.Distance = DistSq;
			Result.RoutePointIndex = i;
			Result.InitialPointIndex = i;
			Result.RoutePointLocation = PatrolPointWorldLocation;
			Result.bCyclic = bIsPathLooped;
			Result.bGoingForward = true;
		}
	}

	Result.PatrolRouteComponent = this;
	return Result;
}

FNpcPatrolRouteData UNpcPatrolRouteComponent::GetClosestRoutePointDistancePathfinding(AAIController* AIController, const FVector& NpcLocation) const
{
	FNpcPatrolRouteData Result;
	Result.Distance = FLT_MAX;
	auto NavSys = UNavigationSystemV1::GetCurrent(GetOwner());
	const ANavigationData* NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation);
	for (int i = 0; i < PatrolPoints.Num(); i++)
	{
		FVector PatrolPointWorldLocation = GetOwner()->GetActorTransform().TransformPosition(PatrolPoints[i].Location);
		FPathFindingQuery Query(AIController, *NavData, NpcLocation, PatrolPointWorldLocation);
		FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Regular);
		float TestDistance = PathFindingResult.Path->GetLength();

		if (PathFindingResult.Result == ENavigationQueryResult::Type::Fail)
			continue;
				
		if (TestDistance < Result.Distance)
		{
			Result.Distance = TestDistance;
			Result.RoutePointIndex = i;
			Result.InitialPointIndex = i;
			Result.RoutePointLocation = PatrolPointWorldLocation;
		}
	}

	Result.PatrolRouteComponent = this;
	return Result;
}