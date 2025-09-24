// 


#include "Subsystems/NpcPatrolRoutesSubsystem.h"

#include "AIController.h"
#include "Components/NpcPatrolRouteComponent.h"
#include "Interfaces/Npc.h"

UNpcPatrolRoutesSubsystem* UNpcPatrolRoutesSubsystem::Get(const UObject* WorldContextOption)
{
	return WorldContextOption->GetWorld()->GetSubsystem<UNpcPatrolRoutesSubsystem>();
}

void UNpcPatrolRoutesSubsystem::RegisterPatrolRoute(UNpcPatrolRouteComponent* PatrolRouteComponent)
{
	PatrolRoutes.Add(PatrolRouteComponent->GetRouteId(), PatrolRouteComponent);
}

void UNpcPatrolRoutesSubsystem::UnregisterPatrolRoute(UNpcPatrolRouteComponent* PatrolRouteComponent)
{
	PatrolRoutes.RemoveSingle(PatrolRouteComponent->GetRouteId(), PatrolRouteComponent);
}

FNpcPatrolRouteData UNpcPatrolRoutesSubsystem::StartPatrolRoute(const APawn* NpcPawn, const FGameplayTag& RouteId,
                                                 bool bPreferClosestRoute, bool bUsePathfinding)
{
	FNpcPatrolRouteData BestPatrolRouteData;
	auto Npc = Cast<INpc>(NpcPawn);
	if (!ensure(Npc))
		return BestPatrolRouteData;
	
	TArray<TWeakObjectPtr<UNpcPatrolRouteComponent>> PatrolRouteComponents;
	PatrolRoutes.MultiFind(RouteId, PatrolRouteComponents);
	if (PatrolRouteComponents.Num() <= 0)
		return BestPatrolRouteData;
	
	auto NpcLocation = NpcPawn->GetActorLocation();

	if (bPreferClosestRoute)
	{
		if (bUsePathfinding)
		{
			auto AIController = Cast<AAIController>(NpcPawn->GetController());
			for (const auto& PatrolRouteComponent : PatrolRouteComponents)
			{
				auto TestPatrolRouteData = PatrolRouteComponent->GetClosestRoutePointDistancePathfinding(AIController, NpcLocation);
				if (TestPatrolRouteData.Distance < BestPatrolRouteData.Distance)
					BestPatrolRouteData = TestPatrolRouteData;
			}
		}
		else
		{
			for (const auto& PatrolRouteComponent : PatrolRouteComponents)
			{
				auto TestPatrolRouteData = PatrolRouteComponent->GetClosestRoutePointDistanceSq(NpcLocation);
				if (TestPatrolRouteData.Distance < BestPatrolRouteData.Distance)
					BestPatrolRouteData = TestPatrolRouteData;
			}
		}
	}
	else
	{
		auto SelectedPatrolRoute = PatrolRouteComponents[FMath::RandRange(0, PatrolRouteComponents.Num() - 1)].Get();
		BestPatrolRouteData = bUsePathfinding
			? SelectedPatrolRoute->GetClosestRoutePointDistancePathfinding(Cast<AAIController>(NpcPawn->GetController()), NpcLocation)
			: SelectedPatrolRoute->GetClosestRoutePointDistanceSq(NpcLocation);
	}

	ActiveNpcsPatrolRoutes.Remove(Npc->GetNpcIdTag());
	ActiveNpcsPatrolRoutes.Add(Npc->GetNpcIdTag(), BestPatrolRouteData);
	return BestPatrolRouteData;
}

FNpcPatrolRouteData UNpcPatrolRoutesSubsystem::GetActivePatrolRoute(const APawn* NpcPawn)
{
	FNpcPatrolRouteData Result;
	auto Npc = Cast<INpc>(NpcPawn);
    if (!ensure((Npc)))
    	return Result;

	auto NpcPatrolRouteData = ActiveNpcsPatrolRoutes.Find(Npc->GetNpcIdTag());
	if (NpcPatrolRouteData)
		Result = *NpcPatrolRouteData;

	return Result;
}

void UNpcPatrolRoutesSubsystem::StopPatrolRoute(const APawn* NpcPawn)
{
	auto Npc = Cast<INpc>(NpcPawn);
	if (!ensure((Npc)))
		return;

	ActiveNpcsPatrolRoutes.Remove(Npc->GetNpcIdTag());
}

FNpcPatrolRouteAdvanceResult UNpcPatrolRoutesSubsystem::GetNextPatrolRoutePoint(const APawn* NpcPawn)
{
	FNpcPatrolRouteAdvanceResult Result;
	auto Npc = Cast<INpc>(NpcPawn);
	auto CurrentRouteDataPtr = ActiveNpcsPatrolRoutes.Find(Npc->GetNpcIdTag());
	if (!CurrentRouteDataPtr)
		return Result;

	int PreviousRoutePointIndex = CurrentRouteDataPtr->RoutePointIndex;
	CurrentRouteDataPtr->RoutePointIndex = CurrentRouteDataPtr->bCyclic || !CurrentRouteDataPtr->bGoingForward
		? (CurrentRouteDataPtr->RoutePointIndex + 1) % CurrentRouteDataPtr->PatrolRouteComponent->GetRoutePointsCount()
		: CurrentRouteDataPtr->RoutePointIndex == 0
			? CurrentRouteDataPtr->PatrolRouteComponent->GetRoutePointsCount() - 1
			: CurrentRouteDataPtr->RoutePointIndex - 1;
	
	Result.NextLocation = CurrentRouteDataPtr->PatrolRouteComponent->GetRoutePointLocation(CurrentRouteDataPtr->RoutePointIndex);
	if (CurrentRouteDataPtr->RoutePointIndex == CurrentRouteDataPtr->InitialPointIndex)
		Result.LoopCount++;

	bool bNeedToReverse = !CurrentRouteDataPtr->bCyclic &&
		(CurrentRouteDataPtr->RoutePointIndex == CurrentRouteDataPtr->PatrolRouteComponent->GetRoutePointsCount() - 1
			|| CurrentRouteDataPtr->RoutePointIndex == 0);
		
	if (bNeedToReverse)
		CurrentRouteDataPtr->bGoingForward = !CurrentRouteDataPtr->bGoingForward;
	
	Result.bReachedEdge = PreviousRoutePointIndex != -1
		&& (CurrentRouteDataPtr->RoutePointIndex == 0 || CurrentRouteDataPtr->RoutePointIndex == CurrentRouteDataPtr->PatrolRouteComponent->GetRoutePointsCount() - 1);
	
	return Result;
}
