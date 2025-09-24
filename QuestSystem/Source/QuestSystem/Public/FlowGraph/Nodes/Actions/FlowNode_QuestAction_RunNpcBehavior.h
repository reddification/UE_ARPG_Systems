// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "Data/QuestActions.h"
#include "FlowNode_QuestAction_RunNpcBehavior.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_RunNpcBehavior : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestActionNpcRunBehavior QuestActionNpcRunBehavior;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;

#if WITH_EDITOR
	virtual FString GetQuestActionDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};
