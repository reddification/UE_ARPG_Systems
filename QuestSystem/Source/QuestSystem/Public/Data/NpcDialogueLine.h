#pragma once

#include "NpcDialogueLine.generated.h"

USTRUCT(BlueprintType)
struct FNpcDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0, UIMin = 0))
	int ParticipantIndex = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USoundCue* Speech;
	
	// how far is the dialogue heard by player and/or other AI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AudibilityDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<UAnimMontage*> GestureOptions;

	// Default line duration if no gesture or voice line involved 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LineDuration = 3.f;
};
