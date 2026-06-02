// 


#include "BehaviorTree/Decorators/BTDecorator_MaybeCurve.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_MaybeCurve::UBTDecorator_MaybeCurve()
{
	NodeName = "Maybe Curve Dependency";
}

float UBTDecorator_MaybeCurve::GetProbability(UBlackboardComponent* Blackboard) const
{
	return ProbabilityCurve.GetRichCurveConst()->Eval(Blackboard->GetValueAsFloat(ProbabilityBBKey.SelectedKeyName));
}
