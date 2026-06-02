#pragma once

#include "CoreMinimal.h"
#include "BTTask_ConversationBase.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "UObject/Object.h"
#include "BTTask_RequestConversation.generated.h"

struct FNpcGoalParameters_Conversate;

UCLASS(Category="Conversation")
class ARPGAI_API UBTTask_RequestConversation : public UBTTask_ConversationBase
{
	GENERATED_BODY()

public:
	UBTTask_RequestConversation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	virtual bool RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner) override;

	// wait for this time for collocutor to join if he/she accepted conversation. 
	// Basically a failsafe from cases when collocutor accepts conversation 
	// but conversation utility doesn't exceed active behavior and active behavior is incompatible with conversation
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.f, ClampMin = 0.f))
	float CollocutorJoinTimeout = 5.f;
	
private:
	bool TryResumeConversation(const APawn* PawnOwner, APawn* Collocutor) const;
	NpcConversation::FRequestParams GetConversationRequestParams(const FNpcGoalParameters_Conversate* ConversationGoalParams) const;
};
