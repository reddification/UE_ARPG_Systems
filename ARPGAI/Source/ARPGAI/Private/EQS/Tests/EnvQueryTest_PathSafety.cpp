#include "EQS/Tests/EnvQueryTest_PathSafety.h"

#include "NavigationSystem.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "NavFilters/NavigationQueryFilter.h"

UEnvQueryTest_PathSafety::UEnvQueryTest_PathSafety()
{
	Cost = EEnvTestCost::Type::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::FilterAndScore;
	FloatValueMin.DefaultValue = 0.25f;
	FloatValueMax.DefaultValue = 1.f;
	DistanceStep.DefaultValue = 150.f;
	ConsiderShortPathSafeLengthThreshold.DefaultValue = 300.f;
	ConsiderSafeDistanceFromThreat.DefaultValue = 1500.f;
	SetWorkOnFloatValues(true);
}

ANavigationData* FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner)
{
	INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner);
	if (NavAgent)
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
	}

	return NavSys.GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
}

void UEnvQueryTest_PathSafety::RunTest(FEnvQueryInstance& QueryInstance) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UEnvQueryTest_PathSafety::RunTest)

	APawn* QueryOwner = Cast<APawn>(QueryInstance.Owner.Get());
	if (!QueryOwner)
		return;
	
	auto NavSys = Cast<UNavigationSystemV1>(QueryInstance.Owner->GetWorld()->GetNavigationSystem());
	if (!NavSys)
		return;

	ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
	if (NavData == nullptr)
		return;

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	DistanceStep.BindData(QueryOwner, QueryInstance.QueryID);
	
	const float MinThresholdValue = FloatValueMin.GetValue();
	const float MaxThresholdValue = FloatValueMax.GetValue();
	const float DistanceStepValue = DistanceStep.GetValue();

	bool bHaveContextActors = false;
	TArray<FVector> ContextLocations;
	TArray<AActor*> ContextActors;
	
	bHaveContextActors = QueryInstance.PrepareContext(ThreatsActorsContext, ContextActors);
	if (!bHaveContextActors)
		QueryInstance.PrepareContext(ThreatsLocationsContext, ContextLocations);
	
    // TODO: unreal insights shows 3ms/2ms on avg/median in development build. consider switching PF mode to hierarchical 
	EPathFindingMode::Type PFMode(bUseHierarchicalPathFinding ? EPathFindingMode::Hierarchical : EPathFindingMode::Regular );  
	FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwner, FilterClass);

	const FVector PawnLocation = QueryOwner->GetActorLocation();
	NavData->BeginBatchQuery();
	ConsiderShortPathSafeLengthThreshold.BindData(QueryOwner, QueryInstance.QueryID);
	const float ConsiderPathLengthSafe = ConsiderShortPathSafeLengthThreshold.GetValue();
	ConsiderSafeDistanceFromThreat.BindData(QueryOwner, QueryInstance.QueryID);
	const float ConsideredSafeDistanceToThreat = ConsiderSafeDistanceFromThreat.GetValue();
	
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It);
		FPathFindingQuery Query(QueryOwner, *NavData, PawnLocation, ItemLocation, NavFilter);
		Query.SetAllowPartialPaths(false);
		FPathFindingResult Result = NavSys->FindPathSync(Query, PFMode);
		if (!Result.IsSuccessful())
		{
			It.ForceItemState(EEnvItemStatus::Failed);
			continue;
		}

		const float TotalPathDistance = Result.Path->GetLength();
		if (TotalPathDistance < ConsiderPathLengthSafe)
		{
			It.SetScore(TestPurpose, FilterType, 1.f, MinThresholdValue, MaxThresholdValue);
			continue;	
		}
		
		bool bItemUnsafeFilteredOut = false;
		float SafePathDistance = 0.f;
		float UnsafePathDistance = 0.f;
		const auto& PathPoints = Result.Path->GetPathPoints();
		bool bPreviousPointSafe = false;
		for (int i = 0; i < PathPoints.Num(); i++)
		{
			if (i > 0)
			{
				UE_VLOG_SEGMENT_THICK(QueryOwner, LogARPGAI_EQS, VeryVerbose, PathPoints[i - 1].Location + FVector::UpVector * 50.f, PathPoints[i].Location + FVector::UpVector * 50.f, FColor::Orange, 3, TEXT(""));
			}
			
			const float SegmentDistance = i > 0 ? (PathPoints[i].Location - PathPoints[i - 1].Location).Size() : DistanceStepValue;
			if (SegmentDistance <= DistanceStepValue)
			{
				TestPointAgainstThreats(QueryOwner, bHaveContextActors, PathPoints[i].Location, ContextLocations, ContextActors, PawnLocation,
				                        SafePathDistance, UnsafePathDistance, bPreviousPointSafe, SegmentDistance, ConsideredSafeDistanceToThreat);
			}
			else
			{
				const FVector SegmentDirection = (PathPoints[i].Location - PathPoints[i - 1].Location).GetSafeNormal();
				float DistanceToCover = SegmentDistance;
				FVector IntermediatePoint = PathPoints[i-1].Location;
				while (DistanceToCover > 0.f)
				{
					const float CurrentStepDistance = FMath::Min(DistanceToCover, DistanceStepValue);
					TestPointAgainstThreats(QueryOwner, bHaveContextActors, IntermediatePoint, ContextLocations, ContextActors, PawnLocation,
					                        SafePathDistance, UnsafePathDistance, bPreviousPointSafe, CurrentStepDistance, ConsideredSafeDistanceToThreat);
					
					DistanceToCover -= CurrentStepDistance;
					IntermediatePoint += SegmentDirection * CurrentStepDistance;
				}
			}

			if (TestPurpose != EEnvTestPurpose::Score && UnsafePathDistance / TotalPathDistance > 1.f - MinThresholdValue)
			{
				bItemUnsafeFilteredOut = true;
				break;
			}
		}

		UE_VLOG_LOCATION(QueryOwner, LogARPGAI_EQS, Log, ItemLocation + FVector::UpVector * 100.f, 25, FColor::Cyan, TEXT("Path safety = %.2f"), SafePathDistance / TotalPathDistance);
		
		if (bItemUnsafeFilteredOut || TotalPathDistance <= 0.f)
			It.ForceItemState(EEnvItemStatus::Failed);
		else
			It.SetScore(TestPurpose, FilterType, SafePathDistance / TotalPathDistance, MinThresholdValue, MaxThresholdValue);
	}

	NavData->FinishBatchQuery();
}

void UEnvQueryTest_PathSafety::TestPointAgainstThreats(APawn* QueryOwner, bool bContextActors, const FVector& TestLocation, const TArray<FVector>& ContextLocations, const TArray<AActor*>& ContextActors,
                                                       const FVector& QuerierLocation, float& SafePathDistance, float& UnsafePathDistance, bool& bPreviousPointSafe, const float SegmentDistance,
                                                       const float ConsideredSafeDistanceToTarget) const
{
	UE_VLOG_LOCATION(QueryOwner, LogARPGAI_EQS, Log, TestLocation + FVector::UpVector * VerticalOffset, 25, FColor::Orange, TEXT("Path safety probe"));
	bool bPointSafe  = false;

	FVector ActualThreatLocation;
	if (bContextActors)
	{
		for (const auto& ThreatActor : ContextActors)
		{
			ActualThreatLocation = ThreatActor->GetActorLocation();
			bPointSafe = IsPointSafe(TestLocation, ActualThreatLocation, ThreatActor->GetActorForwardVector(), QueryOwner);
			if (!bPointSafe)
				break;
		}
	}
	else
	{
		for (const auto& ThreatLocation : ContextLocations)
		{
			ActualThreatLocation = ThreatLocation;
			bPointSafe = IsPointSafe(TestLocation, ThreatLocation, (QuerierLocation - ThreatLocation).GetSafeNormal(), QueryOwner);
			if (!bPointSafe)
				break;
		}	
	}
	
	if (bPointSafe)
	{
		if (bPreviousPointSafe)
			SafePathDistance += SegmentDistance;
		else
			bPreviousPointSafe = true;
	}
	else
	{
		const float DistanceToTarget = (ActualThreatLocation - QuerierLocation).Size();

		UnsafePathDistance += SegmentDistance * FMath::Sqrt(ConsideredSafeDistanceToTarget / DistanceToTarget);
		bPreviousPointSafe = false;
	}
}

bool UEnvQueryTest_PathSafety::IsPointSafe(const FVector& TestLocation, const FVector& ThreatLocation,
                                           const FVector& ThreatForwardVector, const APawn* Querier) const
{
	const float DotProduct = ThreatForwardVector.GetSafeNormal2D() | (TestLocation - ThreatLocation).GetSafeNormal2D();
	if (DotProduct < PointSafeDotProductThreshold)
	{
		UE_VLOG_ARROW(Querier, LogARPGAI_EQS, Verbose, TestLocation, ThreatLocation, FColor::Emerald, TEXT("Safe by dot"));
		return true;
	}
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Querier);
	// const FVector TraceEndLocation = ThreatLocation + (TestLocation - ThreatLocation).GetSafeNormal() * ThreatTraceOffset;
	const FVector TraceEndLocation = ThreatLocation;
	const bool bSightObstructed = Querier->GetWorld()->LineTraceSingleByChannel(HitResult, TestLocation + FVector::UpVector * VerticalOffset,
		TraceEndLocation, TraceChannel, Params);

	if (bSightObstructed)
	{
		UE_VLOG_ARROW(Querier, LogARPGAI_EQS, Verbose, TestLocation, TraceEndLocation, FColor::Emerald, TEXT("Safe by visibility"));
	}
	
	return bSightObstructed;
}


FText UEnvQueryTest_PathSafety::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("Path safety against %s"), IsValid(ThreatsActorsContext)
		? *UEnvQueryTypes::DescribeContext(ThreatsActorsContext).ToString()
		: *UEnvQueryTypes::DescribeContext(ThreatsLocationsContext).ToString()));
}

FText UEnvQueryTest_PathSafety::GetDescriptionDetails() const
{
	FText Result = FText::FromString(FString::Printf(TEXT("Vertical offset = %.2f\nPoint safe if dot product less than %.2f\nDistance step = %s\nConsider short path safe length = %s"),
		VerticalOffset, PointSafeDotProductThreshold, *DistanceStep.ToString(), *ConsiderShortPathSafeLengthThreshold.ToString()));
	Result = FText::Join(FText::FromString(TEXT("\n")), Result, DescribeFloatTestParams());

	return Result;
}