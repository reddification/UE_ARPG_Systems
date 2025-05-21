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
	
public:
	virtual float GetTimeRateSeconds() const = 0;
	virtual FVector GetNpcLocation(const FGameplayTag& LocationTagId, bool bRandomInVolume) const = 0;
	virtual bool IsWorldAtState(const FGameplayTagQuery& GameplayTagQuery) const = 0;
	virtual void ReportNpcSpeak(AActor* Npc, const FGameplayTag& NpcIdTag, const FGameplayTag& AISoundTag, const float Range) = 0;
	virtual const FGameplayTagContainer& GetWorldState() const = 0;
	virtual const FTimespan& GetARPGAIGameTime() const = 0;
	virtual const FGameplayTag& GetForcedAttitudeToActor(const FGameplayTag& NpcIdTag, const FGameplayTagContainer& ActorTags) const = 0;
	virtual APawn* SpawnNpc(const FGameplayTag& NpcId, const FVector& SpawnLocation, const FGameplayTagContainer& WithTags) = 0;
	virtual void TriggerNpcSpawnerWithDelay(UNpcSpawnerComponent* NpcSpawnerComponent, float DelayGameHours) = 0;

	FWorldStateChangedEvent NpcWorldStateChangedEvent;
};
