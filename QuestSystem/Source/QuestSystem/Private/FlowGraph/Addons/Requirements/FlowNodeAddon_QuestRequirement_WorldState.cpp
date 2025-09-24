// 


#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement_WorldState.h"

#include "Data/QuestTypes.h"
#include "Subsystems/WorldStateSubsystem.h"

bool UFlowNodeAddon_QuestRequirement_WorldState::EvaluatePredicate_Implementation() const
{
	return Super::EvaluatePredicate_Implementation()
		&& GetQuestSystemContext().WorldStateSubsystem->IsAtWorldState(WorldStateQuery);
}

#if WITH_EDITOR

FText UFlowNodeAddon_QuestRequirement_WorldState::GetNodeConfigText() const
{
	return FText::FromString(WorldStateQuery.IsEmpty() ? TEXT("Warning! World state filter not set") : *WorldStateQuery.GetDescription());
}

#endif