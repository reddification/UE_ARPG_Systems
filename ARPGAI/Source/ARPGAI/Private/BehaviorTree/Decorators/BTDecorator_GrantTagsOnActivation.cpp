#include "BehaviorTree/Decorators/BTDecorator_GrantTagsOnActivation.h"

#include "AIController.h"
#include "Interfaces/NpcActorTagsInterface.h"

UBTDecorator_GrantTagsOnActivation::UBTDecorator_GrantTagsOnActivation()
{
	NodeName = "Grant tags on activation";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_GrantTagsOnActivation::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (GrantedTags.IsEmpty())
		return;
	
	auto Npc = Cast<INpcActorTagsInterface>(SearchData.OwnerComp.GetAIOwner()->GetPawn());
	if (Npc == nullptr)
		return;
	
	if (bJustOneRandom && GrantedTags.Num() > 1)
	{
		auto BTMemory = GetNodeMemory<FBTMemory_GrantedTags>(SearchData);
		FGameplayTag ActualTag = GrantedTags.GetGameplayTagArray()[FMath::RandRange(0, GrantedTags.Num() - 1)];
		Npc->GiveTags_NPC(ActualTag.GetSingleTagContainer());
		BTMemory->GrantedTag = ActualTag;
	}
	else
	{
		Npc->GiveTags_NPC(GrantedTags);
	}
}

void UBTDecorator_GrantTagsOnActivation::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	if (!GrantedTags.IsEmpty())
	{
		if (auto AIController = SearchData.OwnerComp.GetAIOwner())
			if (auto Pawn = AIController->GetPawn())
				if (auto Npc = Cast<INpcActorTagsInterface>(Pawn))
				{
					if (bJustOneRandom && GrantedTags.Num() > 1)
					{
						auto BTMemory = GetNodeMemory<FBTMemory_GrantedTags>(SearchData);
						if (ensure(BTMemory->GrantedTag.IsValid()))
							Npc->RemoveTags_NPC(BTMemory->GrantedTag.GetSingleTagContainer());
					}
					else
					{
						Npc->RemoveTags_NPC(GrantedTags);
					}
				}
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_GrantTagsOnActivation::GetStaticDescription() const
{
	if (GrantedTags.IsEmpty())
		return TEXT("No tags specified");
	
	FString TagsList;
	for (const auto& GrantedTag : GrantedTags)
		TagsList += FString::Printf(TEXT("\n%s"), *GrantedTag.ToString());
	
	return bJustOneRandom 
		? FString::Printf(TEXT("One random tag from%s"), *TagsList)
		: FString::Printf(TEXT("All tags from%s"), *TagsList);
}
