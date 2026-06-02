// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Debug_DelayedAbort.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_Debug_DelayedAbort : public UBTTaskNode
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_DelayedAbort
	{
		float RemainingAbortTime = 0.f;
	};
	
public:
	UBTTask_Debug_DelayedAbort();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere)
	float AbortDelay = 4.f;
};
