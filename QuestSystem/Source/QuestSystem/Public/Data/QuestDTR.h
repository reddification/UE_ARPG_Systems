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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UFlowAsset> QuestFlow;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FQuestRequirementBase>> QuestRequirements;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FQuestNavigationGuidanceData> NavigationGuideances;
};
