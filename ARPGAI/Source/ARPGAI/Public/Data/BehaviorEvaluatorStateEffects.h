#pragma once
#include "AiDataTypes.h"
#include "BehaviorEvaluatorDataTypes.h"
#include "GameplayTagContainer.h"

#include "BehaviorEvaluatorStateEffects.generated.h"

class UNpcBehaviorEvaluatorComponent2;

USTRUCT()
struct FBehaviorEvaluatorStateEffect
{
	GENERATED_BODY()
	
public:
	virtual ~FBehaviorEvaluatorStateEffect() = default;
	
	virtual void Apply(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const {};
	virtual void Rollback(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const {};
};

USTRUCT(BlueprintType, DisplayName="Block evaluators")
struct FBehaviorEvaluatorStateEffect_BlockEvaluators : public FBehaviorEvaluatorStateEffect
{
	GENERATED_BODY()

	using Super = FBehaviorEvaluatorStateEffect;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Control)
	TArray<FBehaviorEvaluatorBlockRequest> BlockRequests;
	
	virtual void Apply(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const override;
	virtual void Rollback(UNpcBehaviorEvaluatorComponent2& EvaluatorComponent, const FGameplayTag& EvaluatorId) const override;
};

USTRUCT(BlueprintType)
struct FBehaviorEvaluatorStateEffectsContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FBehaviorEvaluatorStateEffect>> Effects;	
};

// 23 May 2026 (aki): currently unused. 
// instead TMap<EBehaviorEvaluatorState, FBehaviorEvaluatorTransitionTasksContainer> is used because it's simpler and currently covers all cases  
USTRUCT(BlueprintType)
struct FBehaviorStateTransitionRule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TOptional<TArray<EBehaviorEvaluatorState>> FromStates;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBehaviorEvaluatorState ToState = EBehaviorEvaluatorState::Relevant;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBehaviorEvaluatorStateEffectsContainer TasksContainer;
};
