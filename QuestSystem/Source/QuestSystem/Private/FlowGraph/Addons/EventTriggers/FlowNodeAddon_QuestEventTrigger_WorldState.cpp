// 


#include "FlowGraph/Addons/EventTriggers/FlowNodeAddon_QuestEventTrigger_WorldState.h"

#include "Data/QuestTypes.h"
#include "Subsystems/WorldStateSubsystem.h"

void UFlowNodeAddon_QuestEventTrigger_WorldState::InitializeEventTrigger(const FQuestSystemContext& QuestSystemContext)
{
	Super::InitializeEventTrigger(QuestSystemContext);
	QuestSystemEventDelegateHandle = QuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.AddUObject(this,
		&UFlowNodeAddon_QuestEventTrigger_WorldState::OnWorldStateChanged);
}

void UFlowNodeAddon_QuestEventTrigger_WorldState::UnregisterEventTrigger()
{
	Super::UnregisterEventTrigger();
	if (AtWorldState.IsEmpty())
		return;

	auto QuestSystemContext = GetQuestSystemContext();
	QuestSystemContext.WorldStateSubsystem->WorldStateChangedEvent.Remove(QuestSystemEventDelegateHandle);
}

bool UFlowNodeAddon_QuestEventTrigger_WorldState::IsEventAlreadyOccured(const FQuestSystemContext& QuestSystemContext)
{
	return !AtWorldState.IsEmpty() && QuestSystemContext.WorldStateSubsystem->IsAtWorldState(AtWorldState);
}

void UFlowNodeAddon_QuestEventTrigger_WorldState::OnWorldStateChanged(const FGameplayTagContainer& NewWorldState)
{
	if (!AreRequirementsFulfilled())
		return;
	
	if (!AtWorldState.IsEmpty() && AtWorldState.Matches(NewWorldState))
		OnEventTriggerOccured();
}

#if WITH_EDITOR
FText UFlowNodeAddon_QuestEventTrigger_WorldState::GetNodeConfigText() const
{
	FString Result;

	if (AtWorldState.IsEmpty()) 
		Result = TEXT("Warning! World state not set");
	else
		Result = AtWorldState.GetDescription();
	
	return FText::FromString(Result);
}
#endif
