// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EquipBestWeapon.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_EquipBestWeapon : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_EquipBestWeapon();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
