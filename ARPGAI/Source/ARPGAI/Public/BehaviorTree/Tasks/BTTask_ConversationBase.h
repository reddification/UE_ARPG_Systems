// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ConversationBase.generated.h"

class INpc;
/**
 * 
 */
UCLASS()
class ARPGAI_API UBTTask_ConversationBase : public UBTTaskNode
{
	GENERATED_BODY()

	
protected:
	struct FBTMemory_Conversation
	{
		FDelegateHandle CollocutorTagsChangedObserverDelegate;
		bool bConversationOnHold = false;
		bool bForceConversationPartnerSuspendActivity = false;
		bool bRestartConversation = false;
	};

public:
	UBTTask_ConversationBase();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_Conversation); };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector ConversationPartnerBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector CollocutorTagsBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutIsConversationPrioritizedBBKey;

	// useful for cases when one NPC has to leave conversation for a short moment but you want them to stay close to each other
	// for example, when 2 NPCs are having a conversation, a player comes up and starts talking to 1 of them.
	// We want NPCs to be able to return to the conversation without re-evaluating their whole behavior 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer ReasonsToHoldConversation;

	// useful for cases when 1 NPC was waiting another one while it was talking to player, but then this NPC has gotten a tag that makes it "busy"
	// i.e. player triggered some quest state that makes NPC to go to some location and interact with something
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer ReasonsToAbortConversation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WaitAbortWhenConversationIsOnHoldTimeSeconds = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bClearConversationBlackboardOnTaskFinished = false;

	virtual bool RequestResumeConversation(FBTMemory_Conversation* BTMemory, APawn* PawnOwner);
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	virtual void ClearBlackboardConversationState(UBlackboardComponent* OwnerBlackboard) const;

	void EnterConversationOnHoldState(UBehaviorTreeComponent& OwnerComp, FBTMemory_Conversation* BTMemory, APawn* ConversationPartner);
	
private:
	EBlackboardNotificationResult OnCollocutorTagsChanged(const UBlackboardComponent& CollocutorBlackboard, FBlackboard::FKey Key, UBehaviorTreeComponent* OwnerComp);
};
