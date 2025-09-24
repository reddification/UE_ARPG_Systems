// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_SendNpcEvent.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_SendNpcEvent : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Event;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
