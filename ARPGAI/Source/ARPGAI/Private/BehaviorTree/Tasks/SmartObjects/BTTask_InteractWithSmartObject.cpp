// 


#include "BehaviorTree/Tasks/SmartObjects/BTTask_InteractWithSmartObject.h"

#include "AIController.h"
#include "BlackboardKeyType_SOClaimHandle.h"
#include "GameplayBehavior.h"
#include "SmartObjectSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"

UBTTask_InteractWithSmartObject::UBTTask_InteractWithSmartObject()
{
	NodeName = "Interact with smart object";
	bNotifyTaskFinished = 1;
	ClaimedSmartObjectClaimHandleBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_SOClaimHandle>(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractWithSmartObject, ClaimedSmartObjectClaimHandleBBKey)));
	ActiveSmartObjectClaimHandleBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_SOClaimHandle>(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractWithSmartObject, ActiveSmartObjectClaimHandleBBKey)));
	InteractionActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InteractWithSmartObject, InteractionActorBBKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_InteractWithSmartObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_InteractWithSmartObject::ExecuteTask)

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto SmartObjectClaimHandle = Blackboard->GetValue<UBlackboardKeyType_SOClaimHandle>(ClaimedSmartObjectClaimHandleBBKey.SelectedKeyName);
	if (!SmartObjectClaimHandle.IsValid())
		return EBTNodeResult::Failed;
	
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Npc)
		return EBTNodeResult::Failed;

	auto InteractionActor = Cast<AActor>(Blackboard->GetValueAsObject(InteractionActorBBKey.SelectedKeyName));
	Npc->StartInteractingWithSmartObject(InteractionActor, SmartObjectClaimHandle);
	// I just hope the actual SO interaction behavior is activated right away and not in the following frames
	if (!Npc->IsNpcInteractingWithSmartObject())
		return EBTNodeResult::Failed;
	
	Blackboard->SetValue<UBlackboardKeyType_SOClaimHandle>(ActiveSmartObjectClaimHandleBBKey.SelectedKeyName, SmartObjectClaimHandle);
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_SmartObjectInteraction_Completed.GetTag().GetTagName());
	return EBTNodeResult::InProgress;
}

void UBTTask_InteractWithSmartObject::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("UBTTask_InteractWithSmartObject::FinalizeInteraction: finalizing interaction"));

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	Blackboard->ClearValue(InteractionActorBBKey.SelectedKeyName);
	Blackboard->ClearValue(ActiveSmartObjectClaimHandleBBKey.SelectedKeyName);
}

EBTNodeResult::Type UBTTask_InteractWithSmartObject::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_InteractWithSmartObject::AbortTask)
	
	UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("UBTTask_InteractWithSmartObject::AbortTask: received abort request"));
	
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_SmartObjectInteraction_Completed.GetTag().GetTagName());
	
	auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	if (Npc)
		Npc->StopInteractingWithSmartObject();
	
	UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("UBTTask_InteractWithSmartObject::AbortTask: waiting for interaction end"));
	return EBTNodeResult::InProgress;
}

void UBTTask_InteractWithSmartObject::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message,
                                                int32 RequestID, bool bSuccess)
{
	if (Message == AIGameplayTags::AI_BrainMessage_SmartObjectInteraction_Completed.GetTag().GetTagName())
	{
		const EBTTaskStatus::Type Status = OwnerComp.GetTaskStatus(this);
		if (Status == EBTTaskStatus::Active)
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		else
			FinishLatentAbort(OwnerComp);
	}
	else
	{
		ensure(false);
		Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
	}
}

FString UBTTask_InteractWithSmartObject::GetStaticDescription() const
{
	return FString::Printf(TEXT("Claimed SOCH BB: %s\n[out]Active SOCH BB: %s\nInteraction Actor BB: %s"),
		*ClaimedSmartObjectClaimHandleBBKey.SelectedKeyName.ToString(),
		*ActiveSmartObjectClaimHandleBBKey.SelectedKeyName.ToString(),
		*InteractionActorBBKey.SelectedKeyName.ToString());
}
