#include "BehaviorTree/Decorators/BTDecorator_Repeat.h"

#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/Composites/BTComposite_SimpleParallel.h"

UBTDecorator_Repeat::UBTDecorator_Repeat()
{
	NodeName = "Repeat";
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

void UBTDecorator_Repeat::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	FBTMemory_Repeat* DecoratorMemory = GetNodeMemory<FBTMemory_Repeat>(SearchData);
	FBTCompositeMemory* ParentMemory = GetParentNode()->GetNodeMemory<FBTCompositeMemory>(SearchData);
	const bool bIsSpecialNode = GetParentNode()->IsA(UBTComposite_SimpleParallel::StaticClass());

	if ((bIsSpecialNode && ParentMemory->CurrentChild == BTSpecialChild::NotInitialized) ||
		(!bIsSpecialNode && ParentMemory->CurrentChild != ChildIndex))
	{
		DecoratorMemory->TimeStarted = GetWorld()->GetTimeSeconds();
		DecoratorMemory->ActualRepeatDuration = FMath::RandRange(RepeatIntervalMin, RepeatIntervalMax);
	}

	DecoratorMemory->SearchId = SearchData.SearchId;
}

void UBTDecorator_Repeat::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	// protect from truly infinite loop within single search
	if (NodeResult != EBTNodeResult::Aborted)
	{
		FBTMemory_Repeat* DecoratorMemory = GetNodeMemory<FBTMemory_Repeat>(SearchData);
		bool bShouldLoop = false;
		if (SearchData.SearchId != DecoratorMemory->SearchId)
		{
			const auto CurrentTime = GetWorld()->GetTimeSeconds();
			bShouldLoop = DecoratorMemory->ActualRepeatDuration < 0.f || DecoratorMemory->TimeStarted + DecoratorMemory->ActualRepeatDuration > CurrentTime;
		}
	
		if (bShouldLoop)
			GetParentNode()->SetChildOverride(SearchData, ChildIndex);
	}
}

FString UBTDecorator_Repeat::GetStaticDescription() const
{
	return FString::Printf(TEXT("Repeat branch for [%.2f, %.2f]\n%s"), RepeatIntervalMin, RepeatIntervalMax, *Super::GetStaticDescription()) ;
}

void UBTDecorator_Repeat::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTMemory_Repeat>(NodeMemory, InitType);
}

void UBTDecorator_Repeat::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTMemory_Repeat>(NodeMemory, CleanupType);
}

void UBTDecorator_Repeat::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);
	const FBTMemory_Repeat* BTMemory = CastInstanceNodeMemory<FBTMemory_Repeat>(NodeMemory);
	if (BTMemory && BTMemory->ActualRepeatDuration > 0.f)
	{
		const double TimeRemaining = FMath::Max(BTMemory->TimeStarted + BTMemory->ActualRepeatDuration - GetWorld()->GetTimeSeconds(), 0.f);
		Values.Add(FString::Printf(TEXT("time remaining: %s"), *FString::SanitizeFloat(TimeRemaining)));
	}
}

#if WITH_EDITOR

FName UBTDecorator_Repeat::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.Loop.Icon");
}

#endif