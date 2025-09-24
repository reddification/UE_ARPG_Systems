// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_UpdateWorldState.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_UpdateWorldState : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer WorldStateTagsChange;
	
	// if false - tags will be removed from world state
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAppend = true;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;

#if WITH_EDITOR
	virtual FString GetQuestActionDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};
