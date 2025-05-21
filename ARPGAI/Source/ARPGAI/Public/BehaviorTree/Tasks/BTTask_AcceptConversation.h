// 

#pragma once

#include "CoreMinimal.h"
#include "BTTask_ConversationBase.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AcceptConversation.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_AcceptConversation : public UBTTask_ConversationBase
{
	GENERATED_BODY()

public:
	UBTTask_AcceptConversation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void ClearBlackboardConversationState(UBlackboardComponent* OwnerBlackboard) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutConversationAcceptedBBKey;
};
