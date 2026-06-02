#include "BehaviorTree/Decorators/BTDecorator_RequestBehaviorEvaluatorState.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"

UBTDecorator_RequestBehaviorEvaluatorState::UBTDecorator_RequestBehaviorEvaluatorState()
{
	NodeName = "Request Behavior Evaluators States";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_RequestBehaviorEvaluatorState::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto EvaluatorComponent = GetNpcBehaviorEvaluatorComponent(SearchData.OwnerComp))
	{
		if (bRequestRelevant)
			EvaluatorComponent->RequestEvaluatorsRelevant(EvaluatorIds, true, RequestId);
		else 
			EvaluatorComponent->RequestEvaluatorsBlocked(EvaluatorIds, true, RequestId);
	}
}

void UBTDecorator_RequestBehaviorEvaluatorState::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto EvaluatorComponent = GetNpcBehaviorEvaluatorComponent(SearchData.OwnerComp))
	{
		if (bRequestRelevant)
			EvaluatorComponent->RequestEvaluatorsRelevant(EvaluatorIds, false, RequestId);
		else 
			EvaluatorComponent->RequestEvaluatorsBlocked(EvaluatorIds, false, RequestId);
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_RequestBehaviorEvaluatorState::GetStaticDescription() const
{
	if (EvaluatorIds.IsEmpty())
		return TEXT("Evaluators not specified");

	FString EvaluatorsListString = "";
	for (const auto& EvaluatorId : EvaluatorIds)
		EvaluatorsListString += TEXT("\n") + EvaluatorId.ToString();
	
	return FString::Printf(TEXT("Request %s%s\nRequest id: %s"),
		bRequestRelevant ? TEXT("relevant") : TEXT("blocked"), *EvaluatorsListString, *RequestId.ToString());
}
