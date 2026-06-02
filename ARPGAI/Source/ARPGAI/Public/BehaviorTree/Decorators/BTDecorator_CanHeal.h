// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanHeal.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_CanHeal : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CanHeal();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
