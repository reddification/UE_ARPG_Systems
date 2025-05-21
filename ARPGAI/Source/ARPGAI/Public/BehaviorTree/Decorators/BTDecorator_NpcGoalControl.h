// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_NpcGoalControl.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_NpcGoalControl : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_NpcGoalControl();

protected:
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
};
