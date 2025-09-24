// 


#include "BehaviorTree/Decorators/BTDecorator_HandleNpcReactionBehavior.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionReactionComponent.h"
#include "Data/AIGameplayTags.h"
#include "ReactionEvaluators/NpcReactionEvaluatorBase.h"

UBTDecorator_HandleNpcReactionBehavior::UBTDecorator_HandleNpcReactionBehavior()
{
	NodeName = "Handle NPC reaction behavior";
	bNotifyDeactivation = true;
	CustomResultTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_HandleNpcReactionBehavior, CustomResultTagsBBKey)));
}

bool UBTDecorator_HandleNpcReactionBehavior::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	auto NpcComponent = OwnerComp.GetAIOwner()->FindComponentByClass<UNpcPerceptionReactionComponent>();
	auto ReactionEvaluatorData = NpcComponent->GetBestBehaviorPerceptionReactionEvaluatorState(ReactionBehaviorType);
	if (!ensure(ReactionEvaluatorData))
		return false;

	OwnerComp.GetBlackboardComponent()->ClearValue(CustomResultTagsBBKey.SelectedKeyName);
	
	return ReactionEvaluatorData->ReactionEvaluator->LoadReactionContext(NpcComponent->GetNpcDTR()->NpcBlackboardDataAsset,
		OwnerComp.GetBlackboardComponent(), ReactionEvaluatorData->EvaluatorMemory);
}

void UBTDecorator_HandleNpcReactionBehavior::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
                                                                EBTNodeResult::Type NodeResult)
{
	auto AIController =  SearchData.OwnerComp.GetAIOwner();
	if (AIController == nullptr)
		return;

	auto Pawn = AIController->GetPawn();
	if (Pawn == nullptr)
		return;
	
	auto NpcComponent = AIController->FindComponentByClass<UNpcPerceptionReactionComponent>();
	if (NpcComponent == nullptr)
		return;

	const auto* ReactionEvaluatorState = NpcComponent->GetBestBehaviorPerceptionReactionEvaluatorState(ReactionBehaviorType);
	if (ReactionEvaluatorState == nullptr)
		return;

	FGameplayTagContainer CustomExecutionResult = SearchData.OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_GameplayTag>(CustomResultTagsBBKey.SelectedKeyName);

	FGameplayTag ReactionBehaviorExecutionResult = FGameplayTag::EmptyTag;
	if (!CustomExecutionResult.IsEmpty())
	{
		ReactionBehaviorExecutionResult = CustomExecutionResult.First();
	}
	else
	{
		switch (NodeResult)
		{
		case EBTNodeResult::Succeeded:
			ReactionBehaviorExecutionResult = AIGameplayTags::AI_ReactionEvaluator_ExecutionResult_Success;
			break;
		case EBTNodeResult::Failed:
			ReactionBehaviorExecutionResult = AIGameplayTags::AI_ReactionEvaluator_ExecutionResult_Failure;
			break;
		case EBTNodeResult::Aborted:
			ReactionBehaviorExecutionResult = AIGameplayTags::AI_ReactionEvaluator_ExecutionResult_Abort;
			break;
		default:
			break;
		}	
	}
	
	ReactionEvaluatorState->ReactionEvaluator->CompleteReaction(NpcComponent, SearchData.OwnerComp.GetBlackboardComponent(), ReactionEvaluatorState->EvaluatorMemory,
		ReactionBehaviorExecutionResult);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_HandleNpcReactionBehavior::GetStaticDescription() const
{
	return FString::Printf(TEXT("On activation: load %s behavior context to blackboard\nOn deactivation: report execution result to reaction behavior evaluator\nCustom execution result BB: %s"),
		*StaticEnum<EReactionBehaviorType>()->GetDisplayValueAsText(ReactionBehaviorType).ToString(), *CustomResultTagsBBKey.SelectedKeyName.ToString());
}
