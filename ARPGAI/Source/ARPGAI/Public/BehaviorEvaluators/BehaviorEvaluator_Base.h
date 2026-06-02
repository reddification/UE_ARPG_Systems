#pragma once

#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardData.h"
#include "Data/AiDataTypes.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "Data/BehaviorEvaluatorDataTypes.h"
#include "Data/BehaviorEvaluatorStateEffects.h"
#include "BehaviorEvaluator_Base.generated.h"

class UNpcComponent;
class AAIController;
class UNpcCombatLogicComponent;
class UNpcPerceptionComponent;

class FBehaviorEvaluator_Base;

UCLASS(Abstract, EditInlineNew)
class UBehaviorEvaluatorConfig_Base : public UObject, public IBlackboardAssetProvider
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Base();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BehaviorEvaluatorTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Utility)
	FBlackboardKeySelector UtilityBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Utility)
	float MaxUtility = 1.f;
	
	// example usages: 
	// when using smart object - unconditionally add +0.25
	// when conversation is requested - conditionally add -0.3
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Utility)
	TMap<FGameplayTag, float> UtilityConditionalOffsets; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation")
	bool bInterpConstant = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Inactive")
	float InactiveUtilityAccumulationRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Inactive")
	float InactiveUtilityDecayRate = 0.5f;
	
	// used as base value. so in order for behaviors to become top utility, the amount of scores from various perception observations
	// must overcome this base value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Inactive")
	float InactiveUtilityOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Active")
	float ActiveUtilityAccumulationRate = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Active")
	float ActiveUtilityDecayRate = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility|Accumulation|Active")
	float ActiveUtilityOffset = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Tick)
	bool bTickable = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition = "bTickable"), Category=Tick)
	float TickInterval = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, EditCondition = "bTickable"), Category=Tick)
	float TickIntervalDeviation = 0.f;
	
	// can only be relevant if owner tags comply with this tags query
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Control)
	FGameplayTagQuery RelevancyOwnerRequirements;
	
	// tasks that have "apply" and "rollback" functions that are called when evaluator enters this state
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Control)
	TMap<EBehaviorEvaluatorState, FBehaviorEvaluatorStateEffectsContainer> StateEffects;	
	
	// Don't decay utility for this duration when evaluator was activated
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Control, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DecayDelayOnActivation = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Control)
	bool bUpdateWhenActivated = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control|Reset")
	bool bResetUtilityOnDeactivated = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control|Reset")
	bool bResetUtilityOnBecomeRelevant = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Control|Reset")
	bool bResetUtilityOnCeaseRelevant = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UBlackboardData> Blackboard;

	virtual UBlackboardData* GetBlackboardAsset() const override;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const { return nullptr; };
};

class FBehaviorEvaluator_Base
{
private:
	using UConfig = UBehaviorEvaluatorConfig_Base;
	
public:
	FBehaviorEvaluator_Base(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual ~FBehaviorEvaluator_Base() = default;
	
	virtual void Update(const float DeltaTime);
	
	FORCEINLINE EBehaviorEvaluatorState GetState() const { return EvaluatorState; }

	virtual void SetState(EBehaviorEvaluatorState NewState);
	virtual void OnBehaviorFinished(EBehaviorEvaluatorResult ExecutionResult);

	float GetMaxUtility() const;
	void SetMaxUtility() const;
	void DelayRegression(float NewDelay, bool bAppendToExisting);
	void RequestRegressionFrozen(bool bFrozen);

	// Evaluators can do whatever with these
	void HandleMessage(const FGameplayTag& MessageTag);
	void SetConditionalUtilityEffectActive(const FGameplayTag& ConditionalEffectTag, bool bActive);
	
	virtual bool CanHandleMessages() const;
	virtual void Cleanup();
	

protected:
	TWeakObjectPtr<UBlackboardComponent> Blackboard;
	TWeakObjectPtr<AAIController> AIController;
	TWeakObjectPtr<APawn> Pawn;
	TWeakObjectPtr<UNpcPerceptionComponent> PerceptionComponent;
	TWeakObjectPtr<UNpcCombatLogicComponent> CombatLogicComponent;
	TWeakObjectPtr<UNpcComponent> NpcComponent;
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Base> BaseConfig;
	
	FDateTime GetGameTime(float GameHoursOffset = 0.f) const;
	
	double BehaviorStartTime = 0.;

	bool CanRegress();
	void InterpolateUtility(float NewDesiredUtility, const float DeltaTime);

	virtual void OnActivated();
	virtual void HandleMessage_Internal(const FGameplayTag& MessageTag) {};
	
	float GetUtilityAccumulationRate() const;
	float GetUtilityDecayRate() const;
	float GetUtilityOffset() const;

	void ChangeUtility(float Delta);
	
private:
	int RegressionFreezeCounter = 0;
	float PostponeRegressionUntil = 0.f;
	bool bRegressionPostponed = false;
	
	EBehaviorEvaluatorState EvaluatorState = EBehaviorEvaluatorState::NotRequested;
	
	FGameplayTagContainer ActiveUtilityConditions;
	float ConditionalUtilityEffect = 0.f;
};
