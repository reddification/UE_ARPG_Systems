// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "NpcRegistrationSubsystem.generated.h"

struct FGameplayTagQuery;
class UNpcComponent;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcRegistrationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UNpcRegistrationSubsystem* Get(const UObject* WorldContextObject);
	
	void RegisterNpc(UNpcComponent* NpcComponent);
	void UnregisterNpc(UNpcComponent* NpcComponent);
	
	TArray<UNpcComponent*> GetNpcsInRange(const FVector& Origin, float Range, const TArray<FGameplayTagQuery>& NpcsFilters);
	// if CountLimit is <= 0 - return all
	TArray<UNpcComponent*> GetNpcsInRange(const FGameplayTag& NpcId, const FVector& Origin, float Range, int CountLimit, const FGameplayTagQuery* NpcsFilter = nullptr);
	UNpcComponent* GetClosestNpc(const FGameplayTag& NpcId, const FVector& QuerierLocation, const FGameplayTagQuery* NpcsFilters = nullptr);
	
private:
	TMultiMap<FGameplayTag, TWeakObjectPtr<UNpcComponent>> Npcs;
};