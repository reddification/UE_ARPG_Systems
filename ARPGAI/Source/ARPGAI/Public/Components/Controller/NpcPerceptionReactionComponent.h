// 

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Data/NpcDTR.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "NpcPerceptionReactionComponent.generated.h"

class UNpcReactionEvaluatorBase;
class UBlackboardComponent;
enum class EReactionBehaviorType : uint8;
class INpc;

USTRUCT()
struct FNpcReactionEvaluatorState
{
	GENERATED_BODY()
	
	UPROPERTY()
	UObject* EvaluatorMemory = nullptr;

	UPROPERTY()
	const UNpcReactionEvaluatorBase* ReactionEvaluator;
	
	float BehaviorUtility = 0.f;

	bool bCanAccumulate = false;
	bool bWorldStateFilterPasses = false;
	FDateTime CooldownUntilGameTime = FDateTime();

	FORCEINLINE bool IsActive(const FDateTime& CurrentGameTime) const
	{
		return (bCanAccumulate || BehaviorUtility > 0.f) && bWorldStateFilterPasses && (CooldownUntilGameTime.GetTicks() == 0 || CooldownUntilGameTime < CurrentGameTime);
	}
	
	bool operator < (const FNpcReactionEvaluatorState & Other) const
	{
		return BehaviorUtility > Other.BehaviorUtility;
	}

};

USTRUCT()
struct FNpcReactionEvaluatorStateWrapper
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FNpcReactionEvaluatorState> NpcReactionEvaluatorStates;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcPerceptionReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	TArray<FNpcReactionEvaluatorState> GetPerceptionReactionEvaluators() const;
	const FNpcReactionEvaluatorState* GetBestBehaviorPerceptionReactionEvaluatorState(EReactionBehaviorType ReactionBehaviorType);
	void UpdatePerceptionReactionBehaviorUtility(EReactionBehaviorType ReactionBehaviorType, const FGuid& ReactionEvaluatorId, float DeltaUtility);
	void ResetReactionBehaviorUtility(EReactionBehaviorType ReactionBehavior, const FGuid& EvaluatorId);
	void InitializeAIController(AAIController* AIController);

	void AddReactionBehaviorEvaluators(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset);
	void AddReactionBehaviorEvaluators(const TArray<UNpcReactionEvaluatorBase*>& ReactionBehaviorEvaluators);
	void RemoveReactionBehaviorEvaluators(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset);
	void RemoveReactionBehaviorEvaluators(const TArray<UNpcReactionEvaluatorBase*>& PerceptionReactionEvaluators);
	
	void SetPawn(APawn* InPawn);
	
	const UNpcBlackboardDataAsset* GetNpcBlackboardKeys() const { return NpcBlackboardKeys; };

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;

	UPROPERTY()
	TObjectPtr<const UNpcBlackboardDataAsset> NpcBlackboardKeys;
	
	TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	UPROPERTY()
	TMap<EReactionBehaviorType, FNpcReactionEvaluatorStateWrapper> BehaviorReactionEvaluators;

	TMap<EReactionBehaviorType, FBlackboardKeySelector> BehaviorReactionUtilityBlackboardKeys;
	
	bool bNpcComponentInitialized = false;
	bool bAIControllerInitialized = false;
	
	void OnNpcStateChanged(const FGameplayTagContainer& NewNpcState);
	void OnWorldStateChanged(const FGameplayTagContainer& NewWorldState);
};
