// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InteractWithSmartObject.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_InteractWithSmartObject : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_InteractWithSmartObject();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;

	virtual FString GetStaticDescription() const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector InteractionActorBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ClaimedSmartObjectClaimHandleBBKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ActiveSmartObjectClaimHandleBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InterpolateToSlotLocationRate = 10.f;
};
