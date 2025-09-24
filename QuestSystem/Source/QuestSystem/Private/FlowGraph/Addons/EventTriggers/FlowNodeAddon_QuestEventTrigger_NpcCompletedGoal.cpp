// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_NpcCompletedGoal.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_NpcCompletedGoal::InitializeEventTrigger(
	const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.QuestSubsystem->QuestNpcGoalCompletedEvent.AddUObject(this,
		&UFlowNodeAddon_QuestEventTrigger_NpcCompletedGoal::OnNpcGoalCompleted);
}

void UFlowNodeAddon_QuestEventTrigger_NpcCompletedGoal::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.QuestSubsystem->QuestNpcGoalCompletedEvent.Remove(QuestSystemEventDelegateHandle);
}

void UFlowNodeAddon_QuestEventTrigger_NpcCompletedGoal::OnNpcGoalCompleted(IQuestCharacter* Npc,
	const FGameplayTagContainer& QuestGoalTags)
{
	if (!AreRequirementsFulfilled())
		return;
	
	if (ensure(!NpcGoalTagsFilter.IsEmpty()))
		return;
	
	if (NpcId.IsValid() && Npc->GetQuestCharacterIdTag() != NpcId)
		return;

	if (!NpcCharacterTagsFilter.IsEmpty())
	{
		FGameplayTagContainer NpcTags = Npc->GetQuestCharacterTags();
		if (!NpcCharacterTagsFilter.Matches(NpcTags))
			return;
	}
	
	if (NpcGoalTagsFilter.Matches(QuestGoalTags))
		OnEventTriggerOccured();
}
