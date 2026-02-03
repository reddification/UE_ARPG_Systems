// 

#pragma once

#include "CoreMinimal.h"
#include "NpcReactionEvaluatorBase.h"
#include "NpcReactionEvaluator_ToCharacter.generated.h"

/**
 * 
 */

class UNpcPerceptionReactionComponent;

UCLASS()
class UNpcReactionEvaluatorMemory_Actor : public UObject
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<const AActor> Actor;
};

UCLASS()
class ARPGAI_API UNpcReactionEvaluator_ToCharacter : public UNpcReactionEvaluatorBase
{
	GENERATED_BODY()

private:
	struct FReactionCauserData
	{
		const AActor* Actor = nullptr;
		float DistSq = FLT_MAX;
	};


public:
	virtual bool LoadReactionContext(const UNpcBlackboardDataAsset* NpcBlackboardDataAsset, UBlackboardComponent* BlackboardComponent, UObject* ReactionEvaluatorMemory) const override;
	virtual UObject* CreateMemory(UObject* OuterObject) override;
	virtual void CompleteReaction(UNpcPerceptionReactionComponent* NpcPerceptionReactionComponent, UBlackboardComponent* Blackboard, UObject* ReactionEvaluatorMemory, const
	                              FGameplayTag& ReactionBehaviorExecutionResult) const override;	
	
protected:
	virtual float ProcessPerceptionInternal(AAIController* NpcController, UObject* ReactionEvaluatorMemory,
	                                        float DeltaTime) const override;

	//29.11.2024 @AK TODO consider making an instanced object/struct to store reaction evaluator parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery CharacterTagsFilter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer OnlyReactToCharactersWithAttitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag GestureTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NpcReactionSpeechTag;
	
	// guards can react to player holding a weapon at 30 meters, merchant private security - at 5 meters 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float ReactionRelevancyDistanceLimit = 2500.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bReactionIndefinite = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UEnvQuery> ReactionEQS;
	
	// in real world time
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bReactionIndefinite == false", UIMin = 0.f, ClampMin = 0.f))
	float ReactionMaxTime = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag NewAttitudeAfterReactionComplete;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAttitudeChangeSharedWithAllies = true;
};