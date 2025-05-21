

#include "BehaviorTree/Decorators/BTDecorator_MaybeBase.h"

bool UBTDecorator_MaybeBase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (bHandledByWeightedRandom)
	{
		return true;
	}

	const float Probability = GetProbability(OwnerComp.GetBlackboardComponent());
	return FMath::RandRange(0.f, 1.f) < Probability;
}

FString UBTDecorator_MaybeBase::GetStaticDescription() const
{
	return FString::Printf(TEXT("Execute this node with some probability %s%s"),
		bHandledByWeightedRandom ? TEXT("\nDecorator is used for weighted random composite\nand will always return true") : TEXT(""),
		bForceSuccess ? TEXT("\nForce success") : TEXT(""));
}

float UBTDecorator_MaybeBase::GetProbability(UBlackboardComponent* Blackboard) const
{
	return 0.f;
}

void UBTDecorator_MaybeBase::OnNodeProcessed(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type& NodeResult)
{
	Super::OnNodeProcessed(SearchData, NodeResult);
	if (bForceSuccess)
	{
		NodeResult = EBTNodeResult::Succeeded;
	}
}
