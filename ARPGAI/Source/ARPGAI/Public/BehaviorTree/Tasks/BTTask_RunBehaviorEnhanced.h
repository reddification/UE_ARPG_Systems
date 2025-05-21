

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RunBehavior.h"
#include "UObject/Object.h"
#include "BTTask_RunBehaviorEnhanced.generated.h"

UCLASS()
class ARPGAI_API UBTTask_RunBehaviorEnhanced : public UBTTask_RunBehavior
{
	GENERATED_BODY()

public:
	UBTTask_RunBehaviorEnhanced();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
