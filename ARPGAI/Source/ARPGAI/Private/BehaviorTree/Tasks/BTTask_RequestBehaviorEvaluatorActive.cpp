// 


#include "BehaviorTree/Tasks/BTTask_RequestBehaviorEvaluatorActive.h"

#include "Activities/ActivityInstancesHelper.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTTask_RequestBehaviorEvaluatorActive::UBTTask_RequestBehaviorEvaluatorActive()
{
	NodeName = "Request behavior evaluator active";
}

EBTNodeResult::Type UBTTask_RequestBehaviorEvaluatorActive::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	auto BehaviorEvaluatorComponent = GetNpcBehaviorEvaluatorComponent(OwnerComp);
	if (BehaviorEvaluatorComponent == nullptr)
		return EBTNodeResult::Failed;

	if (bActive)
		for (const auto& BehaviorEvaluatorTag : BehaviorEvaluatorsTags)
			BehaviorEvaluatorComponent->RequestEvaluatorActive(BehaviorEvaluatorTag, Duration);
	else
		for (const auto& BehaviorEvaluatorTag : BehaviorEvaluatorsTags)
			BehaviorEvaluatorComponent->RequestEvaluatorBlocked(BehaviorEvaluatorTag, Duration);
	
	return EBTNodeResult::Succeeded;
}

FString UBTTask_RequestBehaviorEvaluatorActive::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s behavior evaluators\n%s\nfor %.2f s"),
		bActive ? TEXT("Activate") : TEXT("Block"), *BehaviorEvaluatorsTags.ToStringSimple(), Duration);
}
