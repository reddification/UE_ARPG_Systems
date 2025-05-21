// 

#pragma once

#include "CoreMinimal.h"
#include "NpcReactionsDataAsset.generated.h"

UENUM()
enum class EReactionBehaviorType : uint8
{
	None,
	CallForHelp,
	LookAtIncident,
	DisplayReactionToCharacter,
	StartDialogue,
	UseSmartObject,
	RequestNpcReaction
};

UCLASS()
class ARPGAI_API UNpcPerceptionReactionEvaluatorsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<class UNpcReactionEvaluatorBase*> PerceptionReactionEvaluators;
};
