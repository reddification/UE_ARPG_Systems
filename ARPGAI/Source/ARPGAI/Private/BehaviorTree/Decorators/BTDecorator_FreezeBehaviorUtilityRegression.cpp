#include "BehaviorTree/Decorators/BTDecorator_FreezeBehaviorUtilityRegression.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

UBTDecorator_FreezeBehaviorUtilityRegression::UBTDecorator_FreezeBehaviorUtilityRegression()
{
	NodeName = "Freeze BE Utility Regression";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	bNotifyTick = true;
	bTickIntervals = true;
}

void UBTDecorator_FreezeBehaviorUtilityRegression::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto BEComponent = GetNpcBehaviorEvaluatorComponent_v2(SearchData.OwnerComp))
	{
		auto* BTMemory = GetNodeMemory<FBTMemory_FreezeBehaviorUtilityRegression>(SearchData);
		BTMemory->bFreezeApplied = BTMemory->bFreezeApplied = BEComponent->RequestFreezeRegression(BehaviorId, true);
		if (BTMemory->bFreezeApplied && ForDuration > 0.f)
		{
			SetNextTickTime(reinterpret_cast<uint8*>(BTMemory), ForDuration);
		}
	}
}

void UBTDecorator_FreezeBehaviorUtilityRegression::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto BEComponent = GetNpcBehaviorEvaluatorComponent_v2(SearchData.OwnerComp))
	{
		auto* BTMemory = GetNodeMemory<FBTMemory_FreezeBehaviorUtilityRegression>(SearchData);
		if (BTMemory->bFreezeApplied)
			BEComponent->RequestFreezeRegression(BehaviorId, false);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_FreezeBehaviorUtilityRegression::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if (auto BEComponent = GetNpcBehaviorEvaluatorComponent_v2(OwnerComp))
	{
		auto* BTMemory = reinterpret_cast<FBTMemory_FreezeBehaviorUtilityRegression*>(NodeMemory);
		if (BTMemory->bFreezeApplied)
			BEComponent->RequestFreezeRegression(BehaviorId, false);
	}
	
	SetNextTickTime(NodeMemory, FLT_MAX);
}

FString UBTDecorator_FreezeBehaviorUtilityRegression::GetStaticDescription() const
{
	FString Base = FString::Printf(TEXT("Freeze utility regression for\n%s"), *BehaviorId.ToString());
	if (ForDuration > 0.f)
		Base += FString::Printf(TEXT("\nFor %.2fs (while decorator is active)"), ForDuration);

	return Base;
}
