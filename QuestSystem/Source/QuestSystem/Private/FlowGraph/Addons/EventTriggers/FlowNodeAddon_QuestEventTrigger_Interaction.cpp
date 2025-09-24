// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_Interaction.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_Interaction::InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestCharacterInteractedWithItemEvent.AddUObject(this,
		&UFlowNodeAddon_QuestEventTrigger_Interaction::OnCharacterInteractedWithActor);
}

void UFlowNodeAddon_QuestEventTrigger_Interaction::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestCharacterInteractedWithItemEvent.Remove(QuestSystemEventDelegateHandle);
}

void UFlowNodeAddon_QuestEventTrigger_Interaction::OnCharacterInteractedWithActor(IQuestCharacter* QuestCharacter,
	const FGameplayTag& CurrentInteractionActorId, const FGameplayTagContainer& CurrentInteractionActionId,
	const FGameplayTagContainer& InInteractionActorTags)
{
	if (!AreRequirementsFulfilled())
		return;

	bEventOccured = (QuestCharacter->GetQuestCharacterIdTag() == ByCharacterId || !ByCharacterId.IsValid())
		&& CurrentInteractionActorId == InteractionActorId
		&& (CurrentInteractionActionId.HasAny(InteractionActionsIds) || !InteractionActionsIds.IsValid())
		&& (InteractionActorTagsFilter.IsEmpty() || InteractionActorTagsFilter.Matches(InInteractionActorTags));
	
	if (bEventOccured)
		OnEventTriggerOccured();
}
