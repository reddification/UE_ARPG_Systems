// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNode_QuestAction.h"
#include "FlowNode_QuestAction_SetSublevelState.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestAction_SetSublevelState : public UFlowNode_QuestAction
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SublevelId;

	// Desired sublevel state
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bLoaded = true;

	// Useful to prevent sublevel to load/unload in front of player which can be deduced if player has some location tag on them
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery PostponeIfPlayerHasTags;

	virtual EQuestActionExecuteResult ExecuteInternal(const FQuestSystemContext& Context) override;

#if WITH_EDITOR
	virtual FString GetQuestActionDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

private:
	void OnQuestDataLayerStateChanged(const FGameplayTag& DataLayerId, bool bNewLoaded);
};
