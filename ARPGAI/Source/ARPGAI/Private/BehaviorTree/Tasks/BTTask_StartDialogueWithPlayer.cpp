// 

#include "BehaviorTree/Tasks/BTTask_StartDialogueWithPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

UBTTask_StartDialogueWithPlayer::UBTTask_StartDialogueWithPlayer()
{
	NodeName = "Start dialogue with player";
	bNotifyTaskFinished = true;
	TargetCharacterBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_StartDialogueWithPlayer, TargetCharacterBBKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_StartDialogueWithPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto TargetCharacter = Cast<AActor>(Blackboard->GetValueAsObject(TargetCharacterBBKey.SelectedKeyName));
	if ((TargetCharacter->GetActorLocation() - Pawn->GetActorLocation()).SizeSquared() > MaxAcceptableDistance * MaxAcceptableDistance)
		return EBTNodeResult::Failed;
	
	switch (Reason)
	{
		case ENpcStartDialogueWithPlayerReason::NpcGoal:
			return StartDialogueFromNpcGoal(OwnerComp, Blackboard, NodeMemory, Pawn);
		case ENpcStartDialogueWithPlayerReason::Reaction:
			return StartDialogueFromReaction(OwnerComp, NodeMemory, Pawn);
		default:
			break;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_StartDialogueWithPlayer::StartDialogueFromNpcGoal(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent* Blackboard,
	uint8* NodeMemory, APawn* Pawn)
{
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;

	auto NpcFlowComponent = OwnerComp.GetAIOwner()->FindComponentByClass<UNpcFlowComponent>();
	const FNpcGoalParameters_TalkToPlayer* Parameters = NpcFlowComponent->GetParameters<FNpcGoalParameters_TalkToPlayer>();
	if (Parameters == nullptr)
		return EBTNodeResult::Failed;
	
	TArray<AActor*> SecondaryDialogueMembers;
	if (!Parameters->SecondaryConversationParticipants.IsEmpty())
	{
		// 12.10.2024 @AK: currently secondary NPCs-participants of dialogue BT state is not handled, so they might be doing whatever
		if (!Parameters->SecondaryConversationParticipants.IsEmpty())
		{
			auto NpcSubsystem = UNpcRegistrationSubsystem::Get(Pawn);
			const FVector& PawnOwnerLocation = Pawn->GetActorLocation();
			for (const auto& SecondaryConversationPartner : Parameters->SecondaryConversationParticipants)
			{
				TArray<UNpcComponent*> SecondaryNpcs = NpcSubsystem->GetNpcsInRange(SecondaryConversationPartner.CharacterId, PawnOwnerLocation,
					SecondaryConversationPartner.SearchInRange, SecondaryConversationPartner.Count, &SecondaryConversationPartner.CharacterFilter);

				for (const auto& SecondaryNpcComponent : SecondaryNpcs)
					SecondaryDialogueMembers.Add(SecondaryNpcComponent->GetOwner());
			}
		}
	}
	
	bool bDialogueStarted = Npc->StartDialogueWithPlayer(Parameters->OptionalDialogueId, SecondaryDialogueMembers, Parameters->bInterruptActivePlayerInteraction);
	if (bDialogueStarted)
	{
		Super::ExecuteTask(OwnerComp, NodeMemory);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_StartDialogueWithPlayer::StartDialogueFromReaction(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, APawn* Pawn)
{
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;

	bool bDialogueStarted = Npc->StartDialogueWithPlayer(FGameplayTag::EmptyTag, {}, false);
	if (bDialogueStarted)
	{
		Super::ExecuteTask(OwnerComp, NodeMemory);
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

void UBTTask_StartDialogueWithPlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                     EBTNodeResult::Type TaskResult)
{
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	if (!ensure(Npc))
		return;

	if (TaskResult == EBTNodeResult::Aborted)
		Npc->StopDialogueWithPlayer();
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_StartDialogueWithPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Dialogue_Player_Completed;
}

FString UBTTask_StartDialogueWithPlayer::GetStaticDescription() const
{
	return FString::Printf(TEXT("Start dialogue with %s if it's in range %.2f\nDialogue reason: %s\n%s"),
		*TargetCharacterBBKey.SelectedKeyName.ToString(), MaxAcceptableDistance, *StaticEnum<ENpcStartDialogueWithPlayerReason>()->GetDisplayValueAsText(Reason).ToString(),
		*Super::GetStaticDescription());
}

