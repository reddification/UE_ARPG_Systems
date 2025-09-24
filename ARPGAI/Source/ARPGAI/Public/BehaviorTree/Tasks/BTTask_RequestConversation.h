

#pragma once

#include "CoreMinimal.h"
#include "BTTask_ConversationBase.h"
#include "UObject/Object.h"
#include "BTTask_RequestConversation.generated.h"

struct FNpcGoalParameters_Conversate;

UCLASS()
class ARPGAI_API UBTTask_RequestConversation : public UBTTask_ConversationBase
{
	GENERATED_BODY()
	
public:
	UBTTask_RequestConversation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	virtual bool RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner) override;

	// this BB is set to true, but only for the collocutor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutConversationAcceptedBBKey;

private:
	void SetBlackboardConversationState(UBlackboardComponent* Blackboard, AActor* ConversationPartner, bool bConversationPrioritized, bool bConversationAccepted) const;
	bool StartConversation(UBlackboardComponent* OwnerBlackboard, const FNpcGoalParameters_Conversate* ConversationStartParams,
	                       APawn* PawnOwner, INpc* NpcOwner, APawn* ConversationPartner,
	                       UBlackboardComponent* ConversationPartnerBlackboard);
	
	bool TryStartConversation(APawn* PawnOwner, UBlackboardComponent* CollocutorBlackboard, UBehaviorTreeComponent* OwnerComp);
};
