// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ReactionEvaluator.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_ReactionEvaluator : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ReactionEvaluator();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
