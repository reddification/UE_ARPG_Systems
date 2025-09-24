// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RequestBehaviorEvaluatorActive.generated.h"

struct FBehaviorEvaluatorBlockRequest;
/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_RequestBehaviorEvaluatorActive : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RequestBehaviorEvaluatorActive();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BehaviorEvaluatorsTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.1f, ClampMin = 0.1f))
	float Duration = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bActive = false;
};
