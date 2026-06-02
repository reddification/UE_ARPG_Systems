#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetRandomIntegerInRange.generated.h"

UCLASS(Category="Blackboard")
class ARPGAI_API UBTTask_SetRandomIntegerInRange : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetRandomIntegerInRange();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutIntBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInt32Interval Interval;
};
