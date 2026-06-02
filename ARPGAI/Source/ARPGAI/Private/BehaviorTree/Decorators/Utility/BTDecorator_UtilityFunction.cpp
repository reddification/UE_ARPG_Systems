#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityFunction.h"
#include "BehaviorTree/BTCompositeNode.h"

UBTDecorator_UtilityFunction::UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "UnknownUtilityFunction";
	bAllowAbortNone = true;
	bAllowAbortLowerPri = true;
	bAllowAbortChildNodes = true;
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_UtilityFunction::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto BTMemory = GetNodeMemory<FBTMemory_UtilityDecorator>(SearchData))
		BTMemory->bActive = true;
}

void UBTDecorator_UtilityFunction::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto BTMemory = GetNodeMemory<FBTMemory_UtilityDecorator>(SearchData))
		BTMemory->bActive = false;
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

float UBTDecorator_UtilityFunction::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return 0.0f;
}

void UBTDecorator_UtilityFunction::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTMemory_UtilityDecorator>(NodeMemory, InitType);
}

float UBTDecorator_UtilityFunction::WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBTDecorator_UtilityFunction* NodeOb = bCreateNodeInstance
		? Cast<const UBTDecorator_UtilityFunction>(GetNodeInstance(OwnerComp, NodeMemory))
		: this;
	
	return NodeOb ? NodeOb->CalculateUtilityValue(OwnerComp, NodeMemory) : 0.0f;
}

bool UBTDecorator_UtilityFunction::IsBranchActive(const UBehaviorTreeComponent& OwnerComp) const
{
	auto RawNodeMemory = CastInstanceNodeMemory<FBTMemory_UtilityDecorator>(OwnerComp.GetNodeMemory(this, OwnerComp.FindInstanceContainingNode(this)));
	return RawNodeMemory && RawNodeMemory->bActive;
}



