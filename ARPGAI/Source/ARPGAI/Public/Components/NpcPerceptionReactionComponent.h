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
	FTimespan CooldownUntilGameTime = FTimespan::Zero();

	FORCEINLINE bool IsActive(const FTimespan& CurrentGameTime) const
	{
		return (bCanAccumulate || BehaviorUtility > 0.f) && bWorldStateFilterPasses && (CooldownUntilGameTime.IsZero() || CooldownUntilGameTime < CurrentGameTime);
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

	void AddPerceptionReactionEvaluator(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset);
	void RemovePerceptionReactionEvaluator(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset);
	
	FORCEINLINE const FNpcDTR* GetNpcDTR() const { return NpcDTRH.GetRow<FNpcDTR>(""); }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	FDataTableRowHandle NpcDTRH;

	UPROPERTY()
	TScriptInterface<INpc> OwnerNPC;

	TWeakObjectPtr<UBlackboardComponent> BlackboardComponent;
	
	UPROPERTY()
	TMap<EReactionBehaviorType, FNpcReactionEvaluatorStateWrapper> BehaviorReactionEvaluators;

	TMap<EReactionBehaviorType, FBlackboardKeySelector> BehaviorReactionUtilityBlackboardKeys;
	
	bool bNpcComponentInitialized = false;
	bool bAIControllerInitialized = false;
	
	void OnNpcStateChanged(const FGameplayTagContainer& NewNpcState);
	void OnWorldStateChanged(const FGameplayTagContainer& NewWorldState);
	void StoreReactionBehaviorEvaluators(const UNpcPerceptionReactionEvaluatorsDataAsset* ReactionBehaviorEvaluatorsDataAsset);
};
