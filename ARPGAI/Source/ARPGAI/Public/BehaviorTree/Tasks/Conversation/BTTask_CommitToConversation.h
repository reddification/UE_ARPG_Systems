#pragma once

#include "CoreMinimal.h"
#include "BTTask_ConversationBase.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CommitToConversation.generated.h"

UCLASS(Category="Conversation")
class ARPGAI_API UBTTask_CommitToConversation : public UBTTask_ConversationBase
{
	GENERATED_BODY()

public:
	UBTTask_CommitToConversation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
