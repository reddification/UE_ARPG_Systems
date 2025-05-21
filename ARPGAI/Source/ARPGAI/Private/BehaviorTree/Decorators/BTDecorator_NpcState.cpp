#include "BehaviorTree/Decorators/BTDecorator_NpcState.h"

#include "AIController.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcActivityComponent.h"

UBTDecorator_NpcState::UBTDecorator_NpcState()
{
	NodeName = "Set NPC activity state";
	bNotifyActivation = 1;
	bNotifyDeactivation = 1;
}

void UBTDecorator_NpcState::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto NpcStateComponent = SearchData.OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>();
	if (!ensure(NpcStateComponent))
		return;

	auto NpcActivityComponent = GetNpcActivityComponent(SearchData.OwnerComp);
	FGameplayTag StateTag = IsValid(NpcActivityComponent) ? NpcActivityComponent->GetActivityStateTag() : FGameplayTag::EmptyTag;
	if (StateTag.IsValid())
		NpcStateComponent->SetStateActive(StateTag, SetByCallerParams, true);
}

void UBTDecorator_NpcState::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	auto NpcStateComponent = SearchData.OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>();
	if (!ensure(NpcStateComponent))
		return;

	auto NpcActivityComponent = GetNpcActivityComponent(SearchData.OwnerComp);
	FGameplayTag StateTag = IsValid(NpcActivityComponent) ? NpcActivityComponent->GetActivityStateTag() : FGameplayTag::EmptyTag;
	if (StateTag.IsValid())
		NpcStateComponent->SetStateActive(StateTag, SetByCallerParams, false);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_NpcState::GetStaticDescription() const
{
	return "Set NPC State from activity";
}