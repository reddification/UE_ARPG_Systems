#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "WorldLocationsSubsystem.generated.h"

class UWorldLocationComponent;
class AWorldLocation;
UCLASS()
class QUESTSYSTEM_API UWorldLocationsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UWorldLocationsSubsystem* Get(const UObject* WorldContextObject);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UWorldLocationComponent* GetWorldLocationRandom(const FGameplayTag& WorldLocationTag) const;
    TArray<UWorldLocationComponent*> GetWorldLocations(const FGameplayTag& WorldLocationTag) const;
	
	const UWorldLocationComponent* GetClosestQuestLocationSimple(const FGameplayTag& LocationIdTag,
	                                                             const FVector& QuerierLocation) const;
	const UWorldLocationComponent* GetClosestQuestLocationComplex(const FGameplayTag& QuestLocationTag,
	                                                              const FVector& QuerierLocation,
	                                                              UObject* WorldContextObject);
	
	void RegisterWorldLocation(UWorldLocationComponent* QuestLocation, const FGameplayTag& LocationTagId);
	void UnregisterWorldLocation(const FGameplayTag& LocationIdTag);

private:
	// TODO leave only TMap and its related functions
	// TArray<AQuestLocation*> QuestLocations;

	TMultiMap<FGameplayTag, UWorldLocationComponent*> QuestLocationsMap;
};
