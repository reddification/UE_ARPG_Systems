// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WorldLocationComponent.h"
#include "Data/QuestActions.h"
#include "UObject/Interface.h"
#include "QuestCharacter.generated.h"

class UAbilitySet;
struct FGameplayTag;
// This class does not need to be modified.
UINTERFACE()
class QUESTSYSTEM_API UQuestCharacter : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class QUESTSYSTEM_API IQuestCharacter
{
	GENERATED_BODY()

public:
	virtual int GetCountOfItem(const FGameplayTag& ItemTag, const FGameplayTagQuery& ItemFilter) const = 0;
	virtual bool ItemsSattisfyRequirement(const FGameplayTagQuery& ItemsTagQuery) const = 0;
	virtual FVector GetCharacterLocation() const = 0 ;
	virtual FVector GetQuestCharacterEyesLocation() const = 0;
	virtual bool IsPlayer() const = 0;

	virtual void AddQuestTags(const FGameplayTagContainer& GameplayTags) = 0;
	virtual void RemoveQuestTags(const FGameplayTagContainer& GameplayTags) = 0;
	
	virtual void AddQuestState(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams) = 0;
	virtual void RemoveQuestState(const FGameplayTag& StateTag) = 0;

	virtual void OnWorldLocationEntered(const FGameplayTag& LocationIdTag) = 0;
	virtual void OnWorldLocationLeft(const FGameplayTag& LocationId) = 0;

	virtual FGameplayTag GetQuestCharacterIdTag() const = 0;
	virtual FGameplayTagContainer GetQuestCharacterTags() const = 0;

	virtual void ChangeItemsCount(const FGameplayTag& ItemId, int Count) = 0;
	virtual bool StartQuestDialogueWithNpc(AActor* Npc, const FGameplayTag& OptionalDialogueId) = 0;
};
