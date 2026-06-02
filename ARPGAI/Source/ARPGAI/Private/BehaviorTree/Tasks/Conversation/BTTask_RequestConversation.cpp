#include "BehaviorTree/Tasks/Conversation/BTTask_RequestConversation.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"

UBTTask_RequestConversation::UBTTask_RequestConversation()
{
	NodeName = "Request conversation";
}

EBTNodeResult::Type UBTTask_RequestConversation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BaseResult = Super::ExecuteTask(OwnerComp, NodeMemory);
	if (BaseResult == EBTNodeResult::Failed)
		return BaseResult;
	
	auto OwnerBlackboard = OwnerComp.GetBlackboardComponent();
	APawn* ConversationPartner = Cast<APawn>(OwnerBlackboard->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName));
	if(!ensure(ConversationPartner))
		return EBTNodeResult::Failed;
	
	auto PawnOwner = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());

	auto TargetConversationComponent = GetNpcConversationComponent(ConversationPartner);
	if (!ensure(TargetConversationComponent))
		return EBTNodeResult::Failed;

	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	
	auto NpcFlowComponent = OwnerComp.GetAIOwner()->FindComponentByClass<UNpcFlowComponent>();
	const FNpcGoalParameters_Conversate* ConversationParams = NpcFlowComponent->GetParameters<FNpcGoalParameters_Conversate>();
	if (ConversationParams == nullptr)
		return EBTNodeResult::Failed;
	
	auto OwnerConversationComponent = GetNpcConversationComponent(PawnOwner);
	NpcConversation::FRequestParams RequestParams = GetConversationRequestParams(ConversationParams);
	
	NpcConversation::FRequestResult RequestResult = OwnerConversationComponent->RequestConversation(ConversationPartner, RequestParams, false);
	if (!RequestResult.bSuccess)
	{
		if (RequestResult.RefuseReason.MatchesAny(ReasonsToHoldConversation) && !BTMemory->bConversationOnHold)
		{
			EnterConversationOnHoldState(OwnerComp, BTMemory, ConversationPartner);
			UE_VLOG(PawnOwner, LogARPGAI, Verbose, TEXT("Requested conversation. Can't start it right now, but entering conversation on hold state"));
			return EBTNodeResult::InProgress;
		}
		else
		{
			UE_VLOG(PawnOwner, LogARPGAI, Verbose, TEXT("Requested conversation. Can't start it, refuse reason = %s"), *RequestResult.RefuseReason.ToString());
			return EBTNodeResult::Failed;
		}
	}
	
	WaitForMessage(OwnerComp, AIGameplayTags::AI_BrainMessage_Conversation_Completed.GetTag().GetTagName());
	return EBTNodeResult::InProgress;
}

void UBTTask_RequestConversation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	if (BTMemory->bRestartConversation)
	{
		bool bConversationResumed = false;
		auto PawnOwner = OwnerComp.GetAIOwner()->GetPawn();
		if (auto CollocutorPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName)))
		{
			bConversationResumed = TryResumeConversation(PawnOwner, CollocutorPawn);
			BTMemory->bRestartConversation = false;
			BTMemory->bConversationOnHold = false;
		}

		if (!bConversationResumed)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
		
		SetNextTickTime(NodeMemory, FLT_MAX);
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

EBTNodeResult::Type UBTTask_RequestConversation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto OwnerConversationComponent = GetNpcConversationComponent(OwnerComp))
		OwnerConversationComponent->AbortConversation();	
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

bool UBTTask_RequestConversation::RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner)
{
	bool bResumedBase = Super::RequestResumeConversation(BTMemory, PawnOwner);
	if (!bResumedBase)
		return bResumedBase;

	// must postpone the TryStartConversation to next tick, otherwise a recursion with consecutive stack overflow exception is happening
	BTMemory->bRestartConversation = true;
	SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), 0.01f);
	
	// SetBlackboardConversationState(CollocutorBlackboard, NpcPawn, BTMemory->bForceConversationPartnerSuspendActivity, true);
	return true;
}

bool UBTTask_RequestConversation::TryResumeConversation(const APawn* PawnOwner, APawn* Collocutor) const
{
	auto NpcFlowComponent = GetNpcFlowComponent(PawnOwner);
	const FNpcGoalParameters_Conversate* ConversationGoalParams = NpcFlowComponent->GetParameters<FNpcGoalParameters_Conversate>();
	if (ConversationGoalParams == nullptr)
		return false;
	
	auto OwnerConversationComponent = GetNpcConversationComponent(PawnOwner);
	auto RequestParams = GetConversationRequestParams(ConversationGoalParams);
	NpcConversation::FRequestResult RequestResult = OwnerConversationComponent->RequestConversation(Collocutor, RequestParams, true);
	return RequestResult.bSuccess;
}

NpcConversation::FRequestParams UBTTask_RequestConversation::GetConversationRequestParams(const FNpcGoalParameters_Conversate* ConversationGoalParams) const
{
	NpcConversation::FRequestParams RequestParams;
	RequestParams.ConversationId = ConversationGoalParams->ConversationId;
	RequestParams.bRealtime = true;
	RequestParams.bForceSuspendActivity = ConversationGoalParams->bForceConversationPartnerSuspendActivity;
	RequestParams.bIncludePlayer = ConversationGoalParams->bIncludePlayer;
	RequestParams.SecondaryConversationParticipants = ConversationGoalParams->SecondaryConversationParticipants;
	RequestParams.bByPlayer = false;
	return RequestParams;
}
