// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_ArbitraryQuestAction.generated.h"

class UArbitraryQuestAction;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_ArbitraryQuestAction : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UArbitraryQuestAction> ArbitraryQuestActionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FGameplayTag> TagParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, float> FloatParameters;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
