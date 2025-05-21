

#pragma once

#include "CoreMinimal.h"
#include "BTTask_HandleGameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_LookAt.generated.h"

UCLASS()
class ARPGAI_API UBTTask_LookAt : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_LookAt();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector LookAtBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInverse = false;
};
