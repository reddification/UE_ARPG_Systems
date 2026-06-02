#include "BehaviorTree/Decorators/BTDecorator_ActivateBehavior.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

UBTDecorator_ActivateBehavior::UBTDecorator_ActivateBehavior()
{
	NodeName = "Activate behavior";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_ActivateBehavior::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto BEC = GetNpcBehaviorEvaluatorComponent_v2(SearchData.OwnerComp))
		BEC->ActivateBehavior(BehaviorTag);
}

void UBTDecorator_ActivateBehavior::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	auto MapBTResultToBEResult = [](EBTNodeResult::Type BTResult) -> EBehaviorEvaluatorResult
	{
		switch (BTResult)
		{
			case EBTNodeResult::Succeeded:
				return EBehaviorEvaluatorResult::Success;
			case EBTNodeResult::Failed:
				return EBehaviorEvaluatorResult::Fail;
			case EBTNodeResult::Aborted:
				return EBehaviorEvaluatorResult::Abort;
			default: ;
				ensure(false); // WTF?
			return EBehaviorEvaluatorResult::Success;
		}
	};
	
	if (auto BEC = GetNpcBehaviorEvaluatorComponent_v2(SearchData.OwnerComp))
		BEC->DeactivateBehavior(BehaviorTag, MapBTResultToBEResult(NodeResult));
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_ActivateBehavior::GetStaticDescription() const
{
	return *BehaviorTag.ToString();
}
