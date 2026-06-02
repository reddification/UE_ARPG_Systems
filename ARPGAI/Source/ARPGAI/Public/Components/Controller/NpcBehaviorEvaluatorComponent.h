#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NpcBehaviorEvaluatorComponent.generated.h"

class UBehaviorTreeComponent;
class IBehaviorEvaluator;
struct FGameplayTag;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcBehaviorEvaluatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcBehaviorEvaluatorComponent();
	
	/**
	 * Requests evaluators to be relevant. Depending on state  model, immediate relevancy is not guaranteed.
	 * @param EvaluatorTags Tags of evaluator to effect
	 * @param bActive If applying or removing request
	 * @param SourceId Identifier of request. Requests are stored it a stack-like array. Source id is optional, but can be useful if states change order is not sequential.
	 * Requests are removed in reverse order when first request id matches provided SourceId 
	 */
	virtual void RequestEvaluatorsRelevant(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId = FName("Anonymous")) {};
	
	/**
	 * Requests evaluators to be blocked. With current state models, evaluator is guaranteed to be blocked after
	 * @param EvaluatorTags Tags of evaluator to effect
	 * @param bActive If applying or removing request
	 * @param SourceId Identifier of request. Requests are stored it a stack-like array. Source id is optional, but can be useful if states change order is not sequential
	 * Requests are removed in reverse order when first request id matches provided SourceId 
	 */
	virtual void RequestEvaluatorsBlocked(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId = FName("Anonymous")) {};

	/**
	 * Requests an evaluator to be temporarily relevant. Depending on state model, immediate relevancy is not guaranteed.
	 * @param EvaluatorTag Tag of evaluator to effect
	 * @param Duration Duration of relevancy
	 * @param SourceId Identifier of request. Requests are stored it a stack-like array. Source id is optional, but can be useful if states change order is not sequential.
	 * Requests are removed in reverse order when first request id matches provided SourceId 
	 */
	virtual void RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, float Duration,
	                                      const FName& SourceId = FName("Anonymous")) {};
	
	/**
	 * Requests an evaluator to be temporarily blocked. With current evaluation models, evaluator is guaranteed to be blocked after
	 * @param EvaluatorTag Tag of evaluator to effect
	 * @param Duration Duration of relevancy
	 * @param SourceId Identifier of request. Requests are stored it a stack-like array. Source id is optional, but can be useful if states change order is not sequential.
	 * Requests are removed in reverse order when first request id matches provided SourceId 
	 */
	virtual void RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, float Duration,
	                                     const FName& SourceId = FName("Anonymous")) { }
	
	virtual void RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, bool bActive,
	                                      const FName& SourceId = FName("Anonymous")) { }
	
	virtual void RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, bool bActive,
	                                     const FName& SourceId = FName("Anonymous")) { }
	
	/**
	 * Requests an evaluator to be temporarily blocked. With current evaluation models, evaluator is guaranteed to be blocked after
	 * @param EvaluatorTag Tag of evaluator to effect
	 * @param Cooldown Duration of relevancy
	 * @param SourceId Identifier of request. Requests are stored it a stack-like array. Source id is optional, but can be useful if states change order is not sequential.
	 * Requests are removed in reverse order when first request id matches provided SourceId 
	 */
	virtual bool SetBehaviorEvaluatorCooldown(const FGameplayTag& EvaluatorTag, float Cooldown, const FName& SourceId = FName("Anonymous")) { return false; };
};
