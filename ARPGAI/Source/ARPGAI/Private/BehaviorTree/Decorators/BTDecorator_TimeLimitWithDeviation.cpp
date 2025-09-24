#include "BehaviorTree/Decorators/BTDecorator_TimeLimitWithDeviation.h"

UBTDecorator_TimeLimitWithDeviation::UBTDecorator_TimeLimitWithDeviation()
{
	NodeName = "Time limit with deviation";
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	bTickIntervals = true;
	bNotifyProcessed = true;
	// time limit always abort current branch
	bAllowAbortLowerPri = false;
	bAllowAbortNone = false;
	FlowAbortMode = EBTFlowAbortMode::Self;
}

void UBTDecorator_TimeLimitWithDeviation::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	float ActualTimeLimit = TimeLimit.GetValue(OwnerComp);
	ActualTimeLimit = bUseDeviationFraction
		? FMath::RandRange(ActualTimeLimit * (1.f - DeviationTime), ActualTimeLimit * (1.f + DeviationTime))
		: FMath::RandRange(ActualTimeLimit - DeviationTime, ActualTimeLimit + DeviationTime);

	ensure(ActualTimeLimit >= 0.f);

	auto BTMemory = reinterpret_cast<FBTMemory_TimeLimitWithDeviation*>(NodeMemory);
	BTMemory->ActualTimeLimit = FMath::Max(0.f, ActualTimeLimit);
	SetNextTickTime(NodeMemory, BTMemory->ActualTimeLimit);
}

void UBTDecorator_TimeLimitWithDeviation::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	reinterpret_cast<FBTMemory_TimeLimitWithDeviation*>(NodeMemory)->bElapsed = false;
}

void UBTDecorator_TimeLimitWithDeviation::OnNodeProcessed(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type& NodeResult)
{
	Super::OnNodeProcessed(SearchData, NodeResult);
	if (bFinishWithSuccessOnTimeLimit)
		if (auto BTMemory = GetNodeMemory<FBTMemory_TimeLimitWithDeviation>(SearchData))
			if (BTMemory->bElapsed)
				NodeResult = EBTNodeResult::Succeeded;
}

bool UBTDecorator_TimeLimitWithDeviation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                                     uint8* NodeMemory) const
{
	return !reinterpret_cast<FBTMemory_TimeLimitWithDeviation*>(NodeMemory)->bElapsed;
}

void UBTDecorator_TimeLimitWithDeviation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	FBTMemory_TimeLimitWithDeviation* TimeLimitMemory = CastInstanceNodeMemory<FBTMemory_TimeLimitWithDeviation>(NodeMemory);
	
	// Mark this decorator instance as Elapsed for calls to CalculateRawConditionValue
	TimeLimitMemory->bElapsed = true;

	// Set our next tick time to large value so we don't get ticked again in case the decorator
	// is still active after requesting execution (e.g. latent abort)
	SetNextTickTime(NodeMemory, FLT_MAX);

	OwnerComp.RequestExecution(this);
}

uint16 UBTDecorator_TimeLimitWithDeviation::GetInstanceMemorySize() const
{
	return sizeof(FBTMemory_TimeLimitWithDeviation);
}

#if WITH_EDITOR

FName UBTDecorator_TimeLimitWithDeviation::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.TimeLimit.Icon");
}

#endif

void UBTDecorator_TimeLimitWithDeviation::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                           EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTMemory_TimeLimitWithDeviation>(NodeMemory, InitType);
}

void UBTDecorator_TimeLimitWithDeviation::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTMemory_TimeLimitWithDeviation>(NodeMemory, CleanupType);
}

FString UBTDecorator_TimeLimitWithDeviation::GetStaticDescription() const
{
	FString a = bUseDeviationFraction
		? FString::Printf(TEXT("Wait %s +- %.2f%%"), *TimeLimit.ToString(), DeviationTime * 100.f)
		: FString::Printf(TEXT("Wait %s +- %.2f s"), *TimeLimit.ToString(), DeviationTime);

	FString b = bFinishWithSuccessOnTimeLimit ? TEXT("\nAfter finish with success") : TEXT("\nAfter finish with failure");

	return a.Append(b);
}
