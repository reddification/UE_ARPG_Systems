


#include "BehaviorTree/Decorators/BTDecorator_MaybeBlackboardBase.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_MaybeBlackboardBase::UBTDecorator_MaybeBlackboardBase()
{
	NodeName = "Maybe Blackboard Base";
	ProbabilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_MaybeBlackboardBase, ProbabilityBBKey));
}

FString UBTDecorator_MaybeBlackboardBase::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s\nProbability BB: %s"), *Super::GetStaticDescription(), *ProbabilityBBKey.SelectedKeyName.ToString());
}

float UBTDecorator_MaybeBlackboardBase::GetProbability(UBlackboardComponent* Blackboard) const
{
	return Blackboard->GetValueAsFloat(ProbabilityBBKey.SelectedKeyName);
}
