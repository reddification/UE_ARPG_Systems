// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "QuestNPC.generated.h"

struct FGameplayTag;
// This class does not need to be modified.
UINTERFACE()
class QUESTSYSTEM_API UQuestNPC : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTSYSTEM_API IQuestNPC
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQuestNpcStateChangedEvent, const IQuestNPC* Npc, const FGameplayTagContainer& NewState)
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FGameplayTag GetQuestNpcIdTag() const = 0;
	virtual FVector GetQuestNpcLocation() const = 0;
	virtual FVector GetQuestNpcEyesLocation() const = 0;
	virtual bool IsQuestNpcUnique() const = 0;
	virtual void TeleportToQuestLocation(const FVector& NewLocation) = 0;

	virtual void AddNpcQuestTags(const FGameplayTagContainer& Tags) = 0; // Overlaps with IQuestCharacter, TODO fix somehow
	virtual void RemoveNpcQuestTags(const FGameplayTagContainer& GameplayTags) = 0;
	
	virtual void RunQuestBehavior(const FGameplayTag& QuestBehaviorIdTag) = 0;
	virtual void StopQuestBehavior() = 0;
	virtual void SayQuestPhrase(const FGameplayTag& PhraseId) = 0;
	virtual void StartQuestDialogue(const FGameplayTag& DialogueId, TArray<AActor*> DialogueParticipants) = 0;
	virtual void ReceiveQuestEvent(const FGameplayTag& EventTag) = 0;

	virtual FGameplayTagContainer GetQuestNpcOwnedTags() const = 0;
	virtual void AddQuestAttitudePreset(const FGameplayTag& NewAttitude, float GameHoursDuration) = 0;
	virtual FGameplayTagContainer GetWorldStateUpdateOnDeath() const = 0;

	// 21.11.2024: @AK this might be a bit stupid, i believe interfaces shouldn't know anything about their owners, but i'm kinda short on time so anything goes now
	virtual APawn* GetQuestNpcPawn() = 0;

	mutable FOnQuestNpcStateChangedEvent OnQuestNpcStateChangedEvent;
};
