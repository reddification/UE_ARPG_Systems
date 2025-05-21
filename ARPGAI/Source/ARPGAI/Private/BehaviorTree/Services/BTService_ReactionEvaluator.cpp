// 


#include "BehaviorTree/Services/BTService_ReactionEvaluator.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Components/NpcPerceptionReactionComponent.h"
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

UBTService_ReactionEvaluator::UBTService_ReactionEvaluator()
{
	NodeName = "Perception Reaction Evaluator";
}

void UBTService_ReactionEvaluator::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_ReactionEvaluator::TickNode)

	auto NpcPerceptionReactionComponent = OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcPerceptionReactionComponent>();
	if (!ensure(NpcPerceptionReactionComponent))
	{
		SetNextTickTime(NodeMemory, FLT_MAX);
		return;
	}
	
	const auto& ReactionEvaluators = NpcPerceptionReactionComponent->GetPerceptionReactionEvaluators();
	auto AIController = OwnerComp.GetAIOwner();
	for (const auto& ReactionEvaluator : ReactionEvaluators)
	{
		const float DeltaUtility = ReactionEvaluator.ReactionEvaluator->EvaluatePerceptionReaction(AIController, ReactionEvaluator.EvaluatorMemory, DeltaSeconds);
		NpcPerceptionReactionComponent->UpdatePerceptionReactionBehaviorUtility(ReactionEvaluator.ReactionEvaluator->ReactionBehaviorType, ReactionEvaluator.ReactionEvaluator->Id, DeltaUtility);
	}
}