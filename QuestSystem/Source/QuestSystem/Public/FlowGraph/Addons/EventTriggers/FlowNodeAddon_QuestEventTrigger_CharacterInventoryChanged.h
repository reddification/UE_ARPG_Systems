// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged.generated.h"

class IQuestCharacter;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag CharacterId;

	// Can be non-final tag, like Item.Weapon instead of Item.Weapon.Saber.PirateSaber
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Item,Item"))
	FGameplayTag ItemId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery ItemFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int RequiredCount = 1;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;

private:
	void OnItemAcquired(IQuestCharacter* QuestCharacter, const FGameplayTag& AcquiredItemId,
		const FGameplayTagContainer& ItemTags, int Count);
};
