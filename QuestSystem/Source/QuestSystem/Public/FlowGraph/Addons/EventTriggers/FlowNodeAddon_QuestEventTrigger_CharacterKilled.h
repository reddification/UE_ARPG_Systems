// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_CharacterKilled.generated.h"

class IQuestCharacter;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_CharacterKilled : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag ByCharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag KilledCharacterId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery KilledCharacterTagsFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int RequiredCount = 1;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;

private:
	int CurrentCount = 0;
	
	void OnCharacterKilled(IQuestCharacter* Killer, IQuestCharacter* Killed);
};
