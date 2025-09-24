// 


#include "BehaviorTree/Decorators/BTDecorator_LoadBehaviorContext.h"

#include "AIController.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTDecorator_LoadBehaviorContext::UBTDecorator_LoadBehaviorContext()
{
	NodeName = "Load behavior context";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_LoadBehaviorContext::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto BEComponent = SearchData.OwnerComp.GetAIOwner()->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
	BEComponent->InitiateBehaviorState(BehaviorId);
}

void UBTDecorator_LoadBehaviorContext::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto AIController = SearchData.OwnerComp.GetAIOwner())
	{
		auto BEComponent = AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
		BEComponent->FinalizeBehaviorState(BehaviorId);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_LoadBehaviorContext::GetStaticDescription() const
{
	return FString::Printf(TEXT("Load blackboard context for behavior:\n%s"), *BehaviorId.ToString());
}
