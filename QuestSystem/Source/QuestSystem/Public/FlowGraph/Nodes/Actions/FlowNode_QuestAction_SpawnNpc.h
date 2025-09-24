// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "Data/QuestActions.h"
#include "FlowNode_QuestAction_SpawnNpc.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_SpawnNpc : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuestActionSpawnNpcAndSetBehavior SpawnNpcAndSetBehaviorData;
	
	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
