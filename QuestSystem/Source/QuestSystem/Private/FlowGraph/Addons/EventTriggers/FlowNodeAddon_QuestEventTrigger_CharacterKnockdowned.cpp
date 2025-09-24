// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_CharacterKnockdowned.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_CharacterKnockdowned::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterKnockdownedEvent.AddUObject(this,
		&UFlowNodeAddon_QuestEventTrigger_CharacterKnockdowned::OnCharacterKnockdowned);
}

void UFlowNodeAddon_QuestEventTrigger_CharacterKnockdowned::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestCharacterKnockdownedEvent.Remove(QuestSystemEventDelegateHandle);
}

void UFlowNodeAddon_QuestEventTrigger_CharacterKnockdowned::OnCharacterKnockdowned(IQuestCharacter* KnockdownedBy,
	IQuestCharacter* KnockdownedCharacter)
{
	if (!AreRequirementsFulfilled())
		return;

	bEventOccured = (KnockdownedByCharacterId.IsEmpty() || KnockdownedByCharacterId.HasTag(KnockdownedBy->GetQuestCharacterIdTag()))
		&& (!KnockdownedCharacterId.IsValid() && KnockdownedCharacter->IsPlayer() || KnockdownedCharacterId == KnockdownedCharacter->GetQuestCharacterIdTag());

	if (bEventOccured)
		OnEventTriggerOccured();
}
