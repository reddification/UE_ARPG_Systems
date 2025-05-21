#pragma once
#include "GameplayTagContainer.h"

#include "RealtimeDialogueDataTypes.generated.h"

class USoundCue;

USTRUCT(BlueprintType)
struct FNpcRealtimeDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GestureTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText PhraseText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USoundCue> DialogueSpeech;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.1f, ClampMin = 0.1f))
	float FallbackSpeechDuration = 3.f;
};

USTRUCT(BlueprintType)
struct FNpcRealtimeDialogueLines
{
	GENERATED_BODY()

	// variants of what an NPC can say for a given key. Shouldn't be considered a sequence
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNpcRealtimeDialogueLine> LineOptions;
};

USTRUCT(BlueprintType)
struct FNpcRealtimeDialogue
{
	GENERATED_BODY()

	// Key can be either "command tag" like when NPC alerts other NPCs about spotted enemy
	// so can it be a LineId of a phrase said by an interlocutor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FNpcRealtimeDialogueLines> DialogueLines;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery AtWorldAndCharacterState; 	
};