// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TriggerArbitraryNpcEvent.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_TriggerArbitraryNpcEvent : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TriggerArbitraryNpcEvent();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag EventTag;
	
};
