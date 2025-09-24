// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Addons/FlowNodeAddOnStateful.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "FlowNodeAddon_QuestEventTrigger.generated.h"

struct FQuestSystemContext;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestEventTrigger : public UFlowNodeAddOnStateful, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	virtual void ExecuteInput(const FName& PinName) override;
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnParent_Implementation(const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	virtual void FinishState() override;

#if WITH_EDITOR
	virtual FString GetEventTriggerDescription() const { return TEXT(""); };
#endif
	
protected:
	FQuestSystemContext GetQuestSystemContext() const;

	bool AreRequirementsFulfilled() const;
	virtual bool IsEventAlreadyOccured(const FQuestSystemContext& QuestSystemContext) { return false; };
	virtual void InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext) {};
	virtual void UnregisterEventTrigger() {};
	
	void OnEventTriggerOccured();
	
	FDelegateHandle QuestSystemEventDelegateHandle;
	bool bEventOccured = false;
	
public: // IFlowPredicateInterface
	virtual bool EvaluatePredicate_Implementation() const override;
};
