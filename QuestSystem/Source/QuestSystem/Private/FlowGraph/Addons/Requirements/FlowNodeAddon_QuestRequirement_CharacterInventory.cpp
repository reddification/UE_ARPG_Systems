// 


#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement_CharacterInventory.h"

#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

bool UFlowNodeAddon_QuestRequirement_CharacterInventory::EvaluatePredicate_Implementation() const
{
	if (!Super::EvaluatePredicate_Implementation())
		return false;

	if (ItemFilter.CountedItemsRequirements.IsEmpty())
		return true;

	auto QuestSystemContext = GetQuestSystemContext();
	const IQuestCharacter* QuestCharacter = QuestSystemContext.Player.GetInterface();
	if (!bPlayer)
	{
		auto Npc = QuestSystemContext.NpcSubsystem->FindNpc(CharacterId, QuestSystemContext.Player->GetCharacterLocation());
		if (!Npc)
			return false;

		QuestCharacter = Cast<IQuestCharacter>(Npc.GetObject());
	}

	if (QuestCharacter == nullptr)
		return false;
	
	bool bRequirementSattisfied = true;
	if (!ItemFilter.ItemsRequirementTagQuery.IsEmpty())
		bRequirementSattisfied = QuestCharacter->ItemsSattisfyRequirement(ItemFilter.ItemsRequirementTagQuery);

	if (!bRequirementSattisfied)
		return false;
	
	if (!ItemFilter.CountedItemsRequirements.IsEmpty())
	{
		for (const auto& CountedItemsRequirement : ItemFilter.CountedItemsRequirements)
		{
			int QuestCharacterHasItemCount = QuestCharacter->GetCountOfItem(CountedItemsRequirement.Key, CountedItemsRequirement.Value.WithTagsFilter);
			switch (CountedItemsRequirement.Value.ItemRequirementLogicalExpression)
			{
				case EItemRequirementLogicalExpression::Equal:
					bRequirementSattisfied = (QuestCharacterHasItemCount == CountedItemsRequirement.Value.Count);
					break;
				case EItemRequirementLogicalExpression::Greater:
					bRequirementSattisfied = (QuestCharacterHasItemCount > CountedItemsRequirement.Value.Count);
					break;
				case EItemRequirementLogicalExpression::Less:
					bRequirementSattisfied = (QuestCharacterHasItemCount < CountedItemsRequirement.Value.Count);
					break;
				case EItemRequirementLogicalExpression::NotEqual:
					bRequirementSattisfied = (QuestCharacterHasItemCount != CountedItemsRequirement.Value.Count);
					break;
				default:
					ensure(false);
					break;
			}

			if (!bRequirementSattisfied)
				return false;
		}
	}

	return true;
}
