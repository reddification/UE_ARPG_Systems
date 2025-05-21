#pragma once

#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

class UWorldLocationComponent;
class UWorldLocationsSubsystem;
class IQuestSystemGameMode;
class UWorldStateSubsystem;
class UQuestSubsystem;
class UQuestNpcSubsystem;
class IQuestCharacter;

USTRUCT(BlueprintType)
struct FQuestNavigationGuidanceData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery AtWorldState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery UntilWorldState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag LocationIdTag;
};

struct FQuestNavigationGuidance
{
	const UWorldLocationComponent* QuestLocation = nullptr;
	FGameplayTagQuery UntilWorldState;
	
	void Clear()
	{
		QuestLocation = nullptr;
		UntilWorldState.Clear();
	};
};

struct FQuestSystemContext
{
	TScriptInterface<IQuestCharacter> Player;
	TWeakObjectPtr<UWorldStateSubsystem> WorldStateSubsystem;
	TWeakObjectPtr<UWorldLocationsSubsystem> WorldLocationsSubsystem;
	TWeakObjectPtr<UQuestNpcSubsystem> NpcSubsystem;
	TWeakObjectPtr<UQuestSubsystem> QuestSubsystem;
	TWeakObjectPtr<UWorld> World;
	IQuestSystemGameMode* GameMode = nullptr;
};

USTRUCT(BlueprintType)
struct FDialogueParticipantData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ParticipantId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ParticipantFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float InRange = 1200.f;
	
	// If > 1 - take Count closest NPCs with ParticipantId tag that pass ParticipantFilter
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	int Count = 1;
};