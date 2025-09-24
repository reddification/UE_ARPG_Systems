// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_NpcDisplayReaction.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_NpcDisplayReaction : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PhraseId;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
