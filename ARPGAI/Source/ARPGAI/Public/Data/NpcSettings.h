#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "NpcSettings.generated.h"

class UEnvQuery;

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "NPC"))
class ARPGAI_API UNpcSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer InteractionsInterruptedByDialogue;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery InterruptCurrentActivityForDialogueWhenNpcInStateFilter;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> NpcAttitudesDurationGameTime;

	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UEnvQuery> SmartObjectNpcGoalEqs;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float FollowLeaderPredictionTime = 1.f;

	// priority for AIController::SetFocus
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 3, ClampMin = 3))
	int DialogueFocusPriority = 3;
};
