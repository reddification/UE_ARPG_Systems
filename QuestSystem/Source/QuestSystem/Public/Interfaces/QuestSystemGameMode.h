// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/QuestActionStopConditions.h"
#include "UObject/Interface.h"
#include "QuestSystemGameMode.generated.h"

class IDelayedQuestAction;
struct FGameplayTag;
class IQuestNPC;
// This class does not need to be modified.
UINTERFACE()
class UQuestSystemGameMode : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTSYSTEM_API IQuestSystemGameMode
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual TScriptInterface<IQuestNPC> SpawnQuestNPC(const FGameplayTag& NpcIdTag, const FVector& SpawnLocation, const FGameplayTagContainer& WithTags) = 0;
	virtual void SpawnItem(const FGameplayTag& ItemId, const FGameplayTag& GameplayTag, const FVector& Vector, const FRotator& Rotator, const FGameplayTagContainer& WithTags) = 0;
	virtual void RequestDelayedQuestAction(const FGuid& DelayedQuestActionId, float GameHoursDelay) = 0;
	virtual void RequestDelayedQuestAction(const FGuid& DelayedQuestActionId, const FGameplayTag& AtTimeOfDay) = 0;
	virtual void CancelDelayedQuestActionRequest(const FGuid& DelayedQuestActionId) = 0;

	virtual void SetQuestSublevelState(const FGameplayTag& SublevelId, bool bLoaded, const FGameplayTagQuery& PostponeIfPlayerHasTags) = 0;
	
	virtual const FTimespan& GetQuestSystemGameTime() const = 0;
	virtual void ShowScreenText(const FText& Title, const FText& SubTitle, const FGameplayTag& ScreenType, float ShowDuration) const = 0;
};
