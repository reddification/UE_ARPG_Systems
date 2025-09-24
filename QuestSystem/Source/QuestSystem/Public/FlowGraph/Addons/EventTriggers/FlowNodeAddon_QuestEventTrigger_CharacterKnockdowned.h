// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_CharacterKnockdowned.generated.h"

class IQuestCharacter;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_CharacterKnockdowned : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	// If none - consider player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTag KnockdownedCharacterId;

	// Can be null
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="G2VS2.Character.Id,Character.Id"))
	FGameplayTagContainer KnockdownedByCharacterId;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;

private:
	void OnCharacterKnockdowned(IQuestCharacter* KnockdownedBy, IQuestCharacter* KnockdownedCharacter);
};
