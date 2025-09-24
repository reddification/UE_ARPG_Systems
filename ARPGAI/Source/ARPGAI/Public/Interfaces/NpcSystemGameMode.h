// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Components/NpcSpawnerComponent.h"
#include "NpcSystemGameMode.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNpcSystemGameMode : public UInterface
{
	GENERATED_BODY()
};

class ARPGAI_API INpcSystemGameMode
{
	GENERATED_BODY()

DECLARE_MULTICAST_DELEGATE_OneParam(FWorldStateChangedEvent, const FGameplayTagContainer& NewWorldState);
DECLARE_MULTICAST_DELEGATE_OneParam(FNpcDayTimeChangedEvent, const FGameplayTag& NewDayTime);
	
public:
	virtual float GetTimeRateSeconds() const = 0;
	virtual FVector GetNpcLocation(const FGameplayTag& LocationTagId, const FVector& QuerierLocation, bool bRandomInVolume) const = 0;
	virtual bool IsWorldAtState(const FGameplayTagQuery& GameplayTagQuery) const = 0;
	virtual void ReportNpcSpeak(AActor* Npc, const FGameplayTag& NpcIdTag, const FGameplayTag& AISoundTag, const float Range) = 0;
	virtual const FGameplayTagContainer& GetWorldState() const = 0;
	virtual const FDateTime& GetARPGAIGameTime() const = 0;
	virtual const FGameplayTag& GetForcedAttitudeToActor(const FGameplayTag& NpcIdTag, const FGameplayTagContainer& ActorTags) const = 0;
	virtual APawn* SpawnNpc(const FGameplayTag& NpcId, const FVector& SpawnLocation, const FGameplayTagContainer& WithTags) = 0;
	virtual void TriggerNpcSpawnerWithDelay(UNpcSpawnerComponent* NpcSpawnerComponent, float DelayGameHours) = 0;

	virtual AActor* GetNpcArea(const FGameplayTag& AreaId, const APawn* ForPawn) const = 0;
	virtual TArray<AActor*> GetNpcAreas(const FGameplayTagContainer& AreaIds, const APawn* ForPawn) const = 0;
	virtual float ConvertGameTimeToRealTime(float GameTimeDurationInHours) const = 0;
	virtual const FGameplayTag& GetDayTime() const = 0;

	FWorldStateChangedEvent NpcWorldStateChangedEvent;
	FNpcDayTimeChangedEvent NpcDayTimeChangedEvent;
};
