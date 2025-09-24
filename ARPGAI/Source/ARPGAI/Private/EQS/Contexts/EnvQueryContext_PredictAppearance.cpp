// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS/Contexts/EnvQueryContext_PredictAppearance.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

void UEnvQueryContext_PredictAppearance::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                      FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;
	
	auto AIController = QuerierPawn->GetController<AAIController>();
	if (AIController == nullptr)
		return;

	TRACE_CPUPROFILER_EVENT_SCOPE(UEnvQueryContext_PredictAppearance::ProvideContext)
	
	TArray<FVector> AwaitedLocationsLocations;
	FVector PredictionTargetLocation = FAISystem::InvalidLocation;
	
	auto NpcComponent = QuerierPawn->FindComponentByClass<UNpcComponent>();
	if (auto AwaitedActor = NpcComponent->GetStoredActor(PredictionTargetParameterTag))
		PredictionTargetLocation = AwaitedActor->GetActorLocation();
	else
		PredictionTargetLocation = NpcComponent->GetStoredLocation(PredictionTargetParameterTag);
	
	if (PredictionTargetLocation == FAISystem::InvalidLocation)
	{
		UE_VLOG(AIController, LogARPGAI_EQS, Warning, TEXT("No locations to search predicted appearance position for"));
		return;
	}
	
	auto NavSys = UNavigationSystemV1::GetCurrent(AIController);
	FVector NpcLocation;
	FRotator NpcRotation;
	QuerierPawn->GetActorEyesViewPoint(NpcLocation, NpcRotation);
	const ANavigationData* NavData = NavSys->GetNavDataForProps(AIController->GetNavAgentPropertiesRef(), NpcLocation);
	if (!NavData)
		return;
	
	FPathFindingQuery PathFindingQuery;
	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(15.f);
	FVector PathStart = PredictionTargetLocation;
	FVector PathEnd = NpcLocation;

	if (bInversePath)
	{
		PathStart = NpcLocation;
		PathEnd = PredictionTargetLocation;
	}
	
	FPathFindingQuery Query(AIController, *NavData, PathStart, PathEnd);
	FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, bUseHierarchicalPathfinding ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular);
	if (PathFindingResult.Result == ENavigationQueryResult::Type::Success)
	{
		const FVector SweepEnd = bInversePath ? PredictionTargetLocation : NpcLocation;
		constexpr float VerticalOffset = 150.f;
		const auto& PathPoints = PathFindingResult.Path->GetPathPoints();
		for (const auto& PathPoint : PathPoints)
		{
			FHitResult Hit;
			FVector PredictedAppearanceLocation = PathPoint.Location + FVector::UpVector * VerticalOffset;
			
			bool bCanSee = !QuerierPawn->GetWorld()->SweepSingleByChannel(Hit, PredictedAppearanceLocation, SweepEnd, FQuat::Identity,
				TraceChannel, SweepShape);

			if (bCanSee)
			{
				UE_VLOG_LOCATION(AIController, LogARPGAI_EQS, Verbose, PredictedAppearanceLocation, 20, FColor::White, TEXT("Predicted appearance location"));
				AwaitedLocationsLocations.Add(PredictedAppearanceLocation);
				break;
			}
		}
	}
	else
	{
		UE_VLOG(AIController, LogARPGAI_EQS, Warning, TEXT("Failed to find path %s prediction target"), bInversePath ? TEXT("from") : TEXT("to"));
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, AwaitedLocationsLocations);
}
