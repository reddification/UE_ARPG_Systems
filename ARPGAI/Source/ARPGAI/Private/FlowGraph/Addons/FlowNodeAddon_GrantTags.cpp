#include "FlowGraph/Addons/FlowNodeAddon_GrantTags.h"

#include "AIController.h"
#include "FlowGraph/Nodes/FlowNode_NpcGoal.h"
#include "Interfaces/NpcActorTagsInterface.h"

void UFlowNodeAddon_GrantTags::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (Tags.IsEmpty())
		return;
	
	auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner());
	if (auto Npc = Cast<INpcActorTagsInterface>(Owner->GetPawn()))
		Npc->GiveTags_NPC(Tags);
}

void UFlowNodeAddon_GrantTags::FinishState()
{
	if (Tags.IsEmpty())
		return;
	
	auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner());
	if (auto Npc = Cast<INpcActorTagsInterface>(Owner->GetPawn()))
		Npc->RemoveTags_NPC(Tags);
	
	Super::FinishState();
}

EFlowAddOnAcceptResult UFlowNodeAddon_GrantTags::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return ParentTemplate->IsA<UFlowNode_NpcGoal>() ? EFlowAddOnAcceptResult::TentativeAccept : EFlowAddOnAcceptResult::Reject;
}