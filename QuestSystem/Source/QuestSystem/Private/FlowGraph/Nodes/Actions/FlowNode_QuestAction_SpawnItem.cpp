// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SpawnItem.h"

#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestSystemGameMode.h"
#include "Subsystems/QuestSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SpawnItem::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);

	FVector PlayerLocation = Context.Player->GetCharacterLocation();
	const UWorldLocationComponent* TargetQuestLocation = Context.QuestSubsystem->GetQuestLocation(SpawnItemData.LocationIdTag, PlayerLocation);
	if (!IsValid(TargetQuestLocation))
		return EQuestActionExecuteResult::Failure;

	for (auto i = 0; i < SpawnItemData.Count; i++)
	{
		FVector SpawnLocation = SpawnItemData.bNearPlayer
			? Context.QuestSubsystem->GetRandomNavigableLocationNearPlayer(PlayerLocation, SpawnItemData.NearPlayerRadius, SpawnItemData.FloorOffset)
			: TargetQuestLocation->GetRandomLocationInVolume(SpawnItemData.FloorOffset);
	
		Context.GameMode->SpawnItem(SpawnItemData.ItemId, SpawnItemData.ItemCategory,
			SpawnLocation, FRotator::ZeroRotator, SpawnItemData.WithTags);
	}

	return EQuestActionExecuteResult::Success;
}
