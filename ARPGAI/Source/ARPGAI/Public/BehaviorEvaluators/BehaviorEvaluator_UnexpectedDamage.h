// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluatorBase.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorEvaluator_UnexpectedDamage.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBehaviorEvaluator_UnexpectedDamage : public UBehaviorEvaluatorBase
{
	GENERATED_BODY()

public:
	virtual void Activate(AAIController* InAIController) override;
	virtual void Deactivate() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ReceivedHitFromLocationBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float UtilityFromPerceivingDamage = 1.f;
	
private:
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
