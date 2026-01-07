// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "Interfaces/BehaviorEvaluator.h"
#include "BTService_BehaviorEvaluator_Base.generated.h"

struct FBehaviorEvaluatorBlockRequest;
class UNpcPerceptionComponent;
class UNpcAttitudesComponent;

UCLASS(Abstract, Hidden)
class ARPGAI_API UBTService_BehaviorEvaluator_Base : public UBTService, public IBehaviorEvaluator
{
	GENERATED_BODY()

protected:
	struct FBTMemory_BehaviorEvaluator_Base
	{
		TWeakObjectPtr<UNpcPerceptionComponent> PerceptionComponent;
		// means is utility evaluation running
		bool bBlocked = false;
		// means is the behavior itself currently active. set in IBehaviorEvaluator::InitiateBehaviorState
		bool bActive = false;
		float InactiveUtilityAccumulationRate = 0.f;
		float InactiveUtilityRegressionOffset = 0.f;
		float ActiveUtilityAccumulationRate = 0.f;
		float ActiveUtilityRegressionOffset = 0.f;

		float GetUtilityAccumulationRate() const
		{
			return bActive ? ActiveUtilityAccumulationRate : InactiveUtilityAccumulationRate;
		}

		float GetUtilityRegressionOffset() const
		{
			return bActive ? ActiveUtilityRegressionOffset : InactiveUtilityRegressionOffset;
		}
	};
	
public:
	UBTService_BehaviorEvaluator_Base();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_BehaviorEvaluator_Base); };
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector UtilityBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ActiveEvaluatorsTagsBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float InactiveUtilityAccumulationRate = 0.5f;

	// used as base value. so in order for behaviors to become top utility, the amount of scores from various perception observations
	// must overcome this base value
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float InactiveUtilityRegressionOffset = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float ActiveUtilityAccumulationRate = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float ActiveUtilityRegressionOffset = -0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float MaxUtility = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BehaviorEvaluatorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBehaviorEvaluatorBlockRequest> BlockEvaluatorsWhenActivated;
	
	void ChangeUtility(const float DeltaUtilityRaw, UBlackboardComponent* Blackboard, float DeltaTime, const FBTMemory_BehaviorEvaluator_Base*
	                   BTMemory);

private:
	EBlackboardNotificationResult OnActiveBehaviorEvaluatorsChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	EBlackboardNotificationResult OnUtilityChangeParameterChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	
public: //IBehaviorEvaluator
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void SetCooldown(UBehaviorTreeComponent* BTComponent, float Cooldown) const override;
};
