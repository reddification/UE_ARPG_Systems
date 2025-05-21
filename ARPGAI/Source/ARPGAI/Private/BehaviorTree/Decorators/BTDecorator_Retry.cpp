// 


#include "BehaviorTree/Decorators/BTDecorator_Retry.h"

#include "AIController.h"
#include "BehaviorTree/BTCompositeNode.h"

UBTDecorator_Retry::UBTDecorator_Retry()
{
	NodeName = "Retry";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	FlowAbortMode = EBTFlowAbortMode::None;	
}

void UBTDecorator_Retry::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);

	auto BTMemory = GetNodeMemory<FBTMemory_Retry>(SearchData);
	BTMemory->TryUntilWorldTime = SearchData.OwnerComp.GetAIOwner()->GetWorld()->GetTimeSeconds() + FMath::RandRange(TimeLimitMin, TimeLimitMax);
	BTMemory->RetryCountsRemain = FMath::RandRange(AttemptsMin, AttemptsMax);
}

void UBTDecorator_Retry::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	
	if (NodeResult == EBTNodeResult::Failed)
	{
		auto AIController = SearchData.OwnerComp.GetAIOwner();
		if (AIController == nullptr)
			return;
		
		auto BTMemory = GetNodeMemory<FBTMemory_Retry>(SearchData);
		bool bNeedRetry = false;
		switch (RetryMode)
		{
			case EBehaviorRetryMode::Time:
				bNeedRetry = AIController->GetWorld()->GetTimeSeconds() < BTMemory->TryUntilWorldTime;
			break;
			case EBehaviorRetryMode::Count:
				bNeedRetry = BTMemory->RetryCountsRemain > 0;
				BTMemory->RetryCountsRemain--;
			break;
			default:
				break;
		}
		
		if (bNeedRetry)
			GetParentNode()->SetChildOverride(SearchData, GetChildIndex());
	}
}

FString UBTDecorator_Retry::GetStaticDescription() const
{
	return RetryMode == EBehaviorRetryMode::Count
		? FString::Printf(TEXT("On failure, retry [%d;%d] times"), AttemptsMin, AttemptsMax)
		: FString::Printf(TEXT("On failure, retry for [%f;%f] seconds"), TimeLimitMin, TimeLimitMax);
}
