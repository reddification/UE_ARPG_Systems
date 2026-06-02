#include "EQS/Contexts/EnvQueryContext_ClosestPointToArea.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Interfaces/NpcZone.h"

static bool FindBorderPointOnBox(const UBoxComponent* BoxComponent,
	const FVector& OutsidePointWorld, const FVector& InsidePointWorld,
	const float AreaExtent, FVector& OutBorderPointWorld)
{
	if (!BoxComponent)
		return false;

	const FTransform BoxTransform = BoxComponent->GetComponentTransform();

	const FVector Start = BoxTransform.InverseTransformPosition(OutsidePointWorld);
	const FVector End = BoxTransform.InverseTransformPosition(InsidePointWorld);
	const FVector Dir = End - Start;

	const FVector Extent = BoxComponent->GetScaledBoxExtent() + FVector(AreaExtent);

	float TEnter = 0.0f;
	float TExit = 1.0f;

	auto ClipAxis = [&](float StartAxis, float DirAxis, float MinAxis, float MaxAxis) -> bool
	{
		if (FMath::IsNearlyZero(DirAxis))
			return StartAxis >= MinAxis && StartAxis <= MaxAxis;

		float T1 = (MinAxis - StartAxis) / DirAxis;
		float T2 = (MaxAxis - StartAxis) / DirAxis;

		if (T1 > T2)
			Swap(T1, T2);

		TEnter = FMath::Max(TEnter, T1);
		TExit = FMath::Min(TExit, T2);

		return TEnter <= TExit;
	};

	if (!ClipAxis(Start.X, Dir.X, -Extent.X, Extent.X) ||
		!ClipAxis(Start.Y, Dir.Y, -Extent.Y, Extent.Y) ||
		!ClipAxis(Start.Z, Dir.Z, -Extent.Z, Extent.Z))
		return false;

	const FVector LocalBorderPoint = Start + Dir * TEnter;
	OutBorderPointWorld = BoxTransform.TransformPosition(LocalBorderPoint);
	return true;
}

void UEnvQueryContext_ClosestPointToArea::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                         FEnvQueryContextData& ContextData) const
{
	auto Pawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!Pawn)
		return;
	
	auto AIController = Cast<AAIController>(Pawn->GetController());
	
	auto NpcAreasComponent = GetNpcAreasComponent(Pawn);
	if (NpcAreasComponent == nullptr)
		return;
	
	const auto& NpcAreas = NpcAreasComponent->GetNpcAreas();
	if (NpcAreas.IsEmpty())
		return;
	
	FVector Result = FAISystem::InvalidLocation;
	
	FVector PathStart = GetStartLocation(Pawn);
	if (PathStart == FAISystem::InvalidLocation)
		return;
	
	FVector ClosestLocation = FAISystem::InvalidLocation;
	const INpcZone* ClosestArea = nullptr;
	float ClosestDistSq = FLT_MAX;
	
	for (const auto& NpcAreaType : NpcAreas)
	{
		if (!NpcAreasCategoryFilter.IsEmpty() && !NpcAreaType.Key.MatchesAny(NpcAreasCategoryFilter))
			continue;
		
		for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
		{
			if (NpcArea->IsLocationWithinNpcArea(PathStart, AreaExtent))
			{
				UEnvQueryItemType_Point::SetContextHelper(ContextData, PathStart);
				return;	
			}
			
			FVector TestClosestLocation = NpcArea->GetNpcNavigationLocation(PathStart);
			const float TestDistSq = (PathStart - TestClosestLocation).SizeSquared();
			if (TestDistSq < ClosestDistSq)
			{
				ClosestDistSq = TestDistSq;
				ClosestLocation = TestClosestLocation;
				ClosestArea = NpcArea.GetInterface();
			}
		}
	}
	
	if (ClosestArea == nullptr)
		return;
	
	auto NavSys = UNavigationSystemV1::GetCurrent(AIController);
	const ANavigationData* NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), PathStart);
	if (!NavData)
		return;
	
	FPathFindingQuery PathFindingQuery;
	FVector PathEnd = ClosestLocation;
	// 21 Apr 2026 (aki): TODO decide if must inverse path by comparing [distance between querier and zone location] and [zone extent]
	// if zone extent smaller - inverse path
	bool bInversePath = false;
	if (bInversePath)
		Swap(PathStart, PathEnd);

	FPathFindingQuery Query(AIController, *NavData, PathStart, PathEnd);
	FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, bUseHierarchicalPathfinding 
		? EPathFindingMode::Hierarchical : EPathFindingMode::Regular);
	
	if (PathFindingResult.Result != ENavigationQueryResult::Type::Success)
	{
		UE_VLOG(AIController, LogARPGAI_EQS, Warning, TEXT("Failed to find path %s NPC Area"), bInversePath ? TEXT("from") : TEXT("to"));
		return;
	}
	
	const auto& PathPoints = PathFindingResult.Path->GetPathPoints();
	ensure(!PathPoints.IsEmpty());
	Result = PathPoints[0];
	if (PathPoints.Num() == 1)
	{
		UEnvQueryItemType_Point::SetContextHelper(ContextData, PathPoints[0]);
		return;
	}

	UBoxComponent* PreviousPathPointVolume = ClosestArea->GetVolumeContainingLocation(PathPoints[0].Location, AreaExtent);
	if (PreviousPathPointVolume != nullptr && !bInversePath)
	{
		ensure(false); // WTF? This must mean that the path starts inside the area, which is unexpected and was actually checked before pathfinding
		UEnvQueryItemType_Point::SetContextHelper(ContextData, PathPoints[0]);
		return;		
	}
	
	for (int i = 1; i < PathPoints.Num(); ++i)
	{
		UBoxComponent* CurrentPathPointVolume = ClosestArea->GetVolumeContainingLocation(PathPoints[i].Location, AreaExtent);
		bool bZoneEdge = !bInversePath 
			? PreviousPathPointVolume == nullptr && CurrentPathPointVolume != nullptr
			: PreviousPathPointVolume != nullptr && CurrentPathPointVolume == nullptr;
		
		if (bZoneEdge)
		{
			FVector OutsidePoint = PathPoints[i-1];
			FVector InsidePoint = PathPoints[i];
			if (bInversePath)
				Swap(OutsidePoint, InsidePoint);
			
			UBoxComponent* ActualVolume = !bInversePath ? CurrentPathPointVolume : PreviousPathPointVolume;
			bool bFoundLocation = FindBorderPointOnBox(ActualVolume, OutsidePoint, InsidePoint, AreaExtent, Result);
			if (ensure(bFoundLocation))
			{
				UE_VLOG_LOCATION(AIController, LogARPGAI_EQS, Verbose, Result, 20, FColorList::MandarianOrange, TEXT("Rubberband EQS context destination"));
				UEnvQueryItemType_Point::SetContextHelper(ContextData, Result);
			}
			
			return;
		}
		
		PreviousPathPointVolume = CurrentPathPointVolume;
	}
}

FVector UEnvQueryContext_ClosestPointToArea::GetStartLocation(APawn* Pawn) const
{
	if (StartPointEQSParam.IsValid())
	{
		if (auto NpcComponent = GetNpcComponent(Pawn))
			return NpcComponent->GetStoredLocation(StartPointEQSParam, false);
	}
	else
	{
		return Pawn->GetActorLocation();
	}
	
	return FAISystem::InvalidLocation;
}
