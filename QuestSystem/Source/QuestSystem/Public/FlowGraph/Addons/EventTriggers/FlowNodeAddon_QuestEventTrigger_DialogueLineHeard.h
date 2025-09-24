// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_DialogueLineHeard.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	// Can be none, in this case the event will be triggered disregarding the character
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Quest,Quest"))
	FGameplayTag PhraseId;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
private:
	void OnDialogueLineHeard(const FGameplayTag& NpcId, const FGameplayTag& HeardPhraseId);
};
