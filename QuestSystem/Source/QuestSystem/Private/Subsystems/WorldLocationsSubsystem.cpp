#include "Subsystems/WorldLocationsSubsystem.h"

#include "NavigationSystem.h"
#include "Components/WorldLocationComponent.h"

UWorldLocationsSubsystem* UWorldLocationsSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject ? WorldContextObject->GetWorld()->GetSubsystem<UWorldLocationsSubsystem>() : nullptr;
}

void UWorldLocationsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UWorldLocationsSubsystem::RegisterWorldLocation(UWorldLocationComponent* QuestLocation, const FGameplayTag& LocationTagId)
{
	if (!IsValid(QuestLocation))
		return;
	
	QuestLocationsMap.Add(LocationTagId, QuestLocation);
}

void UWorldLocationsSubsystem::UnregisterWorldLocation(const FGameplayTag& LocationIdTag)
{
	QuestLocationsMap.Remove(LocationIdTag);
}

UWorldLocationComponent* UWorldLocationsSubsystem::GetWorldLocationRandom(const FGameplayTag& WorldLocationTag) const
{
	TArray<UWorldLocationComponent*> QuestLocationsArray;
	QuestLocationsMap.MultiFind(WorldLocationTag, QuestLocationsArray);
	if (QuestLocationsArray.Num() == 0)
		return nullptr;

	return QuestLocationsArray[FMath::RandRange(0, QuestLocationsArray.Num() - 1)];
}

TArray<UWorldLocationComponent*> UWorldLocationsSubsystem::GetWorldLocations(const FGameplayTag& WorldLocationTag) const
{
	TArray<UWorldLocationComponent*> WorldLocationsArray;
	QuestLocationsMap.MultiFind(WorldLocationTag, WorldLocationsArray);
	return WorldLocationsArray;
}

const UWorldLocationComponent* UWorldLocationsSubsystem::GetClosestQuestLocationSimple(const FGameplayTag& LocationIdTag,
                                                                                       const FVector& QuerierLocation) const
{
	TArray<UWorldLocationComponent*> QuestLocationsArray;
	QuestLocationsMap.MultiFind(LocationIdTag, QuestLocationsArray);
	if (QuestLocationsArray.Num() == 1)
		return IsValid(QuestLocationsArray[0]) ? QuestLocationsArray[0] : nullptr;
	
	float ClosestSquaredDistance = FLT_MAX;
	const UWorldLocationComponent* FoundLocation = nullptr;
	for (const auto QuestLocation : QuestLocationsArray)
	{
		if (!IsValid(QuestLocation))
			continue;
		
		float SquaredDistance = FVector::DistSquared(QuestLocation->GetOwner()->GetActorLocation(), QuerierLocation);
		if (SquaredDistance < ClosestSquaredDistance)
		{
			FoundLocation = QuestLocation;
			ClosestSquaredDistance = SquaredDistance;
		}
	}

	return FoundLocation;
}

const UWorldLocationComponent* UWorldLocationsSubsystem::GetClosestQuestLocationComplex(const FGameplayTag& QuestLocationTag,
	const FVector& QuerierLocation, UObject* WorldContextObject)
{
	TArray<UWorldLocationComponent*> QuestLocationsArray;
	QuestLocationsMap.MultiFind(QuestLocationTag, QuestLocationsArray);
	if (QuestLocationsArray.Num() == 1)
		return QuestLocationsArray[0];
	
	double ClosestDistance = DBL_MAX;
	const UWorldLocationComponent* FoundLocation = nullptr;
	for (const auto QuestLocation : QuestLocationsArray)
	{
		double TestedDistance = DBL_MAX;
		auto PathLengthRetrieveResult = UNavigationSystemV1::GetPathLength(WorldContextObject, QuerierLocation,
			QuestLocation->GetOwner()->GetActorLocation(), TestedDistance);
		// if (PathLengthRetrieveResult == ENavigationQueryResult::Success)
		// {
			if (TestedDistance < ClosestDistance)
			{
				FoundLocation = QuestLocation;
				ClosestDistance = TestedDistance;
			}
		// }
	}

	return FoundLocation;
}