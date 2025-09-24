// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "Interfaces/QuestCharacter.h"
#include "FlowNodeAddon_QuestEventTrigger_CrossLocation.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_CrossLocation : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	// if true, event will be triggered when a character enters the location, if false - when leaves
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnter = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Location,Location"))
	FGameplayTag LocationIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterTagId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery CharacterTagsFilter;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;

#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
private:
	void OnLocationCrossed(const FGameplayTag& VisitedLocationIdTag, const IQuestCharacter* QuestCharacter);
};
