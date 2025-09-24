// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_ShowScreenText.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_ShowScreenText : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ScreenTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SubTitle;
	
	// In seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShowDuration = 5.f;
	
	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
