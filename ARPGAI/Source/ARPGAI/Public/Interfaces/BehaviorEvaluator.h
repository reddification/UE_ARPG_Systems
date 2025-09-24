// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BehaviorEvaluator.generated.h"

class UBehaviorTreeComponent;
// This class does not need to be modified.
UINTERFACE()
class UBehaviorEvaluator : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API IBehaviorEvaluator
{
	GENERATED_BODY()

public:
	virtual void InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const = 0;
	virtual void FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const = 0;
};
