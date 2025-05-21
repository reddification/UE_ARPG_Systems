// 


#include "BehaviorTree/Tasks/BTTask_LoadReactionBehaviorContext.h"

#include "AIController.h"
#include "Activities/ActivityInstancesHelper.h"
#include "Components/NpcComponent.h"
#include "Components/NpcPerceptionReactionComponent.h"
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

UBTTask_LoadReactionBehaviorContext::UBTTask_LoadReactionBehaviorContext()
{
	NodeName = "Load Reaction Behavior Context";
}

EBTNodeResult::Type UBTTask_LoadReactionBehaviorContext::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NpcComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcPerceptionReactionComponent>();
	auto ReactionEvaluatorData = NpcComponent->GetBestBehaviorPerceptionReactionEvaluatorState(ReactionBehaviorType);
	if (!ensure(ReactionEvaluatorData))
		return EBTNodeResult::Failed;
	
	bool bSuccess = ReactionEvaluatorData->ReactionEvaluator->LoadReactionContext(NpcComponent->GetNpcDTR()->NpcBlackboardDataAsset,
		OwnerComp.GetBlackboardComponent(), ReactionEvaluatorData->EvaluatorMemory);
	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed; 
}

FString UBTTask_LoadReactionBehaviorContext::GetStaticDescription() const
{
	return FString::Printf(TEXT("Load blackboard data for reaction %s behavior"),
		*StaticEnum<EReactionBehaviorType>()->GetDisplayValueAsText(ReactionBehaviorType).ToString());
}
