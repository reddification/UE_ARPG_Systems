// 


#include "BehaviorTree/Decorators/BTDecorator_BlockBehaviorEvaluators.h"

#include "Activities/ActivityInstancesHelper.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTDecorator_BlockBehaviorEvaluators::UBTDecorator_BlockBehaviorEvaluators()
{
	NodeName = "Block behavior evaluators";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_BlockBehaviorEvaluators::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	GetNpcBehaviorEvaluatorComponent(SearchData.OwnerComp)->RequestEvaluatorsBlocked(BlockedBehaviorTags, true);
}

void UBTDecorator_BlockBehaviorEvaluators::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto NpcBehaviorEvaluatorComponent = GetNpcBehaviorEvaluatorComponent(SearchData.OwnerComp))
		NpcBehaviorEvaluatorComponent->RequestEvaluatorsBlocked(BlockedBehaviorTags, false);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_BlockBehaviorEvaluators::GetStaticDescription() const
{
	return FString::Printf(TEXT("Block behavior evaluators:\n%s"), *BlockedBehaviorTags.ToStringSimple());
}
