


#include "BehaviorTree/Decorators/BTDecorator_MaybeConst.h"

UBTDecorator_MaybeConst::UBTDecorator_MaybeConst()
{
	NodeName = "Maybe";
	bNotifyProcessed = true;

	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

float UBTDecorator_MaybeConst::GetProbability(UBlackboardComponent* Blackboard) const
{
	return Probability;
}

FString UBTDecorator_MaybeConst::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nProbability: %.2f"), *Super::GetStaticDescription(), Probability);
}
