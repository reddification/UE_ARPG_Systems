#include "BehaviorTree/Tasks/Conversation/BTTask_CommitToConversation.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"

UBTTask_CommitToConversation::UBTTask_CommitToConversation()
{
	NodeName = "Commit to conversation";
}

EBTNodeResult::Type UBTTask_CommitToConversation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto ConversationComponent = GetNpcConversationComponent(OwnerComp);
	const bool bConversationJoined = ConversationComponent->CommitToPendingConversation();
	if (bConversationJoined)
	{
		// accepting conversation can happen with both NPC and player so waiting for any of these 2 messages
		WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Conversation_Completed.GetTag().GetTagName());
		return EBTNodeResult::InProgress;
	}
	else
		return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CommitToConversation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto ConversationComponent = GetNpcConversationComponent(OwnerComp))
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_Conversation, VeryVerbose, TEXT("UBTTask_AcceptConversation::AbortTask: Leaving conversation"));
		ConversationComponent->LeaveConversation();
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}
