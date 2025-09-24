// 


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

// copypaste from BTDecorator_Loop
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

	bool bShouldLoop = false;
	// protect from truly infinite loop within single search
	if (SearchData.SearchId != DecoratorMemory->SearchId)
	{
		const float Timeout = DecoratorMemory->ActualRepeatDuration;
		if ((Timeout < 0.f) || ((DecoratorMemory->TimeStarted + Timeout) > GetWorld()->GetTimeSeconds()))
		{
			bShouldLoop = true;
		}
	}

	DecoratorMemory->SearchId = SearchData.SearchId;

	// set child selection overrides
	if (bShouldLoop)
	{
		GetParentNode()->SetChildOverride(SearchData, ChildIndex);
	}
}

FString UBTDecorator_Repeat::GetStaticDescription() const
{
	return FString::Printf(TEXT("Repeat branch for [%.2f, %.2f]\n%s"), RepeatIntervalMin, RepeatIntervalMax, *Super::GetStaticDescription()) ;
}
