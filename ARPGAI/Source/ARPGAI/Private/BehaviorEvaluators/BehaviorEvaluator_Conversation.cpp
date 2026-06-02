#include "BehaviorEvaluators/BehaviorEvaluator_Conversation.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "Data/LogChannels.h"

UBehaviorEvaluatorConfig_Conversation::UBehaviorEvaluatorConfig_Conversation()
{
	OutConversationPartnerBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_Conversation, OutConversationPartnerBBKey), AActor::StaticClass());
	bTickable = false;
	bUpdateWhenActivated = false;
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Conversation::CreateEvaluator(
	UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Conversation>(*BTComponent, this);
}

FBehaviorEvaluator_Conversation::FBehaviorEvaluator_Conversation(UBehaviorTreeComponent& OwnerComp,
	const UBehaviorEvaluatorConfig_Base* const Config) : Super(OwnerComp, Config)
{
	ConversationConfig = Cast<UBehaviorEvaluatorConfig_Conversation>(Config);
	ConversationComponent = GetNpcConversationComponent(Pawn.Get());
}

void FBehaviorEvaluator_Conversation::SetState(EBehaviorEvaluatorState NewState)
{
	auto OldState = GetState();
	Super::SetState(NewState);
	if (OldState == NewState)
		return;

	if (NewState == EBehaviorEvaluatorState::Relevant && (OldState == EBehaviorEvaluatorState::Blocked || OldState == EBehaviorEvaluatorState::NotRequested))
	{
		const auto& ActiveConversation = ConversationComponent->GetCurrentConversation();
		if (ensure(ActiveConversation.IsSet()))		
			SetMaxUtility();
	}
	else if (NewState == EBehaviorEvaluatorState::Blocked || NewState == EBehaviorEvaluatorState::NotRequested)
	{
		const auto& CurrentConversation = ConversationComponent->GetCurrentConversation();
		if (CurrentConversation.IsActive())
		{
			UE_VLOG(AIController.Get(), LogARPGAI_Conversation, VeryVerbose, TEXT("FBehaviorEvaluator_Conversation::SetState: Leaving conversation"));
			// 30 May 2026 (aki):
			// always call leave here and never AbortConversation, because this evaluator is basically only for accepting conversation
			ConversationComponent->LeaveConversation();
		}
	}
}

void FBehaviorEvaluator_Conversation::OnActivated()
{
	Super::OnActivated();
	AActor* ConversationActor = ConversationComponent->GetCurrentConversation().PrimaryCollocutor.Get();
	if (Blackboard.IsValid() && ConversationConfig.IsValid())
		Blackboard->SetValueAsObject(ConversationConfig->OutConversationPartnerBBKey.SelectedKeyName, ConversationActor);
}

void FBehaviorEvaluator_Conversation::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && ConversationConfig.IsValid())
		Blackboard->ClearValue(ConversationConfig->OutConversationPartnerBBKey.SelectedKeyName);
}