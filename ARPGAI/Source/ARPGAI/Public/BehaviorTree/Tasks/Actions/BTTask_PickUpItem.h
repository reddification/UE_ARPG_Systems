#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_PickUpItem.generated.h"

UCLASS(Category="Actions")
class ARPGAI_API UBTTask_PickUpItem : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_PickUpItem();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ItemBBKey;
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
