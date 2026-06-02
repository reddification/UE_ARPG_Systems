#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MaxOutBehaviorEvaluator.generated.h"

UCLASS(Category="Behavior Evaluation")
class ARPGAI_API UBTTask_MaxOutBehaviorEvaluator : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MaxOutBehaviorEvaluator();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EvaluatorTag;
};
