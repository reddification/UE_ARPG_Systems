

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Tasks/BTTask_SetTagCooldown.h"
#include "UObject/Object.h"
#include "BTTask_SetTagCooldownWithDeviation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_SetTagCooldownWithDeviation : public UBTTask_SetTagCooldown
{
	GENERATED_BODY()

public:
	UBTTask_SetTagCooldownWithDeviation();
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float DeviationTime = 2.f;
};
