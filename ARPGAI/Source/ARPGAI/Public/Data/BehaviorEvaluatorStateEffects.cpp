#include "BehaviorEvaluatorStateEffects.h"

#include "Components/Controller/NpcBehaviorEvaluatorComponent2.h"

void FBehaviorEvaluatorStateEffect_BlockEvaluators::Apply(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const
{
	Super::Apply(EvaluatorComponent, EvaluatorId);
	
	FName SourceId = EvaluatorId.GetTagName();
	for (const auto& BlockRequest : BlockRequests)
	{
		if (BlockRequest.bIndefinitely)
			EvaluatorComponent.RequestEvaluatorBlocked(BlockRequest.BehaviorEvaluatorTag, true, SourceId);
		else
			EvaluatorComponent.RequestEvaluatorBlocked(BlockRequest.BehaviorEvaluatorTag, BlockRequest.Duration, SourceId);
	}
}

void FBehaviorEvaluatorStateEffect_BlockEvaluators::Rollback(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const
{
	Super::Rollback(EvaluatorComponent, EvaluatorId);
	
	FName SourceId = EvaluatorId.GetTagName();
	for (const auto& BlockRequest : BlockRequests)
		EvaluatorComponent.RequestEvaluatorBlocked(BlockRequest.BehaviorEvaluatorTag, false, SourceId);
}
