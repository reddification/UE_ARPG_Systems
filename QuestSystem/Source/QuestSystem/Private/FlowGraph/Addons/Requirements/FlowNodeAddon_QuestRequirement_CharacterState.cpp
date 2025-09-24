// 


#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement_CharacterState.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

bool UFlowNodeAddon_QuestRequirement_CharacterState::EvaluatePredicate_Implementation() const
{
	if (!Super::EvaluatePredicate_Implementation())
		return false;
	
	auto QuestSystemContext = GetQuestSystemContext();
	if (RequiresCharacterGameplayState.IsEmpty())
		return true;

	const IQuestCharacter* QuestCharacter = QuestSystemContext.Player.GetInterface();
	if (!bPlayer)
	{
		auto Npc = QuestSystemContext.NpcSubsystem->FindNpc(CharacterId, QuestSystemContext.Player->GetCharacterLocation());
		if (!Npc)
			return false;

		QuestCharacter = Cast<IQuestCharacter>(Npc.GetObject());
	}

	if (QuestCharacter == nullptr)
		return false;

	FGameplayTagContainer CharacterTags = QuestCharacter->GetQuestCharacterTags();
	return RequiresCharacterGameplayState.Matches(CharacterTags);
}
