#include "BehaviorTree/Tasks/Conversation/BTTask_StartDialogueWithPlayer.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"

UBTTask_StartDialogueWithPlayer::UBTTask_StartDialogueWithPlayer()
{
	NodeName = "Start dialogue with player";
	TargetCharacterBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_StartDialogueWithPlayer, TargetCharacterBBKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_StartDialogueWithPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto TargetCharacter = Cast<APawn>(Blackboard->GetValueAsObject(TargetCharacterBBKey.SelectedKeyName));
	if (TargetCharacter == nullptr)
	{
		UE_VLOG_UELOG(NpcPawn, LogARPGAI, Error, TEXT("UBTTask_StartDialogueWithPlayer: no actor in blackboard"));
		return EBTNodeResult::Failed;
	}
	
	if ((TargetCharacter->GetActorLocation() - NpcPawn->GetActorLocation()).SizeSquared() > MaxAcceptableDistance * MaxAcceptableDistance)
		return EBTNodeResult::Failed;
	
	bool bDialogueStarted = false;
	switch (Reason)
	{
		case ENpcStartDialogueWithPlayerReason::NpcGoal:
			bDialogueStarted = StartDialogueFromNpcGoal(NpcPawn, TargetCharacter);
			break;
		case ENpcStartDialogueWithPlayerReason::Reaction:
			bDialogueStarted = StartDialogueFromReaction(NpcPawn, TargetCharacter);
			break;
		default:
			break;
	}

	if (bDialogueStarted)
	{
		Super::ExecuteTask(OwnerComp, NodeMemory);
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

bool UBTTask_StartDialogueWithPlayer::StartDialogueFromNpcGoal(APawn* NpcPawn, APawn* Collocutor)
{
	auto NpcFlowComponent = GetNpcFlowComponent(NpcPawn);
	const FNpcGoalParameters_TalkToPlayer* ConversationGoalParams = NpcFlowComponent->GetParameters<FNpcGoalParameters_TalkToPlayer>();
	if (ConversationGoalParams == nullptr)
	{
		UE_VLOG(NpcPawn, LogARPGAI, Warning, TEXT("UBTTask_StartDialogueWithPlayer::StartDialogueFromNpcGoal - no goal parameters"));
		return false;
	}
	
	auto ConversationComponent = GetNpcConversationComponent(NpcPawn);
	if (ConversationComponent == nullptr)
		return ensure(false);
	
	NpcConversation::FRequestParams ConversationRequestParams;
	ConversationRequestParams.bForceSuspendActivity = ConversationGoalParams->bInterruptActivePlayerInteraction;
	ConversationRequestParams.ConversationId = ConversationGoalParams->OptionalDialogueId;
	ConversationRequestParams.SecondaryConversationParticipants = ConversationGoalParams->SecondaryConversationParticipants;
	
	auto Result = ConversationComponent->RequestConversation(Collocutor, ConversationRequestParams, false);
	return Result.bSuccess;
}

bool UBTTask_StartDialogueWithPlayer::StartDialogueFromReaction(APawn* NpcPawn, APawn* TargetPawn)
{
	auto ConversationComponent = GetNpcConversationComponent(NpcPawn);
	if (ConversationComponent == nullptr)
		return ensure(false);
	
	auto Result = ConversationComponent->RequestConversation(TargetPawn, { }, false);
	return Result.bSuccess;
}

void UBTTask_StartDialogueWithPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_Conversation_Completed;
}

FString UBTTask_StartDialogueWithPlayer::GetStaticDescription() const
{
	return FString::Printf(TEXT("Start dialogue with %s if it's in range %.2f\nDialogue reason: %s\n%s"),
		*TargetCharacterBBKey.SelectedKeyName.ToString(), MaxAcceptableDistance, *StaticEnum<ENpcStartDialogueWithPlayerReason>()->GetDisplayValueAsText(Reason).ToString(),
		*Super::GetStaticDescription());
}

EBTNodeResult::Type UBTTask_StartDialogueWithPlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto ConversationComponent = GetNpcConversationComponent(OwnerComp);
	if (ConversationComponent != nullptr)
		ConversationComponent->AbortConversation();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

