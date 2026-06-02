#include "BehaviorTree/Tasks/BehaviorEvaluators/BTTask_RequestBehaviorEvaluatorTemporalRelevancy.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTTask_RequestBehaviorEvaluatorTemporalRelevancy::UBTTask_RequestBehaviorEvaluatorTemporalRelevancy()
{
	NodeName = "Request behavior evaluator temportal relevancy";
}

EBTNodeResult::Type UBTTask_RequestBehaviorEvaluatorTemporalRelevancy::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	auto BehaviorEvaluatorComponent = GetNpcBehaviorEvaluatorComponent(OwnerComp);
	if (BehaviorEvaluatorComponent == nullptr)
		return EBTNodeResult::Failed;

	if (bRequestRelevant)
		for (const auto& BehaviorEvaluatorTag : BehaviorEvaluatorsTags)
			BehaviorEvaluatorComponent->RequestEvaluatorRelevant(BehaviorEvaluatorTag, Duration, RequestId);
	else
		for (const auto& BehaviorEvaluatorTag : BehaviorEvaluatorsTags)
			BehaviorEvaluatorComponent->RequestEvaluatorBlocked(BehaviorEvaluatorTag, Duration, RequestId);
	
	return EBTNodeResult::Succeeded;
}

FString UBTTask_RequestBehaviorEvaluatorTemporalRelevancy::GetStaticDescription() const
{
	return FString::Printf(TEXT("Request behavior evaluator %s\n%s\nfor %.2f s\nRequest id: %s"),
		bRequestRelevant ? TEXT("relevant") : TEXT("blocked"), *BehaviorEvaluatorsTags.ToStringSimple(), Duration, 
		*RequestId.ToString());
}
