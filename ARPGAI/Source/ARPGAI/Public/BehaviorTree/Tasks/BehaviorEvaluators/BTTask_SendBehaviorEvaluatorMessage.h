// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendBehaviorEvaluatorMessage.generated.h"

/**
 * 
 */
UCLASS(Category="Behavior Evaluation")
class ARPGAI_API UBTTask_SendBehaviorEvaluatorMessage : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SendBehaviorEvaluatorMessage();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	// Optional. If empty - message will be sent to all evaluators
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EvaluatorTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Message;
};
