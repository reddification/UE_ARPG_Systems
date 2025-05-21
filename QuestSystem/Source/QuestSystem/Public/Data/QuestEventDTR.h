#pragma once
#include "QuestEnums.h"
#include "Engine/DataTable.h"
#include "QuestEventTriggers.h"
#include "QuestActions.h"
#include "InstancedStruct.h"
#include "QuestEventDTR.generated.h"

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestEventDTR : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString UserDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine = true))
	FText JournalEntry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bActive = true;
	
	// implicit quest tasks don't show up in journal
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bImplicit = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestState QuestStateChange;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bActive == true", ExcludeBaseStruct))
	TInstancedStruct<FQuestEventTriggerBase> EventCompletedTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bActive == true", ExcludeBaseStruct))
	TInstancedStruct<FQuestEventTriggerBase> EventCoveredTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bActive == true", ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestActionBase>> EventOccuredActions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bActive == true"))
	bool bExecuteActionsWhenCovered = false;
};
