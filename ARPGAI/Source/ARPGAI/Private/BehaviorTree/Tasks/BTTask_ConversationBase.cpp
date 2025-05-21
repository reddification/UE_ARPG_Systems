// 

#include "BehaviorTree/Tasks/BTTask_ConversationBase.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"

UBTTask_ConversationBase::UBTTask_ConversationBase()
{
	ConversationPartnerBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ConversationBase, ConversationPartnerBBKey), AActor::StaticClass());
	OutIsConversationPrioritizedBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ConversationBase, OutIsConversationPrioritizedBBKey));
	CollocutorTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_ConversationBase, CollocutorTagsBBKey)));
	bNotifyTaskFinished = true;
	bTickIntervals = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ConversationBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, FLT_MAX);
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UBTTask_ConversationBase::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message,
                                         int32 RequestID, bool bSuccess)
{
	if (Message == AIGameplayTags::AI_BrainMessage_Conversation_Completed.GetTag().GetTagName())
	{
		// can be player, but we don't do anything in this case - player aborts dialogue - then it's over
		if (auto CollocutorPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName)))
		{
			if (auto CollocutorNpc = Cast<INpc>(CollocutorPawn))
			{
				FGameplayTagContainer CollocutorNpcTags = CollocutorNpc->GetNpcOwnerTags();
				if (CollocutorNpcTags.HasAny(ReasonsToHoldConversation))
				{
					EnterConversationOnHoldState(OwnerComp, reinterpret_cast<FBTMemory_Conversation*>(NodeMemory), CollocutorPawn);
					return;
				}
			}
		}
	}
	
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
}

void UBTTask_ConversationBase::EnterConversationOnHoldState(UBehaviorTreeComponent& OwnerComp, FBTMemory_Conversation* BTMemory, APawn* ConversationPartner)
{
	if (!BTMemory->CollocutorTagsChangedObserverDelegate.IsValid())
	{
		auto NotifyDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTTask_ConversationBase::OnCollocutorTagsChanged, &OwnerComp);
		auto CollocutorAIController = ConversationPartner->GetController<AAIController>();
		auto CollocutorBlackboard = CollocutorAIController->GetBlackboardComponent();
		BTMemory->CollocutorTagsChangedObserverDelegate = CollocutorBlackboard->RegisterObserver(CollocutorTagsBBKey.GetSelectedKeyID(), this, NotifyDelegate);
	}
			
	BTMemory->bConversationOnHold = true;
	SetNextTickTime((uint8*)BTMemory, WaitAbortWhenConversationIsOnHoldTimeSeconds);
	auto NpcOwner = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn());
	NpcOwner->GiveNpcTags(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());
}

//we can only be here if conversation is on hold
EBlackboardNotificationResult UBTTask_ConversationBase::OnCollocutorTagsChanged(
	const UBlackboardComponent& CollocutorBlackboard, FBlackboard::FKey Key, UBehaviorTreeComponent* OwnerComp)
{
	if (Key != CollocutorTagsBBKey.GetSelectedKeyID())
		return EBlackboardNotificationResult::RemoveObserver;

	FGameplayTagContainer NewTags = CollocutorBlackboard.GetValue<UBlackboardKeyType_GameplayTag>(Key);
	if (NewTags.HasAny(ReasonsToAbortConversation))
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
		return EBlackboardNotificationResult::RemoveObserver;
	}
	else if (NewTags.HasAny(ReasonsToHoldConversation))
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}

	auto NodeMemory = OwnerComp->GetNodeMemory(this, OwnerComp->FindInstanceContainingNode(this));
	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);

	RequestResumeConversation(BTMemory, OwnerComp->GetAIOwner()->GetPawn());
	
	return EBlackboardNotificationResult::RemoveObserver;
}

void UBTTask_ConversationBase::ClearBlackboardConversationState(UBlackboardComponent* Blackboard) const
{
	if (ensure(Blackboard))
	{
		Blackboard->ClearValue(ConversationPartnerBBKey.SelectedKeyName);
		Blackboard->ClearValue(OutIsConversationPrioritizedBBKey.SelectedKeyName);
	}
}

bool UBTTask_ConversationBase::RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner)
{
	BTMemory->bConversationOnHold = false;
	BTMemory->CollocutorTagsChangedObserverDelegate.Reset(); // not unregistering from CollocutorBlackboard because returning RemoveObserver
	SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), FLT_MAX);
	if (auto Npc = Cast<INpc>(PawnOwner))
		Npc->RemoveNpcTags(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());
	
	return true;
}

void UBTTask_ConversationBase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	auto Blackboard = OwnerComp.GetBlackboardComponent();

	if (bClearConversationBlackboardOnTaskFinished)
		ClearBlackboardConversationState(Blackboard);
	
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Npc = Cast<INpc>(AIController->GetPawn()))
			Npc->RemoveNpcTags(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());

	// unsubscribe from collocutor tags observation if it has started
	if (Blackboard)
	{
		auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
		if (BTMemory->CollocutorTagsChangedObserverDelegate.IsValid())
		{
			if (auto Collocutor = Cast<APawn>(Blackboard->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName)))
			{
				if (auto CollocutorController = Collocutor->GetController<AAIController>())
				{
					CollocutorController->GetBlackboardComponent()->UnregisterObserver(CollocutorTagsBBKey.GetSelectedKeyID(),
						BTMemory->CollocutorTagsChangedObserverDelegate);
					BTMemory->CollocutorTagsChangedObserverDelegate.Reset();
				}				
			}
		}
	}
}


void UBTTask_ConversationBase::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	BTMemory = {};
}

void UBTTask_ConversationBase::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	BTMemory = {};
}

void UBTTask_ConversationBase::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		CollocutorTagsBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTTask_ConversationBase::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("Have conversation with %s\n[out]Is conversation prioritized BB: %s\nCollocutor NPC tags BB: %s"),
		*ConversationPartnerBBKey.SelectedKeyName.ToString(), *OutIsConversationPrioritizedBBKey.SelectedKeyName.ToString(),
		*CollocutorTagsBBKey.SelectedKeyName.ToString());

	if (ReasonsToHoldConversation.Num() > 0)
		Result += FString::Printf(TEXT("\nHold conversation for %.2f s when collocutor has tags:\n%s"), WaitAbortWhenConversationIsOnHoldTimeSeconds, *ReasonsToHoldConversation.ToStringSimple());

	if (ReasonsToAbortConversation.Num() > 0)
		Result += FString::Printf(TEXT("\nAbort conversation if collocutor has tags:\n%s"), *ReasonsToAbortConversation.ToStringSimple());

	if (bClearConversationBlackboardOnTaskFinished)
		Result += FString::Printf(TEXT("\nClear conversation blackboard on task finished"));
	
	return Result;
}
