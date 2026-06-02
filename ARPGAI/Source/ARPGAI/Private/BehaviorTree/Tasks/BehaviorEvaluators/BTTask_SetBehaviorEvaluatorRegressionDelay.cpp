#include "BehaviorTree/Tasks/BehaviorEvaluators/BTTask_SetBehaviorEvaluatorRegressionDelay.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

UBTTask_SetBehaviorEvaluatorRegressionDelay::UBTTask_SetBehaviorEvaluatorRegressionDelay()
{
	NodeName = "Set BE regression delay";
}

EBTNodeResult::Type UBTTask_SetBehaviorEvaluatorRegressionDelay::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	auto BehaviorEvaluatorComponent = GetNpcBehaviorEvaluatorComponent_v2(OwnerComp);
	if (!BehaviorEvaluatorComponent)
		return EBTNodeResult::Failed;
	
	const float DelayValue = Delay.GetValue(OwnerComp.GetBlackboardComponent());
	bool bSuccess = BehaviorEvaluatorComponent->DelayRegression(EvaluatorTag, DelayValue, bAppendToExisting);
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_SetBehaviorEvaluatorRegressionDelay::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s %ss regression delay for\n%s"), bAppendToExisting ? TEXT("Add") : TEXT("Set"), 
		*Delay.ToString(),  *EvaluatorTag.ToString());
}
