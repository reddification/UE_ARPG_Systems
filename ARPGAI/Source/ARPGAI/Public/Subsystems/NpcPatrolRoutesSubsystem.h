// 

#pragma once

#include "CoreMinimal.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "Data/AiDataTypes.h"

#include "NpcPatrolRoutesSubsystem.generated.h"

struct FGameplayTag;
class UNpcPatrolRouteComponent;

UCLASS()
class ARPGAI_API UNpcPatrolRoutesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UNpcPatrolRoutesSubsystem* Get(const UObject* WorldContextOption);
	
	void RegisterPatrolRoute(UNpcPatrolRouteComponent* PatrolRouteComponent);
	void UnregisterPatrolRoute(UNpcPatrolRouteComponent* PatrolRouteComponent);

	FNpcPatrolRouteData StartPatrolRoute(const APawn* NpcPawn, const FGameplayTag& RouteId, bool bPreferClosestRoute, bool bUsePathfinding);
	FNpcPatrolRouteData GetActivePatrolRoute(const APawn* NpcPawn);
	void StopPatrolRoute(const APawn* NpcPawn);
	
	FNpcPatrolRouteAdvanceResult GetNextPatrolRoutePoint(const APawn* Npc);

private:
	TMultiMap<FGameplayTag, TWeakObjectPtr<UNpcPatrolRouteComponent>> PatrolRoutes;
	TMap<FGameplayTag, FNpcPatrolRouteData> ActiveNpcsPatrolRoutes;
};