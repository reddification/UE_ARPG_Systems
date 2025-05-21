

#include "Data/QuestRequirements.h"
#include "Subsystems/WorldStateSubsystem.h"
#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

bool FQuestRequirementBase::IsApplicable(const FQuestSystemContext& QuestSystemContext) const
{
	return ApplicableAtWorldState.IsEmpty() ? true : QuestSystemContext.WorldStateSubsystem->IsAtWorldState(ApplicableAtWorldState);
}

bool FQuestRequirementWorldState::IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const
{
	if (RequiresWorldState.IsEmpty())
		return true;

	return QuestSystemContext.WorldStateSubsystem->IsAtWorldState(RequiresWorldState);
}

bool FQuestRequirementCharacterState::IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const
{
	if (RequiresCharacterGameplayState.IsEmpty())
		return true;

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

	FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
	return RequiresCharacterGameplayState.Matches(CharacterTags);
}

bool FQuestRequirementCharacterInventory::IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const
{
	if (ItemFilter.CountedItemsRequirements.IsEmpty())
		return true;

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

bool FQuestRequirementPlayerInProximityOfNpc::IsQuestRequirementMet(const FQuestSystemContext& QuestSystemContext) const
{
	const FVector PlayerEyesLocation = QuestSystemContext.Player->GetQuestCharacterEyesLocation();
	auto Npc = QuestSystemContext.NpcSubsystem->FindNpc(NpcId, PlayerEyesLocation);
	if (Npc == nullptr)
		return false;

	if (bTraceVisibility)
	{
		FHitResult HitResult;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(Cast<AActor>(QuestSystemContext.Player.GetObject()));
		auto NpcActor = Cast<AActor>(Npc.GetObject());
		const FVector TraceEnd = PlayerEyesLocation + (Npc->GetQuestNpcEyesLocation() - PlayerEyesLocation).GetSafeNormal() * Proximity; 
		const bool bBlockingHit = QuestSystemContext.World->LineTraceSingleByChannel(HitResult, PlayerEyesLocation, TraceEnd,
			ECC_Visibility, CollisionQueryParams);
		return bBlockingHit && HitResult.GetActor() == NpcActor;
	}
	else
	{
		return (PlayerEyesLocation - Npc->GetQuestNpcEyesLocation()).SizeSquared() < Proximity * Proximity;
	}
}
