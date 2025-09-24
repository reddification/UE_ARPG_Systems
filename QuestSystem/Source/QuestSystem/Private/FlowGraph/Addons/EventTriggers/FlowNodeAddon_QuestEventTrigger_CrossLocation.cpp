// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_CrossLocation.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_CrossLocation::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = bEnter
		? QuestSystemContext.QuestSubsystem->QuestCharacterReachedLocationEvent.AddUObject(this, &UFlowNodeAddon_QuestEventTrigger_CrossLocation::OnLocationCrossed)
		: QuestSystemContext.QuestSubsystem->QuestCharacterLeftLocationEvent.AddUObject(this, &UFlowNodeAddon_QuestEventTrigger_CrossLocation::OnLocationCrossed);
}

void UFlowNodeAddon_QuestEventTrigger_CrossLocation::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	if (bEnter)
		QuestSystemContext.QuestSubsystem->QuestCharacterReachedLocationEvent.Remove(QuestSystemEventDelegateHandle);
	else
		QuestSystemContext.QuestSubsystem->QuestCharacterLeftLocationEvent.Remove(QuestSystemEventDelegateHandle);

}

#if WITH_EDITOR

FText UFlowNodeAddon_QuestEventTrigger_CrossLocation::GetNodeConfigText() const
{
	return FText::FromString(FString::Printf(TEXT("When %s %s %s"),
		*ByCharacterTagId.ToString(), bEnter ? TEXT("enters") : TEXT("leaves"), *LocationIdTag.ToString()));
}

#endif

void UFlowNodeAddon_QuestEventTrigger_CrossLocation::OnLocationCrossed(const FGameplayTag& VisitedLocationIdTag,
                                                                       const IQuestCharacter* QuestCharacter)
{
	if (!ensure(QuestCharacter != nullptr) || !AreRequirementsFulfilled())
		return;

	if (VisitedLocationIdTag == LocationIdTag && QuestCharacter->GetQuestCharacterIdTag() == ByCharacterTagId)
	{
		if (!CharacterTagsFilter.IsEmpty())
		{
			const FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
			if (!CharacterTagsFilter.Matches(CharacterTags))
				return;
		}

		OnEventTriggerOccured();
		QuestSystemEventDelegateHandle.Reset();
	}
}
