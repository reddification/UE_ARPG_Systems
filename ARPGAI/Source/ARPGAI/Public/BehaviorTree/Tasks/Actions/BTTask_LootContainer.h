#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_HandleGameplayAbility.h"
#include "BTTask_LootContainer.generated.h"

UCLASS(Category="Actions")
class ARPGAI_API UBTTask_LootContainer : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()
	
public:
	UBTTask_LootContainer();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;
	
protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ContainerBBKey;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutReactTagBBKey;
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer LootCompletedAIMessages;	
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
