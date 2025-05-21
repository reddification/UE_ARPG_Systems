#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AssessActivityState.generated.h"

UCLASS(Category = "Activities")
class ARPGAI_API UBTTask_AssessActivityState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AssessActivityState();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
