// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_ChangeTagsOnNpcs.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_ChangeTagsOnNpcs : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterFilter;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAdd = true;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
