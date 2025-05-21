// 


#include "Components/NpcPatrolRouteComponent.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Data/LogChannels.h"
#include "Subsystems/NpcPatrolRoutesSubsystem.h"

void UNpcPatrolRouteComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ensure(RouteId.IsValid()))
		return;
	
	UNpcPatrolRoutesSubsystem* PatrolRoutesSubsystem = UNpcPatrolRoutesSubsystem::Get(this);
	PatrolRoutesSubsystem->RegisterPatrolRoute(this);

	// just for testing
	for (int i = 0; i < PatrolPoints.Num(); i++)
	{
		FVector PatrolPointWorldLocation = GetOwner()->GetActorTransform().TransformPosition(PatrolPoints[i]);
		UE_VLOG_LOCATION(GetOwner(), LogARPGAI, VeryVerbose, PatrolPointWorldLocation, 25, FColor::White, TEXT("Point %d"), i);
	}
}

void UNpcPatrolRouteComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UNpcPatrolRoutesSubsystem* PatrolRoutesSubsystem = UNpcPatrolRoutesSubsystem::Get(this))
		PatrolRoutesSubsystem->UnregisterPatrolRoute(this);
	
	Super::EndPlay(EndPlayReason);
}

FVector UNpcPatrolRouteComponent::GetRoutePointLocation(int Index) const
{
	return GetOwner()->GetActorTransform().TransformPosition(PatrolPoints[Index % PatrolPoints.Num()]);	
}

FNpcPatrolRouteData UNpcPatrolRouteComponent::GetClosestRoutePointDistanceSq(const FVector& Location) const
{
	FNpcPatrolRouteData Result;
	Result.Distance = FLT_MAX;
	for (int i = 0; i < PatrolPoints.Num(); i++)
	{
		FVector PatrolPointWorldLocation = GetOwner()->GetActorTransform().TransformPosition(PatrolPoints[i]);
		float DistSq = (Location - PatrolPointWorldLocation).SizeSquared();
		if (DistSq < Result.Distance)
		{
			Result.Distance = DistSq;
			Result.RoutePointIndex = i;
			Result.InitialPointIndex = i;
			Result.RoutePointLocation = PatrolPointWorldLocation;
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
		FVector PatrolPointWorldLocation = GetOwner()->GetActorTransform().TransformPosition(PatrolPoints[i]);
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