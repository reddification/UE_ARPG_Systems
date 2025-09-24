// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged.h"

#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterAcquiredItemEvent.AddUObject(this,
		&UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged::OnItemAcquired);
}

void UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestCharacterAcquiredItemEvent.Remove(QuestSystemEventDelegateHandle);
}

void UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged::OnItemAcquired(IQuestCharacter* QuestCharacter,
	const FGameplayTag& AcquiredItemId, const FGameplayTagContainer& ItemTags, int Count)
{
	if (!AreRequirementsFulfilled())
		return;

	bEventOccured = AcquiredItemId.MatchesTag(ItemId)
		&& QuestCharacter->GetQuestCharacterIdTag() == CharacterId
		&& (ItemFilter.IsEmpty() || ItemFilter.Matches(ItemTags))
		&& (RequiredCount <= 1 || QuestCharacter->GetCountOfItem(AcquiredItemId, ItemFilter) >= RequiredCount);  

	if(bEventOccured)
	{
		OnEventTriggerOccured();
		QuestSystemEventDelegateHandle.Reset();
	}
}

#if WITH_EDITOR

FText UFlowNodeAddon_QuestEventTrigger_CharacterInventoryChanged::GetNodeConfigText() const
{
	FString Result = FString::Printf(TEXT("When %s acquires %d %s"), *CharacterId.ToString(), RequiredCount, *ItemId.ToString());
	if (!ItemFilter.IsEmpty())
		Result += FString::Printf(TEXT("\nItem must comply with filter:\n%s"), *ItemFilter.GetDescription());
	
	return FText::FromString(Result);
}

#endif