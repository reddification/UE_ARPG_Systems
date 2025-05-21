#pragma once

#include "GameplayTagContainer.h"
#include "NpcDialogueLine.h"
#include "Engine/DataTable.h"
#include "QuestDialogue.generated.h"

USTRUCT(BlueprintType)
struct FInteractionStateEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer StateEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAppendState = true;
};

USTRUCT(BlueprintType)
struct FInteractionCharacterStateEffect : public FInteractionStateEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = 0, UIMin = 0))
	int ParticipantIndex = 0;
};

USTRUCT(BlueprintType)
struct FRunDialogueParticipantBehavior
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = 0, UIMin = 0))
	int ParticipantIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FNpcQuestBehaviorDescriptor BehaviorDescriptor;
};

USTRUCT(BlueprintType)
struct FQuestDialogueLine : public FNpcDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(RowType="QuestDTR"))
	TArray<FDataTableRowHandle> TriggerQuests;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(RowType="QuestTaskDTR"))
	TArray<FDataTableRowHandle> TriggerQuestsEvents;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FInteractionStateEffect> WorldStateEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FInteractionCharacterStateEffect> ParticipantStateEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRunDialogueParticipantBehavior RunBehavior;
};

USTRUCT(BlueprintType)
struct FDialogueScenario
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FQuestDialogueLine> DialogueLines;
};

USTRUCT(BlueprintType)
struct FQuestDialogue
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(RowType="NpcDTR"))
	TArray<FDataTableRowHandle> ExtraParticipants;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FDialogueScenario> DialogueScenarios;
};

