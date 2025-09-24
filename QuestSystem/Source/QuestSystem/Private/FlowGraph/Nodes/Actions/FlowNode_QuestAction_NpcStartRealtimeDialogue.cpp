// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_NpcStartRealtimeDialogue.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_NpcStartRealtimeDialogue::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	auto DialogueInitiatorNpc = Context.NpcSubsystem->FindNpc(NpcId, Context.Player->GetCharacterLocation());
	if (!ensure(DialogueInitiatorNpc))
		return Failure;
	
	TArray<AActor*> DialogueParticipantActors;
	if (bIncludePlayer)
		DialogueParticipantActors.Add(Cast<AActor>(Context.Player.GetObject()));	

	FVector DialogueInitiatorLocation = DialogueInitiatorNpc->GetQuestNpcLocation();
	for (const auto& DialogueParticipant : DialogueParticipants)
	{
		auto PotentialDialogueParticipants = Context.NpcSubsystem->GetNpcsInRange(DialogueParticipant.ParticipantId,
			DialogueInitiatorLocation, DialogueParticipant.InRange, &DialogueParticipant.ParticipantFilter);
		if (DialogueParticipant.Count < PotentialDialogueParticipants.Num())
		{
			TArray<TTuple<AActor*, double>> NpcsBySquareDistances;
			for (const auto& Npc : PotentialDialogueParticipants)
			{
				const auto DistSq = (DialogueInitiatorLocation - Npc->GetQuestNpcLocation()).SizeSquared();
				NpcsBySquareDistances.Add( { Npc->GetQuestNpcPawn(), DistSq });
			}

			NpcsBySquareDistances.Sort([](const TTuple<AActor*, double>& A, const TTuple<AActor*, double>& B) { return A.Value < B.Value; });
			for (int i = 0; i < DialogueParticipant.Count; i++)
				DialogueParticipantActors.Add(NpcsBySquareDistances[i].Key);
		}
		else
		{
			for (const auto& Npc : PotentialDialogueParticipants)
				DialogueParticipantActors.Add(Cast<AActor>(Npc.GetObject()));
		}
	}
	
	DialogueInitiatorNpc->StartQuestDialogue(DialogueId, DialogueParticipantActors);
	return Success;
}
