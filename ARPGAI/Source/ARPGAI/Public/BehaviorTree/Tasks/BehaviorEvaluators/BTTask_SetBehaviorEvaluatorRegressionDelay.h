// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTTask_SetBehaviorEvaluatorRegressionDelay.generated.h"

/**
 * 
 */
UCLASS(Category="Behavior Evaluation")
class ARPGAI_API UBTTask_SetBehaviorEvaluatorRegressionDelay : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetBehaviorEvaluatorRegressionDelay();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag EvaluatorTag;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	FValueOrBBKey_Float Delay = 20.f;
	
	UPROPERTY(EditAnywhere)
	bool bAppendToExisting = false;
};
