

#include "BehaviorTree/Decorators/BTDecorator_MaybeBase.h"

#include "BehaviorTree/BTCompositeNode.h"

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
		bool bCanForceSuccess = true;

		// Decorator is always allowed to force success during the search to ignore an optional branch in a sequence.
		// But when used to override the node result on failure we only modify result if search originates
		// from our parent node (abort self) or on our associated node (failure).
		if (!SearchData.bSearchInProgress)
		{
			const int32 InstanceIdx = SearchData.OwnerComp.GetActiveInstanceIdx();
			const FBTNodeIndex ParentNodeIndex(InstanceIdx, GetParentNode() != nullptr ? GetParentNode()->GetExecutionIndex() : 0);
			const UBTNode* MyNode = GetMyNode();
			const FBTNodeIndex MyNodeIndex(InstanceIdx, MyNode != nullptr ? MyNode->GetExecutionIndex() : 0);
			bCanForceSuccess = (SearchData.SearchRootNode == ParentNodeIndex || SearchData.SearchRootNode == MyNodeIndex);
		}

		if (bCanForceSuccess)
		{
			checkf(NodeResult != EBTNodeResult::Aborted, TEXT("Should never change a result set to 'Aborted'"));
			NodeResult = EBTNodeResult::Succeeded;
			BT_SEARCHLOG(SearchData, Log, TEXT("Forcing Success: %s"), *UBehaviorTreeTypes::DescribeNodeHelper(this));
		}
	}
}
