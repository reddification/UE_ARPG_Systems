// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_JournalLog.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_JournalLog : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine = true))
	FText JournalEntry;

	// i guess you can put tags like "important", or some other specifiers to get different fonts/colors
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer JournalEntryTags;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;

#if WITH_EDITOR
	virtual FString GetQuestActionDescription() const override;
#endif
};
