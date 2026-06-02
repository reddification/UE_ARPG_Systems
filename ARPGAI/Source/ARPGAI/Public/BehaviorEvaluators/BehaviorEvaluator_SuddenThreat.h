#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorEvaluator_SuddenThreat.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Sudden threat")
class ARPGAI_API UBehaviorEvaluatorConfig_SuddenThreatReact : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_SuddenThreatReact();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ReceivedHitFromLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector AttackerActorBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PerceiveDamageCauserDistanceThreshold = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIgnoreVisuallyPerceivedCauses = false;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_SuddenThreat : public FBehaviorEvaluator_Base
{
	using Super = FBehaviorEvaluator_Base;

public:
	FBehaviorEvaluator_SuddenThreat(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config);

	virtual ~FBehaviorEvaluator_SuddenThreat() override;
	
	virtual void SetState(EBehaviorEvaluatorState NewState) override;
	void OnPerceptionUpdated(AActor* TriggerActor, const FAIStimulus& Stimulus);

protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	FVector AttackedFromLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> Attacker;
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_SuddenThreatReact> SuddenThreatConfig;
	FDelegateHandle PerceptionUpdateDelegateHandle;

	void SubscribeToPerceptionChanges();
	void UnsubscribeFromPerceptionChanges();
	bool CanObserve(const APawn* NpcPawn, const AActor* AttackerActor, const UBehaviorEvaluatorConfig_SuddenThreatReact& Config) const;
};