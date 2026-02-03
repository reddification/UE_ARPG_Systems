// 

#pragma once

#include "CoreMinimal.h"
#include "FlowGraph/Interfaces/QuestSystemContextProvider.h"
#include "Nodes/FlowNode.h"
#include "Nodes/FlowNodeStateful.h"
#include "FlowNode_QuestEvent.generated.h"

/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNode_QuestEvent : public UFlowNodeStateful, public IQuestSystemContextProvider
{
	GENERATED_BODY()

public:
	UFlowNode_QuestEvent();

	virtual void ExecuteInput(const FName& PinName) override;
	
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(const UFlowNodeAddOn* AddOnTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

	// Called by one of many possible triggers. quest event node must check if all the other triggers are in the right state
	void OnEventTriggerOccured();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer EventTags;	

private:
	bool IsEventOccured();
	void ReevaluateEventTriggers();

public: // IQuestSystemContextProvider
	virtual FQuestSystemContext GetQuestSystemContext() const override;
};
