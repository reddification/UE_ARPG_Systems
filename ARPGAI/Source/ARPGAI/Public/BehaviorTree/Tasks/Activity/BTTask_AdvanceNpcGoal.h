#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_AdvanceNpcGoal.generated.h"

UCLASS(Category="Activities")
class ARPGAI_API UBTTask_AdvanceNpcGoal : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AdvanceNpcGoal();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	bool bGoalSucceeded = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutGoalAdvancedBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector GoalExecutionResultBBKey;
};
