// 

#pragma once

#include "CoreMinimal.h"
#include "FlowNodeAddon_QuestEventTrigger.h"
#include "FlowNodeAddon_QuestEventTrigger_WorldState.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger_WorldState : public UFlowNodeAddon_QuestEventTrigger
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery AtWorldState;

	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) override;
	virtual void UnregisterEventTrigger() override;
	virtual bool IsEventAlreadyOccured(const FQuestSystemContext& QuestSystemContext) override;

private:
	void OnWorldStateChanged(const FGameplayTagContainer& NewWorldState);

public:
#if WITH_EDITOR
	virtual FText GetNodeConfigText() const override;
#endif
};
