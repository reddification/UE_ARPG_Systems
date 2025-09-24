// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluatorBase.h"
#include "GameplayTagContainer.h"
#include "BehaviorEvaluator_Beast_Hunt.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBehaviorEvaluator_Beast_Hunt : public UBehaviorEvaluatorBase
{
	GENERATED_BODY()

public:
	UBehaviorEvaluator_Beast_Hunt();
	virtual void Evaluate() override;

protected:
	// allowed preys
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer PreysIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutPreyTargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUsePathFinding = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HuntRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float UtilityDecayRate = 0.5f;
};
