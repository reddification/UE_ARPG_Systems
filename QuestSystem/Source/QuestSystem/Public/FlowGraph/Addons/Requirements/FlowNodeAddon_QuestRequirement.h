// 

#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "FlowNodeAddon_QuestRequirement.generated.h"

struct FQuestSystemContext;
/**
 * 
 */
UCLASS()
class QUESTSYSTEM_API UFlowNodeAddon_QuestRequirement : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	UFlowNodeAddon_QuestRequirement(const FObjectInitializer& ObjectInitializer);
	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnParent_Implementation(const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	
public: // IFlowPredicateInterface
	virtual bool EvaluatePredicate_Implementation() const override;

	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;
	
protected:
	FQuestSystemContext GetQuestSystemContext() const;
	bool AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const;
};
