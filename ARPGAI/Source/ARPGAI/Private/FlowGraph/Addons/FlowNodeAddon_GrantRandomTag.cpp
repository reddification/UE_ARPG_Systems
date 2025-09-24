// 


#include "FlowGraph/Addons/FlowNodeAddon_GrantRandomTag.h"

#include "AIController.h"
#include "Interfaces/Npc.h"

void UFlowNodeAddon_GrantRandomTag::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
	if (Tags.IsEmpty())
		return;
	
	auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto Npc = Cast<INpc>(Owner->GetPawn());
	if (Tags.Num() > 1)
	{
		const auto& TagsArray = Tags.GetGameplayTagArray();
		GrantedTag = TagsArray[FMath::RandRange(0, TagsArray.Num() - 1)];
	}
	else
	{
		GrantedTag = Tags.First();
	}
	
	Npc->GiveNpcTags(GrantedTag.GetSingleTagContainer());
}

void UFlowNodeAddon_GrantRandomTag::FinishState()
{
	if (!GrantedTag.IsValid())
		return;
	
	auto Owner = Cast<AAIController>(TryGetRootFlowActorOwner());
	auto Npc = Cast<INpc>(Owner->GetPawn());
	Npc->RemoveNpcTags(GrantedTag.GetSingleTagContainer());
	
	Super::FinishState();
}

EFlowAddOnAcceptResult UFlowNodeAddon_GrantRandomTag::AcceptFlowNodeAddOnParent_Implementation(
	const UFlowNodeBase* ParentTemplate, const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const
{
	return EFlowAddOnAcceptResult::TentativeAccept;
}
