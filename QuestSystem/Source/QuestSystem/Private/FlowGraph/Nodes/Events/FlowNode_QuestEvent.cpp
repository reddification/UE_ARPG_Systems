// 


#include "FlowGraph/Nodes/Events/FlowNode_QuestEvent.h"

#include "AddOns/FlowNodeAddOn.h"
#include "AddOns/FlowNodeAddOn_PredicateAND.h"
#include "AddOns/FlowNodeAddOn_PredicateNOT.h"
#include "AddOns/FlowNodeAddOn_PredicateOR.h"
#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger.h"
#include "Subsystems/QuestSubsystem.h"

UFlowNode_QuestEvent::UFlowNode_QuestEvent()
{
	OutputPins.Append( { FName("Occured"), FName("Covered") } );
	
#if WITH_EDITOR
	Category = TEXT("Quests|Events");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFlowNode_QuestEvent::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	bool bEventOccured = IsEventOccured();
	TriggerFirstOutput(false);
	if (bEventOccured)
		TriggerOutput(FName("Covered"), true);
}

EFlowAddOnAcceptResult UFlowNode_QuestEvent::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return AddOnTemplate->IsA<UFlowNodeAddon_QuestEventTrigger>()
		|| AddOnTemplate->IsA<UFlowNodeAddOn_PredicateAND>()
		|| AddOnTemplate->IsA<UFlowNodeAddOn_PredicateOR>()
		|| AddOnTemplate->IsA<UFlowNodeAddOn_PredicateNOT>()
	? EFlowAddOnAcceptResult::TentativeAccept
	: EFlowAddOnAcceptResult::Reject;
	
}

bool UFlowNode_QuestEvent::IsEventOccured()
{
	bool bEventOccured = true;
	for (const auto& Addon : AddOns)
	{
		if (auto PredicateInterface = Cast<IFlowPredicateInterface>(Addon.Get()))
		{
			bEventOccured = PredicateInterface->EvaluatePredicate_Implementation();
			if (!bEventOccured)
				break;
		}
	}

	return bEventOccured;
}

void UFlowNode_QuestEvent::ReevaluateEventTriggers()
{
	if (IsEventOccured())
		TriggerOutput(FName("Occured"), true);
}

FQuestSystemContext UFlowNode_QuestEvent::GetQuestSystemContext() const
{
	return UQuestSubsystem::Get(TryGetRootFlowObjectOwner())->GetQuestSystemContext();
}

void UFlowNode_QuestEvent::OnEventTriggerOccured()
{
	ReevaluateEventTriggers();
}
