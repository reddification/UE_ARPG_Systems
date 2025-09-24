#pragma once
#include "InstancedStruct.h"
#include "QuestActions.h"
#include "QuestRequirements.h"
#include "QuestTypes.h"
#include "FlowAsset.h"
#include "Engine/DataTable.h"
#include "QuestDTR.generated.h"

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestDTR : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine = true))
	FText JournalEntry;

	// Can store tags like is this quest auxiliary, is this quest "silent" so that it is not displayed in journal and no pop ups show up for it
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer QuestTags;

	// TODO consider moving this one to TSoftObjectPtr<UQuestEventsDataAsset>, especially because of usage of instanced objects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(RequiredAssetDataTags="RowStructure=/Script/QuestSystem.QuestEventDTR"))
	class UDataTable* QuestEventsDT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UFlowAsset> QuestFlow;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestRequirementBase>> QuestRequirements;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestActionBase>> BeginQuestActions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestActionBase>> SuccessfulEndQuestActions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestActionBase>> FailureEndQuestActions;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FQuestNavigationGuidanceData> NavigationGuideances;
};
