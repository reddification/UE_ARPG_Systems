// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_UpdateCharacterInventory.h"

#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_UpdateCharacterInventory::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	IQuestCharacter* Character = nullptr;
	if (!CharacterId.IsValid() || CharacterId == Context.Player->GetQuestCharacterIdTag())
	{
		Character = Context.Player.GetInterface();
	}
	else
	{
		auto Npc = Context.NpcSubsystem->FindNpc(CharacterId, Context.Player->GetCharacterLocation());
		if (Npc)
		{
			Character = Cast<IQuestCharacter>(Npc.GetObject());
		}
	}

	if (!ensure(Character))
		return Failure;

	for (const auto& Item : ItemsChange)
	{
		Character->ChangeItemsCount(Item.Key, Item.Value.Count);
	}

	return EQuestActionExecuteResult::Success;
}
