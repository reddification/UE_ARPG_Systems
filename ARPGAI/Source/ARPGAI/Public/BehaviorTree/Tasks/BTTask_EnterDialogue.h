#pragma once

#include "CoreMinimal.h"
#include "BTTask_HandleGameplayAbility.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UObject/Object.h"
#include "BTTask_EnterDialogue.generated.h"

UCLASS()
class ARPGAI_API UBTTask_EnterDialogue : public UBTTask_HandleGameplayAbility
{
	GENERATED_BODY()

public:
	UBTTask_EnterDialogue();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};