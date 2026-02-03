// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_SingleAttack.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_SingleAttack : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBTTask_SingleAttack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
