// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_Interaction.generated.h"

class IQuestCharacter;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_Interaction : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	// Can be none, in this case the event will be triggered disregarding the interactor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InteractionActorId;

	// Can be none, in this case any interaction action will trigger the event. If multiple provided - check is made by "any" operation, not "all". 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Interaction"))
	FGameplayTagContainer InteractionActionsIds;

	// Can be empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery InteractionActorTagsFilter;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override; 
	virtual void UnregisterEventTrigger() override;

private:
	void OnCharacterInteractedWithActor(IQuestCharacter* QuestCharacter, const FGameplayTag& InteractionActorId,
		const FGameplayTagContainer& InteractionActionId, const FGameplayTagContainer& InteractionActorTags);
};
