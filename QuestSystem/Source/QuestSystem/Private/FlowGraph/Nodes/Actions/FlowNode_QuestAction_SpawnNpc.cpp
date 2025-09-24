// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SpawnNpc.h"

#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestNPC.h"
#include "Interfaces/QuestSystemGameMode.h"
#include "Subsystems/QuestNpcSubsystem.h"
#include "Subsystems/WorldLocationsSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SpawnNpc::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	for (auto i = 0; i < SpawnNpcAndSetBehaviorData.Count; i++)
	{
		auto WorldLocation = Context.WorldLocationsSubsystem->GetClosestQuestLocationSimple(SpawnNpcAndSetBehaviorData.LocationIdTag,
			Context.Player->GetCharacterLocation());

		if (!WorldLocation)
			continue;

		const FVector SpawnLocation = WorldLocation->GetRandomLocationInVolume(100.f);
		TScriptInterface<IQuestNPC> Npc = SpawnNpcAndSetBehaviorData.bSpawnNew
			? Context.GameMode->SpawnQuestNPC(SpawnNpcAndSetBehaviorData.NpcIdTag, SpawnLocation, FGameplayTagContainer::EmptyContainer)
			: Context.NpcSubsystem->FindNpc(SpawnNpcAndSetBehaviorData.NpcIdTag, Context.Player->GetCharacterLocation());

		if (Npc == nullptr && !SpawnNpcAndSetBehaviorData.bSpawnNew)
			Npc = Context.GameMode->SpawnQuestNPC(SpawnNpcAndSetBehaviorData.NpcIdTag, SpawnLocation, FGameplayTagContainer::EmptyContainer);

		if (Npc)
		{
			Npc->TeleportToQuestLocation(SpawnLocation);
			Npc->AddNpcQuestTags(SpawnNpcAndSetBehaviorData.WithTags);
			if (SpawnNpcAndSetBehaviorData.OptionalNpcInitialBehavior.RequestedBehaviorIdTag.IsValid())
				Context.NpcSubsystem->RunQuestBehavior(Npc, SpawnNpcAndSetBehaviorData.OptionalNpcInitialBehavior, ActionId, Context);
		}
	}

	return EQuestActionExecuteResult::Success;
}
