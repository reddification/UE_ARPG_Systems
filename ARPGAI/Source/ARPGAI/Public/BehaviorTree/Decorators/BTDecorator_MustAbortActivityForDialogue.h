// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MustAbortActivityForDialogue.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UBTDecorator_MustAbortActivityForDialogue : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_MustAbortActivityForDialogue
	{
		bool bActive = false;
	};	
	
public:
	UBTDecorator_MustAbortActivityForDialogue();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_MustAbortActivityForDialogue); };
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ConversationPartnerBBKey;
	// Min dot product between NPC and conversation partner to keep interaction

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector NpcTagsBBKey;
	
	UPROPERTY(EditAnywhere)
	FGameplayTagQuery NpcRequiredStateToAbortActivityFilter;	
	
	UPROPERTY(EditAnywhere, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float DotProductThreshold = 0.35f;

private:
	EBlackboardNotificationResult OnNpcTagsChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
};
