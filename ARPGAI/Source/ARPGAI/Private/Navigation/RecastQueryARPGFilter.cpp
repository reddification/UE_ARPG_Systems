// Copyright 2019 - 2022 INGAME STUDIOS, a.s., All Rights Reserved.

#include "Navigation/RecastQueryARPGFilter.h"

#include <AIController.h>
#include <NavigationSystem.h>
#include <NavMesh/RecastHelpers.h>
#include <NavMesh/RecastNavMesh.h>

#include "NavLinkCustomInterface.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Navigation/NavLinkTraversalInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "Settings/NpcCombatSettings.h"

DEFINE_LOG_CATEGORY(LogAvoidThreatsPathfollowing_Dot);
DEFINE_LOG_CATEGORY(LogAvoidThreatsPathfollowing_Distance);
DEFINE_LOG_CATEGORY(LogAvoidThreatsPathfollowing_Weight);
DEFINE_LOG_CATEGORY(LogAvoidThreatsPathfollowing_Trace);

#if WITH_RECAST
#if DEBUGING_PATHFINDING
    DECLARE_LOG_CATEGORY_EXTERN(LogAINavigationFilter, Display, All);
    DEFINE_LOG_CATEGORY(LogAINavigationFilter);

    bool   FRecastQueryARPGFilter::DebugPath                 = true;
    uint32 FRecastQueryARPGFilter::DebugCallCount            = 0;
    uint32 FRecastQueryARPGFilter::DebugDrawMaxCallCount     = INT_MAX;

	FRecastQueryARPGFilter::FRecastQueryARPGFilter()
	    : FRecastQueryFilter(true)
	{
	    DebugCallCount = 0;
	    // UE_LOG(LogNavigation, Log, TEXT("  FRecastQueryARPGFilter"));
	}
	FRecastQueryARPGFilter::~FRecastQueryARPGFilter()
	{
	    DebugCallCount = 0;
	    // UE_LOG(LogNavigation, Log, TEXT("~ FRecastQueryARPGFilter"));
	}

#endif

dtReal FRecastQueryARPGFilter::getVirtualCost(const dtReal* pa, const dtReal* pb, const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
		const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly, const dtPolyRef nextRef, const dtMeshTile* nextTile,
		const dtPoly* nextPoly) const
{
	// Super:: version is not called, because this override includes Super's logic  
	// Super::getVirtualCost();
		
    TRACE_CPUPROFILER_EVENT_SCOPE(DangerAvoidanceNavigationQueryFilter::getVirtualCost);

    const FVector recast_nav_mesh_offset = FVector(0.f, 0.f, -10.f);
    const FVector start_location         = Recast2UnrealPoint(pa) + recast_nav_mesh_offset;
    const FVector end_location           = Recast2UnrealPoint(pb) + recast_nav_mesh_offset;

    const float default_area_cost        = data.m_areaCost[curPoly->getArea()];

    // const float disallowed_area_addon    = GetInDisallowedAreaCost(start_location, end_location, default_area_cost);
    // const float wrong_room_addon         = GetWrongRoomAreaCost   (start_location, end_location, default_area_cost, curPoly, nextPoly);
    // const float danger_area_cost			= GetDangerAreaCost      (start_location, end_location);
    const float nav_link_cost            	= GetNavLinkCost         (curRef , curTile , curPoly, default_area_cost, start_location, end_location);
    const float AvoidThreatCost        		= GetAvoidThreatsCost    (start_location, end_location, nav_link_cost);
	const float AvoidSurroundingTargetCost	= GetSurroundTargetCost(end_location);
    const float TotalAreaCost				= nav_link_cost + AvoidThreatCost + AvoidSurroundingTargetCost;

    const float area_cost					= GetDefaultAreaCost(pa, pb, curPoly, nextPoly, FMath::Max(0.f, default_area_cost + TotalAreaCost));
		// + FRecastQueryFilter::getVirtualCost(pa, pb, prevRef, prevTile, prevPoly, curRef, curTile, curPoly, nextRef, nextTile, nextPoly);

#if DEBUGING_PATHFINDING
    ++DebugCallCount;
    if(DebugPath && DebugCallCount < DebugDrawMaxCallCount)
    {
        const auto* const querrier = QuerrierController.Get();
        // UE_LOG(LogAINavigationFilter, Warning, TEXT("[fc: %llu] %s - %d, -- %f, %f, %f, %f, %f, %f, %f"),
        //     GFrameCounter, *querrier->GetName(), curPoly->getArea(),
        //     default_area_cost, disallowed_area_addon, wrong_room_addon, danger_area_cost, nav_link_cost, additional_area_cost, area_cost);

        const auto offset = FVector::UpVector * 30;
        UE_VLOG_SEGMENT (querrier, LogAINavigationFilter, VeryVerbose, start_location+offset, end_location+offset, FColor::Blue, TEXT(""));
        UE_VLOG_LOCATION(querrier, LogAINavigationFilter, VeryVerbose, end_location  +offset, 5,                   FColor::Blue, TEXT("%d"), DebugCallCount);
    }
#endif

    return area_cost;
}

bool FRecastQueryARPGFilter::passVirtualFilter(const dtPolyRef ref, const dtMeshTile* tile, const dtPoly* poly) const
{
    // if (FNavFlagsHelper::IsSet(poly->flags, ENavPolyFlag::Blocked))
    //     return false;

    return FRecastQueryFilter::passVirtualFilter(ref, tile, poly);
}

FVector FRecastQueryARPGFilter::GetNearestPointOnLineSegment(const FVector& inPoint, const FVector& inLineSegmentStart,
	const FVector& inLineSegmentEnd, bool& isHeadingTowards)
{
    isHeadingTowards = false;

    const auto dir_point_start = inPoint - inLineSegmentStart;
    const auto dir_end_start = inLineSegmentEnd - inLineSegmentStart;

    const float dot = FVector::DotProduct(dir_point_start, dir_end_start);
    const float dir_end_start_length = dir_end_start.SizeSquared() ;
    const float distance = dot / dir_end_start_length;

    if (distance < 0)
        return inLineSegmentStart;

    isHeadingTowards = true;

    if (distance > 1)
        return inLineSegmentEnd;

    return inLineSegmentStart + dir_end_start * distance;
}

dtReal FRecastQueryARPGFilter::GetDefaultAreaCost(const dtReal* pa, const dtReal* pb, const dtPoly* curPoly,
                                                  const dtPoly* nextPoly, const float inModifiedAreaCost) const
{
#if WITH_FIXED_AREA_ENTERING_COST
    const dtReal area_change_cost = (nextPoly != nullptr && nextPoly->getArea() != curPoly->getArea()) ? data.m_areaFixedCost[nextPoly->getArea()] : 0.f;
    float result =(dtVdist(pa, pb)) * inModifiedAreaCost + area_change_cost;
	return result;
#else
    return (dtVdist(pa, pb)) * inModifiedAreaCost;
#endif // #if WITH_FIXED_AREA_ENTERING_COST
}

float FRecastQueryARPGFilter::GetNavLinkCost(const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly, float area_cost,
    const FVector& StartLocation, const FVector& EndLocation) const
{
    if(curPoly->getType() == DT_POLYTYPE_GROUND)
        return 0.0f;

    if(PolyRefMask == 0 || NavSys == nullptr)
        return 0.0f;

    const unsigned int polyIdx = curRef & PolyRefMask;
    const int linkIdx = polyIdx - curTile->header->offMeshBase;
    if(linkIdx < 0 || linkIdx >= curTile->header->offMeshConCount)
        return 0.0f;

    auto userId = curTile->offMeshCons[linkIdx].userId;
    if(userId == 0)
        return 0.0f;

	auto NavLink = NavSys->GetCustomLink(FNavLinkId(userId));
    auto link = Cast<INavLinkCustomInterface>(NavLink);
    if(link == nullptr)
        return 0.0f;

	auto TraversalLink = Cast<INavLinkTraversalInterface>(NavLink);
	if (!TraversalLink)
		return 0.f;

	auto AIController = QuerrierController.Get();
	UE_VLOG_LOCATION(AIController, LogNavigation, Verbose, StartLocation, 24, FColor::Cyan, TEXT("NavLink Start"));
	UE_VLOG_LOCATION(AIController, LogNavigation, Verbose, EndLocation, 24, FColor::Orange, TEXT("NavLink End"));
    return TraversalLink->GetCostMultiplier(AIController);
}

float FRecastQueryARPGFilter::GetAvoidThreatsCost(const FVector& StartLocation, const FVector& EndLocation, float BaseWeight) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FRecastQueryARPGFilter::GetAvoidThreatsCost)

	if (!NpcController->IsWantToAvoidThreats())
	    return 0.f;

	auto AIController = QuerrierController.Get();
	// auto AISightInterface = Cast<IAISightTargetInterface>(NpcPawn);
	const float DotProductThreshold = 0.5f; // TODO parametrize
	const float DesiredAvoidDistance = NpcController->GetPathfindingDesiredAvoidThreatsDistance();
	const float DesiredAvoidanceDistanceSq = DesiredAvoidDistance * DesiredAvoidDistance;
	const float PathfindingAvoidThreatsScoreFactor = NpcController->GetPathfindingAvoidThreatsScoreFactor();
	const auto& Threats = NpcController->GetThreats();
	// 1. Distance check
	// 2. Dot product
	// 3. line trace (can be seen from)
	// i believe start location doesnt matter since this code is executed from 1 segment to another so current end segment is start segment in next call

	float Result = 0.f;
	for (const auto& Threat : Threats)
	{
	    if (!Threat.Key.IsValid())
	        continue;

		FVector LastKnownLocation = Threat.Key->GetActorLocation();
        if (LastKnownLocation == FAISystem::InvalidLocation || LastKnownLocation == FVector::ZeroVector)
            continue;

	    float DistScore = 1.f;

	    // float DistanceFromStartSq = FVector::DistSquared(StartLocation, Enemy.SenseHolder.LastKnowLocation);
	    float DistanceFromEndSq = FVector::DistSquared(EndLocation, Threat.Key->GetActorLocation());
	    // DistScore = FMath::Max(1.f - DistanceFromStartSq / DesiredAvoidanceDistanceSq, 1.f - DistanceFromEndSq / DesiredAvoidanceDistanceSq);
	    DistScore = 1.f - DistanceFromEndSq / DesiredAvoidanceDistanceSq;

	    // if threat is far away - continue
	    if (DistScore <= 0.f)
	        continue;

	    const float DotProductEnemyToEnd = Threat.Key->GetActorForwardVector().CosineAngle2D(EndLocation - Threat.Key->GetActorLocation());

#if WITH_EDITOR
	    UE_VLOG_ARROW(AIController, LogAvoidThreatsPathfollowing_Dot, Verbose, Threat.Key->GetActorLocation(),
            EndLocation + FVector::UpVector * 50.f, FColor::White, TEXT("DP = %.2f"), DotProductEnemyToEnd);

	    UE_VLOG_LOCATION(AIController, LogPathFollowing, Verbose, Threat.Key->GetActorLocation(), 25, FColor::Red, TEXT("Threat"));
	    UE_VLOG_ARROW(AIController, LogPathFollowing, Verbose, Threat.Key->GetActorLocation(),
            Threat.Key->GetActorLocation() + Threat.Key->GetActorForwardVector() * 150.f, FColor::White, TEXT("Threat FV"));
	    UE_VLOG_ARROW(AIController, LogAvoidThreatsPathfollowing_Distance, Verbose, Threat.Key->GetActorLocation(),
                    EndLocation + FVector::UpVector * 50.f, FColor::White, TEXT("D = %.2f"), FMath::Sqrt(DistanceFromEndSq));
#endif

	    bool bCanBeSeen = false;
	    if (DotProductEnemyToEnd > DotProductThreshold)
	    {
	        TRACE_CPUPROFILER_EVENT_SCOPE(FRecastQueryARPGFilter::GetAvoidThreatsCost::Trace)
	        if (auto EnemyPawn = Cast<APawn>(Threat.Key.Get()))
	        {
	            FVector EnemyViewLocation;
	            FRotator EnemyViewDirection;
	            EnemyPawn->GetActorEyesViewPoint(EnemyViewLocation, EnemyViewDirection);
	            FHitResult HitResult;
	            FCollisionQueryParams CollisionQueryParams;
	            CollisionQueryParams.AddIgnoredActor(EnemyPawn);

	            bCanBeSeen = !QuerrierController->GetWorld()->LineTraceSingleByChannel(HitResult, EnemyViewLocation,
	                EndLocation + FVector::UpVector * 120.f, ECC_Visibility, CollisionQueryParams);
	            UE_VLOG_ARROW(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Trace, VeryVerbose, EnemyViewLocation,
                    EndLocation + FVector::UpVector * 120.f, bCanBeSeen ? FColor::Green : FColor::Red,
                    TEXT("%s"), bCanBeSeen ? TEXT("Can be seen") : TEXT("Cannot be seen"));
	        }
	    }

	    if (!bCanBeSeen)
	        continue;

	    const float ScoreDelta = PathfindingAvoidThreatsScoreFactor * (DistScore + FMath::Max(0.f, DotProductEnemyToEnd));
	    Result += ScoreDelta;
	    UE_VLOG(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Weight, VeryVerbose,
	        TEXT("Avoiding threat %s added %.2f to %s path segment score"), *Threat.Key->GetName(), ScoreDelta, *EndLocation.ToString());
	    UE_VLOG_ARROW(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Weight, VeryVerbose, Threat.Key->GetActorLocation(),
	        EndLocation + FVector::UpVector * 50, FColor::Orange, TEXT("%.2f"), ScoreDelta);
	}

	// UE_VLOG(QuerrierController.Get(), LogPathFollowing, Verbose, TEXT("Avoiding threats: added total weight = %.2f to base weight = %.2f"), Result, BaseWeight);
	UE_VLOG_LOCATION(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Weight, Verbose, EndLocation + FVector::UpVector * 50, 25, FColor::Cyan,
	    TEXT("Base Weight = %.2f, Avoid weight = %.2f"), BaseWeight, Result);

	return Result;
}

float FRecastQueryARPGFilter::GetSurroundTargetCost(const FVector& EndLocation) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FRecastQueryARPGFilter::GetAvoidAggroThreatLoSCost)

	if (!NpcController->IsSurroundingTarget())
	    return 0.f;

	// auto AISightInterface = Cast<IAISightTargetInterface>(NpcPawn);
	const float DotProductThreshold = 0.f; // TODO parametrize, run-up at 90 degrees at least
    auto AggroTarget = QuerrierController->GetFocusActor();
	if (!AggroTarget)
        return 0.f;

	float Result = 0.f;
	const float DotProductEnemyToEnd = AggroTarget->GetActorForwardVector().CosineAngle2D(EndLocation - AggroTarget->GetActorLocation());
	bool bCanBeSeen = false;
	if (DotProductEnemyToEnd > DotProductThreshold)
	{
	    TRACE_CPUPROFILER_EVENT_SCOPE(FRecastQueryARPGFilter::GetAvoidAggroThreatLoSCost::Trace)
	    FVector EnemyViewLocation;
	    FRotator EnemyViewDirection;
	    AggroTarget->GetActorEyesViewPoint(EnemyViewLocation, EnemyViewDirection);
	    FHitResult HitResult;
	    FCollisionQueryParams CollisionQueryParams;
	    CollisionQueryParams.AddIgnoredActor(AggroTarget);

	    bCanBeSeen = !QuerrierController->GetWorld()->LineTraceSingleByChannel(HitResult, EnemyViewLocation,
	        EndLocation + FVector::UpVector * 120.f, ECC_Visibility, CollisionQueryParams);

#if WITH_EDITOR
	    if (bCanBeSeen)
	    {
	        UE_VLOG_ARROW(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Dot, VeryVerbose, EnemyViewLocation, EndLocation + FVector::UpVector * 120.f,
	            FColor::Orange, TEXT("Can see (dp = %.2f)"), DotProductEnemyToEnd);
	        UE_VLOG_ARROW(QuerrierController.Get(), LogAvoidThreatsPathfollowing_Dot, VeryVerbose, EnemyViewLocation,
	            EnemyViewLocation + EnemyViewDirection.Vector() * 200.f, FColor::Orange, TEXT("FV"));
	    }
#endif
	}

	float SeenScore = 75.f;
	if (auto AISettings = GetDefault<UNpcCombatSettings>())
	{
	    SeenScore = AISettings->SurroundAvoidancePathfindingScore;
	}

	return bCanBeSeen ? SeenScore : 0.f;
}


#endif	// WITH_RECAST
//------------------------------------------------------------------------------------------//
UARPGNavigationQueryFilter::UARPGNavigationQueryFilter()
{
    bInstantiateForQuerier = true;
}

void UARPGNavigationQueryFilter::SetRecastFilter(FNavigationQueryFilter& Filter) const
{
    Filter.SetFilterType<FRecastQueryARPGFilter>();
    check(Filter.GetImplementation());
}

void UARPGNavigationQueryFilter::InitializeFilter(const ANavigationData& NavData, const UObject* Querier, FNavigationQueryFilter& Filter) const
{
	const AAIController* AsAIController = Cast<AAIController>(Querier);
    const auto AsPawn = AsAIController != nullptr ? AsAIController->GetPawn() : Cast<const APawn>(Querier);

    if (AsPawn != nullptr)
    {
#if WITH_RECAST
        SetRecastFilter(Filter);
        const auto danger_avoidance_filter = static_cast<FRecastQueryARPGFilter*>(Filter.GetImplementation());

        // danger_avoidance_filter->PolyRefMask = ((dtPolyRef) 1 << Cast<ARecastNavMesh>(&NavData)->PolyRefNavPolyBits) - 1;

        if(auto const *recast_nm = Cast<ARecastNavMesh>(&NavData))
        {
            //if(auto detour_nm = recast_nm->GetRecastNavMeshImpl()->GetRecastMesh())
            const auto num_of_bits = FMath::Min(31, recast_nm->PolyRefNavPolyBits);
            danger_avoidance_filter->PolyRefMask = ((dtPolyRef)1<<num_of_bits)-1;
        }

        danger_avoidance_filter->NavSys  = FNavigationSystem::GetCurrent<UNavigationSystemV1>(NavData.GetWorld());
    	
        danger_avoidance_filter->QuerrierController = AsAIController;
    	if (auto NpcController = Cast<INpcControllerInterface>(AsAIController))
    	{
    		// danger_avoidance_filter->NpcController.SetObject(AsAIController);
    		// danger_avoidance_filter->NpcController.SetInterface(NpcController);
    		danger_avoidance_filter->NpcController = NpcController;
    	}
    	
        danger_avoidance_filter->QuerrierLocation = GetQuerrierLocation(AsPawn);
        danger_avoidance_filter->DangerAreaCostMultiplier = CostMultiplier;
        danger_avoidance_filter->setHeuristicScale(HeuristicScale);

		
#endif // WITH_RECAST
    }
		
    Super::InitializeFilter(NavData, Querier, Filter);
}

FVector UARPGNavigationQueryFilter::GetQuerrierLocation(const APawn* const Pawn)
{
    return Pawn != nullptr ? Pawn->GetActorLocation() : FVector::ZeroVector;
}
