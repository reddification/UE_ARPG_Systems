#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetUtilityEvaluatorCooldown.generated.h"

UCLASS(Category="Behavior Evaluation")
class ARPGAI_API UBTTask_SetUtilityEvaluatorCooldown : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetUtilityEvaluatorCooldown();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RequestId = FName("BTTask_SetUtilityEvaluatorCooldown");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer UtilityEvaluatorTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Cooldown = 10.f;	
};
