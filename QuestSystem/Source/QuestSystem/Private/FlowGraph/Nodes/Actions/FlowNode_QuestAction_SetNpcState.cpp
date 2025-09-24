// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_SetNpcState.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Interfaces/QuestNPC.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_SetNpcState::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	TArray<TScriptInterface<IQuestNPC>> Npcs = Context.NpcSubsystem->GetNpcs(CharacterId, nullptr);
	const bool bMustCheckFilters = !CharacterFilter.IsEmpty();
	for (const auto& Npc : Npcs)
	{
		auto QuestCharacter = Cast<IQuestCharacter>(Npc.GetObject());
		if (bMustCheckFilters)
		{
			FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
			if (CharacterFilter.Matches(CharacterTags))
				continue;
		}

		if (bAdd)
			QuestCharacter->AddQuestState(StateTag, SetByCallerParams);
		else
			QuestCharacter->RemoveQuestState(StateTag);
	}

	return EQuestActionExecuteResult::Success;
}
