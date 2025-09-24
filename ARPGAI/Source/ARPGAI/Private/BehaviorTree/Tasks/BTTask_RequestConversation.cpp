


#include "BehaviorTree/Tasks/BTTask_RequestConversation.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Struct.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcFlowComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "StructUtils/StructView.h"
#include "GameFramework/Character.h"
#include "Interfaces/Npc.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

UBTTask_RequestConversation::UBTTask_RequestConversation()
{
	NodeName = "Request conversation";
	OutConversationAcceptedBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RequestConversation, OutConversationAcceptedBBKey));
}

EBTNodeResult::Type UBTTask_RequestConversation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BaseResult = Super::ExecuteTask(OwnerComp, NodeMemory);

	if (BaseResult == EBTNodeResult::Failed)
		return BaseResult;
	
#pragma region Check shit
	
	auto OwnerBlackboard = OwnerComp.GetBlackboardComponent();
	APawn* ConversationPartner = Cast<APawn>(OwnerBlackboard->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName));
	if(!ensure(ConversationPartner))
		return EBTNodeResult::Failed;
	
	auto PawnOwner = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	auto NpcOwner = Cast<INpc>(PawnOwner);
	if (!ensure(NpcOwner))
		return EBTNodeResult::Failed;

	auto NpcConversationPartner = Cast<INpc>(ConversationPartner);
	if (!ensure(NpcConversationPartner))
		return EBTNodeResult::Failed;

#pragma endregion	
	
	FGameplayTag RefuseReason;
	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	if (!NpcConversationPartner->CanConversate(PawnOwner, RefuseReason))
	{
		if (ReasonsToHoldConversation.HasTag(RefuseReason) && !BTMemory->bConversationOnHold)
		{
			EnterConversationOnHoldState(OwnerComp, BTMemory, ConversationPartner);
			UE_VLOG(PawnOwner, LogARPGAI, Verbose, TEXT("Requested conversation. Can't start it right now, but entering conversation on hold state"));
			return EBTNodeResult::InProgress;
		}
		else
		{
			return EBTNodeResult::Failed;
		}
	}
	
	auto ConversationPartnerBlackboard = NpcConversationPartner->GetBlackboard();
	
	auto NpcFlowComponent = OwnerComp.GetAIOwner()->FindComponentByClass<UNpcFlowComponent>();
	const FNpcGoalParameters_Conversate* ConversationParams = NpcFlowComponent->GetParameters<FNpcGoalParameters_Conversate>();
	if (ConversationParams == nullptr)
		return EBTNodeResult::Failed;
	
	const bool bStarted = StartConversation(OwnerBlackboard, ConversationParams, PawnOwner, NpcOwner, ConversationPartner,
	                                        ConversationPartnerBlackboard);
	
	if (!ensure(bStarted))
	{
		ClearBlackboardConversationState(OwnerBlackboard);
		// ClearBlackboardConversationState(ConversationPartnerBlackboard);
		return EBTNodeResult::Failed;
	}

	BTMemory->bForceConversationPartnerSuspendActivity = ConversationParams->bForceConversationPartnerSuspendActivity;
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
			if (auto CollocutorAIController = CollocutorPawn->GetController<AAIController>())
			{
				auto CollocutorBlackboard = CollocutorAIController->GetBlackboardComponent();
				bConversationResumed = TryStartConversation(PawnOwner, CollocutorBlackboard, &OwnerComp);
				BTMemory->bRestartConversation = false;
				BTMemory->bConversationOnHold = false;
			}
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
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Npc = Cast<INpc>(AIController->GetPawn()))
			Npc->StopConversation();

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

bool UBTTask_RequestConversation::TryStartConversation(APawn* PawnOwner, UBlackboardComponent* CollocutorBlackboard, UBehaviorTreeComponent* OwnerComp)
{
	auto NpcFlowComponent = OwnerComp->GetAIOwner()->FindComponentByClass<UNpcFlowComponent>();
	const FNpcGoalParameters_Conversate* ConversationParams = NpcFlowComponent->GetParameters<FNpcGoalParameters_Conversate>();
	if (ConversationParams == nullptr)
		return false;
	
	auto CollocutorPawn = CollocutorBlackboard->GetBrainComponent()->GetAIOwner()->GetPawn();
	return StartConversation(OwnerComp->GetBlackboardComponent(), ConversationParams, PawnOwner, Cast<INpc>(PawnOwner),
	                         CollocutorPawn, CollocutorBlackboard);
}


void UBTTask_RequestConversation::SetBlackboardConversationState(UBlackboardComponent* Blackboard,
	AActor* ConversationPartner, bool bConversationPrioritized, bool bConversationAccepted) const
{
	if (ensure(Blackboard))
	{
		Blackboard->SetValueAsBool(OutIsConversationPrioritizedBBKey.SelectedKeyName, bConversationPrioritized);
		Blackboard->SetValueAsObject(ConversationPartnerBBKey.SelectedKeyName, ConversationPartner);
		Blackboard->SetValueAsBool(OutConversationAcceptedBBKey.SelectedKeyName, bConversationAccepted);
	}
}

bool UBTTask_RequestConversation::StartConversation(UBlackboardComponent* OwnerBlackboard,
                                                    const FNpcGoalParameters_Conversate* ConversationStartParams, APawn* PawnOwner,
                                                    INpc* NpcOwner, APawn* ConversationPartner,
                                                    UBlackboardComponent* ConversationPartnerBlackboard)
{
	TArray<AActor*> ConversationParticipants;
	ConversationParticipants.Add(ConversationPartner);

	TArray<UBlackboardComponent*> SecondaryConversationParticipantsBlackboards;
	
	if (!ConversationStartParams->SecondaryConversationParticipants.IsEmpty())
	{
		auto NpcSubsystem = UNpcRegistrationSubsystem::Get(PawnOwner);
		const FVector& PawnOwnerLocation = PawnOwner->GetActorLocation();
		for (const auto& SecondaryConversationPartner : ConversationStartParams->SecondaryConversationParticipants)
		{
			TArray<UNpcComponent*> SecondaryNpcs = NpcSubsystem->GetNpcsInRange(SecondaryConversationPartner.CharacterId, PawnOwnerLocation,
				SecondaryConversationPartner.SearchInRange, SecondaryConversationPartner.Count, &SecondaryConversationPartner.CharacterFilter);

			for (const auto& SecondaryNpcComponent : SecondaryNpcs)
			{
				FGameplayTag SecondaryParticipantRefuseReason;
				auto SecondaryParticipantNpc = Cast<INpc>(SecondaryNpcComponent->GetOwner());
				if (SecondaryParticipantNpc && SecondaryParticipantNpc->CanConversate(PawnOwner, SecondaryParticipantRefuseReason))
				{
					ConversationParticipants.Add(SecondaryNpcComponent->GetOwner());
					SecondaryConversationParticipantsBlackboards.Add(SecondaryParticipantNpc->GetBlackboard());
				}
			}
		}
	}

	if (ConversationStartParams->bIncludePlayer)
	{
		auto PlayerCharacter = UGameplayStatics::GetPlayerCharacter(PawnOwner, 0);
		ConversationParticipants.Add(PlayerCharacter);
	}

	const bool bStarted = NpcOwner->StartConversation(ConversationStartParams->ConversationId, ConversationParticipants, false);
	UE_VLOG(PawnOwner, LogARPGAI, Verbose, TEXT("Attempting to start conversation: %s"), bStarted ? TEXT("success") : TEXT("failure"));

	if (bStarted)
	{
		SetBlackboardConversationState(OwnerBlackboard, ConversationPartner, ConversationStartParams->bForceConversationPartnerSuspendActivity, false);
		SetBlackboardConversationState(ConversationPartnerBlackboard, PawnOwner, ConversationStartParams->bForceConversationPartnerSuspendActivity, true);
		for (auto* SecondaryParticipantBlackboard : SecondaryConversationParticipantsBlackboards)
			SetBlackboardConversationState(SecondaryParticipantBlackboard, PawnOwner, ConversationStartParams->bForceConversationPartnerSuspendActivity, true);
	}

	return bStarted;
}

FString UBTTask_RequestConversation::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\n[out]Conversation accepted BB: %s"), *Super::GetStaticDescription(), *OutConversationAcceptedBBKey.SelectedKeyName.ToString());
}
