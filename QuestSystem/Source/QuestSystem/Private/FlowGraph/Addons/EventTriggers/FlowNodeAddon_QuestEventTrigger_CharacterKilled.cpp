// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_CharacterKilled.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_CharacterKilled::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterKilledEvent.AddUObject(this, &UFlowNodeAddon_QuestEventTrigger_CharacterKilled::OnCharacterKilled);
}

void UFlowNodeAddon_QuestEventTrigger_CharacterKilled::UnregisterEventTrigger()
{
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestCharacterKilledEvent.Remove(QuestSystemEventDelegateHandle);
	Super::UnregisterEventTrigger();
}

void UFlowNodeAddon_QuestEventTrigger_CharacterKilled::OnCharacterKilled(IQuestCharacter* Killer, IQuestCharacter* Killed)
{
	if (!AreRequirementsFulfilled())
		return;
	
	if (Killer->GetQuestCharacterIdTag() == ByCharacterId && Killed->GetQuestCharacterIdTag() == KilledCharacterId)
	{
		if (!KilledCharacterTagsFilter.IsEmpty() && !KilledCharacterTagsFilter.Matches(Killed->GetQuestCharacterTags()))
			return;

		CurrentCount++;
		if (CurrentCount >= RequiredCount)
			OnEventTriggerOccured();
	}
}

#if WITH_EDITOR

FText UFlowNodeAddon_QuestEventTrigger_CharacterKilled::GetNodeConfigText() const
{
	FString Result = FString::Printf(TEXT("When %s kills %d %s"), *ByCharacterId.ToString(), RequiredCount, *KilledCharacterId.ToString());
	if (!KilledCharacterTagsFilter.IsEmpty())
		Result += FString::Printf(TEXT("\nKilled characters must comply with filter:\n%s"), *KilledCharacterTagsFilter.GetDescription());
	
	return FText::FromString(Result);
}

#endif