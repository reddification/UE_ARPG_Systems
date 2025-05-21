#include "BehaviorTree/Decorators/Utility/BTDecorator_UtilityConstant.h"

UBTDecorator_UtilityConstant::UBTDecorator_UtilityConstant(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Constant Utility";
	UtilityValue = 0.0f;
}

float UBTDecorator_UtilityConstant::CalculateUtilityValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return UtilityValue;
}

FString UBTDecorator_UtilityConstant::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility: %.1f"), UtilityValue);
}
