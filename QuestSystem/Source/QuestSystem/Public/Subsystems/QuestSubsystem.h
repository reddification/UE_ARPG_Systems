#pragma once

#include "CoreMinimal.h"
#include "Components/WorldLocationComponent.h"
#include "Data/QuestDTR.h"
#include "Data/QuestProgress.h"
#include "Engine/DataTable.h"
#include "QuestSubsystem.generated.h"

class IQuestSystemGameMode;
class ULevelSequencePlayer;
class IQuestCharacter;

struct FQuestSetSublevelStateAction;
struct FQuestPlayCutsceneAction;
struct FQuestCustomAction;
struct FQuestActions;
struct FQuestActionSpawnItems;
struct FQuestActionSpawnNPCs;

DECLARE_MULTICAST_DELEGATE_OneParam(FQuestStartedEvent, const FQuestDTR* QuestDTR)
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestEventOccuredEvent, const FQuestDTR* QuestDTR, const FQuestEventDTR* QuestEventDTR);
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestCompletedEvent, const FQuestDTR* QuestDTR, bool bAutocompleted)

DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestCharacterCrossedLocationEvent, const FGameplayTag& LocationId, const IQuestCharacter* QuestCharacter);
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestCharacterKilledEvent, IQuestCharacter* KilledBy, IQuestCharacter* KilledCharacter);
DECLARE_MULTICAST_DELEGATE_FourParams(FQuestCharacterInteractedWithItemEvent, IQuestCharacter* Interactor, const FGameplayTag& InteractionActorId, const FGameplayTagContainer& InteractionActionId, const FGameplayTagContainer& InteractionActorTags)
DECLARE_MULTICAST_DELEGATE_FourParams(FQuestCharacterAcquiredItemEvent, IQuestCharacter* Interactor, const FGameplayTag& ItemId, const FGameplayTagContainer& ItemTags, int Count);
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestPlayerHeardPhraseEvent, const FGameplayTag& SpeakerId, const FGameplayTag& PhraseId);
DECLARE_MULTICAST_DELEGATE_OneParam(FQuestArbitraryEvent, const FGameplayTag& ArbitraryEventId);
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestCharacterKnockdownedEvent, IQuestCharacter* KnockdownedBy, IQuestCharacter* KnockdownedCharacter);
DECLARE_MULTICAST_DELEGATE_TwoParams(FQuestNpcGoalCompletedEvent, IQuestCharacter* Npc, const FGameplayTagContainer& GoalTags);

UCLASS()
class QUESTSYSTEM_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UQuestSubsystem* Get(const UObject* WorldContextObject);
	
	void RegisterPlayerCharacter(ACharacter* Character);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void OnItemAcquired(IQuestCharacter* QuestCharacter,
	                    const FGameplayTag& ItemTagId, const FGameplayTagContainer& ItemTags, int Count);
	void OnLocationReached(const FGameplayTag& LocationIdTag, const IQuestCharacter* EnteredQuestCharacter);
	void OnLocationLeft(const FGameplayTag& LocationIdTag, const IQuestCharacter* EnteredQuestCharacter);
	void OnNpcKilled(IQuestCharacter* Killer, IQuestCharacter* Killed);
	void OnNpcGoalCompleted(IQuestCharacter* Npc, const FGameplayTagContainer& GoalTags);
	void OnActorInteracted(IQuestCharacter* Interactor, const FGameplayTag& InteractionActorId, const FGameplayTagContainer& InteractionActionsId,
	                       const FGameplayTagContainer& InteractionActorTags);
	// void OnNpcInteracted(const FDataTableRowHandle& NpcDTRH, const FGameplayTagContainer& NpcTags);
	void OnPlayerHeard(const FGameplayTag& NpcIdTag, const FGameplayTag& NpcPhraseId);
	void StartQuest(const FDataTableRowHandle& QuestDTRH);
	void ExecuteDelayedAction(const FGuid& DelayedActionId);

	FQuestNavigationGuidance GetNavigationGuidance(const FName& RowName);
	const UWorldLocationComponent* GetQuestLocation(const FGameplayTag& LocationIdTag, const FVector& QuerierLocation) const;
	FVector GetRandomNavigableLocationNearPlayer(const FVector& PlayerLocation, float Radius, float FloorOffset) const;
	void OnNpcKnockdowned(IQuestCharacter* KnockdownedBy, IQuestCharacter* KnockdownedCharacter);

	void ExecuteQuestActionsExternal(const TArray<TInstancedStruct<FQuestActionBase>>& QuestActions);
	void CompleteQuestEventExternal(const FDataTableRowHandle& QuestEventDTRH);

	const TMap<FName, FQuestProgress>& GetActiveQuests() const { return ActiveQuests; }
	const TMap<FName, FQuestProgress>& GetCompletedQuests() const { return CompletedQuests; }
	
	FORCEINLINE bool IsStateLoaded() const { return bStateLoaded; }
	FORCEINLINE void SetStateLoaded() { bStateLoaded = true; }
	
	mutable FQuestStartedEvent QuestStartedEvent;
	mutable FQuestCompletedEvent QuestCompletedEvent;
	mutable FQuestEventOccuredEvent QuestEventOccuredEvent;
	
	mutable FQuestCharacterCrossedLocationEvent QuestCharacterReachedLocationEvent;
	mutable FQuestCharacterCrossedLocationEvent QuestCharacterLeftLocationEvent;
	mutable FQuestCharacterKilledEvent QuestCharacterKilledEvent;
	mutable FQuestCharacterAcquiredItemEvent QuestCharacterAcquiredItemEvent;
	mutable FQuestCharacterInteractedWithItemEvent QuestCharacterInteractedWithItemEvent;
	mutable FQuestPlayerHeardPhraseEvent QuestDialogueLineHeardEvent;
	mutable FQuestCharacterKnockdownedEvent QuestCharacterKnockdownedEvent;
	mutable FQuestNpcGoalCompletedEvent QuestNpcGoalCompletedEvent;
	
	UFUNCTION(BlueprintCallable)
	void Load();
	
private:
    UPROPERTY()
    TScriptInterface<IQuestCharacter> PlayerCharacter;

	UPROPERTY()
	TScriptInterface<IQuestSystemGameMode> GameMode;
	
    UPROPERTY(SaveGame)
    TMap<FName, FQuestProgress> ActiveQuests;

	UPROPERTY(SaveGame)
    TMap<FName, FQuestProgress> CompletedQuests;

	// 17.12.2024 @AK: I'm not sure it's safe to store objects of delayed quest actions as TScriptInterface like this,
	// even if the TMap has UPROPERTY above it
	UPROPERTY()
	TMap<FGuid, TScriptInterface<IDelayedQuestAction>> DelayedQuestActions;
	
	bool CanStartQuest(const FDataTableRowHandle& QuestDTRH);
   
	void InitializeQuest(const FDataTableRowHandle& QuestDTRH, const FQuestDTR* QuestDTR);
	
	FQuestSystemContext GetQuestSystemContext();

	void OnQuestEventOccured(UQuestEventTriggerProxy* QuestEventTrigger);
	void OnQuestEventCovered(UQuestEventTriggerProxy* QuestEventTrigger);
	void CompleteQuestEvent(UQuestEventTriggerProxy* QuestEventTrigger, bool bEventCovered);

	void CompleteQuest(FQuestProgress& CompletedQuest, EQuestState QuestFinalState);
	void ExecuteQuestActions(const FQuestSystemContext& QuestSystemContext, const TArray<TInstancedStruct<FQuestActionBase>>& QuestActions);

	void RequestDelayedAction(const FGuid& ActionId, float DelayInGameHours);
	void RequestDelayedAction(const FGuid& ActionId, const FGameplayTag& AtNextTimeOfDay);

	bool bStateLoaded = false;
};
