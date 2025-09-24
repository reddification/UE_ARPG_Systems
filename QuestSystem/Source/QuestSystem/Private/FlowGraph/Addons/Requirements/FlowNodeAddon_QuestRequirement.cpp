// 


#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement.h"

#include "AddOns/FlowNodeAddOn_PredicateAND.h"
#include "Data/QuestTypes.h"
#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger.h"
#include "FlowGraph/Interfaces/QuestSystemContextProvider.h"
#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction.h"

UFlowNodeAddon_QuestRequirement::UFlowNodeAddon_QuestRequirement(const FObjectInitializer& ObjectInitializer)
{
#if WITH_EDITOR
	Category = TEXT("Quests|Requirements");
#endif
}

EFlowAddOnAcceptResult UFlowNodeAddon_QuestRequirement::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return ParentTemplate->IsA<UFlowNode_QuestAction>() || ParentTemplate->IsA<UFlowNodeAddon_QuestEventTrigger>()
		|| (!ParentTemplate->IsA<UFlowNodeAddon_QuestRequirement>() && ImplementsInterfaceSafe(Cast<UFlowNodeAddOn>(ParentTemplate)))
	? EFlowAddOnAcceptResult::TentativeAccept
	: EFlowAddOnAcceptResult::Reject;
}

bool UFlowNodeAddon_QuestRequirement::EvaluatePredicate_Implementation() const
{
	return true;
}

EFlowAddOnAcceptResult UFlowNodeAddon_QuestRequirement::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return Super::AcceptFlowNodeAddOnChild_Implementation(AddOnTemplate, AdditionalAddOnsToAssumeAreChildren);
}

FQuestSystemContext UFlowNodeAddon_QuestRequirement::GetQuestSystemContext() const
{
	auto QuestSystemContextProvider = Cast<IQuestSystemContextProvider>(FlowNode);
	return QuestSystemContextProvider->GetQuestSystemContext();
}

bool UFlowNodeAddon_QuestRequirement::AreRequirementsFulfilled(const FQuestSystemContext& QuestSystemContext) const
{
	for (const auto& Addon : AddOns)
		if (auto PredicateInterface = Cast<IFlowPredicateInterface>(Addon.Get()))
			if (!PredicateInterface->EvaluatePredicate())
				return false;

	return true;
}
