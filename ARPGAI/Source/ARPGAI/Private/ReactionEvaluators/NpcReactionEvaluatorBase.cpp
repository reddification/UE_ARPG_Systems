//
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcPerceptionReactionComponent.h"

UNpcReactionEvaluatorBase::UNpcReactionEvaluatorBase()
{
	Id = FGuid::NewGuid();
}

float UNpcReactionEvaluatorBase::EvaluatePerceptionReaction(AAIController* NpcController, UObject* ReactionEvaluatorMemory, float DeltaTime) const
{
	return ProcessPerceptionInternal(NpcController, ReactionEvaluatorMemory, DeltaTime);
}

bool UNpcReactionEvaluatorBase::LoadReactionContext(const UNpcBlackboardDataAsset* NpcBlackboardDataAsset,
	UBlackboardComponent* BlackboardComponent, UObject* ReactionEvaluatorMemory) const
{
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(NpcBlackboardDataAsset->ReactionBehaviorStateBBKey.SelectedKeyName, ReactionBehaviorStateTag.GetSingleTagContainer());
	BlackboardComponent->SetValue<UBlackboardKeyType_GameplayTag>(NpcBlackboardDataAsset->ReactionBehaviorCustomTagsBBKey.SelectedKeyName, CustomReactionBehaviorTags);

	return true;
}

void UNpcReactionEvaluatorBase::CompleteReaction(UNpcPerceptionReactionComponent* NpcPerceptionReactionComponent, UBlackboardComponent* Blackboard,
                                                 UObject* ReactionEvaluatorMemory, const FGameplayTag& ReactionBehaviorExecutionResult) const
{
	NpcPerceptionReactionComponent->ResetReactionBehaviorUtility(ReactionBehaviorType, Id);
}

float UNpcReactionEvaluatorBase::ProcessPerceptionInternal(AAIController* NpcController,
                                                           UObject* ReactionEvaluatorMemory, float DeltaTime) const
{
	return 0.f;
}