#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_UseHealingItem.generated.h"

UCLASS(Category="Actions")
class ARPGAI_API UBTTask_UseHealingItem : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBTTask_UseHealingItem();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAwaitCompletion = true;
};
