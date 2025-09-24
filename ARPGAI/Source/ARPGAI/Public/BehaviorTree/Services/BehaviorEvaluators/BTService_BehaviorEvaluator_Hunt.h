// 

#pragma once

#include "CoreMinimal.h"
#include "BTService_BehaviorEvaluator_Base.h"
#include "BTService_BehaviorEvaluator_Hunt.generated.h"

/**
 * 
 */
UCLASS(meta=(Category="Behavior evaluators"))
class ARPGAI_API UBTService_BehaviorEvaluator_Hunt : public UBTService_BehaviorEvaluator_Base
{
	GENERATED_BODY()

private:
	struct FBTMemory_BE_Hunt : public FBTMemory_BehaviorEvaluator_Base
	{
		TWeakObjectPtr<AActor> Prey;
		float MyStrength = 0.f;
	};

	struct FHuntingObservationBase
	{
		FHuntingObservationBase() {  }
		FHuntingObservationBase(AActor* InActor) : Actor(InActor) {  }
		
		AActor* Actor = nullptr;
	};
	
	struct FHuntingPrey : public FHuntingObservationBase
	{
		FHuntingPrey() {  }
		FHuntingPrey(AActor* InActor, float InDistance, float InScore) : FHuntingObservationBase(InActor), Distance(InDistance), Score(InScore) {  }
		float Distance = 0.f;
		float Score = 0.f;
	};

	struct FHuntingThreat : public FHuntingObservationBase
	{
		FHuntingThreat() { }
		FHuntingThreat(AActor* InActor, float InStrengthAdvantage) : FHuntingObservationBase(InActor), StrengthAdvantage(InStrengthAdvantage) {  }
		float StrengthAdvantage = 1.f;
	};

public:
	UBTService_BehaviorEvaluator_Hunt();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_BE_Hunt); };
	virtual FString GetStaticDescription() const override;

	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer PreysIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutPreyTargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve PreyDistanceToScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve PreyThreatsProximityDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUsePathFinding = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HuntRange = 2000.f;
};
