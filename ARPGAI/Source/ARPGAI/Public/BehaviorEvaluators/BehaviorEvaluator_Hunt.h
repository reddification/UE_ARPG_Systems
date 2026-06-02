#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Components/NpcCombatLogicComponent.h"
#include "BehaviorEvaluator_Hunt.generated.h"

UCLASS(DisplayName="Hunt")
class UBehaviorEvaluatorConfig_Hunt : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()

public:
	UBehaviorEvaluatorConfig_Hunt();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer PreysIds;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery PreyTagsFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutPreyTargetBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OuHuntStateTagsBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve PreyDistanceToScoreDependency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRuntimeFloatCurve PreyThreatsProximityDependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUsePathFinding = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HuntRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PreyKilledEventTag;

	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Hunt : public FBehaviorEvaluator_Base
{
private:
	using Super = FBehaviorEvaluator_Base;
	
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
	FBehaviorEvaluator_Hunt(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);
	virtual void Update(const float DeltaTime) override;
	
protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	void OnActorKilled(AActor* Actor, const FGameplayTag& LastHitType);
	
	TWeakObjectPtr<AActor> CurrentPrey;
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Hunt> HuntConfig; 
	float MyStrength = 1.f;
	FDelegateHandle ActorKilledDelegateHandle;
};
