#include "BehaviorTree/Tasks/Conversation/BTTask_ConversationBase.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/NpcActorTagsInterface.h"

UBTTask_ConversationBase::UBTTask_ConversationBase()
{
	ConversationPartnerBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ConversationBase, ConversationPartnerBBKey), AActor::StaticClass());
	CollocutorTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTTask_ConversationBase, CollocutorTagsBBKey)));
	bNotifyTaskFinished = true;
	bTickIntervals = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ConversationBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, FLT_MAX);
	return EBTNodeResult::InProgress;
}

void UBTTask_ConversationBase::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message,
                                         int32 RequestID, bool bSuccess)
{
	if (Message == AIGameplayTags::AI_BrainMessage_Conversation_Completed.GetTag().GetTagName())
	{
		// can be player, but we don't do anything in this case - player aborts dialogue - then it's over
		if (auto CollocutorPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName)))
		{
			if (auto CollocutorNpc = Cast<INpcActorTagsInterface>(CollocutorPawn))
			{
				FGameplayTagContainer CollocutorNpcTags = CollocutorNpc->GetTags_NPC();
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
	SetNextTickTime((uint8*)BTMemory, ConversationOnHoldTimeout);
	if (auto NpcOwner = Cast<INpcActorTagsInterface>(OwnerComp.GetAIOwner()->GetPawn()))
		NpcOwner->GiveTags_NPC(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());
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

bool UBTTask_ConversationBase::RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner)
{
	BTMemory->bConversationOnHold = false;
	BTMemory->CollocutorTagsChangedObserverDelegate.Reset(); // not unregistering from CollocutorBlackboard because returning RemoveObserver
	SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), FLT_MAX);
	if (auto Npc = Cast<INpcActorTagsInterface>(PawnOwner))
		Npc->RemoveTags_NPC(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());
	
	return true;
}

void UBTTask_ConversationBase::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	
	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto Npc = Cast<INpcActorTagsInterface>(AIController->GetPawn()))
			Npc->RemoveTags_NPC(AIGameplayTags::AI_State_Conversation_OnHold.GetTag().GetSingleTagContainer());

	// unsubscribe from collocutor tags observation if it has started
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
		if (BTMemory && BTMemory->CollocutorTagsChangedObserverDelegate.IsValid())
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
	InitializeNodeMemory<FBTMemory_Conversation>(NodeMemory, InitType);
	// auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	// BTMemory = {};
}

void UBTTask_ConversationBase::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTMemory_Conversation>(NodeMemory, CleanupType);
	// auto BTMemory = reinterpret_cast<FBTMemory_Conversation*>(NodeMemory);
	// BTMemory = {};
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
	FString Result = FString::Printf(TEXT("Have conversation with %s\nCollocutor NPC tags BB: %s"),
		*ConversationPartnerBBKey.SelectedKeyName.ToString(), *CollocutorTagsBBKey.SelectedKeyName.ToString());

	if (ReasonsToHoldConversation.Num() > 0)
		Result += FString::Printf(TEXT("\nHold conversation for %.2f s when collocutor has tags:\n%s"), ConversationOnHoldTimeout, *ReasonsToHoldConversation.ToStringSimple());

	if (ReasonsToAbortConversation.Num() > 0)
		Result += FString::Printf(TEXT("\nAbort conversation if collocutor has tags:\n%s"), *ReasonsToAbortConversation.ToStringSimple());

	return Result;
}
