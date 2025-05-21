#include "BehaviorTree/Composites/BTComposite_Random.h"

UBTComposite_Random::UBTComposite_Random()
{
	NodeName = "Uniform Random";
}

int32 UBTComposite_Random::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild,
	EBTNodeResult::Type LastResult) const
{
	return PrevChild == BTSpecialChild::NotInitialized
		? FMath::RandRange(0, GetChildrenNum() - 1)
		: BTSpecialChild::ReturnToParent;
}

FString UBTComposite_Random::GetStaticDescription() const
{
	return "Execute single child randomly";
}

void UBTComposite_Random::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	// Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTCompositeMemory>(NodeMemory, InitType);
}

void UBTComposite_Random::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	// Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTCompositeMemory>(NodeMemory, CleanupType);
}
