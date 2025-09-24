// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_SetAttitude.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_SetAttitude : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer SourceCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer TargetCharacterIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Attitude,G2VS2.Npc.Attitude"))
	FGameplayTag NewAttitude;

	// If less than or equal to zero - consider infinite 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin = -1.f, UIMin = -1.f))
	float GameHoursDuration = -1.f;
	
	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;
};
