#include "EQS/Tests/EnvQueryTest_DoesPathExist.h"

#include "AIController.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "NavFilters/NavigationQueryFilter.h"

#define LOCTEXT_NAMESPACE "EnvQueryGenerator"

UEnvQueryTest_DoesPathExist::UEnvQueryTest_DoesPathExist()
{
	Context = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Medium;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);// same as if the test mode was PathExist 
}

void UEnvQueryTest_DoesPathExist::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	PathFromContext.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);

	bool bWantsPath = BoolValue.GetValue();
	bool bPathToItem = PathFromContext.GetValue();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(QueryInstance.World);
	if (NavSys == nullptr || QueryOwner == nullptr)
	{
		return;
	}

	ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
	if (NavData == nullptr)
	{
		return;
	}

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	EPathFindingMode::Type PFMode(EPathFindingMode::Regular);
	AAIController* QueryOwnerController = Cast<AAIController>(Cast<APawn>(QueryOwner)->GetController());
	if (IsValid(QueryOwnerController) == false)
	{
		return;
	}

	const TSubclassOf<UNavigationQueryFilter>& NavQueryFilter =  QueryOwnerController->GetDefaultNavigationFilterClass();
	FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwnerController, NavQueryFilter);
	NavData->BeginBatchQuery();
	if (bPathToItem)
	{
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const bool bFoundPath = TestPathTo(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, NavFilter, QueryOwner);
				It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
			}
		}
	}
	else
	{
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const bool bFoundPath = TestPathFrom(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, NavFilter, QueryOwner);
				It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
			}
		}
	}
	NavData->FinishBatchQuery();
}

FText UEnvQueryTest_DoesPathExist::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("Checking if path exist")));
}

FText UEnvQueryTest_DoesPathExist::GetDescriptionDetails() const
{
	return FText::FromString(TEXT("I don't even know where this text goes"));
}

bool UEnvQueryTest_DoesPathExist::TestPathFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

bool UEnvQueryTest_DoesPathExist::TestPathTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

ANavigationData* UEnvQueryTest_DoesPathExist::FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner) const
{
	INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner);
	if (NavAgent)
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
	}

	return NavSys.GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
}

#undef LOCTEXT_NAMESPACE