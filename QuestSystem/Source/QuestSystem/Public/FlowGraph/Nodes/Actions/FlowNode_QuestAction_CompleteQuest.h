// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "Data/QuestEnums.h"
#include "FlowNode_QuestAction_CompleteQuest.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_CompleteQuest : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestState FinalQuestState = EQuestState::Completed;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
