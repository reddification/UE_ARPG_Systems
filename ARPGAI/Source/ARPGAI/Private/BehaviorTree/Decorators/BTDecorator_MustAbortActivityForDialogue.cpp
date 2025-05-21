#include "BehaviorTree/Decorators/BTDecorator_MustAbortActivityForDialogue.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"

UBTDecorator_MustAbortActivityForDialogue::UBTDecorator_MustAbortActivityForDialogue()
{
	NodeName = "Must abort activity for dialogue";
	ConversationPartnerBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_MustAbortActivityForDialogue, ConversationPartnerBBKey), AActor::StaticClass());
	NpcTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_MustAbortActivityForDialogue, NpcTagsBBKey)));
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

bool UBTDecorator_MustAbortActivityForDialogue::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto ConversationPartner = Cast<AActor>(Blackboard->GetValueAsObject(ConversationPartnerBBKey.SelectedKeyName));
	if (!ConversationPartner)
	{
		UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI, Verbose, TEXT("UBTDecorator_MustAbortActivityForDialogue: conversation partner not set"));
		return false;
	}
	
	auto OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
	FGameplayTagContainer NpcTags = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(NpcTagsBBKey.SelectedKeyName);
	if (!NpcRequiredStateToAbortActivityFilter.IsEmpty())
	{
		if (NpcRequiredStateToAbortActivityFilter.Matches(NpcTags))
			return true;
	}

	if (NpcTags.HasTag(AIGameplayTags::AI_State_Conversation_MaintainActivity_IgnoreOrientationToInvoker))
		return false;
		
	const float DotProduct = OwnerPawn->GetActorForwardVector() | (ConversationPartner->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
	return DotProduct < DotProductThreshold;
}

void UBTDecorator_MustAbortActivityForDialogue::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (!NpcRequiredStateToAbortActivityFilter.IsEmpty())
	{
		auto ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_MustAbortActivityForDialogue::OnNpcTagsChanged);
		OwnerComp.GetBlackboardComponent()->RegisterObserver(NpcTagsBBKey.GetSelectedKeyID(), this, ObserverDelegate);
	}
}

void UBTDecorator_MustAbortActivityForDialogue::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	if (!NpcRequiredStateToAbortActivityFilter.IsEmpty())
	{
		OwnerComp.GetBlackboardComponent()->UnregisterObserversFrom(this);
	}
}

void UBTDecorator_MustAbortActivityForDialogue::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto BTMemory = GetNodeMemory<FBTMemory_MustAbortActivityForDialogue>(SearchData);
	BTMemory->bActive = true;
}

void UBTDecorator_MustAbortActivityForDialogue::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	auto BTMemory = GetNodeMemory<FBTMemory_MustAbortActivityForDialogue>(SearchData);
	BTMemory->bActive = false;
}

EBlackboardNotificationResult UBTDecorator_MustAbortActivityForDialogue::OnNpcTagsChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (Key != NpcTagsBBKey.GetSelectedKeyID())
		return EBlackboardNotificationResult::RemoveObserver;

	auto BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	
	FBTMemory_MustAbortActivityForDialogue* BTMemory = reinterpret_cast<FBTMemory_MustAbortActivityForDialogue*>(BehaviorComp->GetNodeMemory(this,
	BehaviorComp->FindInstanceContainingNode(this)));
	if (BTMemory->bActive) // deliberately not changing BT execution if NPC is already under this decorator branch 
		return EBlackboardNotificationResult::ContinueObserving;

	FGameplayTagContainer NewTags = BlackboardComponent.GetValue<UBlackboardKeyType_GameplayTag>(NpcTagsBBKey.GetSelectedKeyID());
	if (NpcRequiredStateToAbortActivityFilter.Matches(NewTags))
		ConditionalFlowAbort(*BehaviorComp, EBTDecoratorAbortRequest::ConditionResultChanged);
		
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_MustAbortActivityForDialogue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		NpcTagsBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTDecorator_MustAbortActivityForDialogue::GetStaticDescription() const
{
	return FString::Printf(TEXT("Abort current activity if needed\nConversation partner: %s\nNpc tags BB: %s\nMin dot product between NPC and conversation partner to keep interaction = %.2f"),
		*ConversationPartnerBBKey.SelectedKeyName.ToString(), *NpcTagsBBKey.SelectedKeyName.ToString(), DotProductThreshold);
}

