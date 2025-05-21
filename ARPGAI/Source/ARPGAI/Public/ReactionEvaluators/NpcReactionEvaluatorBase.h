// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_ResetBlackboardKeys.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "UObject/Object.h"
#include "NpcReactionEvaluatorBase.generated.h"


class UNpcPerceptionReactionComponent;
enum class EReactionBehaviorType : uint8;
class UEnvQuery;
class UNpcComponent;
class UBlackboardComponent;
class AAIController;

UCLASS(Abstract, EditInlineNew)
class ARPGAI_API UNpcReactionEvaluatorBase : public UObject
{
	GENERATED_BODY()

public:
	UNpcReactionEvaluatorBase();

	float EvaluatePerceptionReaction(AAIController* NpcController, UObject* ReactionEvaluatorMemory,
	                                 float DeltaTime) const;
	FORCEINLINE const EReactionBehaviorType& GetEvaluatorBehaviorType() const { return ReactionBehaviorType; }
	
	virtual UObject* CreateMemory(UObject* OuterObject) { return nullptr; }
	virtual bool LoadReactionContext(const UNpcBlackboardDataAsset* NpcBlackboardDataAsset, UBlackboardComponent* BlackboardComponent, UObject* ReactionEvaluatorMemory) const;
	virtual void CompleteReaction(UNpcPerceptionReactionComponent* NpcComponent, UBlackboardComponent* Blackboard, UObject* ReactionEvaluatorMemory, const
	                              FGameplayTag& ReactionBehaviorExecutionResult) const;
	
protected:
	virtual float ProcessPerceptionInternal(AAIController* NpcController, UObject* ReactionEvaluatorMemory,
	                                        float DeltaTime) const;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGuid Id;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString DeveloperDescription;
	
	// Main identifier to find and store memory for this evaluator
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReactionBehaviorType ReactionBehaviorType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ReactionBehaviorStateTag;

	// Can be used in behavior tree to branch logic
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer CustomReactionBehaviorTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery ActiveAtWorldState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRemoveOnWorldStateDoesntPassAnymore = false;

	// unlike ActiveAtWorldState, even when owner state doesn't comply to CanAccumulateOnlyAtCharacterState,
	// the evaluator is still considered "active" and can still be the one with the highest utility (priority).
	// It also loses utility over time if the reactions conditions are not met anymore 
	// This is useful for situations when NPC accumulates utility when it's doing something (i.e. interacting with smart object)
	// and then when the behavior is triggered for this reaction evaluator,
	// the NPC brain needs to load context from the reaction evaluator with the most utility into behavior tree for a respective behavior
	// hence this reaction evaluator needs to remain "active" and remain its utility, at least for the moment of activation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery CanAccumulateOnlyAtCharacterState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReactionBehaviorCooldownGameTimeHours = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxUtility = 1.f;

	// Useful for cases when you want to add some randomness to NPC reacting to something
	// For example, you might not want it to be guaranteed that merchants immediately react to passing by citizens
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, UIMax = 1.f, ClampMin = 0.f, ClampMax = 1.f))
	float ChanceToIncreaseUtility = 1.f;
	
	// New utility = utility + DeltaTime * UtilityAccumulationRate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UtilityAccumulationRate = 0.5f;
	
	// New utility = utility - DeltaTime * UtilityDecayRate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UtilityDecayRate = 0.5f;
};
