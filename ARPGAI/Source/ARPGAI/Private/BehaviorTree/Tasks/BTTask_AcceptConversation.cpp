// 


#include "BehaviorTree/Tasks/BTTask_AcceptConversation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_AcceptConversation::UBTTask_AcceptConversation()
{
	NodeName = "Accept conversation";
	OutConversationAcceptedBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AcceptConversation, OutConversationAcceptedBBKey));
}

EBTNodeResult::Type UBTTask_AcceptConversation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// accepting conversation can happen with both NPC and player so waiting for any of these 2 messages
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Conversation_Completed.GetTag().GetTagName());
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Dialogue_Player_Completed.GetTag().GetTagName());
	// WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Conversation_OnHold.GetTag().GetTagName());
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_AcceptConversation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Npc = Cast<INpc>(AIController->GetPawn()))
			Npc->LeaveConversation();
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

FString UBTTask_AcceptConversation::GetStaticDescription() const
{
	return Super::GetStaticDescription() + FString::Printf(TEXT("\n[out]Conversation accepted BB: %s"),
		*OutConversationAcceptedBBKey.SelectedKeyName.ToString());
}

void UBTTask_AcceptConversation::ClearBlackboardConversationState(UBlackboardComponent* OwnerBlackboard) const
{
	Super::ClearBlackboardConversationState(OwnerBlackboard);
	OwnerBlackboard->ClearValue(OutConversationAcceptedBBKey.SelectedKeyName);
}
