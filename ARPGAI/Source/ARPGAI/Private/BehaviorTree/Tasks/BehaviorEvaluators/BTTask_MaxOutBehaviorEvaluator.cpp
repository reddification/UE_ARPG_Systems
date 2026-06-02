#include "BehaviorTree/Tasks/BehaviorEvaluators/BTTask_MaxOutBehaviorEvaluator.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

UBTTask_MaxOutBehaviorEvaluator::UBTTask_MaxOutBehaviorEvaluator()
{
	NodeName = "Max Out Behavior Evaluator";
}

EBTNodeResult::Type UBTTask_MaxOutBehaviorEvaluator::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto BehaviorEvaluatorComponent = GetNpcBehaviorEvaluatorComponent_v2(OwnerComp);
	if (!BehaviorEvaluatorComponent)
		return EBTNodeResult::Failed;
	
	bool bSuccess = BehaviorEvaluatorComponent->SetMaxUtility(EvaluatorTag);
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_MaxOutBehaviorEvaluator::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set max utility for %s"), *EvaluatorTag.ToString());
}
