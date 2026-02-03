// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_FollowActor.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_FollowActor : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FBTMemory_FollowActor
	{
		bool bStartedFollowingNpc = false;
	};	
	
public:
	UBTDecorator_FollowActor();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_FollowActor); }
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector FollowTargetBBKey;
	
private:
	bool IsNeedToFollowTarget(UBlackboardComponent* Blackboard) const;
};
