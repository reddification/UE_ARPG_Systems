// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger.h"

#include "AddOns/FlowNodeAddOn_PredicateAND.h"
#include "AddOns/FlowNodeAddOn_PredicateNOT.h"
#include "AddOns/FlowNodeAddOn_PredicateOR.h"
#include "Data/QuestTypes.h"
#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement.h"
#include "FlowGraph/Nodes/Events/FlowNode_QuestEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestSystemGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/QuestSubsystem.h"
#include "Subsystems/WorldLocationsSubsystem.h"
#include "Subsystems/WorldStateSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	auto QuestSystemContext = GetQuestSystemContext();
	if (IsEventAlreadyOccured(QuestSystemContext))
		OnEventTriggerOccured();
	else
		InitializeEventTrigger(QuestSystemContext);
}

EFlowAddOnAcceptResult UFlowNodeAddon_QuestEventTrigger::AcceptFlowNodeAddOnChild_Implementation(
	const UFlowNodeAddOn* AddOnTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return AddOnTemplate->IsA<UFlowNodeAddon_QuestRequirement>() ||
		AddOnTemplate->IsA<UFlowNodeAddOn_PredicateAND>() ||
		AddOnTemplate->IsA<UFlowNodeAddOn_PredicateOR>() ||
		AddOnTemplate->IsA<UFlowNodeAddOn_PredicateNOT>()
		? EFlowAddOnAcceptResult::TentativeAccept : EFlowAddOnAcceptResult::Reject;
}

EFlowAddOnAcceptResult UFlowNodeAddon_QuestEventTrigger::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return ParentTemplate->IsA<UFlowNode_QuestEvent>() ? EFlowAddOnAcceptResult::TentativeAccept : EFlowAddOnAcceptResult::Reject;
}

void UFlowNodeAddon_QuestEventTrigger::FinishState()
{
	Super::FinishState();
	if (QuestSystemEventDelegateHandle.IsValid())
	{
		UnregisterEventTrigger();
		QuestSystemEventDelegateHandle.Reset();
	}
}

FQuestSystemContext UFlowNodeAddon_QuestEventTrigger::GetQuestSystemContext() const
{
	return UQuestSubsystem::Get(TryGetRootFlowObjectOwner())->GetQuestSystemContext();
}

bool UFlowNodeAddon_QuestEventTrigger::AreRequirementsFulfilled() const
{
	for (const auto& Addon : AddOns)
		if (auto PredicateInterface = Cast<IFlowPredicateInterface>(Addon.Get()))
			if (!PredicateInterface->EvaluatePredicate_Implementation())
				return false;

	return true;
}

void UFlowNodeAddon_QuestEventTrigger::OnEventTriggerOccured()
{
	bEventOccured = true;
	UnregisterEventTrigger();
	auto OwnerEventNode = Cast<UFlowNode_QuestEvent>(FlowNode);
	if (ensure(OwnerEventNode))
		OwnerEventNode->OnEventTriggerOccured();
}

bool UFlowNodeAddon_QuestEventTrigger::EvaluatePredicate_Implementation() const
{
	return bEventOccured;
}
