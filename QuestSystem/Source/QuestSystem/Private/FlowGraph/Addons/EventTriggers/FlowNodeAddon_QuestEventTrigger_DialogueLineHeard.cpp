// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_DialogueLineHeard.h"

#include "Data/QuestTypes.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestDialogueLineHeardEvent.AddUObject(this, &UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard::OnDialogueLineHeard);
}

void UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestDialogueLineHeardEvent.Remove(QuestSystemEventDelegateHandle);
}

#if WITH_EDITOR

FText UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard::GetNodeConfigText() const
{
	return FText::FromString(FString::Printf(TEXT("When %s said %s"),
		ByCharacterId.IsValid() ? *ByCharacterId.ToString() : TEXT("anyone"), *PhraseId.ToString()));
}

#endif

void UFlowNodeAddon_QuestEventTrigger_DialogueLineHeard::OnDialogueLineHeard(const FGameplayTag& NpcId,
                                                                             const FGameplayTag& HeardPhraseId)
{
	if (!AreRequirementsFulfilled())
		return;
	
	if ((NpcId == ByCharacterId || !ByCharacterId.IsValid()) && HeardPhraseId == PhraseId)
		OnEventTriggerOccured();
}
