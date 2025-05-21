// 


#include "BehaviorTree/Tasks/BTTask_CompleteReaction.h"

#include "AIController.h"
#include "Activities/ActivityInstancesHelper.h"
#include "Components/NpcComponent.h"
#include "Components/NpcPerceptionReactionComponent.h"
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

UBTTask_CompleteReaction::UBTTask_CompleteReaction()
{
	NodeName = "Complete reaction";
}

EBTNodeResult::Type UBTTask_CompleteReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto NpcComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcPerceptionReactionComponent>();
	if (!ensure(NpcComponent))
		return EBTNodeResult::Failed;

	const auto* ReactionEvaluatorState = NpcComponent->GetBestBehaviorPerceptionReactionEvaluatorState(ReactionBehaviorType);
	if (!ensure(ReactionEvaluatorState != nullptr))
		return EBTNodeResult::Failed;

	ReactionEvaluatorState->ReactionEvaluator->CompleteReaction(NpcComponent, OwnerComp.GetBlackboardComponent(), ReactionEvaluatorState->EvaluatorMemory, FGameplayTag::EmptyTag);
	return EBTNodeResult::Succeeded;
}

FString UBTTask_CompleteReaction::GetStaticDescription() const
{
	return FString::Printf(TEXT("Complete reaction %s"), *StaticEnum<EReactionBehaviorType>()->GetDisplayValueAsText(ReactionBehaviorType).ToString());
}
