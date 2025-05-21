

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamic.h"
#include "BTTask_RunBehaviorDynamicEnhanced.generated.h"

UCLASS()
class ARPGAI_API UBTTask_RunBehaviorDynamicEnhanced : public UBTTask_RunBehaviorDynamic
{
	GENERATED_BODY()

public:
	UBTTask_RunBehaviorDynamicEnhanced();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
