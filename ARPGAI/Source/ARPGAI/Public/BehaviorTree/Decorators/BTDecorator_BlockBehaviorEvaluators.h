// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_BlockBehaviorEvaluators.generated.h"

/**
 * 
 */
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_BlockBehaviorEvaluators : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_BlockBehaviorEvaluators();
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	// Recommended to be unique in request stack, but in general - optional
	UPROPERTY(EditAnywhere)
	FName RequestId = FName("BTDecorator_BlockBehaviorEvaluators");
	
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer BlockedBehaviorTags;
};
