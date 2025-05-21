// Copyright 2019 - 2022 INGAME STUDIOS, a.s., All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <NavFilters/NavigationQueryFilter.h>

#if WITH_RECAST
#include <Detour/DetourNavMesh.h>
#include <NavMesh/RecastQueryFilter.h>
#endif //WITH_RECAST

#include "RecastQueryARPGFilter.generated.h"

class AAIController;
DECLARE_LOG_CATEGORY_EXTERN(LogAvoidThreatsPathfollowing_Dot, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(LogAvoidThreatsPathfollowing_Distance, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogAvoidThreatsPathfollowing_Weight, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogAvoidThreatsPathfollowing_Trace, Log, All)

/// See into commit where this define was declared.
/// There are other code used for debugging.
/// Merge what you need and try retest it.
#define DEBUGING_PATHFINDING 1

#if WITH_RECAST

class ARPGAI_API FRecastQueryARPGFilter : public FRecastQueryFilter
{
public:
    /// Returns cost to move from the beginning to the end of a line segment
    /// that is fully contained within a polygon.
    ///  @param[in]		pa			The start position on the edge of the previous and current polygon. [(x, y, z)]
    ///  @param[in]		pb			The end position on the edge of the current and next polygon. [(x, y, z)]
    ///  @param[in]		prevRef		The reference id of the previous polygon. [opt]
    ///  @param[in]		prevTile	The tile containing the previous polygon. [opt]
    ///  @param[in]		prevPoly	The previous polygon. [opt]
    ///  @param[in]		curRef		The reference id of the current polygon.
    ///  @param[in]		curTile		The tile containing the current polygon.
    ///  @param[in]		curPoly		The current polygon.
    ///  @param[in]		nextRef		The reference id of the next polygon. [opt]
    ///  @param[in]		nextTile	The tile containing the next polygon. [opt]
    ///  @param[in]		nextPoly	The next polygon. [opt]
    /// virtual scoring function implementation (defaults to getInlineCost). @see getCost for parameter description
    virtual dtReal getVirtualCost(const dtReal* pa, const dtReal* pb, const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
        const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly, const dtPolyRef nextRef, const dtMeshTile* nextTile,
        const dtPoly* nextPoly) const override;

    virtual bool passVirtualFilter(const dtPolyRef ref, const dtMeshTile* tile, const dtPoly* poly) const override;
    
    float DangerAreaCostMultiplier = 5;
    float IdleObstacleCostMultiplier = 100.f;
    float WrongRoomCostMultiplier = 100.f;
    float ScriptDissalovedAreaCostMulitplier = 100.f;
    //float AngleCostMultiplier = 5;

    dtPolyRef PolyRefMask = 0;

    TWeakObjectPtr<const UNavigationSystemV1> NavSys;
    TWeakObjectPtr<const AAIController> QuerrierController;

    // TODO might be dangerous to just store interface like this, but I can't really have a UPROPERTY here
    const class INpcControllerInterface* NpcController = nullptr;
    
    FVector QuerrierLocation = FVector::ZeroVector;

#if DEBUGING_PATHFINDING
    explicit FRecastQueryARPGFilter();
    virtual ~FRecastQueryARPGFilter() override;

    static bool DebugPath;
    static uint32 DebugCallCount;
    static uint32 DebugDrawMaxCallCount;
#endif

protected:
    dtReal GetDefaultAreaCost(const dtReal* pa, const dtReal* pb, const dtPoly* curPoly, const dtPoly* nextPoly,
                              float area_cost) const;
    float GetNavLinkCost(const dtPolyRef curRef , const dtMeshTile* curTile , const dtPoly* curPoly, float area_cost,
        const FVector& StartLocation, const FVector& EndLocation) const;

    float GetAvoidThreatsCost(const FVector& StartLocation, const FVector& EndLocation, float BaseWeight) const;
    float GetSurroundTargetCost(const FVector& EndLocation) const;

    static FVector GetNearestPointOnLineSegment( const FVector& inPoint, const FVector& inLineSegmentStart, const FVector& inLineSegmentEnd, bool& isHeadingTowards);
};
#endif	// WITH_RECAST

UCLASS( Abstract, Blueprintable )
class ARPGAI_API UARPGNavigationQueryFilter : public UNavigationQueryFilter
{
    GENERATED_BODY()

public:
    UARPGNavigationQueryFilter();

    /** setup filter for given navigation data, use to create custom filters */
    virtual void InitializeFilter( const ANavigationData& NavData, const UObject* Querier, FNavigationQueryFilter& Filter ) const override;

protected:
    UPROPERTY( EditAnywhere, Category = NavigationQueryFilter, meta = (ClampMin = "1.0") )
    float RadiusMultiplier = 2;

    UPROPERTY( EditAnywhere, Category = NavigationQueryFilter, meta = (ClampMin = "1.0") )
    float CostMultiplier = 5;

    UPROPERTY( EditAnywhere, Category = NavigationQueryFilter, meta = (ClampMin = "0.1") )
    float HeuristicScale = 0.9f;

    //If false, the system will not alter the cost of the nodes
    UPROPERTY( EditAnywhere, Category = NavigationQueryFilter )
    bool IsActive = true;

    virtual void SetRecastFilter( FNavigationQueryFilter& Filter ) const;

private:
    static FVector GetQuerrierLocation(const APawn* Pawn);
};
