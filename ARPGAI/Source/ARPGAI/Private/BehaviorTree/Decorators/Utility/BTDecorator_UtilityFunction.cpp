#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityFunction.h"
#include "BehaviorTree/BTCompositeNode.h"

UBTDecorator_UtilityFunction::UBTDecorator_UtilityFunction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "UnknownUtilityFunction";
	bAllowAbortNone = true;
	bAllowAbortLowerPri = true;
	bAllowAbortChildNodes = true;
}

float UBTDecorator_UtilityFunction::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return 0.0f;
}

float UBTDecorator_UtilityFunction::WrappedCalculateUtility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBTDecorator_UtilityFunction* NodeOb = bCreateNodeInstance
		? Cast<const UBTDecorator_UtilityFunction>(GetNodeInstance(OwnerComp, NodeMemory))
		: this;
	
	return NodeOb ? NodeOb->CalculateUtilityValue(OwnerComp, NodeMemory) : 0.0f;
}



