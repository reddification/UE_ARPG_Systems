// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_NpcCompletedGoal.generated.h"

class IQuestCharacter;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_NpcCompletedGoal : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcCharacterTagsFilter;
	
	// Can be null
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery NpcGoalTagsFilter;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;
	
private:
	void OnNpcGoalCompleted(IQuestCharacter* Npc, const FGameplayTagContainer& QuestGoalTags);
};
