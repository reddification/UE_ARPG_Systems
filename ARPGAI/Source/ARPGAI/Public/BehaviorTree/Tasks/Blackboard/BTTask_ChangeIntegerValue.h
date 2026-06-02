// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChangeIntegerValue.generated.h"

/**
 * 
 */
UCLASS(Category="Blackboard")
class ARPGAI_API UBTTask_ChangeIntegerValue : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ChangeIntegerValue();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector IntegerBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int DeltaValue = 0;	
};
