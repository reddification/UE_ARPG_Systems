// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Tasks/BTTask_CompleteReaction.h"
#include "BTDecorator_HandleNpcReactionBehavior.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_HandleNpcReactionBehavior : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HandleNpcReactionBehavior();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere)
	EReactionBehaviorType ReactionBehaviorType;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector CustomResultTagsBBKey;
};
